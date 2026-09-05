# *PVCplus*

{#INDEX}**INDEX**

- [**INTRODUCTION**](#INRODUCTION)

- [**INSTALLATION**](#INSTALLATION)

> [***UNPACKING***](#INSTALLATION_UNPACK)\
> [***SETTING THE DESTINATION DIRECTORY***](#INSTALLATION_DIRECTORY)\
> [***COMPILING THE BINARIES***](#INSTALLATION_COMPILE)

- [**UNIX COMMAND**](#UNIXCOMMAND-LINEFORMAT)

> [***INFORMATION PAGE***](#INFORMATION_PAGE)\
> ***SETTING FLAG VALUES***
> ***CONTROLLING PARAMETERS WITH
> FUNCTIONS***
> ***RUNNING COMMANDS WITH SHELL
> SCRIPTS***

- **[ROUTINES: SHORT DESCRIPTIONS](#ROUTINES:SHORTDESCRIPTIONS)**\
 [***BASIC ROUTINES***](#BASIC_ROUTINES)\
 [***PLAINPV***](#PLAINPV_DESCRIPTION)\
 [***TWARP***](#TWARP_DESCRIPTION)

> [***AMPLITUDE WARPING***](#AMPLITUDE_WARPING)\
> [***NOISEFILTER***](#NOISEFILTER_DESCRIPTION)\
> [***COMPANDER***](#COMPANDER_DESCRIPTION)\
> ***[SPECTWARPER](#SPECTWARPER)
> [SPECTRALEXTRACTOR](#SPECTRALEXTRACTOR)***
>
> ***ADDITIVE SYNTHESIS***
> [***HARMONIZER***](#HARMONIZER_DESCRIPTION)\
> [***INHARMONATOR***](#INHARMONATOR)\
> [***CHORDMAPPERPLUS***](#CHORDMAPPERPLUS_DESCRIPTION)
>
> [***SUBTRACTIVE SYNTHESIS***](#SUBTRACTIVE_SYNTHESIS)\
> ***[FREQRESPONSE](#FREQRESPONSE_DESCRIPTION)\
> [FILTER](#FILTER_DESCRIPTION)***\
> [***CHORDRESPONSEMAKER***](#CHORDRESPONSEMAKER_DESCRIPTION)\
> [***FILTRESPONSEMAKER***](#FILTRESPONSEMAKER)\
> [***PVANALYSIS***](#PVANALYSIS_DESCRIPTION)\
> [***TVFILTER***](#TVFILTER_DESCRIPTION)\
> ***[CONVOLVER](#CONVOLVE_DESCRIPTION)\
> [IMPULSERESPONSE](#IMPULSERESPONSE)\
> [IRCONVOLVER](#IRCONVOLVER)
> [CHANNELCOLLECT](#CHANNELCOLLECT)
> [CHANNELMIX](#CHANNELMIX)
> [MIXFILES](#MIXFILES)***
>
> ***[SYNTHESIS BY RECONFIGURATION](#SYNTHESISBYRECONFIGURATION)\
> [FORMANTSMAPPER](#FORMANTSMAPPER)\
> [RESONANCE/REVERB](#RESONANCE_REVERB)***\
> [***RING***](#RING_DESCRIPTION)\
> ***RINGFILTER***
> [***RINGTVFILTER***](#RINGTVFILTER_DESCRIPTION)
>
> [***NONLINEAR FREQUENCY DEVIATION***](#NONLINEAR_FREQUENCY_DEVIATION)\
> [***FILTDEVIATOR***](#FILTDEVIATOR)\
> [***TVFILTDEVIATOR***](#TVFILTDEVIATOR)
>
> [***FEATURE EXTRACTION***](#FEATURE_EXTRACTION)\
> [***ENVELOPE***](#ENVELOPE_DESCRIPTION)\
> [***CENTROID***](#CENTROID)\
> [***FLUXOID***](#FLUXOID)\
> ***[PITCHTRACKER](#PITCHTRACKER)
> [PEAKFORMANT](#PEAKFORMANT)***
>
> [***CONTROL FUNCTION PROCESSING***](#CONTROL_FUNCTION_PROCESSING)\
> ***[RESHAPE](#RESHAPE_DESCRIPTION)\
> \*
> [UTILITIES](#UTILITIES)
> [*READHEADER*](#READHEADER_DESCRIPTION)**\
>***[SHOWME](#SHOWME)\
> [CMUSIC GEN FUNCTION ROUTINES](#CMUSIC_GEN_FUNCTIONS)\***

- [**TERMS AND COMMON FEATURES**](#TERMSANDCOMMONFEATURES)

> ***OVERLAP/ADD METHOD VS. OSCILLATOR BANK METHOD AND RESYNTHESIS
> THRESHOLDS***\
> [***SOURCE***](#SOURCE)\
> ***[MULTIPLE CHANNELS](#MULTIPLE_CHANNELS)\
> [AUTOMATIC PLAYBACK OF OUTPUT](#AUTOMATICPLAYBACKOFOUTPUT)*\
> [*INPUT SOUND FILE*](#INPUT_SOUND_FILE)**
> ***OUTPUT SOUND FILE***
> ***AMPLITUDE
> RESCALING***\
> [***OUTPUT STATISTICS***](#STATS)\
> ***FREQIUENCY RESPONSE TERMINAL
> OUTPUT***\
> [***ANALYSIS FILES***](#ANALYSIS_FILES)\
> [***DECIBELS***](#DECIBELS)\
> ***LOW/HI SHELF EQUALIZATION***
> ***WARP INDEX***
> [***PITCH TRANSPOSITION***](#PITCH_TRANSPOSITION)\
> [***FREQUENCY SHIFT***](#FREQUENCY_SHIFT)\
> [***ENVELOPE RESPONSE TIME***](#RESPONSE_TIME)\
> [***RING DECAY TIME***](#RING_DECAY_TIME)\
> [***FFT SIZE***](#FFT_SIZE)\
> [***WINDOW SIZE***](#WINDOW_SIZE)\
> [***WINDOW TYPE***](#WINDOW_TYPE)\
> [***FRAMES PER SECOND***](#FRAMES_PER_SECOND)\
> [***TIME EXPANSION/CONTRACTION***](#TIME_EXPANSION)\
> [***BEGIN/END TIMES***](#BEGIN_END_TIMES)\
> [***GAIN***](#GAIN)\
> [***FILTERING: SOURCE SIGNAL LEVEL***](#filter_source_dB_floor)\
> ***TRANSPOSITION/SHIFT APPLICATION
> FLAG***\
> ***[FILTER TYPES: PASS OR REJECT](#filter_pass_or_reject)\
> [BRICK WALL FILTER](#BRICK_WALL_FILTER)***\
> [***RESPONSE FUNCTION SMOOTHING***](#Response_Function_Smoothing)\
> ***[SOUND ANALYSIS DATA: ACCESS MODES](#Data_Access)\
> [TIME BOUNDARIES](#Time_Boundaries)\
> [AUTO-STOP MODE](#AUTO_STOP_MODE)\
> [SAMPLER LOOP MODE](#SAMPLER_LOOP_MODE)\
> [LOOP TIME WINDOW](#LOOP_TIME_WINDOW)\
> [ONSET AND RELEASE SEGMENT MODE](#ONSET_AND_RELEASE_SEGMENT_MODE)\
> [LOOP AMPLITUDE NORMALIZATION](#LOOP_AMPLITUDE_NORMALIZATION)\
> [LOOP SMOOTH TIME](#LOOP_SMOOTH_TIME)\
> [TIME POINT DITHERING](#TIME_POINT_DITHERING)\
> RANDOM AMPLITUDE
> VARIATION
> RANDOM FREQUENCY VARIATION\
> [RANDOM VARIATION ROLLOFF](#RANDOM_VARIATION_ROLLOFF)\
> FRAME NORMALIZATION DECIBEL
> LIMIT\
> \***
> [***CONVOLVER PANPOT***](#Convolver_Panpot)\
> ***FREQUENCY RESPONSE ACCUMULATION
> METHOD***\
> [***RING ROUTINES: FILTER PLACEMENT***](#Filter_placement)\
> ***[COMPRESSION AND EXPANSION](#Compression_Expansion)\
> [SAMPLER LOOP WINDOW](#SAMPLER_LOOP_WINDOW)***
>
> [**USING SHELL SCRIPTS**](#USING_SHELL_SCRIPTS)\
> ***FUNCTION CONTROL OF
> PARAMETERS***
>
> {#INRODUCTION}
> **INTRODUCTION**

PVCplus, or simply PVC, is a collection of phase vocoder signal processing routines and accompanying shell scripts for use in the transformation and manipulation of sounds. The routines are written in C and designed to run in a UNIX (i.e. LINUX) shell environment using libsndfile software. Even though processing speed in some routines may exceed real time, PVC is not designed to produce real time output, but rather, sound files, which can be subsequently auditioned through the integrated libsndfile sound file play feature or through the sound file player of your choice.

PVC has come about as a result of my path of education and research into phase vocoder technology. It follows in the spirit of the work by Eric Lyon (out of which PVC is built) and Chris Penrose whose particular digital signal processing research springs from the coding and tutorial work of F. Richard Moore and Mark Dolson. Moore's book *The* *Elements of Computer Music*, published by Prentice Hall, is consequently a great resource for making sense of the phase vocoder engine, which is not explained in this text. Curtis Road's book *The Computer Music Tutorial,* published by MIT Press, has sections on the phase vocoder as well (as do many others); these texts can introduce the beginner to the mechanics and control of this technology. Short of the explanations these sources provide, I have attempted to offer various explanations below, particularly as needed for control of the parameters in these routines.

The routines in PVC reflect my need and desire for tools capable of performing different types of spectral processing, from the simple to the experimental. Work on PVC began in about 1993 and underwent continuous development until about 2002. Faced with other research interests, development fell silent for about a decade until 2011 when new goals in non-real-time synthesis and a desire to incorporate *libsndfile* into PVC's audio I/O caused me to pick it up with renewed interest; so as to distinguish the new developments and I/O from past incarnations, the *PVCplus* name was adopted in 2011, along with an open source release through Sourceforge. (See <http://sourceforge.net/projects/pvcplus/> for download and most recent release.)

The development and refinement of PVC and then PVCplus has advanced with my growing skills and curiosity. Most of these routines can be viewed in terms of traditional additive or subtractive synthesis tasks, coming about as they did from my desire for greater finesse and control of these two basic types of synthesis. While the speculative nature of some of the routines give them an idiosyncratic character, most should, with practice, reveal the transparency of their names and the role they can play in the shaping of sound. All require a good ear tuned towards sound and idea, as none provide automatic results at first pass, although many hold great potential for the diligent. Paul Koonce\ <koonce@ufl.edu>

[RETURN TO INDEX](http://sourceforge.net/projects/pvcplus/)

**INSTALLATION**

The *PVCplus* package contains the *PVC* routines, along with the *CMUSIC* *GEN* functions written by F.R. Moore (each in a separate directory). Both are collections of UNIX routines that are run from within a UNIX terminal window. Moore's standalone GEN functions are useful tools for creating function files for the time-varying control of parameters. GEN functions include: *cspline, gen0, gen1, gen2, gen3, gen4, gen5, gen6, and genraw*. A one-line summary can be obtained by running each command without any arguments. A detailed explanation of each can be found in the appendix to Moore's *Elements of Computer Music*.

You can compile and install both the *PVC* and *GEN* function routines separately or together, following unpacking and setting of the destination directory. To do so, follow the steps given below. As suggested, a minimal knowledge of the UNIX operating system and its related utilities is required.

**1) Unpack:**

First move the *PVCplus* compressed tarball (i.e. the *PVCplus* download file with the post-fix of tar.gz) to the directory of your choice. Unzip it with *gunzip* by typing the following terminal command:

*gunzip PVCplus.tar.gz* (or whatever is the current distribution file.)

Then unarchive it with the *tar* program by entering:

*tar xvf PVCplus.tar*

This will produce a *PVCplus* master directory in which you will find several other directories.

**2) Set the Destination Directory:**

The makefile is set to install all routines in the system files directory called */usr/local/bin*, which is standard for most UNIX environments. You may change the destination directory by simply editing the "DESTDIR = /usr/local/bin" line in *Makefile,* which can be found in the topmost directory of *PVCplus*, to the directory of your choice.

(Note: Your UNIX path must have */usr/local/bin* in it in order for the terminal window shell to find the installed routines. Type: *printenv* in order to see the setting of your \$PATH variable search directories. If */usr/local/bin* is not included in your search path, then modify your *.cshrc* file to add the directory or change the destination directory to a directory that is included in your \$PATH search path.)

**3) Compilation:**

To compile *PVCplus*, you will first need to install the audio I/O utilities provided by *libsndfile.* Download *libsndfile* and install it.

If compilation and installation of *libsndfile* is successful, then proceed to the installation of *PVCplus* by entering

*make clean*

and then

*make PVC*

from the topmost directory of *PVCplus*.

(Should there be any, the *make clean* command removes older object files, forcing the creation of new, system-specific binaries and *libsndfile* linkages.)

The *CMUSIC GEN* functions may also be compiled by entering:

*make GEN*

Both *PVCplus* and the *GEN* functions may be compiled in one step by entering:

*make all*

If no error messages are encountered, then install the *PVC* and/or *CMUSIC GEN* function binaries and shell script utilities using

*make install*

If your destination directory is */usr/local/bin*, or any other system-owned directory, you will need to perform the installation using the *sudo* command (i.e. *superuser do),* which will require an administrator password to complete.

*sudo make install*

The *make install* command moves the compiled routines, previously written to the *bin* directory of the topmost directory, to the destination directory along with the sound file utilities located in the utilities directory. If installation is successful, then you should be able to type any of the routine names and see their flag information page.

You will need to get the routine names into the search-path tables, which you can do by either entering

*source \~/.cshrc*

or, alternately, opening a new terminal shell window.

If everything is working correctly, you should be able to type the name of any routine and see the *Flag information page* for the routine. For example, try typing:

*plainpv*

RETURN TO INDEX

**UNIX COMMAND-LINE FORMAT**{#UNIXCOMMAND-LINEFORMAT}

While PVC routines are typically run using UNIX shell scripts, edited in a text editor (see below), the routines themselves are actually UNIX command line routines that take one of the following forms.

*routine \[flags\] input_soundfile output_soundfile*

*routine \[flags\] input_soundfile*

*routine \[flags\] output_soundfile*

*routine \[flags\]*

**Routine:** The routine is the named routine, such as *plainpv* or *twarp.*

**Flags:** Flags always follow the routine name and consist of the initial *dash* followed by a *character,* which identifes the parameter, and a *value* (e.g. *--P2* ).

Many parameters can be controlled using a series of values stored in a function file; the function file values allow the parameter to be continuously changed over the synthesis duration. To specify a function file, simply replace the parameter *value* with a pre-made function file (e.g. instead of *-P2*, specify *-P\~/myFunctionFile*, which will link the parameter and its dynamic control to the sequence of file values to be distributed and linearly interpolated across the routine's synthesis time).

In most cases, parameters are initialized to the default values listed in brackets on the {#INFORMATION_PAGE}information page. Function file control is allowed if the word *(func)* appears in the flag explanation on the information page, as seen below.

**Sound Files:** The *input_soundfile* and *output_soundfile* specify the sound file to be processed and the location of its output, respectively. While most routines produce sound file output, some produce only data, while others have no input sound file.

With the integration of *libsndfile,* PVCplus supports all libsndfile-supported sound file formats, both input and output*.* The format of the output file is taken from the named output sound file, which must be a preexisting sound file written in a format known to *libsndfile*. All internal processing is done in floating-point numbers.

RETURN TO INDEX

**INFORMATION PAGE**

Parameter and flag information can be found by entering the name of the routine alone. For example, entering *plainpv* produces the following information page.

plainpv: basic phase vocoder\
plainpv \[flags\] \[input file\] \[output file\]\
Most formats accepted. Output format copied from input file.\
(Values in brackets denote defaults.)\
N: FFT length (must be a power of 2) \[1024\]\
M: window size in samples (must be a power of 2) \[2\*FFT\]\
(0 will automatically set window to 2\*FFT size or larger)\
w: window type: 0 = hamming, 1 = rectangular\
2 = Blackman, 3 = Bartlett triangular \[0.\]\
4-12 = Kaiser windows for alpha = 4-12, respectively\
(representative sidelobe levels for alpha:\
4 = -30dB, 8 = -58 dB, 12 = -90 dB)\
13 = Blackman-Harris, 14 = Nuttal, 15 = Blackman-Nuttal\
16 = Flat top\
D: analysis frames per second \[200\]\
I: time expansion/contraction factor \[1.\]\
(duration = duration \* factor, 1. = original time)\
P: pitch transposition in semitones (func) \[0\]\
a: frequency shift factor\
(bin frequency adder, before pitch transposition )(func) \[0.\]\
b: begin time in seconds \[0.\]\
e: end time in seconds ( 0. = end of file) \[0.\]\
C: resynthesis channel (1 -\> ?) (0 = all) \[0\]\
SHELF EQ:(post transpose/shift)\
H: shelf EQ: Low shelf gain in dB (func) \[0.\]\
X: shelf EQ: High shelf gain in dB (func) \[0.\]\
m: shelf EQ: Low shelf frequency in Hz (func) \[200.\]\
R: shelf EQ: High shelf frequency in Hz (func) \[2000.\]\
W: warp index for reshaping magnitude response (func) \[0.\]\
Values \> 0 expand the dynamic range,\
values \< 0 compress the dynamic range.\
A: gain in decibels (func) \[0.\]\
l: amplitude attack time (func) \[0.\]\
L: amplitude release time (func) \[0.\]\
T: brickwall filter type: 0 = bandpass, not 0 = band reject \[0\]\
f: frequency window: low boundary\
(before -P and -a) (in Hz) \[0.\]\
F: frequency window: high boundary\
(before -P and -a)(in Hz) \[Nyquist frequency\]\
t: oscillator re-synthesis threshold in decibels \[ -96 \]\
p: amplitude reports print mode: 0 = off, 1 = on \[0\]\
i: time interval between amplitude reports \[.25\]\
\_: auto output sound file play:\
0 = off\
-1 = interactive: Prompt for each play.\
-2 = interactive: Play once, then prompt for more.\
1 or greater = Auto-repeat for specified repetitions. \[0\]\
=: peak rescale level 0 to -96 dB\
1 = Rescale to level of input file.\
2 = Bypass rescaling.\
3 = Rescale only if peak exceeds 0dB. \[ 1 \]\
TERMINAL DISPLAY AND GRAPH FILE OUTPUT\
n: number of frames \[0\]\
u: low bin frequency \[-1\]\
U: high bin frequency\
(-1 = nyquist) \[Nyquist frequency\]\
S: TERMINAL DISPLAY: display option \[0\]\
(0 = off, 1 = phase data, 2 = amp data, 3 = both)\
c: GRAPH FILE: WRITE ascii to FILE\
0 = off, 1 = freq, 2 = decibels \[0\]\
3 = decibels - waterfall plot\
(When on, this flag writes ascii point pairs\
(with time frame on x axis) for plotting\
with gnuplot.)

RETURN TO INDEX

**SETTING FLAG VALUES**

If no output file is specifed, the name *pvc.out* will be used, with *pvc.out* placed in the local directory. The bracketed value at the end of each parameter represents its default setting; defaults can be changed by adding a flag, consisting of a minus sign, parameter letter, and value; spaces separate flag settings and file names, and must not appear elsewhere. For example, the following changes the FFT size to 2048 and the pitch transposition to two semitones higher.

*plainpv -N2048 --P2 \~/inputfile \~/outputfile*

The tilde sign (\~) may be used in all file designations to indicate the user's home directory.

Some flags require files rather than constants. For these, simply supply the file name, as in

*twarp --F\~/analysis_file*

RETURN TO INDEX

**CONTROLLING PARAMETERS WITH FUNCTIONS:**

Parameters for which the word (func) appears just before the default value on the information page can be controlled with a *function file*, as seen in the following *plainpv* flag for pitch transposition.

*P: pitch transposition in semitones (func) \[0\]*

The function file must be a header-less series of values indicating how the parameter should evolve over the synthesis time. The file should contain only ordinate values, which will be distributed equidistantly over the time of synthesis and parameter control.

Function file values may be either 32-bit floating-point values or ASCII numbers, arranged one-per-line---PVC deciphers the format of the values by detecting the presence of ASCII, which if missing, causes a default to binary. The function file may have any number of values, as the series is fitted and linearly interpolated to produce the necessary intervening values. Function files in 32-bit floating-point form can be created with the CMUSIC gen routines provided with this package. As well, the PVC routine *reshape* can be used to shape and filter function files.

RETURN TO INDEX

**RUNNING THE COMMANDS WITH SHELL SCRIPTS:**

PVC routines are UNIX commands designed to be run in a shell or terminal window; however, they are best handled using the shell scripts found in the SCRIPTS directory. These scripts can be used for saving and managing parameter data; in many ways they are a poor-man's GUI. All scripts contain a top section, in which variables are set, and a bottom section where those variables are placed into a UNIX command line string and then run. Beginners should be aware that a UNIX shell script functions similar to a terminal window, which is itself a UNIX shell running interactively. A file, *myFileOfCommands* for example, containing a series of commands, such as the following

*ls --l\
pwd\
date*

could be run using the *sh* command; in this example, a temporary shell is created so as to run the commands listed in the file.\ Therefore, entering

[*sh myFileOfCommands*](#SHELL_SCRIPTS_SECTION)

would produce a long format listing of files in the current directory, the name of the working directory, and the date.

PVC shell scripts work in this way---as lists of commands; their one difference is that they are preset to function as executable files; that is, by adding *!/bin/sh* at the beginning of each script and by changing their *file mode* with *chmod,* they can be run without the *sh* command. The executable status of any file can be examined with the *ls --l* command, which is a place to start should a shell script simply not run.

The shell structure of PVC scripts therefore allows users to include other commands and processes that are related to the files being used in the script. For example, a call to a CMUSIC GEN function for a needed set of values could be inserted into the top section of a script, conveniently below the variable its values will control; placed there, the call makes the file of values while setting variables, all just before the routine is run.

Some scripts perform two routines such as a short analysis routine followed by the main synthesis routine; other scripts run just one routine. The shell script variables that set the parameter flags of the routines are set in the top section, and are, at times, shared in scripts performing two routines. Take note that shell script variable assignments do not allow for spaces. Therefore, the following variable setting:

[*pitch_transposition_in_semitones = 3*](#SHELL_SCRIPTS_SECTION)

[would result in an error. It should be](#SHELL_SCRIPTS_SECTION)

[*pitch_transposition_in_semitones=3*](#SHELL_SCRIPTS_SECTION)

The numerous parameters, numbering as high as 53 in some routines, make these scripts a necessity. They will be your friend if you take care to leave the bottom part alone, and don't corrupt your variable names. Someday I will make a better way to interface with the routines; for now this is the way it is.

To run the scripts, simply type the name of the file (or appropriate pathname when running from outside the directory in which the script resides). For example:

[*S.plainpv*](#SHELL_SCRIPTS_SECTION)

If things are working correctly, a listing of the resulting command, the function files being used, the setting of parameters, and the working results of the routine should follow, printed to the terminal window.

(See the explanation below about using shell scripts.)

RETURN TO INDEX

**ROUTINES: SHORT DESCRIPTIONS**{#ROUTINES:SHORTDESCRIPTIONS}

Below are short descriptions of the routines contained in this release.{#BASIC_ROUTINES}

**PLAINPV**

*Plainpv* is a basic phase vocoder routine offering control of pitch transposition, frequency shift, time expansion and contraction, [amplitude warping](#WARP_INDEX), [amplitude response time smoothing](#RESPONSE_TIME), brickwall bandpass filtering, and [low/high shelf equalization](#SHELF_EQ).

Run *plainpv* with *S.plainpv*. See *S.plainpv tutorial* for more information.

RETURN TO INDEX

**TWARP**

*Twarp,* which stands for time warping, functions much like *plainpv,* except that it works from an input analysis file, created with *pvanalysis* rather than an input sound file. The use of an analysis file allows the time of the input sound to be addressed freely (see [sound analysis: data access](#Data_Access)). Processing can begin anywhere in the time of the analyzed sound, proceeding forwards or backwards in time at a constant or variable rate, determined by numerical constant or function file---functions can also be used to address, not rate, but position in time.

The use of an analysis file also allows for the specification of [low and high time boundaries; time boundaries can be used to stop synthesis, using the [auto-stop mode](#AUTO_STOP_MODE), or to control a [loop time window](#SAMPLER_LOOP_WINDOW) as a part of a sample loop process. Sample loops can be wrapped, folded, or clipped, and can be modified through [loop point smoothing](#LOOP_SMOOTH_TIME) and loop amplitude normalization; onset and release segments, time point dithering, and [amplitude](#RANDOM_AMPLITUDE_VARIATION) and [frequency](#RANDOM_FREQUENCY_VARIATION) randomization can be added as well.](#Data_Access)

Like *[plainpv](#PLAINPV_DESCRIPTION), twarp* allows for control of [pitch transposition](#PITCH_TRANSPOSITION), frequency shift, time expansion and contraction, [amplitude warping](#AMPLITUDE_WARPING), [amplitude response time smoothing](#RESPONSE_TIME), and low/high shelf equalization.

Use [*pvanalysis*](#PVANALYSIS_DESCRIPTION) through the script *S.pvanalysis* to make the analysis file; then run *twarp* with *S.twarp*.\ See [*S.twarp tutorial*](#TWARP_TUTORIAL) for more information.

RETURN TO INDEX

**NOISEFILTER**

*Noisefilter* filters out the noise in a sound by subtracting out a predetermined, average or peak frequency response, constructed from a section of the file's sound presumed to be associated with its background noise. A threshold mode is available for sounds whose background noise cannot be isolated. Gurgling caused by the audible fluctuation of amplitude gating around each bin's threshold can be effectively eliminated using amplitude response time smoothing.

[Run *noisefilter* with *S.noisefilter*.](#RESPONSE_TIME)

Like *plainpv, noisefilter* allows for control of pitch transposition, frequency shift, time expansion and contraction, [amplitude warping](#AMPLITUDE_WARPING), [amplitude response time smoothing](#RESPONSE_TIME), and low/high shelf equalization.

[Run *noisefilter* with *S.noisefilter.*](#SHELF_EQ)

RETURN TO INDEX

***AMPLITUDE WARPING***

**COMPANDER**

*Compander* functions as a classic compressor/expander. Unique to *compander* is the use of a *frequency response* to represent the peak bin amplitudes of the sound. The peaks of the frequency response serve as unity gain (0 dB) reference levels against which bin amplitudes are compressed or expanded. Compression and expansion may be used simultaneously, assuming compression thresholds lie above expansion thresholds.\ The entire process (including the preparatory analysis*)* can be run using the script *S.compander*.

Like *plainpv, compander* allows for control of pitch transposition, frequency shift, time expansion and contraction, [amplitude warping](#AMPLITUDE_WARPING), [amplitude response time smoothing](#RESPONSE_TIME), and low/high shelf equalization.

RETURN TO INDEX

**SPECTWARPER**

*Spectwarper* uses an expanded *compander* scheme to highlight either a sound's stronger, resonant pitched components or its weaker residual noise. *Spectwarper* is fairly similiar to *compander*; however, unlike [*compander*](#COMPANDER_DESCRIPTION), which processes bins against constant analyzed peaks, averaged across a segment of the input sound, *spectwarper* bins compress and expand against changing peaks, identified through an ongoing, narrow frequency band analysis of each bin frequency region.

When focus is placed on the elimination of strong amplitudes, through severe compression, the result is the accentuation of weaker residual or noise components. When focus is placed on the elimination of weak amplitudes, through expansion, resonant pitch components of the sound are highlighted.

The *complement spectrum proportion* allows the focus on pitch or noise to be inverted, i.e. a value of 1 inverts the output from pitch to noise, or noise to pitch, depending on how the thresholds and compression/expansion levels are set; a value of 0 produces no inversion. Controlling the *complement spectrum proportion* with a 0-1 function file allows users to address gradual shifts between pitch and noise.

The use of functions to control the *complement spectrum proportion* can produce wildly different amplitudes, as processing shifts from focus on stronger pitch components to weaker noise elements, or vice versa; to offset the disparity between pitch and noise amplitude levels, the ongoing output frame can be dynamically normalized against its input frame using the *frame normalization decibel limit,* which determines the degree to which normalization is applied.

With well-chosen sounds, rich in both pitch and noise, *Spectwarper* can be a surprising routine, as it offers a simple and powerful control over the noise and pitch characteristics of a sound. Run this routine with *S.spectwarper*.

**SPECTRALEXTRACTOR**
*Spectralextractor* uses the frequency instability to discriminate between pitch and noise. The results produced are very similar to *spectwarper*, as frequency instability and low amplitude are both correlates of noise. Overall, the output from *spectralextractor* is less consistent or stable than *spectwarper*.\ With *spectralextractor,* the frequency of each bin is measured in Hz for its ongoing speed of variance; greater variance is associated with noise, less variance with pitch. The *frequency change threshold* determines when a bin's variance is presumed to correlate with pitch or noise; when the variance speed falls below the threshold, the bin is identified with the more pitched components, if it falls above, with the noise.

Two low pass filters are provided for smoothing of the analysis and the output. The first, (incorrectly) named the *threshold accumulator response time,* slows the rate at which each bin frequency is allowed to change, which reduces transient perturbations and analysis artifacts from the process. The second low pass filter, named the *amplitude change response time,* slows the rate at which each bin's frequency variance signal is allowed to cross the threshold, which helps to prevent gurgle noise.\ A switch parameter is used to set the routine's output to noise or pitch; like *spectwarper,* the *complement spectrum proportion* allows the output to be inverted from one to the other, or dynamically changed using a function; the amplitude disparities incurred through the use of a function can be lessened using the *frame normalization decibel limit.*\ While *spectralextractor* processes differently than *spectwarper,* its results are very similar. I prefer *spectwarper,* which has less grit to it, although both routines are interesting. *Spectralextractor* is newer---a work-in-progress---and needs more refinement, as its parameters of control are not yet designed to change in response to different FFT sizes, which are possible, but which will change the output---so, beware.

RETURN TO INDEX

***ADDITIVE SYNTHESIS---*HARMONIZER, INHARMONATOR, AND CHORDMAPPERPLUS:**

*Harmonizer, inharmonator, and chordmapperplus* all allow for the additive re-synthesis of the input sound or data, based on the remapping of analysis data according to some model. Each requires an ASCII data file, which specifies how groups of analysis bins, selected according to different criteria, will be replicated or mapped. This mapping is constant for the run of the routine.

**HARMONIZER**

*Harmonizer* works like commercial harmonizers, which allow users to sing harmony with themselves by transposing their input voice, in real time, to other pitch levels. *Harmonizer* extends the concept, albeit in a non-real-time application, to allow for multiple harmonization levels, constructed through the transposition, gain, and delay of user-selected frequency bands. An ASCII data file is used to specify the upper and lower frequency bounds, center frequency, stop band or edge dB, gain, Q, and delay time of each band. Transposition of bands can be achieved via frequency multiplication or addition, or by pitch transposition in semitones. Several macro controls are available for the pre-synthesis scaling or shifting of the ASCII input data; macro synthesis controls allow the amplitude, frequency, and time delay of bins to be globally interpolated between their original and modified states.

*Harmonizer* is a useful routine for building up unusual accumulated derivations of an input sound, which are suggestive of its spatial or resonant reverberation.

Run this routine using the script *S.harmonizer*.

**INHARMONATOR**

Like *harmonizer, inharmonator* allows for the remapping of frequency bands; however, with *inharmonator,* frequency bands are restricted to the partials of a user-specified fundamental frequency. As such, the routine is principally designed for use with constant-frequency tones with harmonically organized partials. That being the case, *inharmonator's* approach to additive re-synthesis, through the accumulation of harmonically related frequency bands, can be adapted to function as a harmonically structured, band pass filtering scheme. Used this way, the specified frequency bands come to serve as de facto band pass filters that impose harmonic order and resonance onto an inharmonic or noise-like input sound.\ With *inharmonator,* the bins identified with re-mapped partials are called the *target bins,* in distinction to the *non-target bins,* which comprise all remaining, unmapped analysis bins. Separate macro gain, transposition, and frequency shift controls, controlling the two, allow users to create a changing output mix of *targets* and *non-targets*. *Non-targets* have added macro control of their delay and feedback decay time. The *input source* may, as well, be added to the mix and manipulated through separate gain, transposition, and frequency shift controls.

An ASCII data file is used to specify the *target bin* mappings. All partials up to the Nyquist frequency may be remapped; partials may not be re-used, as there is a one-to-one connection between input bins and partials. Each line of partial mapping data contains: the partial number, amount or point of frequency shift (in accord with the *shift data format*), decibel gain, delay time, and decay time. The *shift data format* determines the nature of the shift specified, which may be a frequency multiplier or a mapping point, specified as a frequency, pitch (octave.pitchclass), or partial of the original fundamental. In addition, each partial band may be delayed in time and fed back as a decaying echo, repeating at the rate of the band's delay time. Several macro controls are available for the pre-synthesis scaling or shifting of the ASCII input data; macro synthesis controls allow the amplitude, frequency, and time delay of bins to be globally interpolated between their original and modified states.*\ Inharmonator* is related to *chordmapperplus,* in which the basic idea of partial mapping and residual energy control is incorporated into a highly developed, powerful tone re-synthesis routine. Users interested in the control and extension of expressive instrumental tones should, at some point, explore *chordmapperplus---*probably following lessons learned from *inharmonator.*

**CHORDMAPPERPLUS**

*Chordmapperplus* allows users to expressively manipulate the vibrato of an input tone; synthesis can focus on the production of single tones or creatively orchestrated chord mixtures. *Chordmapperplus* works much like a commercial harmonizer in which an input signal is replicated and transposed to produce chords; in *chordmapperplus,* however, the input is assumed to be a single, harmonically organized tone, which, once analyzed with *pvanalysis* and identified through user data, can be re-mapped to produce chords or tones.\ *ASCII Data File: Chordmapperplus* employs a highly developed synthesis scheme that allows for the creative extension of the inherent noise, vibrato, and general expressivity of an instrumental tone. Synthesis is structured through an ASCII data file, which specifies how the harmonic partials and noise of the input tone will be reproduced to create one or more tones. File data is organized into *tone lines* of 22 parameters, the first of which indicates the frequency of the tone's fundamental or *tone source point,* followed by various parameters that address the selection, transposition, and control of the tone's partials and noise. As a unique feature of *chordmapperplus,* many parameters can be controlled using function files that, like command-line flag data, are simply listed in place of the constant. (See scripts or tutorials for details regarding which parameters allow functions.)\ *Pitch and Noise Banks:* To allow the independent manipulation of pitch and noise components, *chordmapperplus* first uses information from the ASCII data file to determine how the bins of the analysis will be grouped into pitch and noise banks; the pitch bank consists of all bins used by any tone line in the synthesis of partials; the noise bank consists of all remaining analysis bins, the assumption being that any bin not used by a tone line must be associated with the residual or background noise features of the sound. (Consequently, any synthesis containing both pitch and noise should be designed so that all known pitch components are identified in at least one of the specified tone lines; the components do not need to be audible---only identified.) In the synthesis of each tone line, a separate copy of the *noise bank* is made and controlled relative to the line's specifications and controls. *Partial Groups:* Each tone line specifies a group of equidistantly spaced harmonic partials, identified through the *low partial*, the *partial spacing* interval, and the desired *number of partials*; for example, four partials, spaced at two partial intervals, beginning with the third partial would reproduce partials 3, 5, 7, and 9. A value of 0 for the number of partials will include all partials up to the Nyquist frequency, beginning with the low partial, spaced by the spacing rule. A *partial pass band bandwidth* parameter determines the frequency bandwidth for each partial, which determines the bins that will be included in the reproduction of each partial.\ *Spectral Shift, Stretch, and Compression.* A tone line's group of partials may be frequency *shifted* up or down or spectrally *stretched/compressed*. *Partial shift* is specified in partial units; for example, a shift of 1 would cause partials 1-10 to be shifted to where partials 2-11 would be; negative shifts are allowed as well. The *stretch/compress* factor changes the frequency of each partial through the following equation.\ partial frequency = partial frequency + ( ( partial_number - 1) \* fundamental frequency \* stretch/compress factor)\ Positive values stretch the spectrum, negative values compress it, and a value of zero causes no change. The stretch/compress factor is best viewed as a frequency deviation, specified in partial units, that accumulates, be it negative or positive, as one progresses upward through the numbered partials---i.e. the spectrum is stretched or compressed relative to the fundamental, which stays in place. For example, a factor of .01 would cause partials 1 through 5 to be located at the inharmonic positions of 1.0, 2.01, 3.02, 4.03, and 5.04; a factor of -.01 would shift them to 1.0, 1.99, 2.98, 3.97, and 4.96.\ *Transposition and Gain:* Each tone line includes parameters for the transposition and gain of the tone's identified partials and accompanying noise bank.

*\
Expressive Re-synthesis:* For input tones recorded using expressive vibrato (or which are, in general, spectrally varied), *chordmapperplus* includes an analysis and re-synthesis scheme offering users unique control over a tone's expressive signature. Re-synthesizing a tone's inherent expressive characteristics requires control of two principal components: *vibrato periods* and *vibrato depth*. *Chordmapperplus* offers independent control of both, which, when coordinated, can produce remarkably expressive tones.

*Resynthesis by Detected Vibrato Periods:* While *chordmapperplus* allows synthesis according to standard PVC time boundary and rate controls (see *twarp*), with vibrato-rich input tones, movement through and around the tone's time can be structured according to detected vibrato periods. In vibrato period mode, *chordmapperplus* applies an analysis routine to identify useable vibrato periods (as determined by the user-specified selection criteria) and then randomly selects different sequences of them during synthesis. When synthesizing vibrato periods, time becomes measured in terms of, not seconds, but vibrato periods per second, with each vibrato period time-scaled so as to produce a thoroughly regular stream of vibrato periods. Users may synthesize vibrato periods in a mechanically regular manner or utilize the period randomization feature, which introduces more of the variability of a live performer.

*Vibrato Depth Control through Force:* With expressive vibrato already encoded into the original input tone, synthesis control of vibrato or vibrato depth becomes a matter not of adding vibrato, but of suppressing it. Vibrato suppression is achieved through the *force* parameter, which controls the degree to which the original tone characteristics (and data) are adopted relative to a static spectrum designed to represent or simulate the input tone without vibrato. As the force parameter is ramped up from 0 to 1, increasing amounts of the original tone's frequency and amplitude deviation are introduced; at a force of 1, the original tone is essentially reproduced. With function control, the force parameter becomes a kind of expression parameter, allowing for the progressive introduction and removal of the tone's original vibrato. *\*

*Tone and Stasis Medians: Chordmapperplus* uses analytically constructed *stasis medians,* or static spectral data, for both pitch (tone) and noise, in order to create a static, baseline sound ideal against which the force parameter (see above) can be used to reintroduce vibrato and spectral animation. *Chordmapperplus* performs a preliminary analysis that creates, not one, but a large collection of static spectral frames; specifically, the analysis collects together frames of similar dynamic level and then averages their amplitudes and harmonically tunes their partial bin data in order to create a continuum of electronic-sounding, static spectra, seemingly correlated with the timbre of the input tone across its normalized dynamic range. The continuum of static spectra is accessed through the tone and noise *stasis medians,* indexed by their respective decibel levels, which extend from the normalized peak of 0dB down to -80dB.\ Following preparation, the static spectra can be accessed and reproduced through the respective *tone and noise stasis medians,* which can be set at a constant dynamic level or addressed with functions across the -80 to 0 dB index range. For example, with the *force* parameter set to 0 (see above), a function progressing from -80 to 0 and back to -80 would produce a vibrato-less, pure tone representing what the instrument might produce while attempting a vibrato-less, dynamic swell. Stasis medians exist for both *tone* and *noise* and can be addressed separately so as to control the balance and interplay of pitch and noise.\ *Synthetic Vibrato:* For tones having no recorded vibrato, a synthetic vibrato is available.

*Rate Correlated Modifiers: Chordmapperplus* offers several modifiers for mitigating artifacts or for enhancing the effect of a reduction in the playback rate.

*Rate-correlated Force Suppression:* The role of the *force* parameter is to progressively remove suppression of the input tone's vibrato, thus releasing the recorded vibrato encoded in the analysis data. While this approach is effective at playback rates at or above the tone's original rate, a reduction of rate, or elimination of temporal movement altogether, will result in a sound that is filled with classic time expansion, phase vocoder artifacts. To mitigate this effect, *chordmapperplus* offers *rate-correlated force suppression*. When invoked, this feature causes rates below 1.0 to increasingly suppress the force parameter as the rate slows. A rate of 0 will cause all force to be neutralized, with synthesis following the static spectra entirely. (The stasis median level selected, in this case, is one that allows the dynamic level to remain constant despite force suppression.) The effect of this is that any slowing of rate below 1 (or the vibrato rate equivalent when working with vibrato periods) causes the sound to go from expressive animation to a static spectra of similar dynamic level, as the speed of the vibrato slows and finally stops.

*Rate-Correlated Noise and Tone Level Controls:* The principal purpose of the *rate-correlated noise and tone level controls* is to allow automatic adjustment of the noise and pitch dynamic levels by linkage to vibrato speed, which is indirectly controlled through playback rate. The assumption with these two controls is that, with acoustic instruments and any slowing of vibrato, a sound will change in both its overall level and balance of pitch and noise. These two controls, alone or together, can be used to simulate this change.\ *Rate-Correlated Randomization of Noise Bank:* With the slowing of the playback rate, the synthesis of noise bins becomes less and less true to its original random behavior; the *rate-correlated randomization switch* causes playback rates of less than 1 to progressively introduce increased amounts of random amplitude and frequency modulation into noise bank synthesis. Like the tuning of partial bins with the stasis median, the randomization of noise bins at slow-to-zero playback rates prevents the artifacts that would otherwise result from extreme time expansion. *\ Tone Line Band Pass Filter:* Each tone line includes a band pass filter option, with center frequency and bandwidth controls, that can be used to process the tone, noise, tone and noise, or neither (bypass) using a set of filter bus toggles.

*Tone Line Delay Time and Control Functions Scalerswitch:* Each tone line can be set to have a separate time delay using the delay time. With the introduction of delay, the question arises as to whether functions used in the tone line should be time delayed as well. The *control functions delay time scalerswitch* specifies how the delay time for the line's functions should be managed. A scalerswitch value is both switch and delay time scaler, as it is multiplied against the delay time to produce the amount time delay used in functions; a value of 1 would not change the delay time for functions; a value of 0 would omit the delay; and values in between would scale the delay by the value.

RETURN TO INDEX

***SUBTRACTIVE SYNTHESIS***

**FILTER**

*Filter* allows for the subtractive spectral manipulation of a sound using a single-frame frequency response, created through either synthesis or analysis. Synthetic responses can be designed with collections of tones using *chordresponsemaker* or with linear frequency segments using [*filtresponsemaker*](#FILTRESPONSEMAKER). Analyzed responses can be made with [*freqresponse*](#FREQRESPONSE_DESCRIPTION), which finds the average or peak spectrum of a segment of sound. Filtering is achieved through multiplication of the filter response against the time-varying amplitudes of the input sound. *Filter* allows time-varying control of the response shape (warp), transposition/shift, compression and expansion, smoothing, and mix of the filter output and input source, making this a very useful tool for quickly manipulating the spectral characteristics of a sound according to your synthetic or analytic goals.\ The synthetic forms are run with the scripts *S.filter_with_chord_synthesis* or *S.filter_with_breakpoint_synthesis;* the analysis-based form with *S.filter_with_analysis*. The analytic form is a useful tool for bringing the color of one sound into the realm of another.

RETURN TO INDEX

**FREQRESPONSE**

*Freqresponse* performs spectrum and formant analysis of a user-specified sound file segment and writes spectrum, formant, and plot data files for subsequent viewing or use by other routines or scripts involved with filtering, formant mapping, or plotting.
{#CHORDRESPONSEMAKER_DESCRIPTION} *Freqresponse* offers a
number of controls for refining the identification of formants, which is always a proposition given the problem of thresholds and definitions.

To run *freqreponse* separately, use *S.freqresponse.*

**CHORDRESPONSEMAKER**

*Chordresponsemaker* is a routine that uses a collection of harmonic tones, variable in number and characteristics, to create a synthetic frequency response. It is found in several scripts.

**FILTRESPONSEMAKER**

*Filtresponsemaker* is a routine that uses breakpoints and straight lines to create a synthetic frequency response. It is found in several filtering scripts.

**PVANALYSIS**

*Pvanalysis* is the time varying form of *freqresponse*. It is used to create a phase vocoder analysis for use by other routines. Run this routine using the script *S.pvanalysis*.

[**TVFILTER**](#FREQRESPONSE_DESCRIPTION)

*Tvfilter* is the time-varying (tv) form of *filter*. *Tvfilter* uses a [*pvanalysis*](#PVANALYSIS_DESCRIPTION) file to change the magnitudes of the input sound file. Like *[filter](#FILTER_DESCRIPTION), [tvfilter](#TVFILTER_DESCRIPTION)* multiplies the amplitudes of the pvanalysis data against the amplitudes of the input sound analysis while preserving the frequency/phase characteristics of the input sound. Multiplying amplitudes against each other while preserving the phase of the input sound results in a cross-synthesis that covers or suppresses the input with the shadow of the analysis file. Like *[filter](#FILTER_DESCRIPTION), [tvfilter](#TVFILTER_DESCRIPTION)* offers a variety of controls for manipulating the characteristics of the changing filter, including manipulation of the filter data time location, thus allowing, like *twarp,* highly flexible movement, forwards and backwards, through the time of the filter data.

[Run this routine using the script *S.tvfilter*.](#TVFILTER_DESCRIPTION)

RETURN TO INDEX

**CONVOLVER**

The general setup and control of *convolver* is similar to *tvfilter*; processing, however, is handled differently. In *[tvfilter](#TVFILTER_DESCRIPTION),* the amplitudes of the two analyses (i.e magnitudes in polar form), are multiplied, with the phases (or frequencies) left intact. In *convolver,* the Cartesian form is used with both orthogonal magnitude components of the analysis multiplied together; in this form, frequency and amplitude are not isolated parameters, but are, rather, coded together into the orthogonal components of the analysis. The effect of the Cartesian multiply is to create a true cross-filtering or spectral intersection, which allows only frequency components that are common to both sounds to be heard. The effect is a sound that, in comparison to *tvfilter,* is somewhat garbled, as it outputs the more intermittently common spectral components of the two.\ The form of the multiplication in *convolver* does not allow for some of the filter transposition controls associated with [*tvfilter*](#TVFILTER_DESCRIPTION). There is however a convolution panpot that offers control of the mix between the convolution and source sounds.

It is very important to point out that *convolver* uses the *short-term Fourier transform,* as do virtually every other routine in PVCplus, save one (i.e. *irconvolver*). Because of this, the output of *convolver* is very different from classic, true convolution of one signal against another. For the latter, PVCplus users are strongly advised to explore *irconvolver,* which employs the *fast convolution* algorithm; fast convolution allows for the use of very long sound files (without dynamic modification), be it input sound or impulse file, which *convolver* is not designed to accommodate. In short, for those interested in *classic convolution* schemes with no dynamic control of relevant parameters, please see *irconvolver,* which is powerful in its own way.

[Run this routine using the script *S.convolver*.](#Convolver_Panpot)

RETURN TO INDEX

**IMPULSERESPONSE**
Impulseresponse*creates a spectral analysis of an impulse response (or input sound), which it then stores in a file for subsequent use by *irconvolver* in fast convolution processing*.*The input sound may have an unlimited number of channels, which are analyzed according to the user's specifications. ***** Impulseresponse* is typically run with *S.irconvolver,* which uses the data produced to run *irconvolver.\ \* With the above being the typical case, the script *S.impulseresponse* can also be run separately so as to explore the various channel analysis plots written by *impulseresponse.* Separate plot files are written for each channel of a multi-channel input, with each appended with the respective channel number. View the plot files with *showme.\ **\ IRCONVOLVER** Irconvolver* performs classic convolution using the fast convolve method following spectral analysis of impulse responses with *impulseresponse.* Multi-channel processing is available, allowing convolution of input sound channels with either a single channel of *impulseresponse* analysis data or with multiple analysis channels using a loop approach to the assignment of analysis channels (e.g. given three *impulse response* analysis channels, A, B, and C, and five *irconvolver* input sound channels, 1, 2, 3, 4, and 5, analysis channels would be assigned as follows: A-1, B-2, C-3, A-4, B-5).

Multi-channel processing allows *irconvolver* to be used in exploration of multi-channel reverberation and space using recorded or synthesized multi-channel impulse responses.\ In addition to space and color, *irconvolver* may be used to generate compositional arrangements of an input sound. In this approach, a sound is distributed in time by convolving it with an algorithmically generated pulse file, in which the location of pulses corresponds to the desired placements of the sound. In this approach, the user first designs an impulse sound file with pulses filtered, gain-scaled, and placed in time so as to represent the desired distribution of the sound in time. Once the pulse sound file is constructed, it is then convolved with an *impulseresponse* analysis using *irconvolver,* which effectively places a copy of the input sound at each pulse location, filtering and gain-scaling the sound by the particular pulse's gain and filter characteristics.

Part of the power of PVCplus is its ability to address an unlimited number of sound channels. With *irconvolver* and the pulse file approach discussed above, the channel organization of sound files can be used to structure and vary the assignment of *impulseresponse* analysis channels with distributing pulse sounds. Conceptually, this is very similar to MIDI sequencing in which MIDI instruments or sounds are assigned to channels of MIDI performance data, the *impulsresponse* analysis channels corresponding to MIDI instruments and the pulse sound file channels to the channels of MIDI performance data. However, the advantage of convolution over MIDI is that, first, the quantity of instruments or copies of a sound in any one channel are unlimited, as spectral processing is blind to concepts of composed and perceived sound quantity, and second, reverberation and echo can be seamlessly connected to the compositional arrangement of sound.

One last detail: sound file inputs to *impulseresponse* and *irconvolver* are effectively interchangeable, as the process of convolution makes no distinction between either; with convolution, the two are equal. Consequently, the only criteria worth considering in the process of assigning sound files as input to either *impulseresponse* or to *irconvolver* is available memory, since *irconvolver* is limited by the memory available to it for loading *analyses*. Large or long impulse response analyses could potentially create problems as they call for large amounts of memory, although with contemporary memory standards, problems rarely arise. ******
{#CHANNELCOLLECT}**CHANNELCOLLECT**
Channelcollect*collects sounds together into a single output file by interleaving input sound channels into a multi-channel file or by concatenating them into a monophonic file. Input sound channels come from the listed input files, which may have multiple channels; multi-channel files are effectively treated as a series of monophonic sound files listed in their channel order. *Channelcollect* is designed to allow easy construction and variation of multi-channel files for use with *impulseresponse* and *irconvolver.* Parameter controls allow for the indexing and ordering of channels, as well for their normalization, truncation, and modification by amplitude envelopes.\
*Channelcollect* is typically run directly through the command line structure. ******
{#CHANNELMIX}**CHANNELMIX**
Channelmix* performs quick mixes of multi-channel files to mono or stereo for quick audition. *Channelmix* is designed for use with *irconvolver* in multi-channel convolution schemes.\ *Channelmix* is typically run directly through the command line structure. ***\***

**MIXFILES**
Mixfiles* offers command line mixing of sound files. Parameters are available for controlling delay time, normalization, and amplitude gain.

*Mixfiles* is typically run directly through the command line structure. ***\***

***\
RESONANCE/REVERBERATION:***

**RING**

*Ring* uses the phase vocoder to create an all-pass resonator for every analysis bin. It works by structuring the FFT re-synthesis of analysis data as a bank of feedback filters that feed back the sinusoid of each bin in strengths proportional to the amplitude of the bins (after adjustment by global feedback controls). This allows the sound to \"ring\" in a way that is something like reverb or comb filter resonance. The difference from comb filtering is that with *ring,* spectral resonance is created not through a collection of comb filters selected for their ability to resonate various pulse wave spectra, which include all partials of the specified fundamentals, but rather, through an array of feedback filters (corresponding to the partials of the FFT fundamental) that resonate a sine wave spectrum while dynamically tuning their feedback frequencies to the frequencies of the input sound. In short, it creates a kind of \"self resonance\".\ Ring is a nice way of increasing the resonant pitch characteristics of a sound, although it has its weaknesses. Ring works best with larger FFT sizes as it is attempting to synthesize or accentuate the more pitched/harmonic characteristics of the sound; this is something that larger FFTs, with their increased frequency resolution, handle better. Use of the Kaiser window or more sideband-discriminating windows, which have lower side-lobe amplitudes, helps as well. In addition, there is a threshold that prevents (or acentuates) resonance of noise features, plus an EQ that can be positioned to filter either the source input to the feedback loop, or the feedback return.

Run this routine using the script *S.ring*.

**RINGFILTER**

*Ringfilter* marries [*filter*](#FILTER_DESCRIPTION) with [*ring*](#RING_DESCRIPTION) by allowing a frequency response to be imposed on the resonance created with *ring*. *Ringfilter* begins to look more like multiple-delay, comb filter resonance since the static frequency response selects which frequencies will feed back. What is unique here is that the frequency response can come from a number of sources, be they from synthesis, via *chordresponsemaker* or *filtresponsemaker*, of analysis, via *freqresponse*. Like the EQ in [*ring*](#RING_DESCRIPTION), the filter in *ringfilter* can be positioned to either filter the source input to the feedback loop, or the feedback return where it will have the effect of introducing the filter characteristic more slowly through the resulting variable rates of decay.\ Run *ringfilter* with *S.ringfilter_with_chord_synthesis* to create a synthetic frequency repsonse, and with *S.ringfilter_with_analysis* for an analyzed frequency response.

[**RINGTVFILTER**](#RING_DESCRIPTION)

*Ringtvfilter* is to [*ringfilter*](#RINGFILTER_DESCRIPTION) what [*tvfilter*](#TVFILTER_DESCRIPTION) is to [*filter*](#FILTER_DESCRIPTION); that is, it makes the filter in *ringfilter* time-varying. This is a sophisticated idea, i.e. time-varying how the resonance of a time-varying sound is filtered. The best characterization would be to say that *Ringtvfilter* imprints the shadow of one sound onto the reverb of another.\ *Ringtvfilter* requires some thought in order to separate and articulate the evolutions of the source, resonance, and filter. The best results are created using dynamic, high-profiled source sounds, rich with transient noise, and more constant, pitch/harmonic sounds for the time-varying filter. Like [*tvfilter*](#TVFILTER_DESCRIPTION), *ringtvfilter* requires an analysis file.\ Run this routine using *S.ringtvfilter*.

RETURN TO INDEX

**GROUPDELAYMAKER**
*Groupdelaymaker* is used to prepare filter and time delay data for *delayfilter.* The routine is organized similar to *chordresponsemaker,* with the important addition of a delay time for each tone. As well, amplitude and delay time can be set for remaining bins, i.e. all bins not included in any tone line. **\ DELAYFILTER** *Delayfilter* uses a set of FFT structured decibel and delay times, pre-made with *groupdelaymaker,* to control the amplitude and delay time of bins. The process is essentially designed to work like *filter,* controlled with a *chordresponsemaker* frequency response, with the addition of delay times to each tone-organized component. Hence, *delayfilter* allows users to not only adjust the amplitude of harmonically related collections of bins, but to place them at specific points in time as well.\ *\ Delayfilter* is best used with relatively noisy sounds, as the harmonic grouping and placing in time has the effect of displacing the sound's energy into different tones occurring at different points in time. Cymbals, for example can produce interesting results.\ Note: The delay of specific spectral components is a feature now available in both *harmonizer* and *inharmonator,* which provide different ways of isolating spectral components.\ Run *delayfilter* with *S.delayfilter_with_groupdelaymaker*

***NONLINEAR FREQUENCY DEVIATION:***

**FILTDEVIATOR**

The idea behind *filtdeviator* is to use a frequency response function to not only change the topology of a sound's amplitudes (i.e. to filter the sound as done with *filter*), but to also change the frequency topology, in correlation with the filter and its spectral shape---i.e. frequencies and amplitudes are changed in tandem using the same frequency response structure. *Filtdeviator* is essentially *filter* with parameters added for specifying how the filter frequency response function will be mapped into the *deviation* of frequency.\ Added parameters include base and peak deviation boundaries that determine how the filter response will be mapped into frequency deviation. As with the typical PVCplus control of frequency, the deviation can be applied as either pitch transposition or frequency shift. The function can also be warped within the range set by the limits so as to change its overall distribution.\ A master (0-1) control allows global control of all deviation. All *filtdeviator* controls allow dynamic variation of the presence and effect of amplitude filtering and frequency deviation, making *filtdeviator* an interesting routine for exploring the way filters can be used to impede/transform the resonant signature of a sound. Using small amounts of frequency deviation, with no amplitude filtering and a sweeping transposition of the filter, will produce an effect something akin to commercial guitar phase shifters; larger amounts of deviation take it into another place entirely. With the addition of correlated amplitude filtering, the frequency deviation is more concealed (as it is positioned more at the edges of formants), sounding something like a slide whistle with its changing resonances.\ The scripts to run *filtdeviator* \-- *S.filtdeviator_with\_ chord_synthesis* and *S.filtdeviator_with_analysis \--* are designed with frequency response synthesis/analysis sections like those for *filter* and *ringfilter*. Run this routine using either *S.filtdeviator_with_analysis* or *S.filtdeviator_with_chord_synthesis*.

[**TVFILTDEVIATOR**](#FILTER_DESCRIPTION)

*Tvfiltdeviator* is to [*filtdeviator*](#FILTDEVIATOR) what [*tvfilter*](#TVFILTER_DESCRIPTION) is to [*filter*](#FILTER_DESCRIPTION); i.e. it uses a time-varying filter response in place of the constant one. This routine blows the lid off of what is unusual about *tvfiltdeviator*. It's great for making wacky sounds out of ones with nice, fixed harmonies. The best use is to use *tvfiltdeviator* to deviate itself, i.e. use the trajectory of bin amplitudes to simultaneously control how their frequencies will deviate. The best use of this routine is with sounds that have a pronounced amplitude envelope, one identified with the action of making sound, such as harpsichord, guitar, or pitched percussion.

First perform an analysis of the sound with *pvanalysis*. Then use the analysis to deviate the same sound, which will connect the amplitude of bins to their frequency deviation, producing a kind of \"sproing\" sound whenever a bin has any amplitude. Makes tonal music sound really broken.\ Run this routine with *tvfiltdeviator.*

RETURN TO INDEX

***FEATURE EXTRACTION:***

**ENVELOPE**

*Envelope* tracks the amplitude envelope of a sound and writes it to a file; output can be in ASCII or binary. Data units can be in amplitude or decibels and inverted within the respective range, if desired.\ Run this routine with *S.envelope*.

**CENTROID**

*Centroid* tracks the centroid of a sound, which is the average of all frequencies, each weighted by its amplitude. The analysis can be restricted to a band of frequencies, allowing the centroid to track a particular frequency component (although *pitchtracker* can do this as well).\ Run this routine with *S.centroid*.

**FLUXOID**

*Fluxoid* is a routine for tracking the average frequency change of a sound, with the frequencies weighted by their amplitudes (best) or not.\ Run this routine with *S.fluxoid*.

**PITCHTRACKER**

*Pitchtracker* is a routine for tracking the fundamental pitch trajectory of a monophonic sound. It is an experimental routine that works well in a number of situations, but forever has its quirks, as does much pitch tracking. Three detection methods are available. Different output units allow for the creation of different function files for different applications.\ Run this routine with *S.pitchtracker*.\
{#PEAKFORMANT}**PEAKFORMANT**
*Peakformant* tracks the peak formant of an input sound and writes the trajectory to a headerless output file for use in analysis-driven synthesis control schemes. The frequency range can be limited and the output smoothed before translation into a number of different output formats.\ Run this routine with *S.peakformant.*

RETURN TO INDEX\
***\
SYNTHESIS BY RECONFIGURATION\***

**FORMANTSMAPPER\**

*Formantsmapper* allows users to map the formants of a *source* sound to the nearest formants of a *target* sound. Unlike filters, which block or pass spectral energy depending on their design, formant mapping preserves the ongoing spectral energy of a source sound as it transforms it into a design reflective of the target sound.\ *Formantsmapper* uses *freqresponse* to provide the required formant data needed to connect source formants to target formants. Spectral mapping is achieved by adjusting the frequency and amplitude of each *source* formant to match the value of its assigned *target* formant. The degree of amplitude and frequency change is controlled by macro interpolation parameters, which, when synchronized, control the seeming transformation of the source into the target.

In the mechanics of structuring how formants will be mapped, source and target formants will often vary in their number as well as position; because of this, *formantsmapper* duplicates source formants when it is useful to do so, thus allowing source formants to not only converge into adjacent target formants, but, in an attempt to fully represent the target, to also split and diverge into adjoining ones as well.\ Depending on user settings, formants that are close in frequency may vary dramatically in amplitude. This can create problems for source formants whose low amplitude, suggestive of noise, subjects them to extreme amplification, as they are matched to their target formant. To prevent this, a limit can be placed on the amount of amplitude increase allowed in any source formant mapping.\ The number of source and target formants may be reduced through the use of frequency boundaries and a minimum amplitude requirement for formants. As well, the number of source and target formants may be extended, following any reduction in number, through the synthetic addition to the remaining formants of either harmonic partials or upper octaves.\ Two banks of synthesis, identified as banks A and B, are available for synthesis; users may elect to use both banks or just bank A. Interpolation parameters, valued from 0 to 1, are available for both banks to control the degree of frequency and amplitude modification.\ Interpolation paths may follow a common linear trajectory or can be randomly diffused within a range to follow a variety of accelerating or decelerating mapping trajectories.

Run this routine with *S.formantsmapper,* which includes two preliminary *freqresponse* scripts that prepare the formant analysis data for the two sounds. ****
{#CONTROL_FUNCTION_PROCESSING}***CONTROL FUNCTION
PROCESSING:***

**RESHAPE**

*Reshape* is a routine for transforming control function files to meet the needs of different parameters. The routine expects header-less streams of values in either 32-bit, floating-point binary or ASCII form---the routine automatically detects the format by conducting a search for an ASCII structure of numbers and spaces, failure of which invokes binary reading. A large number of flags are available for specifying how the input is to be transformed. Values may be piped from one call to another, and may have multiple flags, which are addressed in their listed order, cumulatively applying processing to the previous flag's output. *Reshape* has a variety of uses that are best addressed through study of the flag information page.
RETURN TO INDEX

**TERMS AND COMMON FEATURES**

A collection of terms, parameter, or ways of doing things permeates PVC. Below is a listing of many, with accompanying explanations follwoing.

**OVERLAP/ADD VS. OSCILLATOR BANK METHODS AND RESYNTHESIS THRESHOLDS:**

The phase vocoder resynthesizes the sound using one of two methods, depending on the type of changes made to the FFT. If changes are made to only the magnitudes (amplitudes) of the polar form analysis, then the faster *overlap/add method* is used. If, however, the frequencies are changed, then the underlying structure of the FFT data is compromised relative to the overlap/add method, necessitating use of the *oscillator bank method*, in which each analysis bin is synthesized as a sine wave varying in frequency and amplitude. Oscillator bank synthesis is slower, although the re-synthesis threshold can be set to reduce computation and increase speed. A threshold of -60dB is appropriate, although safety warrants using a lower threshold if the spectrum is thin and its decays exposed; use your ear.

**SOURCE**

The source sound is the original input sound. Some routines allow for the mix of the processed sound with the original source sound.

**MULTIPLE CHANNELS**

All routines allow for the processing of multiple input channels, from one to infinity. (Many of the supported sound file formats allow for the writing of an unlimited number of interleaved channels, which, once written, can be addressed as input channels.) PVC allows for the processing of either one particular channel or all channels, however many there may be. Channels are numbered beginning with one; a channel designation of *zero* causes processing of all available input channels, addressed one-at-a-time, in their numerical file order.

RETURN TO INDEX

**AMPLITUDE RESCALING\**
Given the unpredictable results of some types of processing, it is useful to rescale a routine's audio output. The rescale flag allows rescaling to any level up to 0 dB, which, is a common peak amplitude for audio interface's and integer sound files. A rescale level of 1 triggers rescaling of the output to the input file peak amplitude; a value of 2 prevents any rescaling of the output, which may result in a clipped audio output.

**AUTOMATIC PLAYBACK OF OUTPUT**

Upon completion of processing and writing of the output sound file, PVC uses system calls to run *sndfile-play,* which is a *libsndfile* playback utility*.* The underscore flag ("\_"), common to all audio output routines, determines the type of playback to be used, with options for interactive playback, with or without an initial audition of the output, automatic playback with a preset number of repetitions, or bypassed playback.

**INPUT SOUND FILE**
Input sound files may be in any format supported by *libsndfile* and may have multiple channels*.* Output will consist of either one user-specified channel or all input channels; multiple channels are processed one-at-a-time, in their file order. The "\~" sign may be used to represent the user's home directory.

**OUTPUT SOUND FILE**

The output sound file *format* is taken from the named output file; therefore, the output sound file must be a pre-existing sound file written in a format supported by *libsndfile.*

**OUTPUT STATISTICS**

Two flags are provided for controlling the output amplitude statistics; one turns the statistics on or off, the other sets how often they will be reported. The statistics provide the ongoing peak output level in amplitude and decibels. When amplitude rescaling is turned off, output values exceeding the normalized peak amplitude of 1. (0 dB) are clipped to a value of 1.0 and amplitude reports placed into clip mode, which only reports when clipping occurs. At the completion of processing, clip mode produces report of the peak amplitude, its location, and the number of clipped samples. In rescale mode, however, file values, in a second pass, are rescaled, with before and after levels reported at the end.

RETURN TO INDEX

**FREQUENCY RESPONSE TERMINAL OUTPUT**

In many filtering routines or routines involved in the creation or analysis of a single spectrum, a crude terminal print of the spectrum is available. A flag sets the high cutoff frequency for this output; a value of 0 (0 Hz) turns printing off.

**INPUT ANALYSIS FILES**

*Pvanalysis* is used to create input analysis files*.* The files, which are written in binary, contain frames of FFT analysis data for one or more channels. Analysis files contain a header with useful information about the analysis; headers may be read with *readheader*. Analysis files are much larger than the sound files they represent and increase in proportion to the FFT size used.

RETURN TO INDEX

**DECIBELS**

Amplitude in *PVC* is always handled in decibel units. 0 dB, which corresponds to an amplitude of 1.0 and which is known as unity gain, is the peak amplitude in matters of compression, expansion, and amplitude windowing. A change of +/- 6 dB represents a respective doubling or halving of amplitude and a change of +/- 10 dB is loosely associated with a change in one dynamic level.

**LOW/HIGH SHELF EQUALIZATION**

*Low/High Shelf Equalization* is included in many routines for general adjustment of spectra, either as add-on post-processing of the output or as part of feedback reverberation. The EQ consists of low and high shelf segments whose width is adjusted through control of the shelf breakpoint frequencies. The region between shelves is represented by a linear decibel gradient that interpolates between the two shelf levels.

*LOW_SHELF_EQ_gain_in_decibels=-30\
LOW_SHELF_EQ_frequency=400\
HIGH_SHELF_EQ_gain_in_decibels=0\
HIGH_SHELF_EQ_frequency=2000\
\* Some routines implement the EQ before pitch changes, others after. EQ introduced before pitch changes (pre-transpose/shift) will cause the EQ to be transposed with the pitch changes, whereas EQ introduced afterwards (post-transpose/shift) will remain fixed, as shifts and transpositions occur.

**WARP INDEX**

Many of the routines employ the principle of warping in which a distribution of values is transformed by an identity function. An exponential function, varied through the *warp index,* is used to provide a range of nonlinear shapes, from accelerating (concave) to decelerating (convex). In applications where a warp index is indicated, the respective input range is normalized to a 0-1 range of values, reshaped through the exponential function, and then refitted into the parameter's original range.\ Warping causes the minima and maxima of a range to remain fixed while bringing the distribution of values closer to either extreme, depending on the curvature of the selected *warp index* function. Mathematically, warp index *w* will reorient the input *x* through the function below (\^ = exponentiation).

*y = (1. - (e\^(x \* w))) / (1. - (e\^w))*

An index of 0 will result in a linear function and an untransformed output. Positive indices of increasing magnitude produce curves of increasing concavity (increasing slope) that draw values toward the minimum, reducing the function integral; negative values do the opposite, drawing values towards the maximum.

The practical use of this mechanism is found in various places, such as the reshaping of frequency responses. In these, positive warp indices will accentuate frequency response peaks and expand out weaker frequencies. Negative values will have the opposite effect, as they compress the dynamic range of the response and raise the relative level of the weaker noise components.\ Another place where the warp index applies is in the remapping of FFT amplitudes through the spectrum parameter *warpshape*. In these cases, amplitudes are remapped, causing expansion or compression of the dynamic range.\
RETURN TO INDEX

**PITCH TRANSPOSITION**

Pitch transposition is almost always specified in semitones, be it above or below; the appropriate frequency multiplier is computed.

**FREQUENCY SHIFT**

The frequency shift control adds a value to all bin frequencies; the linear translation of *frequency* results in a nonlinear *pitch* translation of the spectrum. The effect of frequency shifting is very similar to ring modulation, which causes a nonlinear pitch shift. Use either small amounts, either constant or varying, to create small distortions of the harmonic integrity of a sound, or large amounts related to the fundamental harmonic structure of the sound to create shifts in the placement of formants.

RETURN TO INDEX

**ENVELOPE RESPONSE TIME**

The rate at which amplitude is allowed to change affects the smoothness of a sound's spectral evolution. The concept of *response time* is a common parameter in many routines, showing up in varied contexts.

Response time is often separated into *attack* and *decay times*, which are translated into the coefficients of the following one-pole low-pass filter*.*

*y(n) = (1. - A) \* x(n) + A \* y(n)*

The filter slows the introduction of sudden changes to a signal, with increase in the coefficient *A* increasing suppression. Control of the coefficient is achieved through the *response time*, which represents the time in seconds it takes a drop in value to reach -60 dB of its change.

The separation into attack and decay times offers control over the increasing or decreasing signal. Short attack/decay response times can be used in places where the use of thresholds induces garble or even pops. Longer response times are generally useful in smoothing or blurring the onset/offset of sound components, particularly if the response controls are being applied to a time-varying filter.

**RING DECAY TIME**

*Decay time* is an issue in the feedback of the *ring* routines. Like response time, addressed above, *decay time* is the time it takes a drop in value to decay to -60dB of its former state (i.e. the time it takes the reverb to decay to -60dB).

RETURN TO INDEX

**FFT SIZE:**

The FFT size must be a power of 2. Larger FFT sizes resolve frequencies better but transients less well. Choose your FFT size according to the sound you are working with. In most cases, 1024 or 2048 work well; *ring* routines work best with very large FFT sizes.

**WINDOW SIZE**

The window size is a less opaque parameter; like the FFT, it must be a power of 2. Windows twice the size of the FFT work well. Larger window sizes may resolve frequencies better. Specifying 0 for the window size will automatically set the window to twice the FFT size, which is what I have always done.

**WINDOW TYPE**

In order to reduce side lobe energy and improve the isolation of analysis bins, the FFT is computed by first applying a symmetrical envelope or window to the segment of signal. Like the FFT size, the shape of the window affects the quality of the analysis and synthesis. (See F.R.Moore, Stieglitz, or Roads for further explanation.) Available windows include: Hamming, rectangular, Blackman, Bartlett (triangular), Kaiser windows for alpha values from 4 through 12, Blackman-Harris, Nuttal, Blackman-Nuttal, and the Flat top window. Blackman, Kaiser, and Flat top produce the best frequency results with the least amount of bleeding between adjacent bins.

RETURN TO INDEX

**FRAMES PER SECOND**

The frames per second parameter controls how often the input signal is analyzed; it is a translation of the classic decimation control, which specifies the number of samples to skip between analysis frames. Increased frame rates increase the resolution of time but decrease computation speed. 200 frames per second is a good reference point. If you expand time you should increase the frame rate proportionately to maintain about 200 or more frames per second in the output sound.

**TIME EXPANSION/CONTRACTION**

Once spectral modifications are made to the FFT analysis, an inverse FFT is used to produce the samples of a time-domain signal. The number of samples produced per frame is determined through the *interpolation* value and its relation to *decimation*. In PVC, decimation and interpolation are translated into *time expansion*, which offers more transparent control of time scaling. Values greater than one expand time, values less than one contract it.

Extreme time expansion is not recommended, as the slowing of noise into pitch results in a sound that, while novel and beautiful to first-time users, is quickly recognized by experienced users for the artifact-ridden cliché that it is. Learn to hear and avoid this sound.

RETURN TO INDEX

**BEGIN/END TIMES**

Processing may be performed on an entire file or a segment of it, as indicated by the beginning and end times. End times less than or equal to 0 default to the end of the input file.

**GAIN:**

The *gain* control offers control of amplitude in decibel units; 0 dB represents unity gain, no change. See decibels.

[**FILTERING: SOURCE SIGNAL LEVEL**](#DECIBELS)

The *source decibels floor* parameter determines the mix of input source and filter output. The parameter functions as a proportional control for the two components with an upper range limit of unity gain or 0dB. Consequently, whereas a value of 0dB would eliminate the filter output from the mix, a value of -96dB would effectively eliminate the source.

RETURN TO INDEX

**TRANSPOSITION/SHIFT APPLICATION FLAG**

A switch is included in filter routines that specifies whether the pitch changes should be applied before or after filtering; when applied before, the change is applied only to the source, with the filter remaining constant, when after, to the source and filter both.
{#filter_pass_or_reject}

**FILTER TYPES: PASS OR REJECT**

Filters can be toggled to use frequency responses in pass or rejection mode. In pass mode, the response's stronger magnitudes are used to pass source through the filter, whereas in rejection mode, they impede or reject components. In rejection mode, the response is created by inversion of the response within the decibel range (not amplitude). In time-varying filtering (*tvfilter*), inversion of the response is either against a constant 0 dB peak or against the current analysis frame's peak amp. Spectral warping is always applied after the response has been transformed by rejection.

**BRICK WALL FILTER**
Brick wall filter are distinguished by their unusually sharp stop band cutoff in amplitude. While complicated to design in the time domain, with the FFT, brick wall filters are easily made through the direct specification of pass or stop bin amplitudes according to desired cutoff points, the only limitation being the precision of the cutoff and roll-off, both being a function of the FFT size, with larger FFT sizes offering greater precision of cutoff placement and rate of stop-band, amplitude roll-off. Along with the brick wall filter in *plainpv,* brick wall filtering can be achieved using *filter* with *filtresponsemaker* (*S.filter_with_breakpoint_synthesis*).
RETURN TO INDEX

**RESPONSE FUNCTION SMOOTHING**

Many routines that use frequency response files to filter or warp amplitudes have a control that allows the response to be "smoothed." The process involves replacing the magnitude of a frequency bin with an average taken from a band centered on that bin and is controlled through the manipulation of the bandwidth, specified in octave units. Larger bandwidths produce smoother frequency responses; 0 turns smoothing off.

**SOUND ANALYSIS DATA: ACCESS MODES**

Routines that use [*pvanalysis*](#PVANALYSIS_DESCRIPTION) files access file data using two parameters: the *time point origin* and the *rate multiplier.* The *time point origin* controls the data time pointer directly, thus offering direct control of how re-synthesis moves through the time of the analyzed sound (with speed of movement being a consequence of changes in position). The *rate multiplier,* conversely, offers direct controls of the speed at which the data time pointer moves, thus offering the more salient control of the passing of time (with actual position in time being a consequence of speed). Both variables can take numerical constants or function files, which makes for three different data time-point access modes: *direct, indirect, and combined.*

In *direct mode,* position in time is controlled directly through the *time point origin* parameter*;* set the *rate multiplier* to 0 and the *time point origin* to a function whose values address the time range of the analysis file. (Note: This time range always spans from zero to the duration of the analyzed segment, regardless of where in the original sound file the analysis was taken from.)

In *indirect mode,* position in time is controlled indirectly through the *rate multiplier* parameter; set the *time point origin* to a constant (which will serve as a starting point) and the *rate multiplier* to a non-zero numerical constant or function file.

In *combined mode,* position in time is a consequence of both the *time point origin* and the *rate multiplier* variables, whose respective changes in value are simply combined, much like frequency and phase in waveform synthesis---i.e. the subsequent changes to the *time point origin* are added to the changes in position incurred through the *rate multiplier.* However esoteric, this access mode is entirely possible.
{#Time_Boundaries}**TIME BOUNDARIES**
The low and high time boundaries set the analysis data time range, in which synthesis is allowed, either through the auto-stop or [sampler loop](#SAMPLER_LOOP_MODE) mode. Time boundaries can be constant or changed continuously in conjunction with the sampler loop mode and a re-synthesis scheme that explores the effect of a changing or shifting loop window.

**AUTO-STOP MODE**\
In auto-stop mode, synthesis is ended with the crossing of a time boundary. Auto-stop can be useful in circumstances in which a varying rate multiplier makes prediction of the overall duration difficult.

**SAMPLER-LOOP MODE**
In sampler-loop mode, low and high time boundaries become the loop points for a sample loop-time window.

**LOOP-TIME WINDOW**
In sampler-loop mode, sample looping can be set to *wrap, fold,* or *clip.* In wrapped loops, loop time is addressed modularly, with the loop window re-entered at the boundary opposite from the boundary crossed. Folded loops bounce back and forth between the two time boundaries. And clipped loops stop movement through the loop once they reach a boundary.

**ONSET AND RELEASE SEGMENT MODE**
Sampler loop approaches to re-synthesis can be expanded to include onset and release segments. In onset/release mode, looping is preceded and followed by onset and release stages, which are the time segments lying before and after the loop time window. To use this feature, initialize the [time point origin](#Data_Access) to zero, turn on [sampler loop mode](#SAMPLER_LOOP_MODE) and onset/release mode, and then control the [data time](#Data_Access) using a constant or variable [rate multiplier](#Data_Access) value. As the data time passes from the onset segment into the loop region, sampler-loop mode will be triggered and maintained until the length of the release segment and output duration trigger movement into the release segment, which terminates synthesis upon reaching the data end time.\
{#LOOP_AMPLITUDE_NORMALIZATION}**LOOP AMPLITUDE NORMALIZATION**
Loop time boundary amplitudes can be equalized using the normalization parameter; amplitude normalization brings greater continuity to looping, as it evens out and, to some extent, hides the effect of looping. A value of 1 will cause amplitudes inside the loop to be normalized to the higher of the two time boundary amplitudes. Onset and release segments are ramped into and out of normalization.

**LOOP SMOOTH TIME**
For wrapped and folded loops, the respective shift in position or direction at loop points can be smoothed using the *loop smooth time* parameter. Loop point smoothing is achieved by introducing low pass filtering into each bin's amplitude and frequency trajectory for the specified time. The filter loop smooth time is distributed equally across the loop time point and ramped up and down so as to smooth any sudden changes in amplitude or frequency.\
{#TIME_POINT_DITHERING}**TIME POINT DITHERING**
Dither of the time point can be introduced by setting the *time point dither window*; with dither, the data time access point is randomly selected from within the shifting window. Roughness can be smoothed with the *time point change response time,* which uses a low pass filter to slow and smooth the effect of the randomized movement. Time point dithering can be useful with longer time scaling.\
{#RANDOM_AMPLITUDE_VARIATION}**RANDOM AMPLITUDE VARIATION**
The ongoing amplitude of each bin can be randomly varied through the *decibels of variation* parameter and its accompanying *random variation response time.* Positive decibel values determine the degree of amplitude deviation above and below the bin's current amplitude. Positive response times slow and smooth the random perturbations. A high pass shaping of the overall spectral variation can be added using the *[random variation rolloff* control.\
{#RANDOM_FREQUENCY_VARIATION}**RANDOM FREQUENCY VARIATION**
The ongoing bin frequencies may be randomized as well by setting the *variation proportion, response time, and distribution curve.* The ongoing, analyzed frequency is randomly deviated within a band, the width of which is twice the product of the *variation proportion* and the analyzed frequency. The distribution curve index controls the shape of the randomization around the bin's analyzed frequency; positive values of increasing magnitude draw values close to the bin's frequency, much like the Q control on a band pass filter. Positive response times slow and smooth the random perturbations.**\** ](#Data_Access)

**RANDOM VARIATION ROLLOFF**
The limits and shaping of amplitude and frequency are controlled by the *random variation rolloff.* Variation is limited to frequency bins above the *cutoff frequency* and is progressively ramped out between the *rolloff* and *cutoff* frequencies. The *rolloff shape index* determines the rate at which variation is rolled off: 0 produces a linear decline, negative values produce an accelerating decline, and positive values a sudden, decelerating decline. *\ \*
{#FRAME_NORMALIZATION_DECIBEL_LIMIT}**FRAME NORMALIZATION
DECIBEL LIMIT**\
Output amplitudes can be scaled to match, or approach, the corresponding input frame amplitudes using the frame normalization; scaling is produced using a scaling ratio that is the sum of the input frame amplitudes over sum of the output frame amplitudes. Scaling is limited by the *frame normalization decibel limit,* which prevents excessive amplification in frames having low overall output level; values greater than zero set the decibel normalization limit, a limit of 0 dB prevents any normalization.

RETURN TO INDEX

**CONVOLVER PANPOT**

The [*convolver*](#CONVOLVE_DESCRIPTION) routine has a unique panpot mechanism for controlling the mix of input sounds (A and B) with their convolution. The panpot is a crossfade mechanism that uses a -1 to +1 control range to accentuate sound A, B or the convolution. Values of -1, 0, and 1, respectively produce A, A and B convolved, or B, with values in between producing crossfade mixes. For example, a trajectory from -1 to 1 would crossfade from sound A into the convolution, and then on to sound B.\ Separate gain controls for the three nodes (A, B, and A and B convolved ) make it possible to balance the nodes. As well, the presence or spread of the convolution into crossfade regions can be tuned using the domain warp controls, which shape movement through the crossfade regions and allow for more gradual transitions. Positive domain warp values (specified independently for each side) expand the crossfade area. If you want to hear more convolution in your crossfade regions, then increase the panpot domain warp values.

[**FREQUENCY RESPONSE ACCUMULATION METHOD**](#CONVOLVE_DESCRIPTION)

Frequency response analysis of sound segments is achieved by collecting peaks or by taking an average. Whereas a collection of peaks represents a sound's extremes, an averages represent its most common characteristics. While intermittent sounds may prompt use of the peak method, averaging is more common.

RETURN TO INDEX

**RING ROUTINES: FILTER PLACEMENT**

[*Ringfilter*](#RINGFILTER_DESCRIPTION) and [*ringtvfilter*](#RINGTVFILTER_DESCRIPTION) use frequency response functions in the production of their reverberation. The response is used to filter either the source input or the fed back signal. Filtering of the source takes effect immediately, whereas filtering of the feedback signal appears more gradually depending on the specified decay time.

[**COMPRESSION AND EXPANSION**](#RINGTVFILTER_DESCRIPTION)

Spectral compression and expansion play a role in many routines. Implementation uses thresholds and magnitudes of compression/expansion to reduce or enlarge the dynamic range of a signal. Spectral compression reduces amplitude above a threshold by a decibel amount. Expansion works in a similar fashion while changing only amplitudes below a threshold.

The term companding comes from a merger of the two names in routines where both are available. In addition to *compander,* combined compression and expansion can be found in several other routines that offer filtering. It is not uncommon, in those routines, to reduce the dynamic range of an analyzed frequency response, particularly if it is time-varying, since the goal in filtering is more about color than dynamic range.

With compression and expansion, the dynamic peak of the unprocessed signal or response is assumed to be no greater tha 0 dB and probably no less than -96 dB; thresholds should be chosen from within this range. The degree of compression or expansion, expressed in decibels, represents how much the signal lying beyond the threshold will be reduced. With compression, a reduction of -6 dB would halve the dynamic range above the threshold; in expansion, a -6 dB expansion would double the range below the threshold.

C*ompander* applies compression or expansion, not to the sum of output bins, but separately to each bin using a unique normative bin level defined through a preliminary average or peak frequency response analysis performed over a segment of sound.

RETURN TO INDEX

**UTILITIES**

**READHEADER**
Readheader* is a simple command-line program for listing the header information of an analysis file created with pvanalysis. The program returns the header data for the listed analysis files, as seen below.\ SOUND FILE ANALYZED: /Users/koonce/ridecymbal.NF.aiff

[INPUT ANALYSIS: FFT SIZE: 8192](#PVANALYSIS_DESCRIPTION)

[INPUT ANALYSIS: SAMPLE RATE: 44100](#PVANALYSIS_DESCRIPTION)

[INPUT ANALYSIS: DECIMATION: 44](#PVANALYSIS_DESCRIPTION)

[INPUT ANALYSIS: FRAMES PER SECOND: 1002](#PVANALYSIS_DESCRIPTION)

[INPUT ANALYSIS: TOTAL NUMBER OF FRAMES: 6042](#PVANALYSIS_DESCRIPTION)

[INPUT ANALYSIS: DURATION: 6.028299](#PVANALYSIS_DESCRIPTION)

[INPUT ANALYSIS: NUMBER OF CHANNELS: 2](#PVANALYSIS_DESCRIPTION)

INPUT ANALYSIS: FILE SIZE (in 32-bit floats): 99016328

INPUT ANALYSIS: FILE SIZE (in bytes): 396065312

INPUT ANALYSIS: FILE SIZE (in Mbytes):
377.717316

***\
SHOWME***

Following installation of *gnuplot*, users may view function files using *showme. Showme* is a shell script designed to take a list of header-less function files and graph them, one at a time, using gnuplot. *Showme* translates function files, which may be in ASCII or binary form, using *reshape* and prepares a command to plot them with *gnuplot.*\
{#CMUSIC_GEN_FUNCTIONS}***CMUSIC GEN FUNCTION ROUTINES***
The CMUSIC GEN function routines are a small collection of utilities originally created in the 1980's to complement the CMUSIC sound synthesis language. While the language has since developed into alternate forms, the GEN function routines continue to be useful, particularly for PVC applications where dynamic control of a parameter is desired. The UNIX command-line routines generate header-less series of 32-bit floating-point binary values, which can be stored in a file or processed using *[reshape](#RESHAPE_DESCRIPTION).\ \* Five function generating routines are available (*gen1, gen2, gen3, gen4, and gen5*) plus the stochastic routine *cannon* and the cubic spline routine *cspline.* Entering each command without input specification produces a short explanation of the routine, as seen below in the explanation for each. An output length (-L) is required for each; with *cspline* the flag is omitted (length only). Time points (t) should begin at 0 but may span any length as time points are scaled to the output length. (v = value, a = amplitude, h = harmonic partial number)

*gen1:* Straight line segment generator with control of both time-point and value.\ Usage: gen1 -Llength t1 v1 \... tN vN\ Example: gen1 -L1000 0 1 1 .5 4 3 7 3.5 \> /tmp/myFile\ *\* *gen2:* Fourier synthesis function generator for specifying amplitude of sine (a) and cosine (b) components of the first N partials; function may be in open form (-o the default), which is designed to repeat (concatenating with successive periods), or closed form (-c), which represents the full period length.\ Usage: gen2 -Llength \[-o (default) or -c\] a1 \... aN b0 \... bM N\ Example: gen2 --L1000 --c 1 .5 .3 .2 0 0 0 0 0 5 \> /tmp/myFile (five sine partials with decreasing amplitude, no cosine partials)

*gen3* Simple line segment generator with input values spaced equidistantly.\ Usage: gen3 -Llength v1 v2 \... vN\ Example: gen3 --L1000 0 10 5 2 4 7 9 \> /tmp/myFile\
{#GEN4}*gen4:* Exponential curve segment generator with
control of time-point, value, and transition curvature using index.\
Usage: gen4 -Llength t1 v1 a1 \... tN vN\
A transition index of 0 produces a linear transition, positive indices produce transitions of increasing acceleration or concavity, and negative indices produce transitions of increasing deceleration or convexity. Note: Transitions are formed relative to the points of departure and arrival; hence, descending transitions with the same transition index will "appear" inverted.\ Example: gen4 --L1000 0 1 -3 1 12 2 4 0 \> /tmp/myFile

*gen5:* Fourier synthesis generator with control of harmonic number, amplitude, and phase in radians.\ Usage: gen5 -Llength h1 a1 p2 \... hN aN pN\ Example: gen5 -L1000 1 .5 0 1 .1 .5 5 .2 0 \> /tmp/myFile

*cspline:* Cubic spline function generator\
Usage: cspline len_flag \[flags\] x0 y0 x1 y1 \... xN yN\
*Cspline* uses cubic splines to construct a function through the specified points.\ Note: As part consequence of its computation, *cspline* can and often will slightly exceed the least and greatest values specified. Because of this, it is advisable to rescale the output function into the desired range using *reshape*, as seen in the example below.\ Example: *cspline 1000 0 1 2 3 6 0 7 1 9 6 10 0 \| reshape --b0 --B6 \> /tmp/myFile*

*cannon:* Stochastic function generator\
(See usage page by entering routine name alone. )*\
Cannon* generates a stochastic function based on the specified distribution, parameters, and input. Available distributions include arcsin, beta, Cauchy, exponential delta, gamma, Gauss, hyperbolic, linear, logistic, Laplace, urand, frand, crand, randfi, randfc. Available noise forms include white, Brownian, 1/f, and correlated, and may be drawn from the standard input. Routine may be seeded for reproduction of sequences.
RETURN TO INDEX

**USING SHELL SCRIPTS**

The top section of a PVC script is used to set variables and the bottom section for using those settings to build and execute one or more commands. To use a script, simply set the variables of the top section with the appropriate files and/or constants (do not use spaces), and then run the script in a terminal window.\ To run scripts, you will need to first change to the directory of the script, after which you can enter *S.plainpv.* If you have not included the current directory in your \$PATH variable, then you will need to type include "./" (period and forward-slash) before *S.plainpv,* as in ./S.plainpv; "./ " tells the shell to look in the current directory for the following command.

When the shell runs the script, it copies the assigned variable values into the respective flag positions and runs the command, much as you would were you to enter the command yourself. When you run the script it prints the command to the terminal window just before running it, so you can see and examine the flag settings, if necessary.

**FUNCTION CONTROL OF PARAMETERS**

Parameter flags and their explanation can be found on the information page of each routine, which is printed when you run the routine without flags or files. Parameters that have "(func)" following their definition may be controlled with a function file.\ Function files are header-less\ Function files can be generated while running the script; GEN functions and *reshape* commands placed in the top section of the script will be run along with variable assignments, just before the building and running of the command; output from the commands can be seen just before the routine's output.\ Function-making commands are best placed directly beneath their associated variables, as doing so facilitates organization and composition.

**TUTORIALS\**

**\**

***S.plainpv***
As the most basic of PVC routines, *plainpv* is an appropriate place to begin learning about PVC.\
***Exercise 1: Input Equals Output***

1. Locate the *S.plainpv* file in the PVC *scripts* directory and open
it in TextEdit (MAC) or some equivalent ASCII text editor. (Note: Shell scripts made and edited in more sophisticated text file formats will not run in the shell interpreter, as these contain hidden information, added to handle formatting, which the interpreter will not understand nor filter out---if your script simply does not run, this may be the reason why.) **\**

2. Select a short sound file for processing; an instrumental tone with
a duration less than five seconds would work best. Enter the sound file as the *input_file* in the script. Choose and enter the name of a pre-made [*output_file*](#OUTPUT_SOUND_FILE) as well. Then set all other variables according to the *S.plainpv default values* listed tutorials appendix. Study the variables through their linked definitions, as many are common to other routines.\
3. Run the shell script in a terminal window. (See *Using Shell
Scripts* for assistance.)\
If everything is working correctly, you should see the printing of variable settings and processing statistics, after which *sndfile-play,* called from within the routine, will play the file and then pause, as set. In this example, the output sound should be identical to the input sound.

Study the printed output and work to understand its format and messages. Notice the actual command printed at the beginning of the terminal output, as you may need to examine it some time when questions arise about variable settings.\
***Exercise 2: Changing Values***
Using the same script, experiment with the changing of values; in particular, try changing the pitch level or time-scale by changing the *pitch_transposition_in_semitones* or the *[time_expansion_contraction_factor](#TIME_EXPANSION). Plainpv* is a good routine in which to explore PVC's basic controls, so spend time with it.
***Exercise 3: Function Control of Parameters***
As indicated on the information page, many parameters can be [controlled](#FUNCTIONAL_CONTROL) using function files. The goal of this exercise is to create and use a function file.
******
A. ASCII Control File:

A file of ASCII values may serve as a function file; the following uses *echo* to create one*.\ echo 0 1 0 \> /tmp/semitone_change*

The command puts the sequence 0, 1, 0 into a file, separated by spaces (newlines work as well). When used to control a PVC parameter, the series of three values are distributed across the synthesis duration and used to interpolate a control change from 0 to 1 and back. When applied to *pitch_transposition_in_semitones,* the output sound shifts in pitch, up and back a semitone.

Any collection of UNIX commands, including those used to make function files, may be placed in the top half of a PVC script. Thus, the following would suffice to make the file and assign it to a parameter.

[*[pitch_transposition_in_semitones](#PITCH_TRANSPOSITION)=/tmp/semitone_change*](#PITCH_TRANSPOSITION)

*echo 0 1 0 \> /tmp/semitone_change*

The making of the file may follow (or precede) the parameter assignment, as it is not accessed until the routine is run.\ Apply the example to your script and run it. B. CMUSIC GEN Routine Control File:\ Control files may also be made using the CMUSIC GEN routines, as seen in the following, which uses a binary file of 1000 values, created with *[gen1](#GEN1),* instead of the ASCII file of three values.

[*[pitch_transposition_in_semitones](#PITCH_TRANSPOSITION)=/tmp/semitone_change*](#GEN1)

*gen1 --L1000 0 0 1 1 2 0 \> /tmp/semitone_change*

A more complex trajectory of multiple segments and shapes can be created using *gen4,* which uses indices to specify accelerating and decelerating transitions.

[*[pitch_transposition_in_semitones](#PITCH_TRANSPOSITION)=/tmp/semitone_change*](#GEN4)

*gen4 --L1000 0 0 2 1 1 5 3 -1 -4 7 0 \> /tmp/semitone_change*

The shape and design of control files can be explored before synthesis using the plotting program *gnuplot,* which users must first install. A script called *showme* is available for interfacing with *gnuplot.* Simply enter the *gen4* command in a terminal window, followed by *showme,* as seen below.

**\**
*gen4 --L1000 0 0 2 1 1 5 3 -1 -4 7 0 \> /tmp/semitone_change*\
*showme**/tmp/semitone_change\
\*
{#TWARP_TUTORIAL}***S.twarp***
The principal feature and lesson to be learned from *twarp* is how to manipulate the analysis data time controls, with or without their integrated sampler loop features. The following exercises focus on different applications of these features.

*\* ***Exercise 1. Reproducing the Original Sound through Indirect Control of Data Time*** The goal of this example is to reproduce the input sound using the *rate multiplier*. ******

1. To begin, select a short, decay-oriented sound, such as a cymbal hit
with a stick. Analyze the sound using *pvanalysis* and list the resulting analysis file as the *[input_analysis_file](#ANALYSIS_FILES)* in your *S.twarp* script.\
2. Select an *[output sound file](#OUTPUT_SOUND_FILE)* and set the
remaining variables to those found in the *S.twarp default values* script.\
3. Set the output duration to the value of the analyzed sound, and run
the script.\
With other features turned off, synthesis proceeds at the original rate, progressing from beginning to end, which triggers [auto-stop](#AUTO_STOP_MODE), along with the output duration, to end synthesis. ****** ***Exercise 2. Reproducing the Original Sound through Direct Control of Data Time*** The goal of this example is to replicate exercise 1 by controlling the data time point using the *time point origin.*
4. Set the *rate multiplier* to 0.
5. Use *gen1* to create a linear function file from 0 to the sound's
original duration; give the *time point origin* the name of the file, as seen below.

*[time_point_origin](#Data_Access)=/tmp/time_point_origin_function*\
*gen1 --L1000 0 0 1 "original sound duration" \> /tmp/time_point_origin_function\ \* The sound output should be very similar to the output from exercise
1.

***Exercise 3. Functional Control of the Rate Multiplier and Auto-stop***
The goal of this exercise is to use a function to control the *rate multiplier.* The control function will start at a rate that is two times faster than real time and quickly slow to a quarter of real time by the end. The necessary exponential shape will be created using [*gen4*.\

1. Duplicate parameter settings from exercise 1.\
2. Create an exponentially decaying function using *[gen4](#GEN$)* and
use it to control the *rate multiplier,* as in the following.](#Data_Access)

[*[rate_multiplier](#RATE_MULTIPLIER)=/tmp/rate_multiplier_function\
gen4 --L1000 0 2 -3 1 .25 \> /tmp/rate_multiplier_function\
\*
3. Set the output *duration* to a value that is about twice the duration
of the input sound.\
Rate time control will proceed as a function of time fitted across the specified duration. However, with a long output duration, the end of the data will be reached short of the full duration, causing the *auto stop* feature to terminate synthesis; while doing this ends the function prematurely, it avoids computing a tail of unwanted silence.\ ](#GEN$)
{#TWARP_TUTORIAL_EXERCISE_4}***Exercise 4. Ratchet Sounds
using Sampler Loop Mode***\
The goal of this exercise is to use a short percussion sound and *sampler loop mode* in combination with function-controlled [*rate multiplier*](#DATA_ACCESS) and amplitude [*gain*](#GAIN) values to create a ratchet-like sound of changing speed and amplitude.\

1. Duplicate parameter settings from exercise 1.\
2. Choose a short, decaying percussion sound of a quarter second or
less; use [*pvanalysis*](#PVANALYSIS_DESCRIPTION) to create an analysis file and list the file as the [*input analysis file*](#ANALYSIS_FILES).\
3. Set the data time access mode to [*sampler loop*](#SAMPLER_LOOP_MODE)
and specify a ratchet-like *[rate multiplier](#DATA_ACCESS)* function, something like the one specified and pictured below.

*[rate_multiplier](#Data_Access)=/tmp/rate_multiplier_function*\
*gen4 --L1000 0 1 -3 1 10 2 4 2 -1 7 5 1 10 1 \> /tmp/rate_multiplier_function\ \*
4. Set the [*gain*](#GAIN) level to a function file and use
[*reshape*](#RESHAPE_DESCRIPTION)

to transform the rate multiplier into a decibel control file with the same shape but different range. *Reshape* performs this change easily using the --b and --B flags, which specify the new base and peak function boundaries. Like gen functions, calls to *[reshape](#RESHAPE_DESCRIPTION)* can be placed within the script, assuming the file they process is made before (i.e. above in the script), as seen in the following.\ *[gain_in_decibels](#DECIBELS)=/tmp/gain_in_decibels_function*

*[rate_multiplier](#Data_Access)=/tmp/rate_multiplier_function*\
*gen4 --L1000 0 1 -3 1 10 2 4 2 -1 7 5 1 10 1 \> /tmp/rate_multiplier_function\ [reshape](#RESHAPE_DESCRIPTION) --b-20 --B0 /tmp/rate_multiplier_function \> /tmp/gain_in_decibels_function*\
5. Set the duration to about one or two seconds.\
In mimicry of a stick scraping a ridged surface, the looping sample will speed up and slow down according to the function, with a comparable change in amplitude. Explore further refinement of the sound by adding other correlated (or uncorrelated) changes to [*pitch*](#PITCH_TRANSPOSITION), [*frequency shift*](#FREQUENCY_SHIFT), or [shelf EQ](#SHELF_EQ).\
***Exercise 5. Creating Sustain with Sampler Loop Mode and Onset/Release Segments***\ The goal of this exercise is to use the *sampler loop mode* and

***Tutorials Appendix***
{#PLAINPV_DEFAULT_VALUES}***S.plainpv Default Values*\**

# !/bin/sh\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \...\...\...\...\...\..... PLAINPV \...\...\...\...\...\...\...\....\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* OUTPUT
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
output_file=/Users/koonce/output.au\
# \...\...\...\...\...AUTO-PLAY OF OUTPUT FILE\...\...\...\...\...\
[auto_output_sound_file_play\_\_repetitions](#AUTOMATICPLAYBACKOFOUTPUT)=-2\
\#\
\# -2: interactive: Play once, then prompts for more.\
\# -1: interactive: Prompt for each play of file.\
\# 0: off\
\# 1 or greater: Auto-repeat for specified repetitions.\
# \...\...\...\...\.... RESCALE \...\...\...\...\...\...\...\...\.....\
[rescale_level_in_decibels](#AMPLITUDE_RESCALING)=2\
\#\
\# -96 to 0 dB.\
\# Set to 1 to rescale to peak of input file.\
\# Set to 2 to bypass rescale.\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* INPUT
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
[input_file](#INPUT_SOUND_FILE)=\~/my_input_sound_file\
# \...\..... BEGIN/END TIMES \...\...\...\...\...\...\...\...\.....\
[begintime](#BEGIN_END_TIMES)=0\
[endtime](#BEGIN_END_TIMES)=0\
\# End time of 0 or less defaults to end of file.\
# ======================================================\
# \*\*\* ANALYSIS PARAMETERS
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*

[[FFT_length](#FFT_SIZE)=2048](#BEGIN_END_TIMES)

\# Powers of two: 512, 1024, 2048, 4096\...\
\# Larger values increase bins, adding frequency resolution while\
\# decreasing time resolution; smaller values effect the opposite.\
\# Start with 1024.\
window_type=2\
\# hamming(0), rectangular(1) Blackman(2), Bartlett(3)\
\# Kaiser(4-12) for corresponding alpha values:\
\# alpha = 4: sidelobe of -30dB, narrow center lobe\
\# alpha = 8: sidelobe of -58dB, medium center lobe\
\# alpha = 12: sidelobe of -90dB, wide center lobe\
[windowsize](#WINDOW_SIZE)=0\
\# 0 sets windowsize to 2 \* FFT or larger.\
[frames_per_second](#FRAMES_PER_SECOND)=200\
\# See time expansion note.\
# ======================================================\
# \*\*\* RESYNTHESIS PARAMETERS
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \...\...\..... OUTPUT CHANNEL(S) \...\...\...\...\...\...\.....\
[output_channel](#MULTIPLE_CHANNELS)=0\
\# Channels are numbered from 1 to the maximum.\
\# 0 = all channels\
# \...\...\...\....OSCIL THRESHOLD \...\...\...\...\...\...\...\...\
[oscillator_resynthesis_threshold_in_dB](#OVER_BANK_AND_THRESH)=-96\
\# Try -60 to -70 unless dropouts become audible.\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* MODIFICATIONS
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \...\...\...\...\...\... TIME \...\...\...\...\...\...\...\...\...\...\
[time_expansion_contraction_factor](#TIME_EXPANSION)=1\
\#\
\# Adjust frames_per_second in proportion to keep a\
\# constant output of about 200 frames per second;\
\# i.e. if expansion = 2, double the frames to 400.\
\#\
# \...\...\...\...\...\... DECIBELS \...\...\...\...\...\...\...\.....\
[gain_in_decibels](#DECIBELS)=0\
# \...\...\...\...\...\... PITCH \...\...\...\...\...\...\...\...\.....\
[frequency_shift_in_Hz](#FREQUENCY_SHIFT)=0\
\# Frequency shift is added to all bin frequencies.\
[pitch_transposition_in_semitones](#PITCH_TRANSPOSITION)=0\
# \...\...\...\... AMPLITUDE RESPONSE \...\...\...\...\...\...\....\
[release_time_in_seconds](#RESPONSE_TIME)=0\
[attack_time_in_seconds](#RESPONSE_TIME)=0\
# \...\...\...\... SPECTRUM WARPSHAPE \...\...\...\...\...\...\....\
[spectrum_warpshape_index](#WARP_INDEX)=0\
# \...\...\...\... [BRICKWALL FILTER](#BRICK_WALL_FILTER)
\...\...\...\...\...\...\...\...\
FILTER_TYPE=0\
\# 0: bandpass\
\# 1: bandreject\
# \...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\
BRICKWALL_FILTER_window_low_frequency=0\
BRICKWALL_FILTER_window_high_frequency=-1\
\# -1 selects respective lowest or highest frequency.\
# ======================================================\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\* [LOW/HIGH SHELF EQ](#SHELF_EQ)
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
LOW_SHELF_EQ_gain_in_decibels=-0\
LOW_SHELF_EQ_frequency=500\
HIGH_SHELF_EQ_gain_in_decibels=-0\
HIGH_SHELF_EQ_frequency=5000\
# \*\*\*\*\*\*\*\*\*\* AMPLITUDE STATISTICS
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
print_amplitude_statistics_0_no\_\_1_yes=1\
amplitude_statistics_time_interval=1

[***S.twarp Default Values***](#SHELF_EQ)

# !/bin/sh\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \...\...\...\...\...\...\....TWARP \...\...\...\...\...\...\...\.....\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* OUTPUT
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
output_file=\~/output.wav\
# \...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\
# \...\...\...\...\...AUTO-PLAY OF OUTPUT FILE\...\...\...\...\...\
[auto_output_sound_file_play\_\_repetitions](#AUTOMATICPLAYBACKOFOUTPUT)=1\
\# -2: interactive: (Play once, then prompt for more.)\
\# -1: interactive: (Prompt for each play of file.)\
\# 0: off\
\# 1 or greater = Auto-repeat for specified repetitions.\
# \...\...\...\...\.... RESCALE \...\...\...\...\...\...\...\...\.....\
[rescale_level_in_decibels](#AMPLITUDE_RESCALING)=0\
\# -96 to 0 dB.\
\# Do not use 1 as analysis file levels are very low.\
\# Set to 2 to bypass rescale.\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* INPUT
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# ======================================================\
# \*\*\* ANALYSIS PARAMETERS
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
[window_type](#WINDOW_TYPE)=8\
\# hamming(0), rectangular(1) Blackman(2), Bartlett(3)\
\# Kaiser(4-12) for corresponding alpha values:\
\# alpha = 4: sidelobe of -30dB, narrow center lobe\
\# alpha = 8: sidelobe of -58dB, medium center lobe\
\# alpha = 4: sidelobe of -90dB, wide center lobe\
[windowsize](#WINDOW_SIZE)=0\
[frames_per_second](#FRAMES_PER_SECOND)=200\
\# For inexplicable reasons, it is best to set the frame rate\
\# to a value \>= the frame rate used in the analysis.\
\# Lesser frame rates create loss of amplitude and phasiness.\
# ======================================================\
# \*\*\* RESYNTHESIS PARAMETERS
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \...\...\..... OUTPUT CHANNEL(S) \...\...\...\...\...\...\.....\
[output_channel](#MULTIPLE_CHANNELS)=0\
\# Channels are numbered from 1-maximum\
\# 0 = all channels\
# \...\...\...\....OSCIL THRESHOLD \...\...\...\...\...\...\...\...\
[oscillator_resynthesis_threshold_in_dB](#OVER_BANK_AND_THRESH)=-96\
\# -60 to -70 is fairly standard.\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* MODIFICATIONS
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \...\...\...\...\...\... DECIBELS \...\...\...\...\...\...\...\.....\
[gain_in_decibels](#DECIBELS)=-0\
# \...\...\...\...\...\... PITCH \...\...\...\...\...\...\...\...\.....\
[frequency_shift_in_Hz](#FREQUENCY_SHIFT)=0\
[pitch_transposition_in_semitones](#PITCH_TRANSPOSITION)=0\
# \...\...\...\...\...\...\... RESPONSE \...\...\...\...\...\...\....\
[release_time_in_seconds](#RESPONSE_TIME)=0\
[attack_time_in_seconds](#RESPONSE_TIME)=0\
frequency_change_response_time_in_seconds=0\
[input_analysis_file](#ANALYSIS_FILES)=\~/pvanalysis\
duration=12\
\# Durations of 0. or less default to analysis duration.\
# ======================================================\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* [DATA TIME](#Data_Access)
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
[time_point_origin](#Data_Access)=0\
[rate_multiplier](#Data_Access)=1\
\# \*\* STANDARD USE: By Time Point or Rate \*\*\*\
\# Movement through data time is controlled either directly,\
\# through control of the time_point_origin, or indirectly, through the\
\# accumulated shifts of the rate control.\
\#\
\# \*\* Time Point Mode: Direct Control of Time Point \*\*\
\# Set rate_multiplier to 0 and use a continuous time_point_origin function\ \# to move through the time range of the analyzed sound.\ \# or\ \# \*\* Rate Mode: Indirect Control of Time Point \*\*\ \# Set time_point_origin to an initial analysis time point and scale/modulate\ \# movement through original sound file time by a constant or a function, which\ \# produces a respective fixed or changing scaling of time. Negative\ \# rate_multiplier values reverse time; zero stops movement through time.\ \# \*\*\* Combining Time Point and Rate \*\*\*\ \# If functions are used for both the time_point_origin and rate_multiplier,\ \# the two are simply combined, i.e. the subsequent changes to the origin\ \# are added to the changes in position incurred through rate.\
# \...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\...\
# \...\...\..... TIME BOUNDARIES AND SAMPLER LOOP MODE
\...\...\...\.....\
\# The low/high data time boundaries serve as either auto-stop\
\# boundaries that terminate synthesis at the crossing of a\
\# data time boundary or as sampler loop points to which can\
\# be added pre-loop and post-loop onset and release segments.\
[low_time_boundary](#TIME_BOUNDARIES)=-1\
[high_time_boundary](file:///\\host.lan\Data\files\tasks\95fe28ed-0474-47f4-8364-76b4718938e2\q#TIME_BOUNDARIES)=-1\
\# -1 for low defaults to 0.0, and, for high, to analysis file duration.\ [Mode\_\_sampler_loop_0\_\_autostop_1](#AUTO_STOP_MODE)=1\ \# In autostop mode, synthesis terminates with the crossing of\ \# a time boundary. In sampler loop mode, time boundaries\ \# function as sampler loop points.\ [Sampler_Loop_Mode\_\_wrap_0\_\_fold_1\_\_clip_2](#SAMPLER_LOOP_MODE)=0\ \# In Sampler Loop Mode, all timepoints are constrained to\ \# region between boundaries using the specified method.\ \# 0: Wrap time into loop range. (/////)\ \# 1: Fold time into loop range. (/\\/\\/\\/\\)\ \# 2: Clip or limit time to the nearest boundary. (\\\_\_\_\_)\ [Onset_and_Release_Segment_Mode\_\_off_0\_\_on_1](#ONSET_AND_RELEASE_SEGMENT_MODE)=0\ \# Setting onset/release mode in Sampler Mode causes looping\ \# to be preceded by a pre-loop/onset and a post-loop/release\ \# segment. The data time origin is initialized to 0.0 and\ \# progresses according to data time controls. The first entrance\ \# into the loop region triggers loop mode, which is maintained\ \# until trigger of the release segment. The release segment\ \# progresses from the current loop point to the analysis data\ \# end time, conforming to the data time rate control and specified\ \# synthesis duration.\ [Normalize_Loop_Amplitudes\_\_off_0\_\_on_1](#LOOP_NORMALIZATION)=0\ \# Set to 1, amplitudes in loop range are normalized to peak\ \# amplitude of low/high boundary frames. Onset and release\ \# segments are ramped in and out of loop range gain change\ \# as needed.\ [Loop_Smooth_Time_in_Seconds](#LOOP_SMOOTH_TIME)=0.2\ \# Loop junctures for wrapped and folded loops are smoothed\ \# with a lowpass filter, the smooth time of which is ramped\ \# up and down before and after the loop juncture points to\ \# smooth any sudden change in amplitude and frequency values.\
# \...\...TIMEPOINT DITHER \...\...\...\...\...\...\...\...\...\....\
[timepoint_dither_window_in_seconds](#TIME_POINT_DITHERING)=0\
[time_point_change_response_time_in_seconds](#TIME_POINT_DITHERING)=0.0\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \*\*\*\*\*\*\*\*\*\* SPECTRUM MODIFICATIONS
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
# \...\...\...\... SPECTRUM WARPSHAPE \...\...\...\...\...\...\....\
[spectrum_warpshape_index](#WARP_INDEX)=0\
# \.... RANDOM AMPLITUDE VARIATION \...\...\...\...\...\...\....\
[decibels_of_variation](#RANDOM_AMPLITUDE_VARIATION)=0\
\# Use positive values; 0 = no variation.\
[random_variation_response_time_in_seconds](#RANDOM_AMPLITUDE_VARIATION)=0.0\
# \...\...RANDOM FREQUENCY VARIATION \...\...\...\...\...\...\...\
[random_frequency_variation_proportion](#RANDOM_FREQUENCY_VARIATION)=0.\
[frequency_variation_response_time_in_seconds](#RANDOM_FREQUENCY_VARIATION)=0.05\
[frequency_variation_distribution_curve_index](#RANDOM_FREQUENCY_VARIATION)=2\
\# A band of randomization is created around each\
\# bin's analyzed frequency. The band is determined by\
\# the proportion, which sets the limits above and below.\
\# A value of .1 creates a band spanning a 1/10th\
\# above/below the bin analysis frequency. The curve\
\# index controls the shape of the random distribution;\
\# positive values distribute the randomization closer\
\# to the center of the band, like a higher Q. The response\
\# time is a lowpass filter that smooths the randomization\
\# function, which slows the the random change in value.\
# \.... (FREQ AND AMP) RANDOM VARIATION ROLLOFF \...\...\...\
[random_variation_rolloff_frequency](#RANDOM_VARIATION_ROLLOFF)=5000\
[random_variation_cutoff_frequency](#RANDOM_VARIATION_ROLLOFF)=500\
[random_variation_rolloff_shape_index](#RANDOM_VARIATION_ROLLOFF)=0\
\# Rolloff frequency is above cutoff frequency.\
\# Below the low cutoff frequency there is no variation.\
\# Above the rolloff frequency there is full variation.\
\# The rolloff shape index specifies the high-to-low,\
\# rolloff-frequency to cutoff-frequency curve:\
\# 0 = linear rolloff, negative values = a slow rolloff,\
\# positive values = a sudden rolloff.\
# ======================================================\
# \*\*\*\*\*\*\*\*\*\*\*\*\*\*\* [LOW/HIGH SHELF EQ](#SHELF_EQ)
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
LOW_SHELF_EQ_gain_in_decibels=-0\
LOW_SHELF_EQ_frequency=400\
HIGH_SHELF_EQ_gain_in_decibels=0\
HIGH_SHELF_EQ_frequency=2000\
# ======================================================\
# \*\*\*\*\*\*\*\*\*\*\*\* AMPLITUDE STATISTICS
\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\
print_amplitude_statistics_0_no\_\_1_yes=1\
amplitude_statistics_time_interval=1.0\
# ======================================================\
# ========= SCRATCH SPACE ==============================\
# ======================================================\
# ====================================================\
\# COMMAND LINE SETUP \-- OFFICE USE ONLY\
\# (DO NOT WRITE BELOW THIS LINE)\
# ====================================================\
\# SYNTHESIS\
pvroutine=twarp\
PVFLAGS=\"\\\
\\\
-M\$windowsize \\\
-w\$window_type \\\
-D\$frames_per_second \\\
-d\$duration \\\
\\\
-a\$frequency_shift \\\
-P\$pitch_transposition_in_semitones \\\
-A\$gain_in_decibels \\\
\\\
-C\$output_channel \\\
-t\$oscillator_resynthesis_threshold_in_dB \\\
\\\
\\\
-F\$input_analysis_file \\\
\\\
-Q\$time_point_origin \\\
-Y\$rate_multiplier \\\
-g\$low_time_boundary \\\
-G\$high_time_boundary \\\
-o\$Sampler_Loop_Mode\_\_wrap_0\_\_fold_1\_\_clip_2 \\\
-r\$Onset_and_Release_Segment_Mode\_\_off_0\_\_on_1 \\\
-T\$timepoint_dither_window_in_seconds \\\
-K\$time_point_change_response_time_in_seconds \\\
\\\
-L\$release_time_in_seconds \\\
-l\$attack_time_in_seconds \\\
-f\$frequency_change_response_time_in_seconds \\\
\\\
-W\$spectrum_warpshape_index \\\
\\\
-B\$decibels_of_variation \\\
-I\$random_variation_rolloff_frequency \\\
-J\$random_variation_cutoff_frequency \\\
-N\$random_variation_rolloff_shape_index \\\
-e\$random_variation_response_time_in_seconds \\\
\\\
-j\$random_frequency_variation_proportion \\\
-k\$frequency_variation_response_time_in_seconds \\\
-n\$frequency_variation_distribution_curve_index \\\
\\\
\\\
-H\$LOW_SHELF_EQ_gain_in_decibels \\\
-m\$LOW_SHELF_EQ_frequency \\\
\\\
-X\$HIGH_SHELF_EQ_gain_in_decibels \\\
-R\$HIGH_SHELF_EQ_frequency \\\
\\\
-p\$print_amplitude_statistics_0_no\_\_1_yes \\\
-i\$amplitude_statistics_time_interval \\\
\\\
-S\$Mode\_\_sampler_loop_0\_\_autostop_1 \\\
\\\
-v\$Normalize_Loop_Amplitudes\_\_off_0\_\_on_1 \\\
-b\$Loop_Smooth_Time_in_Seconds \\\
\\\
-\_\$auto_output_sound_file_play\_\_repetitions \\\
-=\$rescale_level_in_decibels \\\
\\\
\\\
\"\
\$pvroutine \$PVFLAGS \$output_file ;
