#!/bin/sh
#******************************************************
#.................... RING ............................
#******************************************************

#******************** OUTPUT **************************

output_file=/tmp/output.au

# The following may be used to copy a formatted
# sound file into the output location listed above.   

#cp ~/32bit.BLANK.au $output_file

#...............AUTO-PLAY OF OUTPUT FILE...............
auto_output_sound_file_play__repetitions=0
#(-2 = interactive: (Play once, then prompts for more.)
#(-1 = interactive: (Prompt for each play of file.)
#( 0 = off)
#( 1 or greater = Auto-repeat for specified repetitions.)

#................ RESCALE .............................

rescale_level_in_decibels=-0
#(-96 to 0 dB. )
#(Set to 1 to rescale to peak of input file.)
#(Set to 2 to bypass rescale.)


#******************** INPUT ***************************

input_file=~/voice.NF.aiff
 

#........ BEGIN/END TIMES .............................

begintime=0
endtime=0
# (0 end time defaults to end of file)

#======================================================
#*** ANALYSIS PARAMETERS ******************************

FFT_length=4096
window_type=16
windowsize=0

#( Use larger FFT sizes for ring routines.)

frames_per_second=200

#======================================================
#*** RESYNTHESIS PARAMETERS ***************************


#........... OUTPUT CHANNEL(S) .......................

output_channel=0
# (channels are numbered from 1-maximum)
# (0 = all channels)

#.............OSCIL THRESHOLD ........................

oscillator_resynthesis_threshold_in_dB=-96

#****************** MODIFICATIONS *********************

#.................. TIME ..............................

time_expansion_contraction_factor=1

#.........  MASTER DECIBELS (source + reverb) .........

MASTER_gain_in_decibels=0

#******************************************************
#.......... SOURCE PARAMETERS .........................
#******************************************************

#.................. DECIBELS ..........................

SOURCE_gain_in_decibels=-100

#.................. PITCH .............................

SOURCE_frequency_shift=0
SOURCE_pitch_transposition_in_semitones=0

#******************************************************
#.......... REVERB PARAMETERS ........................
#******************************************************

#...REVERB OUTPUT CONTROLS ** y(n) ***...............

#.................. DECIBELS ..........................
REVERB_gain_in_decibels=0

#.................. PITCH .............................
REVERB_frequency_shift=0
REVERB_pitch_transposition_in_semitones=0


#(This control is for tuning the FFT analysis/synthesis 
# freq. )
#-----------------------------------------------------
#........... OUTPUT EQ .............................. 
REVERB_OUTPUT_LOW_SHELF_EQ_gain_in_decibels=0
REVERB_OUTPUT_LOW_SHELF_EQ_frequency=1000
REVERB_OUTPUT_HIGH_SHELF_EQ_gain_in_decibels=-0
REVERB_OUTPUT_HIGH_SHELF_EQ_frequency=4000
#-----------------------------------------------------

#-----------------------------------------------------
#-------------> RANDOM DEVIATION CONTROLS <----------
#......... RANDOM  FREQUENCY DEVIATION.................
REVERB_random_freq_deviation_in_Hz=50
#( This is the maximum deviation added to the bin)
REVERB_random_deviation_response_time_in_seconds=.01

#(this low pass filters the change, slowing it; .1 is good.)

#......... RANDOM  AMPLITUDE DEVIATION...............

REVERB_random_amplitude_deviation_floor_in_decibels=-0
#(0 (none) to -96 (full range) )
REVERB_random_amplitude_deviation__response_time_in_seconds=.01
#(this low pass filters the change, slowing it.)
#-----------------------------------------------------

#=====================================================

#...REVERB INPUT CONTROLS ** x(n) ***..................

#........... INPUT THRESHOLD CONTROLS .................

REVERB_threshold_in_dB=-30
REVERB_threshold_passmode__above_1_below_0=1

#( The source bins are input into the reverb path depending
#  on whether they lie above or below the threshold. The
#  passmode determines whether to pass bins above or below 
# the threshold. For harmonic sounds,  passing above -20
#  works well. For some noisy sounds,  passing stuff below
# -20 can be interesting. )

#------------------------------------------------------
#*** REVERB INPUT x(n) ENVELOPE RESPONSE

REVERB_INPUT_envelope_attack_time_in_seconds=0.0
REVERB_INPUT_envelope_release_time_in_seconds=0.0

#-----------------------------------------------------
#........... INPUT EQ .............................. 
REVERB_INPUT_LOW_SHELF_EQ_gain_in_decibels=-0
REVERB_INPUT_LOW_SHELF_EQ_frequency=500
REVERB_INPUT_HIGH_SHELF_EQ_gain_in_decibels=-0
REVERB_INPUT_HIGH_SHELF_EQ_frequency=1000
#-----------------------------------------------------

