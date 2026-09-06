//! Phase-vocoder analysis file (`.pva`) I/O.
//!
//! Two formats, per the plan's §8.1.3 decision (no legacy compatibility
//! required for *writing*):
//!
//! - **Legacy** (`read_legacy_pva`, reader only): the format
//!   `legacy/pvc_src/pvanalysis.c` writes. A 32-native-f32 header -
//!   `[N, D, R, chans, window_type]` followed by 27 more f32s - then
//!   `frame_count` frames, each frame holding all channels' `N+2` f32s
//!   (mag/freq pairs) *interleaved* one after another (frame 0's channel
//!   0, frame 0's channel 1, ..., frame 1's channel 0, ...) - **not**
//!   channel 0's frames followed by channel 1's frames. Confirmed against
//!   a real 2-channel `pvanalysis` run (the mono golden fixture this
//!   reader was first tested against can't distinguish the two layouts at
//!   all): `pvanalysis.c`'s per-channel loop re-seeks to the start of the
//!   frame data on every channel pass and writes that channel's frame at
//!   position `k` among `ochan` slots per frame, skipping the others (or,
//!   on channel 0's pass only, filling them with a copy of channel 0's
//!   own data as a placeholder later overwritten by the real channel).
//!   Frame count isn't stored anywhere explicit; it's derived from the
//!   file size. Kept only so the golden harness can read `pvanalysis`'s
//!   C-oracle output; `pvc analyze` never writes this format.
//!
//!   The 27 "spare" header floats aren't actually all spare: pvanalysis.c
//!   seeks back after writing every channel's frames and overwrites the
//!   first `ochan` of them with per-channel peak amplitudes, then writes
//!   the source sound file's name (newline-terminated, not NUL-terminated)
//!   into the floats after that. Neither is read here - the plan's stated
//!   test requirement is only N/D/R/chans/frame count - but it's real data
//!   in a real legacy file, not padding, and worth knowing if anyone goes
//!   looking for why bytes 20..128 aren't all zero.
//!
//! - **New** (`read_pva`/`write_pva`): magic `PVA1`, u32 version, u32 N,
//!   u32 D, u32 sample_rate, u32 channels, u32 window_type, u64
//!   frames_per_channel, `channels` f32 LE peak amplitudes (one per
//!   channel, explicit rather than packed into header padding), then per
//!   channel `frames_per_channel` frames of `N+2` f32 LE mag/freq pairs -
//!   same per-channel frame layout as the legacy format, but with an
//!   explicit frame count instead of one derived from file size.

use std::fs;
use std::io::{Read, Write};
use std::path::Path;

use thiserror::Error;

pub const PVA_MAGIC: &[u8; 4] = b"PVA1";
pub const PVA_VERSION: u32 = 1;
const LEGACY_HEADER_FLOATS: usize = 32;

#[derive(Debug, Error)]
pub enum PvaError {
    #[error(transparent)]
    Io(#[from] std::io::Error),
    #[error("{0}: file too short for a legacy .pva header ({1} bytes, need at least {2})")]
    LegacyTooShort(String, usize, usize),
    #[error(
        "{0}: channel data ({1} bytes) isn't an exact multiple of channels*(N+2)*4 ({2} bytes)"
    )]
    LegacyFrameMismatch(String, usize, usize),
    #[error("{0}: bad magic {1:?}, expected {PVA_MAGIC:?}")]
    BadMagic(String, [u8; 4]),
    #[error("{0}: unsupported PVA1 version {1} (this reader supports {PVA_VERSION})")]
    UnsupportedVersion(String, u32),
    #[error("{0}: truncated file")]
    Truncated(String),
}

/// Shared header fields, however the file was read.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PvaHeader {
    pub n: u32,
    pub d: u32,
    pub sample_rate: u32,
    pub channels: u32,
    pub window_type: u32,
}

