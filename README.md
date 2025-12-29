# PVC

source pulled from: <https://sourceforge.net/projects/pvcplus/>

## INDEX

- [INTRODUCTION](#introduction)
- [INSTALLATION](#installation)
  - [Verify Installation](#verify-installation)
- [UNIX COMMAND](#unix-command-line-format)
  - [INFORMATION PAGE](#information-page)
  - [SETTING FLAG VALUES](#setting-flag-values)
  - [CONTROLLING PARAMETERS WITH FUNCTIONS](#controlling-parameters-with-functions)
  - [RUNNING COMMANDS WITH SHELL SCRIPTS](#running-the-commands-with-shell-scripts)
- [ROUTINES: SHORT DESCRIPTIONS](#routines-short-descriptions)
  - **BASIC ROUTINES**
    - [PLAINPV](#plainpv)
    - [TWARP](#twarp)
  - **AMPLITUDE WARPING**
    - [NOISEFILTER](#noisefilter)
    - [COMPANDER](#compander)
    - [SPECTWARPER](#spectwarper)
  - **ADDITIVE SYNTHESIS**
    - [HARMONIZER](#harmonizer)
    - [INHARMONATOR](#inharmonator)
    - [CHORDMAPPER](#chordmapper)
  - **SUBTRACTIVE SYNTHESIS**
    - [FILTER](#filter)
    - [FREQRESPONSE](#freqresponse)
    - [CHORDRESPONSEMAKER](#chordresponsemaker)
    - [FILTRESPONSEMAKER](#filtresponsemaker)
    - [PVANALYSIS](#pvanalysis)
    - [TVFILTER](#tvfilter)
    - [CONVOLVER](#convolver)
  - **RESONANCE/REVERB**
    - [RING](#ring)
    - [RINGFILTER](#ringfilter)
    - [RINGTVFILTER](#ringtvfilter)
  - **NONLINEAR FREQUENCY DEVIATION**
    - [FILTDEVIATOR](#filtdeviator)
    - [TVFILTDEVIATOR](#tvfiltdeviator)
  - **FEATURE EXTRACTION**
    - [ENVELOPE](#envelope)
    - [CENTROID](#centroid)
    - [FLUXOID](#fluxoid)
    - [PITCHTRACKER](#pitchtracker)
  - **CONTROL FUNCTION PROCESSING**
    - [SHAPE](#reshape)
- [TERMS AND COMMON FEATURES](#terms-and-common-features)
  - [OVERLAP/ADD METHOD VS. OSCILLATOR BANK METHOD AND RESYNTHESIS THRESHOLDS](#overlapadd-vs-oscillator-bank-methods-and-resynthesis-thresholds)
  - [SOURCE](#source)
  - [MULTIPLE CHANNELS](#multiple-channels)
  - [PLAYBACK DURING PROCESSING](#playback-during-processing)
  - [INPUT SOUND FILE](#input-sound-file)
  - [OUTPUT SOUND FILE](#output-sound-file)
  - [FLOATING-POINT AMPLITUDE RESCALING](#floating-point-amplitude-rescaling)
  - [OUTPUT STATISTICS](#output-statistics)
  - [FREQUENCY RESPONSE TERMINAL OUTPUT](#frequency-response-terminal-output)
  - [ANALYSIS FILES](#analysis-files)
  - [DECIBELS](#decibels)
  - [LOW/HI SHELF EQUALIZATION](#lowhi-shelf-equalization)
  - [WARP INDEX](#warp-index)
  - [PITCH TRANSPOSITION](#pitch-transposition)
  - [FREQUENCY SHIFT](#frequency-shift)
  - [ENVELOPE RESPONSE TIME](#envelope-response-time)
  - [RING DECAY TIME](#ring-decay-time)
  - [FFT SIZE](#fft-size)
  - [WINDOW SIZE](#window-size)
  - [WINDOW TYPE](#window-type)
  - [FRAMES PER SECOND](#frames-per-second)
  - [TIME EXPANSION/CONTRACTION](#time-expansioncontraction)
  - [BEGIN/END TIMES](#beginend-times)
  - [GAIN](#gain)
  - [FILTERING: SOURCE SIGNAL LEVEL](#filtering-source-signal-level)
  - [TRANSPOSITION/SHIFT APPLICATION FLAG](#transpositionshift-application-flag)
  - [FILTER TYPES: PASS OR REJECT](#filter-types-pass-or-reject)
  - [RESPONSE FUNCTION SMOOTHING](#response-function-smoothing)
  - [ANALYSIS DATA ACCESS MODE](#analysis-data-access-modes)
  - [CONVOLVER PANPOT](#convolver-panpot)
  - [FREQUENCY RESPONSE ACCUMULATION METHOD](#frequency-response-accumulation-method)
  - [RING ROUTINES: FILTER PLACEMENT](#ring-routines-filter-placement)
  - [COMPRESSION AND EXPANSION](#compression-and-expansion)
- [UTILITIES](#utilities)
  - [FILE CONVERSION: aiffs, aiffd, nexts, nextd, nextfloats](#file-conversion)
  - [FUNCTION VIEWING: showme, showspect](#function-viewing)
- [USING THE SHELL SCRIPTS](#using-the-shell-scripts)
  - [SAMPLE SCRIPT: S.PLAINPV](#sample-script-splainpv)
  - [SHELL SCRIPT OUTPUT SOUND FILE CONVERSION](#shell-script-output-sound-file-conversion)
  - [GEN FUNCTION CONTROL OF PARAMETERS](#gen-function-control-of-parameters)
  - [SAMPLE OUTPUT: S.PLAINPV](#sample-of-output-from-splainpv)

---

## INTRODUCTION

PVC is a collection of phase vocoder signal processing routines and accompanying shell scripts for use in the transformation and manipulation of sounds. It is written in C and designed to be used in a UNIX environment. It has come about as a result of my path of education and research into phase vocoder technology. It follows in the spirit of the work by Eric Lyon (out of which PVC is built) and Chris Penrose whose particular dsp research springs from the coding and tutorial work of F.R. Moore and Mark Dolson. Moore's book, *Elements of Computer Music*, published by Prentice Hall, is therefore a great resource for making sense of the phase vocoder engine which I am unable to go into here. Curtis Road's book, *The Computer Music Tutorial*, published by MIT Press, has sections on the phase vocoder as well; these may better introduce the beginner to the practical concerns of this technology. Short of the explanations these sources provide, I have attempted to offer below some explanations, particularly as needed for control of the parameters in these routines. A manual and tutorial would be great to have; unfortunately time has not yet made it so.

These routines reflect my need for tools which can perform different spectral resynthesis tasks; both simple and experimental. Their refinement has advanced with my growing skills and curiosity, which I expect will continue as long as I have questions about sound. Most of these routines can be viewed in terms of traditional additive or subtractive synthesis tasks, coming about as they did from the desire for greater finesse and control of these two basic types of synthesis. While the speculative nature of some give them an idiosyncratic character, most should, with practice, reveal the transparency of their names if not the role they can play in the shaping of sound. All require a good ear tuned towards sound and idea as none of these routines are automatic, although many hold great potential for the diligent.

This 3.0 release contains only those routines which I think are stable, useful and moderately transparent. Some earlier versions have been omitted, replaced or consolidated into newer routines. For example, *compander* remains, but the ideas behind *bandamp* have ripened into *spectwarper*, a remarkable "super companding" tool for windowing amplitude, and balancing the resonance/noise-residues of a sound. The harmonic tone reorganizer, *chordmapper*, has continued to grow in its controls (however arcane), offering increasingly subtle ways to reorganize harmonic spectra. The *noisefilter* routine is now very good, having become a PVC first encounter routine for many whose noisy lives cross my path. *Tvfiltdeviator* now joins the arcane but novel *filtdeviator* routine. In addition, I have added a set of feature analysis routines (*pitchtracker, centroid, envelope, fluxoid*); which should be useful in generating function files to control different synthesis strategies. There are other, more experimental routines (some actually appeared in 2.0) which are still proving themselves; in time they will appear or reappear. As with 2.0, floating-point files (combined with a rescale feature) continue to be readable and writable. Someday I will deal with AIFF headers (although they do not offer floating-point values), but not for now.

Paul Koonce
<koonce@music.princeton.edu>

---

## INSTALLATION

### Docker (Recommended)

Build and run using Docker:

```bash
# Build the container
docker build -t pvcplus .

# Run a PVC tool (mount your audio directory)
docker run --rm -v /path/to/audio:/audio pvcplus plainpv -N1024 /audio/input.wav /audio/output.wav

# List available tools
docker run --rm pvcplus

# Interactive shell
docker run --rm -it -v /path/to/audio:/audio pvcplus /bin/bash
```

### Verify Installation

This walkthrough confirms the container is working correctly by processing a test audio file.

**Step 1: Create a test directory and audio file**

```bash
# Create a working directory
mkdir -p ~/pvctest
cd ~/pvctest

# Generate a 2-second test tone (440Hz sine wave) using sox, ffmpeg, or your DAW
# With sox:
sox -n -r 44100 -c 1 test_input.wav synth 2 sine 440

# Or with ffmpeg:
ffmpeg -f lavfi -i "sine=frequency=440:duration=2" -ar 44100 test_input.wav
```

**Step 2: Verify tools are available**

```bash
docker run --rm pvcplus
```

You should see a list of 54 tools including `plainpv`, `harmonizer`, `noisefilter`, etc.

**Step 3: View tool help**

```bash
docker run --rm pvcplus plainpv
```

This displays the flag options for plainpv. No input file means it shows help.

**Step 4: Process audio - Time stretch to 2x duration**

```bash
docker run --rm -v ~/pvctest:/audio pvcplus \
  plainpv -N1024 -I2 /audio/test_input.wav /audio/test_stretched.wav
```

The `-I2` flag stretches time by 2x. The output file should be ~4 seconds.

**Step 5: Process audio - Pitch shift up 7 semitones (perfect fifth)**

```bash
docker run --rm -v ~/pvctest:/audio pvcplus \
  plainpv -N1024 -P7 /audio/test_input.wav /audio/test_pitched.wav
```

The `-P7` flag transposes pitch up 7 semitones. Play the output to hear ~659Hz (E5).

**Step 6: Verify output files**

```bash
ls -la ~/pvctest/
# You should see:
#   test_input.wav      (original 440Hz tone, ~2 sec)
#   test_stretched.wav  (440Hz tone, ~4 sec)
#   test_pitched.wav    (659Hz tone, ~2 sec)
```

Use any audio player to verify the results sound correct.

## UNIX COMMAND-LINE FORMAT

The routines are UNIX, command-line routines in the form of: `routine [flags] input_soundfile output_soundfile`

At present the only soundfile formats accepted (both input and output) are NEXT/SUN format files in either 16-bit short samples, or 32-bit floating-point samples. In PVC, the float form has a required rescale function, which is the whole reason for using floats in this case. All processing is done in floats. Control of each routine's parameters is done through flags, as in: `plainpv -N1024 -p12 input.snd output.snd`

In most cases, the parameter flag inputs allow you to specify either a constant (i.e. -p12) or a function file (i.e. -p/tmp/pitch_change); function files give you time-varying control over a parameter (see below). In most cases, parameters are initialized to default values.

### INFORMATION PAGE

Information about any routine can be seen by typing the name of the routine without any arguments/files. Typing: `plainpv` produces the following information about plainpv.

```shell
plainpv:  generic phase vocoder with dynamic controls
plainpv   [flags] [input file (16-bit shorts)] [output file (optional)]
           (values in brackets denote defaults)
       N:      FFT length (must be a power of 2) [1024]
       M:      window size in samples (must be a power of 2) [2*FFT]
                   (0 will automatically set window to 2*FFT size or larger)
       w:      window type: 0 = hamming,  1 = rectangular
                   2 = Blackman,  3 = Bartlett triangular [0.]
                   4-12 = Kaiser windows for alpha = 4-12,  respectively
                   (representative sidelobe levels for alpha:
                     4 = -30dB,  8 = -58 dB,  12 = -90 dB)
       D:      analysis frames per second [200]
       I:      time expansion/contraction factor  [1.]
                 (duration = duration * factor, 1. = original time)
       P:      pitch transposition in semitones (func) [0]
       a:      frequency shift factor
                   (bin frequency adder, before -P )(func) [0.]
       b:      begin time in seconds  [0.]
       e:      end time in seconds ( 0. = end of file) [0.]
       C:      resynthesis channel (1 -> ?) (0 = all) [0]
            SHELF EQ:(post transpose/shift)
       H:      SHELF EQ: Low shelf gain in dB (func) [0.]
       X:      SHELF EQ: High shelf gain in dB (func) [0.]
       m:      SHELF EQ: Low shelf frequency in Hz (func) [200.]
       R:      SHELF EQ: High shelf frequency in Hz (func) [2000.]
       W:      warp index for reshaping magnitude response (func) [0.]
                   Values > 0 expand the dynamic range,
                   values < 0 compress the dynamic range.
       A:      gain in decibels (func) [0.]
       l:      envelope attack time  (func) [0.]
       L:      envelope release time   (func) [0.]
       T:      BRICKWALL FILTER TYPE: 0 = bandpass, not 0 = band reject [0]
       f:      frequency window: low boundary
                   (before -P and -a) (in Hz) [0.]
       F:      frequency window: high boundary
                   (before -P and -a)(in Hz) [Nyquist frequency]
       p:      amplitude reports print mode: 0 = off, 1 = on [0]
       i:      time interval between amplitude reports [.25]
       _:       OUTPUT FORMAT: 0 = taken from input file
                   1 = 16-bit integer, 2 = 32-bit floats [0]
       =:       PEAK RESCALE LEVEL (float output only) 0 to -96 dB
                   Set to 1 to rescale to level of input file. [ 1 ]
               TERMINAL DISPLAY AND GRAPH FILE OUTPUT
       n:          number of frames  [0]
       u:          low bin frequency  [-1]
       U:          high bin frequency
                   (-1 = nyquist) [Nyquist frequency]
       S:      TERMINAL DISPLAY: display option  [0]
                 (0 = off,  1 = phase data,  2 = amp data, 3 = both)
       c:      GRAPH FILE: WRITE ascii to FILE
                   0 = off,  1 = freq,  2 = decibels [0]
                   3 = decibels - waterfall plot
                   (When on,  this flag writes ascii point pairs
                    (with time frame on x axis) for plotting
                     with gnuplot.)
       d:      TERMINAL DISPLAY FILE NAME for -c [./ascii.out]
       t:      oscillator resynthesis threshold in decibels [ -96 ]
```

### SETTING FLAG VALUES

If no output file is specified, the name pv.out.snd will be used in the local directory. The bracketed values at the end of each parameter represent the default value; it can be changed by specifying the flag letter preceded by a minus sign and followed by the new value, with no spaces on either side. For example, the following: `plainpv -N2048 inputfile outputfile`

would change the FFT size to 2048. Some flags require files rather than constants. For these, simply supply the full pathname of the needed file as in: `twarp -F/path/to/analysis_file`, which supplies twarp with the necessary analysis file.

### CONTROLLING PARAMETERS WITH FUNCTIONS

Parameters which have the word (func) on the info page just before the default as in:

*W: warp index for reshaping magnitude response (func) [0.]*

can be controlled dynamically. This is done by providing a full pathname file in place of the constant. The file is assumed to be a headerless series of values representing how the parameter will evolve as a function of time. The values may be either 32-bit floating-point values, or ASCII numbers, arranged one-per-line (the routine deciphers which it is). The function file can have any number of values as the series is fitted to the specified duration, linearly interpolated to produce the values in between. Function files in 32-bit floating-point form can be created with the CMUSIC gen routines provided with this package.

### RUNNING THE COMMANDS WITH SHELL SCRIPTS

While all routines can be run at the commandline, they are most easily run using the shell scripts found in the `scripts/` directory. These scripts are useful for saving and managing the parameters; in many ways they are a poor-man's GUI. All scripts contain a top section for setting variables, and a bottom section where those variables are placed into the commandline flag structure and run. Some scripts perform two routines such as a short analysis routine followed by the main synthesis routine, while others run just one routine. The variables for the routines in a shell are set in the top section. Take note that shell script variable assignments do not allow for spaces. The numerous parameters, which in some routines run as high as 53, make these scripts a necessity. They will be your friend if you take care to leave the bottom part alone, and don't corrupt your variable names. Someday I will make a better way to interface with the routines; for now this is the way it is.

To run the scripts, simply type the name of the file (or appropriate pathname when running from outside the directory in which it resides). For example: `S.plainpv`.

(If this does not work, check to make sure the script is executable, and that the first line contains `#!/bin/sh`).

If things are working correctly, the resynthesis should begin which you will know from the output streaming to the terminal.

(See [the explanation below about using shell scripts](#using-the-shell-scripts).)

---

## ROUTINES: SHORT DESCRIPTIONS

Below is a listing of the routines contained in this release along with a short description of what each does.

### BASIC ROUTINES

#### PLAINPV

*Plainpv* is a basic phase vocoder with control of pitch transposition, frequency shift, time scale, amplitude warp and low/high shelf equalization. It also has some nice controls for looking at the data produced by the phase vocoder. Run this routine with `S.plainpv`. If you are interested in looking and/or graphing segments of the data, run the `S.plainpv_with_printout_and_graph_files` script instead and use the *showspect* utility. You will need to have *gnuplot* installed.

#### TWARP

*Twarp* is like *plainpv* except that it works from an analysis file rather than a soundfile. This allows you to move forwards/backwards through time according to a time function file. Use [*pvanalysis*](#pvanalysis) through the script `S.pvanalysis` to make the analysis file; then run the script `S.twarp`.

#### NOISEFILTER

*Noisefilter* filters out the noise in a sound by subtracting out a frequency response. The frequency response is analyzed from a short segment in the file where noise alone is found. For sounds that do not have segments of isolated noise, there is a threshold mode. Run with `S.noisefilter`.

### AMPLITUDE WARPING

#### COMPANDER

*Compander* is a classic compressor/expander. What is different here is the use of a peaks response file. The peaks response file is a frequency response, analyzed from a segment of the sound, that is taken to represent the peak bin amplitudes for the sound. Each frequency bin of the peaks frequency response functions as the 0 dB reference point for that frequency bin. The amplitude of the frequency bin is companded relative to this reference. The entire analysis/companding process (including the analysis segment using [*freqresponse*](#freqresponse)) can be run using the script `S.compander`.

#### SPECTWARPER

*Spectwarper* uses an expanded compansion scheme to highlight either a sound's stronger, resonant components or its weaker noise/residual components. *Spectwarper* is fairly similar to [*compander*](#compander); however, unlike *compander* which compands bins against the constant peak of an input response file, *spectwarper* compands bins using a peak drawn (in the current frame) from a narrow frequency band centered around the value being processed. This causes the compansion or "warping" of the amplitudes to accentuate (expansion) or mask (compression) *formants* located within the frequency bands; the result being the noise/pitch highlighting mentioned earlier. Part of this comes from the treatment of compression in *Spectwarper*. Unlike *compander* which only reduces the amplitude above the threshold when compressing, *spectwarper* reduces the amplitude of the entire range, becoming, in effect, an expander of the strongest amplitudes that expands them (when the compression level is severe) out of the picture. *Spectwarper* is one of my favorite routines of late simply because it provides such a simple and powerful control over the noise and pitch characteristics of a sound. I love it, and use it often. Run this routine with `S.spectwarper`.

### ADDITIVE SYNTHESIS

**HARMONIZER, CHORDMAPPER, AND INHARMONATOR:**

These routines all allow for a kind of additive synthesis based on the remapping of phase vocoder data according to some model. Each requires an ascii data file specifying how phase vocoder information will be replicated or mapped. This mapping is constant for the run of the routine.

#### HARMONIZER

*Harmonizer* works much like a commercial harmonizer in that it allows you to create harmony against the source by adding a transposed copy of it. Here the concept is extended by allowing for multiple harmonizations, each taken from a different band of frequencies, output with separate gain. Run this using the script `S.harmonizer`.

#### CHORDMAPPER

*Chordmapper* lets you specify how harmonically related groups of partials will be replicated or mapped to produce chords. An input data file organizes the remapping into tone groups, and includes ways to tune or neutralize the frequency deviations of partials. Time-varying control of these features is available as well. You can use this routine to build up thick chords from single tones, or to delicately reorganize a harmonic spectrum. Run this using the script `S.chordmapper`.

#### INHARMONATOR

*Inharmonator* lets you specify how the partials of one fundamental will be remapped or deviated. While the more recent and developed routine [*chordmapper*](#chordmapper) is probably better for this task, I have decided to leave this routine in for now. (Think chordmapper.)

### SUBTRACTIVE SYNTHESIS

#### FILTER

*Filter* is a very useful routine for filtering a sound by a frequency response. Filtering is achieved by first creating the frequency response through either synthesis or analysis, followed by filtering with *filter*. Synthetic responses are created using either [*chordresponsemaker*](#chordresponsemaker) (which synthesizes a spectrum as a collection of harmonic tones), or [*filtresponsemaker*](#filtresponsemaker) (which synthesizes a frequency response using lines and breakpoints). Analyzed responses can be made with [*freqresponse*](#freqresponse) (which analyzes a sound file segment and constructs a response representing the peak or average amplitudes). Once made, the magnitudes of the FFT response are multiplied against the time varying magnitudes of the input sound's FFT. *Filter* allows time-varying control of the response shape (warp), transposition/shift, compansion, smoothing, and source/filter mix, making this a very useful tool for quickly manipulating the spectral characteristics of a sound according to your synthetic or analytic goals. The synthetic forms can be run with the scripts `S.filter_with_chord_synthesis` or `S.filter_with_breakpoint_synthesis`; the analysis-based form with `S.filter_with_analysis`. The analytic form is a powerful tool for bringing the color of one sound into the realm of another.

#### FREQRESPONSE

*Freqresponse* is a routine used by several others to prepare a spectrum for use with routines that filter, compress or limit. The response can be normalized or not depending on the needs of the routine which will use the response.

#### CHORDRESPONSEMAKER

*Chordresponsemaker* is a routine that uses a collection of harmonic tones, variable in size, to create a synthetic frequency response. It is found in various filtering scripts.

#### FILTRESPONSEMAKER

*Filtresponsemaker* is a routine that uses breakpoints and straight lines to create a synthetic frequency response. It is found in various filtering scripts.

#### PVANALYSIS

*Pvanalysis* is the time varying form of [*freqresponse*](#freqresponse) that creates a phase vocoder analysis for use by other routines. The routines which require pvanalysis files are [*twarp*](#twarp), [*convolver*](#convolver), [*tvfilter*](#tvfilter), [*ringtvfilter*](#ringtvfilter), and [*tvfiltdeviator*](#tvfiltdeviator). Run this using the script `S.pvanalysis`.

#### TVFILTER

*Tvfilter* is the time-varying (tv) form of [*filter*](#filter). *Tvfilter* uses a [*pvanalysis*](#pvanalysis) file to change the magnitudes of the input sound file. As it is with *filter*, *tvfilter* multiplies the magnitudes of the analysis FFT against the magnitudes of the input sound's FFT, while preserving the frequency/phase characteristics of the input sound. Preserving the phase of the input sound file results in a cross-synthesis which sounds like the input sound file covered or suppressed by the shadow of the analysis file. Like *filter*, *tvfilter* offers a variety of controls for manipulating the filter characteristic. The use of a phase vocoder analysis to represent the filter characteristic also makes possible the temporal control of the filter file (i.e. backwards/forwards control) as found with [*twarp*](#twarp). Run this using the script `S.tvfilter`.

#### CONVOLVER

In its setup and control, *convolver* is the same as [*tvfilter*](#tvfilter). Its processing, however, is different. In *tvfilter* filtering is produced by multiplying the magnitudes from the polar form of the two analyses; leaving the phases (or frequencies) of the source intact while modifying the amplitudes of those frequencies. *Convolver* goes a bit further by multiplying the two analyses in their Cartesian forms. This produces an intersection of the two spectra. Unlike *tvfilter* which produces a shadowlike intersection, shadowing the analysis file characteristic onto the input sound file, *convolver* creates a true spectral intersection, allowing only that which is common to both sounds to be heard. The effect is a sound which is somewhat garbled as it outputs the more intermittently common spectral components of the two. The form of the multiplication in *convolver* does not allow some of the filter transposition controls associated with *tvfilter*. There is however a [convolution panpot](#convolver-panpot) which offers control of the mix between the convolution and source sounds. Run this using the script `S.convolver`.

### RESONANCE/REVERB

#### RING

*Ring* uses the phase vocoder to create an all-pass resonator. It works by structuring the FFT resynthesis as a bank of feedback filters that feed back the sinusoid of each bin in a strength proportional to the amplitude of that bin (after adjustment by global feedback controls). This allows the sound to "ring" in a way something like reverb or comb filter resonance. The difference from comb filtering is that with *ring* spectral resonance is created not through a collection of comb filters selected for their ability to resonate various pulse wave spectra, but rather, through an array of feedback filters (sized by the FFT) that resonate a sine wave spectrum while dynamically tuning their feedback frequencies to the frequencies of the input sound. In short, it creates a kind of "self resonance". Ring is a nice way of increasing the resonant pitch characteristics of a sound, although it has its weaknesses. Ring works best with larger FFT sizes as it is attempting to synthesize or accentuate the more pitched/harmonic characteristics of the sound; this is something larger FFTs, with their increased frequency resolution, handle better. Use of the Kaiser window, with its low sidelobe amplitudes, helps as well. In addition, there is a threshold for preventing the noise features of a sound from being resonated, plus an EQ which can be positioned to filter either the source input to the feedback loop, or the feedback return. Run this using the script `S.ring`.

#### RINGFILTER

*Ringfilter* marries [*filter*](#filter) with [*ring*](#ring) by allowing a frequency response to be imposed on the resonance created with *ring*. *Ringfilter* begins to look more like multiple-delay, comb filter resonance since the static frequency response selects which frequencies will feed back. What is unique here is that the frequency response can come from an analysis, allowing the input sound to be resonated by the average spectral characteristic of another sound. A synthesized frequency response can be used as well. Like the EQ in *ring*, the filter in *ringfilter* can be positioned to either filter the source input to the feedback loop, or the feedback return where it will have the effect of introducing the filter characteristic more slowly through the resulting variable rates of decay. Run *ringfilter* with `S.ringfilter_with_chord_synthesis` to create a synthetic frequency response, and with `S.ringfilter_with_analysis` for an analyzed frequency response.

#### RINGTVFILTER

*Ringtvfilter* is to [*ringfilter*](#ringfilter) what [*tvfilter*](#tvfilter) is to [*filter*](#filter); that is, it makes the filter in *ringfilter* time-varying. This is a sophisticated idea, that is, time-varying filtering of the resonance of a time-varying sound. The best characterization would be to say that *Ringtvfilter* imprints the shadow of one sound onto the reverb of another. *Ringtvfilter* requires some thought and finesse in order to separate and articulate the evolutions of the source, resonance, and filter. The best results are created using dynamic, high-profiled source sounds, rich with transient noise; and more constant, pitch/harmonic sounds for the time-varying filter. Like *tvfilter*, *ringtvfilter* requires an analysis file. Run this routine using `S.ringtvfilter`.

### NONLINEAR FREQUENCY DEVIATION

#### FILTDEVIATOR

The idea behind *filtdeviator* is to use a frequency response function to not only filter a sound (as with [*filter*](#filter)), but to create a topology of frequency deviation working in correlation with the filter. Consequently, *filtdeviator* is *filter* with added parameters for specifying how the filter frequency response function will be mapped into the deviation of frequency. The added parameters set the base and peak deviation for how the response will be mapped into both pitch transposition and frequency shift, and how the function will be warped within the range set by these limits. There is also a master (0-1) deviation control for globally controlling the deviation. All the controls of *filtdeviator* allow you to dynamically vary the presence and effect of amplitude filtering and frequency deviation, making *filtdeviator* an interesting routine for exploring the way filters can be used to impede/transform the resonant signature of a sound. Using small amounts of frequency deviation, with no amplitude filtering, and a sweeping transposition of the filter will produce an effect something akin to the commercial guitar phase shifter; larger amounts of deviation take it into another place entirely. Adding the correlated amplitude filtering conceals the deviation more (positioning it more at the edges of formants), producing a sound something like the floppy resonant behavior of slide whistles. The scripts to run *filtdeviator* -- `S.filtdeviator_with_chord_synthesis` and `S.filtdeviator_with_analysis` -- are designed with frequency response synthesis/analysis sections like those for *filter* and *ringfilter*. Run this routine using either `S.filtdeviator_with_analysis` or `S.filtdeviator_with_chord_synthesis`.

#### TVFILTDEVIATOR

*Tvfiltdeviator* is to [*filtdeviator*](#filtdeviator) what [*tvfilter*](#tvfilter) is to [*filter*](#filter); i.e. it uses a time-varying filter response in place of the constant one. This routine blows the lid off of what was unusual about *filtdeviator*. It's great for making wacky sounds out of ones with nice, fixed harmonies. The best use is to use it to deviate itself. Try taking something like a harpsichord or guitar (pitched stuff with decay) and do an analysis of the sound with [*pvanalysis*](#pvanalysis). Then use the analysis to deviate the same sound. What happens is the strength of each of the sound's components becomes a control over the frequency deviation of that component, one that causes the sound to go "sproing" whenever it has any amplitude. Makes tonal music sound really broken. Run this routine with `S.tvfiltdeviator`.

### FEATURE EXTRACTION

#### ENVELOPE

*Envelope* is a routine for tracking the amplitude envelope of a sound. Output can be ASCII, floats or a NeXT soundfile. Selecting floats or ASCII will produce a file suitable for use in the control of a parameter. Run this routine with `S.envelope`.

#### CENTROID

*Centroid* is a routine for tracking the centroid of a sound. The centroid is the average of all the frequencies weighted by their amplitudes. It essentially gives you a kind of center frequency value for your spectrum. The analysis can be restricted to a band of frequencies, allowing the centroid to track a particular frequency component (although *pitchtracker* can do this as well). Selecting floats or ASCII will produce a file suitable for use in the control of a parameter. Run this routine with `S.centroid`.

#### FLUXOID

*Fluxoid* is a routine for tracking the average frequency change of a sound. The average can be weighted (best) or not by the amplitudes. Selecting floats or ASCII will produce a file suitable for use in the control of a parameter. Run this routine with `S.fluxoid`.

#### PITCHTRACKER

*Pitchtracker* is a routine for tracking the fundamental pitch trajectory of a sound. It is an experimental routine that works, I believe, but forever has its quirks. Three detection methods are available for following the 1) fundamental of the harmonic collection, 2) the strongest formant, or 3) a band-limited centroid. Different output formats let you see, hear and eventually use the fruits of your pitch tracking. Run this routine with `S.pitchtracker`.

### CONTROL FUNCTION PROCESSING

#### RESHAPE

*Reshape* is a routine for transforming function streams to meet the needs of different parameters. It takes a headerless float or ASCII function file as input and outputs a headerless stream of float or ASCII values. With the appropriate flags, it can be used to *limit, resample, translate, warp, expand, shrink, invert, quantize*, and *lowpass filter* the input values. The output can be translated into different amp or pitch units depending on your needs. Run reshape at the command line.

---

## TERMS AND COMMON FEATURES

Below are various terms, parameters, or ways of doing things which are common to many of the routines.

### OVERLAP/ADD VS. OSCILLATOR BANK METHODS AND RESYNTHESIS THRESHOLDS

The phase vocoder resynthesizes the signal using one of two methods, depending on the type of changes made to the FFT. If the changes are only to the magnitudes (amplitudes), then the faster overlap/add method is used. If however changes in frequency are made, then the FFT integrity is compromised, necessitating use of the oscillator bank method in which each bin is synthesized as a sine wave changing in frequency and amplitude. This method is slower, although a resynthesis threshold is available which can be used to increase the computation speed by turning off bins whose amplitude falls below the threshold. A threshold of -60dB is appropriate, although safety warrants using a lower threshold if the spectrum is thin and its decays exposed; *use your ear*.

### SOURCE

The source sound is the original input sound. Some routines allow for the mix of the processed sound with the original source sound.

### MULTIPLE CHANNELS

All routines allow both monophonic and multi-channel input files to be processed. With multi-channeled files, you can either select one channel and produce a monophonic output file, or process all the channels. Channels are numbered beginning with 1. Processing of multi-channeled files is done one channel at a time beginning with channel 1, with zeros written to channels which have yet to be processed. Processing one channel at a time requires less memory and allows you to audition the output sooner than if you did all channels at once.

### INPUT SOUND FILE

The input sound file must be a NeXT/Sun format sound file in either 16-bit short or 32-bit floating point format. It may have one or more channels.

### OUTPUT SOUND FILE

The output sound file is written as a NeXT/Sun format sound file in either 16-bit short or 32-bit floating point format, of one or more channels. The channels are processed one at a time beginning with the first channel. The first pass writes zeros in the channels yet to be processed, replacing them when processing proceeds to those channels.

### FLOATING-POINT AMPLITUDE RESCALING

Selection of the floating-point, output-file format invokes an amplitude rescaling feature. Once processing is complete, a second pass through the sound file is made to rescale the values to the decibel level specified. A dB rescale level of 1 causes rescaling to the level of the original input file.

### PLAYBACK DURING PROCESSING

The header of the output soundfile is updated often, so if your peak amplitude has not exceeded the 16-bit limit of the converters, you may play the float or integer output file before processing has completed.

### OUTPUT STATISTICS

Two flags are provided for controlling the output amplitude statistics; one turns the statistics on or off, and the other sets how often they will be reported. The statistics provide the peak output level in amplitude and decibels. With integer format output files, output values exceeding the normalized peak amplitude of 1. (0 dB) are clipped to a value of 1.0, and the statistics placed in clip mode; in clip mode reports are made only for frames where clipping occurs. The peak amplitude, its time, and the number of clipped samples are reported at the end of processing. With floating-point format output files, output values exceeding the normalized peak amplitude of 1. are not clipped since they will be rescaled in the second pass; output statistics proceed normally throughout. The levels before and after rescaling are reported at the end of processing.

### FREQUENCY RESPONSE TERMINAL OUTPUT

In many filtering or companding routines, a crude terminal print of the frequency response is available. A flag sets the high cutoff frequency for this output; a value of 0 (0 Hz) turns printing off.

### ANALYSIS FILES

Analysis files are binary, 32-bit floating-point files written by *pvanalysis*, containing frames of FFT analysis data for one or more channels. Analysis file data is preceded by a header containing information about the analysis. Analysis files are much larger than the sound files they represent, and increase in proportion to the FFT size used. As such, files can become very large, so it is advisable to only make them when needed unless you have disk space to spare.

### DECIBELS

Amplitude is always handled in decibel units. The greatest magnitude of the 16-bit short integer is equated with an amplitude of 1.0 or 0 dB. 0 dB functions as unity gain, and the peak amplitude in issues of compression, expansion, and amplitude windowing. A change of +/- 6 dB represents a doubling or halving of the amplitude. Increments of 10 dB are loosely associated with one change in dynamic level. 16-bit shorts allow for a 96 dB dynamic range. Take care not to lose signal level as a consequence of processing since quantization noise will emerge when you attempt to regain your signal level by amplifying the integer sound file.

### LOW/HI SHELF EQUALIZATION

Equalization has been provided at various points in routines to allow for the needed adjustment of spectra. The EQ consists of low and hi shelf segments, whose width is adjusted through control of the shelf breakpoint frequency. The region between the shelf segments is represented by a linear decibel gradient between the decibel levels of the two shelves. Some routines implement the EQ before pitch changes, others after. EQ placed before pitch changes (pre-transpose/shift) will cause the EQ to be transposed with the pitch changes, whereas afterwards (post-transpose/shift) will keep them fixed as shifts and transpositions occur.

### WARP INDEX

Many of the routines employ the principle of warping in which a distribution of values is transformed by an identity function. In these places an exponential function is employed to remap a 0-1 range of values into a new orientation that preserves the minima (0) and maxima (1) while bringing the distribution closer to either extreme as a result of the curvature of the exponential function selected. The curvature of the exponential function is selected through a warp index. Specifically, warp index *w* will reorient the input *x* through the function below (^ = exponentiation).

`y = (1. - (e^(x * w))) / (1. - (e^w))`

In this function, the warp index of 0 produces a linear function and an untransformed output. Positive warp index values of increasing magnitude produce curves of increasing concavity (increasing slope) that draw values towards the 0-valued minima, and reduce the function integral. Negative values do the opposite, drawing values towards the maxima of 1, increasing the integral.

The practical use of this mechanism is found in various places. One such place is the reshaping of the frequency response distribution characteristics. In this, positive warp indices cause the peaks of the response to be accentuated while the weaker frequencies are expanded out (i.e. pushed towards 0). Negative values have the opposite effect as they compress the dynamic range of the response and raise the relative level of the weaker noise components. Another place where warp applies is in the remapping of FFT amplitudes through the spectrum warpshape. In this, the successive FFT frames have their amplitudes remapped by the identity function, similarly expanding or compressing the dynamic range depending upon the warp specified; 0 (linear warp function) leaves the amplitudes unchanged.

### PITCH TRANSPOSITION

With the pitch transposition control, a constant or function value is multiplied against all bin frequencies. This is classic transposition, here specified in semitones of transposition (12 semitones equals an octave). Conversion is made to produce the appropriate frequency multiplier.

### FREQUENCY SHIFT

With the frequency shift control, a constant or function value is added to all the bin frequencies to produce a nonlinear pitch domain translation of the spectrum. Frequency shift is related to things like ring modulation and their similarly nonlinear shifts of pitch characteristics. Use this to create small distortions of the harmonic integrity of a sound.

### ENVELOPE RESPONSE TIME

The rate at which amplitude changes are allowed to occur effects how smooth spectral evolutions will be. To control this, many routines contain attack and decay response times controls: once translated these controls manipulate the coefficients of the following filter.

`y(n) = (1. - A) * x(n) + A * y(n)`

The filter is a lowpass designed to increasingly smooth the sudden changes in a signal as the value of the coefficient, *A*, is increased. Its control is through the response time parameter which is the time in seconds it takes a signal, shifting from one state to another, to decay to -60 dB of its former state. Response times are transformed to create the necessary coefficients for the selected frame rate. The response time is separated into attack and decay; this allows separate control of the smoothing of the signal depending upon whether it is increasing or decreasing in amplitude. Short attack/decay response times can be used in places where dynamic processing induces garble or even pops. You can use longer response times to generally smooth or blur the onset/offset of sound components, particularly if the response controls are being applied to a time-varying filter. When applied to amplitudes, longer decay response-times do not sound good, for in their delay of the decay, they end up amplifying the residual noise of a sound.

### RING DECAY TIME

Decay time is an issue in the feedback of the ring routines. Like response time, it is the time it takes the signal to decay to -60dB of its former state, or better, the time it takes the reverb to decay to -60dB.

### FFT SIZE

The FFT size must be a power of 2. Larger FFT sizes resolve frequencies better but transient behavior more poorly. Choose your FFT size according to the sound you are working with. A size of 1024 or 2048 works well in most cases.

### WINDOW SIZE

The window size is a less opaque parameter; like the FFT, it must be a power of 2. Windows which are twice the size of the FFT work well. Larger window sizes may resolve frequencies better. Specifying 0 for the window size will automatically set the window to twice the FFT size, a feature I have always used.

### WINDOW TYPE

The FFT and inverse FFT are computed using a window. Like the FFT size, the shape of the window used can effect the quality of the analysis and resynthesis. (See F.R.Moore, Stieglitz, or Roads for further explanation.) A variety of windows are available including: Hamming, Rectangular, Blackman, Triangular, and Kaiser (in 8 different forms as related to 8 different alpha values). Blackman (-w2) or Kaiser (-w8) are recommended for most applications. In some unusual cases where transient behavior is being lost, consider using other windows such as the Rectangular, although take care to assure that it is not producing pops or a buzzy sound.

### FRAMES PER SECOND

This controls how often the phase vocoder will perform an analysis on the signal. It is a translation of the classic decimation control which specifies how many samples to skip between analysis frames. More frames increases the resolution of time but decrease speed. 200 frames per second is a good reference point. If you expand time you should increase this proportionately to maintain about 200 or more frames per second.

### TIME EXPANSION/CONTRACTION

Once the spectral modifications are made to the FFT analysis, an inverse FFT is invoked to produce the samples of a time-domain signal. The classic phase vocoder paradigm controls the number of samples through the interpolation value and its relation to the decimation. The arcane relationship of decimation and interpolation is here translated into the parameter of time expansion/contraction, allowing for the direct scaling of time. Use values greater than 1 to expand time, less than 1 contract it.

### BEGIN/END TIMES

Processing may be performed on an entire file or a segment of it by specifying begin and end times. End times less than or equal to 0 default to the end of the input file.

### GAIN

The output and other components can be gained. 0 dB represents unity gain, no change. See [decibels](#decibels).

### FILTERING: SOURCE SIGNAL LEVEL

The mix of source and filtered sounds in the filter routines can be controlled by the source decibels floor. This value, taken from the -96 to 0 dB range, specifies the level of the source signal. The filtered signal level is equal to (1 - source amplitude floor). Consequently, the source level functions as a floor over which lies the filtered signal. A source floor of 0 dB would neutralize filtering since there would be no filter range above the floor, a floor of -96 dB would produce the full effect of the filter.

### TRANSPOSITION/SHIFT APPLICATION FLAG

Filter routines which allow for transposition and frequency shifting of both filter and source have a flag which specifies whether transposition/shift should be applied before or after filtering. If it is applied before, the pitch transposition trajectory will evolve independent of the filter's trajectory of transposition. If it is applied after, then the pitch transposition trajectory will be added to the filter transposition trajectory, causing the filter to move in parallel with the pitch transposition movements plus any movements the filter transposition parameter adds.

### FILTER TYPES: PASS OR REJECT

Filters can be toggled to use frequency responses in pass or rejection mode. In pass mode, the response's stronger magnitudes are used to pass source through the filter; in rejection mode, to impede or reject components. In rejection mode, the response is created by inversion in the decibel range, not amplitude. In time-varying filtering (*tvfilter*), rejection can be in mode 1 in which the response is inverted against a constant 0 dB peak, or in mode 2 in which the response is inverted against the current analysis frame's peak amp. Spectral warping is always applied after the response has been transformed by rejection.

### RESPONSE FUNCTION SMOOTHING

Many routines which use frequency response files to filter or warp amplitudes have a control which allows the response to be smoothed. The smoothing is produced by replacing the magnitude of a frequency bin with an average taken from a band centered around that bin. The degree of smoothing is controlled through manipulation of a bandwidth value, specified in octave units. Larger bandwidths produce greater degrees of smoothness, 0 turns smoothing off.

### ANALYSIS DATA: ACCESS MODES

Routines which use analysis data made with [*pvanalysis*](#pvanalysis) -- [*twarp*](#twarp), [*convolver*](#convolver), [*tvfilter*](#tvfilter), [*ringtvfilter*](#ringtvfilter), and [*tvfiltdeviator*](#tvfiltdeviator) -- access data the same; using the *time-point, rate*, and *data window boundary* parameters, set to function in either *rate* or *explicit* mode. In *rate mode*, the *rate* determines the speed of movement through a data file; the *time-point* sets the starting position. The *rate* may be positive (forward in time) or negative (backwards in time), and vary according to a function. *Explicit* mode uses the time point parameter to specify exactly where the analysis data should come from (units here are in the time of the analyzed sound). (*Explicit* mode does not use the *rate* control, and makes sense only if the *time-point* is controlled with a function.) Both *rate* and *explicit* modes abide by the upper and lower *data window boundaries* which delimit the data range. When the *time-pointer* moves beyond the specified upper and lower time boundaries, it re-enters the window from the other end, making the window into a circular/modular structure. The boundaries can be controlled with functions as well, giving this mode an expressive dimension far surpassing the time expansion/contraction parameter. There is also an *auto-stop* feature that, when turned on, causes processing to stop when it reaches the end of the analysis.

### CONVOLVER PANPOT

The [*convolver*](#convolver) routine has a unique panpot mechanism for controlling the mix of input sounds (A and B) with their convolution. The panpot is a crossfade mechanism that uses a -1 to 1 control range to accentuate either sound A, B or their convolution. A value of -1 produces an output consisting entirely of sound A, a value of 1, sound B. The 0 between these extremes produces the convolution of A and B. Values between these points produce a crossfade mix of either A or B and the convolution. For example, a trajectory from -1 to 1 would crossfade from sound A into the convolution, and on to sound B. Separate gain controls for A, B and the convolution make it possible to tune the continuity of this trajectory. In addition, the presence or spread of the convolution into the crossfade range can be tuned with the domain warp controls. The domain warp reshapes the movement through the crossfade range, allowing you to create a more gradual approach from A or B into the convolution center. This is achieved through a simple nonlinearizing of the crossfade domain in *warp index style*. Increasingly positive domain warp values (specified independently for each side) transform the linear trajectory towards the convolution into a decelerating one, causing the subtle mix area around 0 to be expanded. Therefore, if you want to hear more convolution in your crossfade, increase the panpot domain warps.

### FREQUENCY RESPONSE ACCUMULATION METHOD

Several of the response-producing routines have the option of accumulating the response by either peak or average means. Whereas peak responses represent the record of a sound's thresholds (or synthesis specification's highest values), average responses represent the most common characteristics. If the sound you are analyzing has intermittent moments of sound whose peak characteristics you wish fully represented in the response, use the peak mode; otherwise use the average.

### RING ROUTINES: FILTER PLACEMENT

[*Ringfilter*](#ringfilter) and [*ringtvfilter*](#ringtvfilter) use frequency response functions to filter the reverb. Two filtering modes are available in which either the source input to the feedback is filtered, or the feedback. When the response is used to filter the source input, it filters the signal before it enters the feedback mechanism, imposing its characteristic, from the start, on the feedback. However, when positioned to filter the feedback component, the appearance of the response's spectral characteristic, in the reverb, appears gradually as the signal decays. In this mode, the time it takes the signal to decay into the response characteristic is controlled by an additional decay time associated with the filter.

### COMPRESSION AND EXPANSION

Spectral compression and expansion play a role in many routines. Its implementation here is according to the traditional model that uses thresholds and magnitudes of compression/expansion to reduce or enlarge the dynamic range of a signal. With spectral compression, amplitudes that exceed the specified compression threshold are reduced by an amount determined by the decibels of compression (a multiplier of the bin's amplitude lying above the threshold). Expansion works in a similar fashion, except that it changes the amplitudes below, rather than above, the expansion threshold; this results in an expansion of the dynamic range as the bins falling below the threshold are made to cover a wider range.

The term companding or compander is a merging of the two names, useful in situations where they are both available. While *compander* is the most obvious example of a routine using companding, traditional compression can be found in several other routines that involve filtering. It is not uncommon, in those routines, to reduce the dynamic range of an analyzed frequency response, particularly if it is time-varying, since the goal in filtering is more about color than dynamic range.

In all routines that use some form of companding, the dynamic range of the unprocessed signal/response is assumed to lie between 0 and -96 dB; thresholds are chosen from within this range. The degree of compression or expansion, expressed in decibels, represents how much the signal lying beyond the threshold will be reduced. A value of -6 dB would halve the dynamic range above the threshold in compression, or double the range below the threshold in expansion.

*Compander* applies compression for each frequency bin separately rather than as a macro gain change. It does this by using a frequency response file, created with *freqresponse*, to establish a unique, 0 dB point of reference for every bin; using its unique point of reference, every bin is compressed or expanded.

---

## UTILITIES

### FILE CONVERSION

#### aiffs, aiffd, nexts, nextd, nextfloats

The sound file conversion scripts: aiffs, aiffd, nexts, nextd, and nextfloats are shell scripts available for converting sound files back and forth between aiff and next formats, or from next to floats. They are all effectively SGI scripts since they use the SGI sound file format conversion utility, *sfconvert*. *Aiffs* and *aiffd* take next integer files and write new aiff files, *nexts* and *nextd* the opposite; in addition *aiffs* and *aiffd* can be used to write new aiff integer files converted from next float files. *Nextfloats* writes a new float file from a next integer file. The *s* or *d* following the *aiff* or *next* in the name stands for the action taken on the original file once the new file is made; the *s* saves the original file (i.e. does not delete it), the *d* causes it to be deleted. Multiple files may be converted with the same run of the command. Running the command without any input files will produce a description of the routine.

### FUNCTION VIEWING

#### showme, showspect

Two graphing scripts are available for viewing functions and spectral data. You must have gnuplot installed on your computer to use them (Type gnuplot <CR> to see if you do). *Showme* is a simple script for viewing function files. Run without an input file for a description. *Showme* takes headerless floating-point or ASCII (give -a flag) function files and plots them. Showspect plots the file of FFT amplitude or frequency data produced by the plainpv script, `S.plainpv_with_printout_and_graph_files`. *Showspect* is useful for seeing a graphic representation of a very particular part of an analysis, it is not a substitute for a standard spectrogram application.

---

## USING THE SHELL SCRIPTS

### SAMPLE SCRIPT: S.plainpv

Below is a copy of the complete script for running *plainpv* which you can examine here to understand the basic structure of this shell script mechanism. In it you find a top section for variables and a bottom section for execution of the *plainpv* command. Set the variables of the top section with the appropriate files and constants; do not use spaces. To run the script, simply type it as a command as in the following: `S.plainpv`.

(See [Running Commands with Shell Scripts](#running-the-commands-with-shell-scripts))

When the shell runs the script, it copies the value of the assigned variables into the respective flag positions and runs the command as if you had typed it at the prompt in a shell window. Each shell script is set up to print the command to the terminal just before running it. A sample of the output follows the script below.

### SHELL SCRIPT OUTPUT SOUND FILE CONVERSION

The conversion routine, [*aiffd*](#file-conversion), has been added at the end of every script that writes an output sound file: `aiffd $output_file;`

To prevent it from unexpectedly converting the output of your sound file, it has been commented out with the # sign. If you would like your next integer or floating-point output files to be automatically converted to aiff sound file format, simply remove the # sign. (See [file conversion](#file-conversion).)

### GEN FUNCTION CONTROL OF PARAMETERS

Any parameter whose flag on the routine's [information page](#information-page) has the word (func) after it can be controlled by a [function file](#controlling-parameters-with-functions). An easy way of doing this is to generate the file when the script is run. To do this, simply place the call to the gen function anywhere in the top section of the script; the script will then run the command and make the file needed by the shell variable. By the time the PVC routine is run in the bottom section, the file will be ready.

The best place to locate your gen function calls is immediately following the variable assignment, as in the example below which sets the variable controlling the pitch transposition.

```shell
pitch_transposition_in_semitones=/tmp/ptrans
gen4 -L1000 0 -3 0 1 3 > /tmp/ptrans ;
```

In this example, the variable, *pitch_transposition_in_semitones*, is set with the file name, */tmp/ptrans*, which has in it the 1000 values output by the gen4 command.

While the output of the gen4 command is in 32-bit floating point values, PVC allows them to be ASCII as well. So, it would be possible to replace the segment of text above with the following.

```shell
pitch_transposition_in_semitones=/tmp/ptrans
echo 0 1 2 2.5 5 10 > /tmp/ptrans ;
```

Here the gen4 call has been replaced with a simple call to *echo* which places the ASCII values, *0 1 2 2.5 5 10*, into the file */tmp/ptrans*. When PVC looks at the file, it figures out that it is text rather floats.

While the values in both cases are linearly interpolated by PVC, to create the continuous function needed by the routine, the gen4 case would be smoother since it has more values.

Lines in shells can be continued onto new lines with the backslash, which comes in handy with gen functions. The above, for example, could be entered as:

```shell
gen4 -L1000 \
\
0 -3 0 \
\
1 3 \
\
> /tmp/ptrans ;
```

which would simplify our parsing of it.

### Sample S.plainpv Script

```sh
#!/bin/sh
#******************************************************
#.................... PLAINPV .........................
#******************************************************
#******************** OUTPUT **************************
output_file=/S1/cm.mix.snd
#......................................................
output_data_format=1
#( 0: Same as input file )
#( 1: integers )
#( 2: rescaled floats )
#................ RESCALE .............................
rescale_level_in_decibels=1
#(-96 to 0 dB. Set to 1 to rescale to peak of input file.)
#******************** INPUT ***************************
input_file=/S1/t.snd
#........ BEGIN/END TIMES .............................
begintime=0
endtime=0
# (End time of 0 or less defaults to end of file.)
#======================================================
#*** ANALYSIS PARAMETERS ******************************
FFT_length=1024
window_type=2
windowsize=0
# (0 sets windowsize to 2 * FFT (or larger))
frames_per_second=200
#======================================================
#*** RESYNTHESIS PARAMETERS ***************************
#........... OUTPUT CHANNEL(S) .......................
output_channel=0
# (channels are numbered from 1 to the maximum.)
# (0 = all channels)
#.............OSCIL THRESHOLD ........................
oscillator_resynthesis_threshold_in_dB=-96
#( Try -60 to -70 unless dropouts become audible. )
#****************** MODIFICATIONS *********************
#.................. TIME ..............................
time_expansion_contraction_factor=1
# (Adjust frames_per_second in proportion to keep a
# constant rate.)
#.................. DECIBELS ..........................
gain_in_decibels=0
#.................. PITCH .............................
frequency_shift_in_Hz=-0
pitch_transposition_in_semitones=0
#............ AMPLITUDE RESPONSE ......................
release_time_in_seconds=0
attack_time_in_seconds=0
#............ SPECTRUM WARPSHAPE ......................
spectrum_warpshape_index=0
#............ BRICKWALL FILTER ........................
FILTER_TYPE=0
#( 0 = bandpass )
#( 1 = bandreject )
#......................................................
BRICKWALL_FILTER_window_low_frequency=-1
BRICKWALL_FILTER_window_high_frequency=-1
# (-1 selects respective lowest or highest frequency)
#======================================================
#*************** LOW/HIGH SHELF EQ *********************
LOW_SHELF_EQ_gain_in_decibels=0
LOW_SHELF_EQ_frequency=200
HIGH_SHELF_EQ_gain_in_decibels=0
HIGH_SHELF_EQ_frequency=2000
#********** AMPLITUDE STATISTICS **********************
print_amplitude_statistics_0_no__1_yes=1
amplitude_statistics_time_interval=.25
#======================================================
#========= SCRATCH SPACE ==============================
#======================================================
#====================================================
# COMMAND LINE SETUP -- OFFICE USE ONLY
# (DO NOT WRITE BELOW THIS LINE)
#====================================================
pvroutine=plainpv
PVFLAGS="\
\
-N$FFT_length \
-M$windowsize \
-w$window_type \
-D$frames_per_second \
-I$time_expansion_contraction_factor \
\
-a$frequency_shift_in_Hz \
-P$pitch_transposition_in_semitones \
-A$gain_in_decibels \
\
-C$output_channel \
-t$oscillator_resynthesis_threshold_in_dB \
\
-b$begintime \
-e$endtime \
\
-H$LOW_SHELF_EQ_gain_in_decibels \
-m$LOW_SHELF_EQ_frequency \
\
-X$HIGH_SHELF_EQ_gain_in_decibels \
-R$HIGH_SHELF_EQ_frequency \
\
-L$release_time_in_seconds \
-l$attack_time_in_seconds \
\
-W$spectrum_warpshape_index \
\
-T$FILTER_TYPE \
-f$BRICKWALL_FILTER_window_low_frequency \
-F$BRICKWALL_FILTER_window_high_frequency \
\
-_$output_data_format \
-=$rescale_level_in_decibels \
\
-p$print_amplitude_statistics_0_no__1_yes \
-i$amplitude_statistics_time_interval \
"

echo "\n\n$pvroutine $PVFLAGS $input_file $output_file "

$pvroutine $PVFLAGS $input_file $output_file
```
---

### SAMPLE OF OUTPUT FROM S.PLAINPV

Below is a sample of the output from *S.plainpv*.

```shell
plainpv -N1024 -M0 -w2 -D400 -I2 -a-0 -P2 -A0 -C0 -t-96 -b0 -e0 -H0 -m200
-X0 -R2000 -L0 -l0 -W0 -T0 -f-1 -F-1 -_1 -=1 -p1 -i.25  /S1/t.snd /S1/cm.mix.snd

/////////////////////////////////////////////////////////////////////
---------------------------------------------------------------------

============================== PLAINPV ==============================


---------------------------------------------------------------------

========================== INPUT SOUNDFILE ==========================


INPUT FILE: FILENAME  = /S1/t.snd
INPUT FILE: SAMPLE RATE = 44100
INPUT FILE: NUMBER OF CHANNELS = 2
INPUT FILE: DURATION = 2.770386
INPUT FILE: BEGIN TIME = 0.000000
INPUT FILE: END TIME = 2.770386
INPUT FILE FORMAT: 16-BIT INTEGER

========================== OUTPUT SOUNDFILE =========================


OUTPUT FILE: FILENAME  = /S1/cm.mix.snd
OUTPUT FILE: SAMPLE RATE = 44100
OUTPUT FILE: NUMBER OF CHANNELS = 2
OUTPUT FILE FORMAT: 16-BIT INTEGER
OUTPUT FILE: DURATION = 5.540771

======================== ANALYSIS PARAMETERS ========================


FFT SIZE = 1024
*
FUNDAMENTAL ANALYSIS FREQUENCY = 43.066406
*
WINDOW SIZE = 2048
FRAMES/SECOND = 400
DECIMATION SAMPLES (samples between analysis frames) = 110

======================= RESYNTHESIS PARAMETERS ======================


TIME EXPANSION/CONTRACTION FACTOR = 2
*
INTERPOLATION SAMPLES (samples between resynthesis frames) = 220
*
OSCILLATOR RESYNTHESIS THRESHOLD (in dB) = -96.000000
*
GAIN (in dB) =    0.000
PITCH TRANSPOSITION (in semitones) =    2.000
FREQUENCY SHIFT (in Hz) =    0.000
*
ENVELOPE ATTACK TIME (in seconds) =    0.000
ENVELOPE RELEASE TIME (in seconds) =    0.000
*
SPECTRUM WARPSHAPE INDEX =    0.000
*
FREQUENCY WINDOW: LOW BOUNDARY = 0.000000
FREQUENCY WINDOW: HIGH BOUNDARY = 22050.000000
*
*............. LOW/HIGH SHELF EQ............*
LOW SHELF FREQUENCY =  200.000
.......... LOW SHELF DECIBELS =    0.000
HIGH SHELF FREQUENCY = 2000.000
.......... HIGH SHELF DECIBELS =    0.000
*...........................................*
*
=====================================================================
ANALYSIS: CHANNEL = 1
..............USING BLACKMAN WINDOW
.....USING OSCILLATOR BANK RESYNTHESIS

*********************************************************************
**  PEAK AMPLITUDE STATISTICS **
*********************************************************************
     TIME          PEAKAMP      DECIBELS    (LAST DECIBELS PEAK)
*********************************************************************
(  0.00 -  0.25)    0.0005       -66.295     -66.295
(  0.25 -  0.50)    0.2052       -13.754     -13.754
(  0.50 -  0.75)    0.3285        -9.668      -9.668
(  0.75 -  1.00)    0.3066       -10.269
(  1.00 -  1.25)    0.3176        -9.962
(  1.25 -  1.50)    0.2731       -11.275
(  1.50 -  1.75)    0.2655       -11.518
(  1.75 -  2.00)    0.2416       -12.337
(  2.00 -  2.25)    0.2930       -10.661
(  2.25 -  2.50)    0.2915       -10.707
(  2.50 -  2.75)    0.3067       -10.267
(  2.75 -  3.00)    0.4094        -7.757      -7.757
(  3.00 -  3.25)    0.3076       -10.241
(  3.25 -  3.50)    0.2841       -10.930
(  3.50 -  3.75)    0.2843       -10.924
(  3.75 -  4.00)    0.3241        -9.786
(  4.00 -  4.25)    0.3340        -9.524
(  4.25 -  4.50)    0.3612        -8.845
(  4.50 -  4.75)    0.3113       -10.136
(  4.75 -  5.00)    0.3094       -10.189
(  5.00 -  5.25)    0.3141       -10.058
(  5.25 -  5.50)    0.1142       -18.846

============= PEAK AMPLITUDE ========================================
CHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)
.....................................................................
1            2.898           0.4094      -7.757
*********************************************************************


=====================================================================
ANALYSIS: CHANNEL = 2
..............USING BLACKMAN WINDOW
*********************************************************************
**  PEAK AMPLITUDE STATISTICS **
*********************************************************************
     TIME          PEAKAMP      DECIBELS    (LAST DECIBELS PEAK)
*********************************************************************
(  0.00 -  0.25)    0.0004       -67.948     -67.948
(  0.25 -  0.50)    0.2301       -12.763     -12.763
(  0.50 -  0.75)    0.2477       -12.122     -12.122
(  0.75 -  1.00)    0.1969       -14.115
(  1.00 -  1.25)    0.2631       -11.599     -11.599
(  1.25 -  1.50)    0.2086       -13.613
(  1.50 -  1.75)    0.2559       -11.840
(  1.75 -  2.00)    0.2671       -11.465     -11.465
(  2.00 -  2.25)    0.2768       -11.157     -11.157
(  2.25 -  2.50)    0.1762       -15.082
(  2.50 -  2.75)    0.2113       -13.502
(  2.75 -  3.00)    0.2549       -11.872
(  3.00 -  3.25)    0.2673       -11.460
(  3.25 -  3.50)    0.2869       -10.847     -10.847
(  3.50 -  3.75)    0.2841       -10.931
(  3.75 -  4.00)    0.1991       -14.019
(  4.00 -  4.25)    0.2131       -13.427
(  4.25 -  4.50)    0.2540       -11.904
(  4.50 -  4.75)    0.2235       -13.014
(  4.75 -  5.00)    0.2407       -12.369
(  5.00 -  5.25)    0.2941       -10.629     -10.629
(  5.25 -  5.50)    0.1166       -18.667

============= PEAK AMPLITUDE ========================================
CHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)
.....................................................................
2            5.103           0.2941     -10.629
*********************************************************************


=====================================================================

                 PEAK AMPLITUDES: ALL CHANNELS
---------------------------------------------------------------------
CHANNEL       TIME          PEAKAMP    DECIBELS    (CLIPPED SAMPLES)
.....................................................................
1            2.898           0.4094      -7.757
2            5.103           0.2941     -10.629
=====================================================================


PLAINPV: RESYNTHESIS COMPLETED
```
