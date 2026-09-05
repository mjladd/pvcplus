pub mod audio;

pub use audio::{
    read_audio, write_aiff, write_flac, write_wav, AudioBuffer, AudioError, SampleFormat,
};