#=====================================================
#...REVERB FEEDBACK CONTROLS ** y(n-1) ***............

REVERB_decay_time_in_seconds=4
#gen4 -L1000 0 1 2    1 12 > /tmp/rtime
#( This is a master decay time control.)

#...................................................... 

REVERB_FEEDBACK_EQ_decay_time_in_seconds=4
#( This controls the decay into the feedback eq.)


#-----------------------------------------------------
#........... FEEDBACK_EQ .............................. 
REVERB_FEEDBACK_LOW_SHELF_EQ_gain_in_decibels=-0
REVERB_FEEDBACK_LOW_SHELF_EQ_frequency=100
REVERB_FEEDBACK_HIGH_SHELF_EQ_gain_in_decibels=-0
REVERB_FEEDBACK_HIGH_SHELF_EQ_frequency=500
#-----------------------------------------------------

#............BALANCE...................................
REVERB_FEEDBACK_EQ_signal_balance_gain_limiter_in_dB=0
#(0 to 96dB, 0 = balancing off (full limiting), 
#               96 = balance full-on (no limiting)

#******************************************************
#********** AMPLITUDE STATISTICS ********************** 
print_amplitude_statistics_0_no__1_yes=1
amplitude_statistics_time_interval=1


#======================================================
#========= SCRATCH SPACE ==============================
#======================================================




#====================================================
# COMMAND LINE SETUP -- OFFICE USE ONLY
#      (DO NOT WRITE BELOW THIS LINE)
#====================================================
pvroutine=ring 

PVFLAGS="\
\
-N$FFT_length \
-M$windowsize \
\
-W$window_type \
-D$frames_per_second \
-I$time_expansion_contraction_factor \
\
-C$output_channel \
-t$oscillator_resynthesis_threshold_in_dB \
\
-b$begintime \
-e$endtime \
\
-A$MASTER_gain_in_decibels \
\
-S$SOURCE_gain_in_decibels \
-f$SOURCE_frequency_shift \
-p$SOURCE_pitch_transposition_in_semitones \
\
-F$REVERB_gain_in_decibels \
-H$REVERB_frequency_shift \
-P$REVERB_pitch_transposition_in_semitones \
-z$REVERB_threshold_in_dB \
-V$REVERB_threshold_passmode__above_1_below_0 \
\
\
-Z$REVERB_decay_time_in_seconds \
\
-j$REVERB_random_freq_deviation_in_Hz \
-K$REVERB_random_deviation_response_time_in_seconds \
\
-x$REVERB_random_amplitude_deviation_floor_in_decibels \
-q$REVERB_random_amplitude_deviation__response_time_in_seconds \
\
-L$REVERB_INPUT_envelope_release_time_in_seconds \
-l$REVERB_INPUT_envelope_attack_time_in_seconds \
\
-T$REVERB_FEEDBACK_EQ_decay_time_in_seconds \
-E$REVERB_FEEDBACK_EQ_signal_balance_gain_limiter_in_dB \
\
-X$REVERB_FEEDBACK_LOW_SHELF_EQ_gain_in_decibels \
-U$REVERB_FEEDBACK_LOW_SHELF_EQ_frequency \
-Q$REVERB_FEEDBACK_HIGH_SHELF_EQ_gain_in_decibels \
-m$REVERB_FEEDBACK_HIGH_SHELF_EQ_frequency \
\
-O$REVERB_INPUT_LOW_SHELF_EQ_gain_in_decibels \
-d$REVERB_INPUT_LOW_SHELF_EQ_frequency \
-Y$REVERB_INPUT_HIGH_SHELF_EQ_gain_in_decibels \
-n$REVERB_INPUT_HIGH_SHELF_EQ_frequency \
\
-k$REVERB_OUTPUT_LOW_SHELF_EQ_gain_in_decibels \
-s$REVERB_OUTPUT_LOW_SHELF_EQ_frequency \
-c$REVERB_OUTPUT_HIGH_SHELF_EQ_gain_in_decibels \
-G$REVERB_OUTPUT_HIGH_SHELF_EQ_frequency \
\
-w$print_amplitude_statistics_0_no__1_yes \
-i$amplitude_statistics_time_interval \
\
-_$auto_output_sound_file_play__repetitions \
-=$rescale_level_in_decibels \
\
"
echo "$pvroutine $PVFLAGS $input_file $output_file "

$pvroutine  $PVFLAGS $input_file $output_file   ; 
 
#aiffd $output_file ; 