/// One channel's worth of analysis frames: `frames.len()` frames, each
/// `n + 2` floats of interleaved (mag, freq) pairs - `frame[2*bin]` is the
/// magnitude and `frame[2*bin + 1]` the frequency for that bin, matching
/// `convert()`'s output layout in the legacy C.
#[derive(Debug, Clone, PartialEq)]
pub struct PvaData {
    pub header: PvaHeader,
    /// `channels[ch][frame]` is one frame's `n + 2` mag/freq floats.
    pub channels: Vec<Vec<Vec<f32>>>,
    /// Per-channel peak amplitude, in whatever scale the analysis stage
    /// tracked it in - for a legacy file, `pvanalysis.c`'s own
    /// `peakamps[]` (an FFT-*magnitude*-domain peak, not a waveform
    /// peak; the two are easy to conflate - see `tools::twarp`'s doc
    /// comment on why that distinction matters to a caller). One value
    /// per channel, same order as `channels`.
    pub peak_amps: Vec<f32>,
}

impl PvaData {
    pub fn frames_per_channel(&self) -> usize {
        self.channels.first().map_or(0, |c| c.len())
    }
}

/// Reads a **legacy** `.pva` file (native-endian f32, frame count derived
/// from file size). Reader only - see the module doc comment.
pub fn read_legacy_pva(path: &Path) -> Result<PvaData, PvaError> {
    let display = path.display().to_string();
    let bytes = fs::read(path)?;
    let header_bytes = LEGACY_HEADER_FLOATS * 4;
    if bytes.len() < header_bytes {
        return Err(PvaError::LegacyTooShort(display, bytes.len(), header_bytes));
    }

    let read_f32 = |i: usize| -> f32 {
        let o = i * 4;
        f32::from_ne_bytes([bytes[o], bytes[o + 1], bytes[o + 2], bytes[o + 3]])
    };

    let n = read_f32(0) as u32;
    let d = read_f32(1) as u32;
    let sample_rate = read_f32(2) as u32;
    let channels = read_f32(3) as u32;
    let window_type = read_f32(4) as u32;

    let frame_floats = n as usize + 2;
    let frame_bytes = frame_floats * 4;
    let data_bytes = bytes.len() - header_bytes;
    let per_channel_bytes = channels as usize * frame_bytes;
    if per_channel_bytes == 0 || !data_bytes.is_multiple_of(per_channel_bytes) {
        return Err(PvaError::LegacyFrameMismatch(
            display,
            data_bytes,
            per_channel_bytes,
        ));
    }
    let frames_per_channel = data_bytes / per_channel_bytes;

    // `pvanalysis.c` seeks back after writing every channel's frames and
    // overwrites the first `chans` of the header's 27 "spare" floats
    // with per-channel peak amplitudes (see this module's doc comment) -
    // floats 5..5+chans.
    let peak_amps: Vec<f32> = (0..channels as usize).map(|i| read_f32(5 + i)).collect();

    // Interleaved by frame across channels - frame 0's channel 0, frame
    // 0's channel 1, ..., frame 1's channel 0, ... - not channel 0's
    // frames followed by channel 1's frames. See this module's doc
    // comment for how that was confirmed.
    let mut channel_data: Vec<Vec<Vec<f32>>> =
        vec![Vec::with_capacity(frames_per_channel); channels as usize];
    let mut offset = header_bytes;
    for _ in 0..frames_per_channel {
        for ch in channel_data.iter_mut() {
            let mut frame = Vec::with_capacity(frame_floats);
            for i in 0..frame_floats {
                let o = offset + i * 4;
                frame.push(f32::from_ne_bytes([
                    bytes[o],
                    bytes[o + 1],
                    bytes[o + 2],
                    bytes[o + 3],
                ]));
            }
            ch.push(frame);
            offset += frame_bytes;
        }
    }

    Ok(PvaData {
        header: PvaHeader {
            n,
            d,
            sample_rate,
            channels,
            window_type,
        },
        channels: channel_data,
        peak_amps,
    })
}

/// Writes the new versioned `.pva` format.
pub fn write_pva(path: &Path, data: &PvaData) -> Result<(), PvaError> {
    let mut f = fs::File::create(path)?;
    f.write_all(PVA_MAGIC)?;
    f.write_all(&PVA_VERSION.to_le_bytes())?;
    f.write_all(&data.header.n.to_le_bytes())?;
    f.write_all(&data.header.d.to_le_bytes())?;
    f.write_all(&data.header.sample_rate.to_le_bytes())?;
    f.write_all(&data.header.channels.to_le_bytes())?;
    f.write_all(&data.header.window_type.to_le_bytes())?;
    f.write_all(&(data.frames_per_channel() as u64).to_le_bytes())?;
    for &peak in &data.peak_amps {
        f.write_all(&peak.to_le_bytes())?;
    }
    for ch in &data.channels {
        for frame in ch {
            for &v in frame {
                f.write_all(&v.to_le_bytes())?;
            }
        }
    }
    Ok(())
}

/// Reads the new versioned `.pva` format.
pub fn read_pva(path: &Path) -> Result<PvaData, PvaError> {
    let display = path.display().to_string();
    let mut f = fs::File::open(path)?;

    let mut magic = [0u8; 4];
    f.read_exact(&mut magic)
        .map_err(|_| PvaError::Truncated(display.clone()))?;
    if &magic != PVA_MAGIC {
        return Err(PvaError::BadMagic(display, magic));
    }

    let read_u32 = |f: &mut fs::File, display: &str| -> Result<u32, PvaError> {
        let mut b = [0u8; 4];
        f.read_exact(&mut b)
            .map_err(|_| PvaError::Truncated(display.to_string()))?;
        Ok(u32::from_le_bytes(b))
    };

    let version = read_u32(&mut f, &display)?;
    if version != PVA_VERSION {
        return Err(PvaError::UnsupportedVersion(display, version));
    }
    let n = read_u32(&mut f, &display)?;
    let d = read_u32(&mut f, &display)?;
    let sample_rate = read_u32(&mut f, &display)?;
    let channels = read_u32(&mut f, &display)?;
    let window_type = read_u32(&mut f, &display)?;

    let mut frames_bytes = [0u8; 8];
    f.read_exact(&mut frames_bytes)
        .map_err(|_| PvaError::Truncated(display.clone()))?;
    let frames_per_channel = u64::from_le_bytes(frames_bytes) as usize;

    let mut peak_amps = Vec::with_capacity(channels as usize);
    for _ in 0..channels {
        let mut b = [0u8; 4];
        f.read_exact(&mut b)
            .map_err(|_| PvaError::Truncated(display.clone()))?;
        peak_amps.push(f32::from_le_bytes(b));
    }

    let frame_floats = n as usize + 2;
    let mut rest = Vec::new();
    f.read_to_end(&mut rest)?;
    let expected_bytes = channels as usize * frames_per_channel * frame_floats * 4;
    if rest.len() < expected_bytes {
        return Err(PvaError::Truncated(display));
    }

    let mut channel_data: Vec<Vec<Vec<f32>>> =
        vec![Vec::with_capacity(frames_per_channel); channels as usize];
    let mut offset = 0;
    for ch in channel_data.iter_mut() {
        for _ in 0..frames_per_channel {
            let mut frame = Vec::with_capacity(frame_floats);
            for i in 0..frame_floats {
                let o = offset + i * 4;
                frame.push(f32::from_le_bytes([
                    rest[o],
                    rest[o + 1],
                    rest[o + 2],
                    rest[o + 3],
                ]));
            }
            ch.push(frame);
            offset += frame_floats * 4;
        }
    }

    Ok(PvaData {
        header: PvaHeader {
            n,
            d,
            sample_rate,
            channels,
            window_type,
        },
        channels: channel_data,
        peak_amps,
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tempfile(name: &str) -> std::path::PathBuf {
        let dir = std::env::temp_dir().join(format!(
            "pvc-io-pva-test-{}-{}",
            std::process::id(),
            std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_nanos()
        ));
        std::fs::create_dir_all(&dir).unwrap();
        dir.join(name)
    }

    fn sample_data(n: u32, channels: u32, frames_per_channel: usize) -> PvaData {
        let frame_floats = n as usize + 2;
        let channel_data = (0..channels)
            .map(|ch| {
                (0..frames_per_channel)
                    .map(|frame| {
                        (0..frame_floats)
                            .map(|i| (ch * 1000 + frame as u32 * 10 + i as u32) as f32)
                            .collect()
                    })
                    .collect()
            })
            .collect();
        PvaData {
            header: PvaHeader {
                n,
                d: 220,
                sample_rate: 44100,
                channels,
                window_type: 0,
            },
            channels: channel_data,
            peak_amps: (0..channels).map(|ch| ch as f32 * 0.1 + 0.05).collect(),
        }
    }

    #[test]
    fn new_format_round_trip() {
        let data = sample_data(1024, 2, 5);
        let path = tempfile("test.pva");
        write_pva(&path, &data).unwrap();
        let read_back = read_pva(&path).unwrap();
        assert_eq!(read_back, data);
    }

    #[test]
    fn new_format_rejects_bad_magic() {
        let path = tempfile("bad.pva");
        std::fs::write(&path, b"NOPE12345678").unwrap();
        let err = read_pva(&path).unwrap_err();
        assert!(matches!(err, PvaError::BadMagic(_, _)));
    }

    /// Writes a file matching pvanalysis.c's actual on-disk layout (32-f32
    /// native-endian header, then per-channel frame data) directly, rather
    /// than depending on this module's own writer, so the test exercises
    /// read_legacy_pva against an independently-constructed legacy-shaped
    /// file rather than round-tripping through code under test.
    /// Builds a legacy `.pva` fixture with frames interleaved by channel
    /// (frame 0 ch0, frame 0 ch1, frame 1 ch0, ...) - the real
    /// `pvanalysis.c` layout (see this module's doc comment), not
    /// sequential per-channel blocks. An earlier draft of this fixture
    /// wrote sequential blocks, matching what `read_legacy_pva` wrongly
    /// assumed at the time - a self-consistent test that never actually
    /// exercised the real file format, only caught once a real 2-channel
    /// `pvanalysis` run was checked byte-for-byte.
    fn write_legacy_pva_fixture(path: &Path, n: u32, d: u32, r: u32, chans: u32) {
        let mut bytes = Vec::new();
        for v in [n as f32, d as f32, r as f32, chans as f32, 0.0f32] {
            bytes.extend_from_slice(&v.to_ne_bytes());
        }
        // Floats 5..5+chans: per-channel peak amplitudes, matching where
        // pvanalysis.c overwrites them after the fact (see this module's
        // doc comment).
        let mut spare = [0.0f32; 27];
        for (ch, s) in spare.iter_mut().take(chans as usize).enumerate() {
            *s = ch as f32 * 0.1 + 0.05;
        }
        for v in spare {
            bytes.extend_from_slice(&v.to_ne_bytes());
        }
        let frame_floats = n as usize + 2;
        let frames_per_channel = 3;
        for frame in 0..frames_per_channel {
            for ch in 0..chans {
                for i in 0..frame_floats {
                    let v = (ch * 1000 + frame as u32 * 10 + i as u32) as f32;
                    bytes.extend_from_slice(&v.to_ne_bytes());
                }
            }
        }
        std::fs::write(path, bytes).unwrap();
    }

    #[test]
    fn legacy_reader_parses_header_and_frame_count() {
        let path = tempfile("legacy.pva");
        write_legacy_pva_fixture(&path, 1024, 220, 44100, 2);
        let data = read_legacy_pva(&path).unwrap();
        assert_eq!(data.header.n, 1024);
        assert_eq!(data.header.d, 220);
        assert_eq!(data.header.sample_rate, 44100);
        assert_eq!(data.header.channels, 2);
        assert_eq!(data.channels.len(), 2);
        assert_eq!(data.frames_per_channel(), 3);
        assert_eq!(data.channels[0][0].len(), 1026); // N + 2
        assert_eq!(data.channels[1][2][5], (1000 + 20 + 5) as f32);
        assert_eq!(data.peak_amps, vec![0.05, 0.15]);
    }

    #[test]
    fn legacy_reader_reads_real_pvanalysis_output() {
        // If the golden harness has been run, this is real pvanalysis
        // C-oracle output - exercise the reader against it, not just a
        // hand-built fixture, per the plan's explicit Task 2.3 test
        // requirement. Skips (rather than fails) if the harness hasn't
        // been run in this environment, since golden/expected/ is
        // gitignored and regenerated on demand (see tests/golden/README
        // conventions) - this crate shouldn't require running the C
        // harness just to `cargo test`.
        let candidates = ["../../../tests/golden/expected/pvanalysis/basic_analysis/output.pva"];
        let Some(path) = candidates.iter().map(Path::new).find(|p| p.exists()) else {
            eprintln!("skipping: run tests/golden/run_legacy.sh first to exercise this test");
            return;
        };
        let data = read_legacy_pva(path).unwrap();
        assert_eq!(data.header.n, 1024);
        assert_eq!(data.header.sample_rate, 44100);
        assert_eq!(data.header.channels, 1);
        assert!(data.frames_per_channel() > 0);
        assert_eq!(data.channels[0][0].len(), 1026);
    }

    #[test]
    fn legacy_reader_deinterleaves_real_stereo_output_correctly() {
        // Directly validates the interleaved-by-frame layout (see this
        // module's doc comment) against real 2-channel `pvanalysis`
        // output: re-derives frame N's channel 0 and channel 1 blocks
        // from the raw file bytes using that layout, and checks they
        // match what `read_legacy_pva` returns - rather than hardcoding
        // expected values, which would only prove the parser agrees with
        // itself. Skips if the golden harness hasn't been run.
        let candidates = ["../../../tests/golden/expected/pvanalysis/stereo_analysis/output.pva"];
        let Some(path) = candidates.iter().map(Path::new).find(|p| p.exists()) else {
            eprintln!("skipping: run tests/golden/run_legacy.sh first to exercise this test");
            return;
        };
        let bytes = fs::read(path).unwrap();
        let data = read_legacy_pva(path).unwrap();
        assert_eq!(data.header.channels, 2);
        assert!(data.frames_per_channel() > 10);

        let frame_floats = data.header.n as usize + 2;
        let header_bytes = LEGACY_HEADER_FLOATS * 4;
        let read_frame = |frame: usize, ch: usize| -> Vec<f32> {
            let base = header_bytes + (frame * 2 + ch) * frame_floats * 4;
            (0..frame_floats)
                .map(|i| {
                    let o = base + i * 4;
                    f32::from_ne_bytes([bytes[o], bytes[o + 1], bytes[o + 2], bytes[o + 3]])
                })
                .collect()
        };

        for frame in [0usize, 5, 10] {
            assert_eq!(
                data.channels[0][frame],
                read_frame(frame, 0),
                "frame {frame} ch0"
            );
            assert_eq!(
                data.channels[1][frame],
                read_frame(frame, 1),
                "frame {frame} ch1"
            );
        }
        // The two channels carry different tones - their frames
        // shouldn't be identical (a sequential-by-channel misparse would
        // often coincidentally satisfy the raw-byte check above at frame
        // 0 while still being wrong throughout, since both "channel 0"
        // read paths start at the same file offset - this catches that).
        assert_ne!(data.channels[0][10], data.channels[1][10]);
    }
}
