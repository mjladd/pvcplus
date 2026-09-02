{\rtf1\ansi\ansicpg1252\cocoartf1265\cocoasubrtf210
\cocoascreenfonts1{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;\red0\green0\blue0;\red0\green0\blue191;\red96\green96\blue96;
\red191\green0\blue0;}
\pard\tx560\tx1120\tx1680\tx2240\tx2800\tx3360\tx3920\tx4480\tx5040\tx5600\tx6160\tx6720\pardirnatural

\f0\fs24 \cf2 (\
\
\cf3 var\cf2  addCommand = \{ \cf3 arg\cf2  command ;\
	command = command.replace( \cf4 "\\n"\cf2 , \cf4 " "\cf2  ).replace( \cf4 "\\t"\cf2 , \cf4 " "\cf2  ) ;  \
	commands = commands ++ command ++ \cf4 " \\n"\cf2  ; \
\} ; \
\
\
\
\cf3 var\cf2  runCommands = \{\
	\cf3 var\cf2  homeDirectory ; \
	homeDirectory =  \cf4 "HOME"\cf2 .getenv ;\
	commands = commands.replace( \cf4 "~/"\cf2 , homeDirectory ++ \cf4 "/"\cf2  ) ;\
	commands.replace(  \cf4 "\\n"\cf2 , \cf4 " ;\\n\\n"\cf2  ).postln  ; \
	commands.runInTerminal ; \
\} ; \
\
\cf3 var\cf2  commands=\cf4 ""\cf2  ;\
\
\cf3 var\cf2  ring = \{ \cf3 arg\cf2  iterations=1 ; \
\
	\cf3 var\cf2  temp,\
		dataString, \
		commandLines=\cf4 ""\cf2 , \
		reverb_input_low_shelf_eq_gain_in_decibels, \
		reverb_random_amplitude_deviation__response_time_in_seconds, \
		reverb_feedback_high_shelf_eq_gain_in_decibels, \
		windowsize, \
		reverb_gain_in_decibels, \
		frames_per_second, \
		reverb_output_low_shelf_eq_frequency, \
		window_type, \
		reverb_input_high_shelf_eq_gain_in_decibels, \
		reverb_input_low_shelf_eq_frequency, \
		amplitude_statistics_time_interval, \
		output_channel, \
		reverb_random_amplitude_deviation_floor_in_decibels, \
		reverb_pitch_transposition_in_semitones, \
		reverb_feedback_low_shelf_eq_frequency, \
		reverb_random_deviation_response_time_in_seconds, \
		reverb_frequency_shift, \
		reverb_feedback_high_shelf_eq_frequency, \
		master_gain_in_decibels, \
		auto_output_sound_file_play__repetitions, \
		reverb_threshold_passmode__above_1_below_0, \
		reverb_feedback_eq_signal_balance_gain_limiter_in_db, \
		reverb_random_freq_deviation_in_hz, \
		begintime, \
		reverb_output_high_shelf_eq_gain_in_decibels, \
		endtime, \
		source_frequency_shift, \
		reverb_output_high_shelf_eq_frequency, \
		fft_length, \
		reverb_threshold_in_db, \
		input_file, \
		print_amplitude_statistics_0_no__1_yes, \
		reverb_input_high_shelf_eq_frequency, \
		reverb_feedback_eq_decay_time_in_seconds, \
		reverb_decay_time_in_seconds, \
		oscillator_resynthesis_threshold_in_db, \
		reverb_feedback_low_shelf_eq_gain_in_decibels, \
		reverb_input_envelope_attack_time_in_seconds, \
		source_gain_in_decibels, \
		source_pitch_transposition_in_semitones, \
		reverb_output_low_shelf_eq_gain_in_decibels, \
		output_file, \
		rescale_level_in_decibels, \
		time_expansion_contraction_factor, \
		reverb_input_envelope_release_time_in_seconds\
	; \
		\cf5 // *** INTERNAL FUNCTION FOR SETTING ROUTINE VARIABLES ***\cf2 \
\
	\cf3 var\cf2  setRoutineVariables = \{\cf3 arg\cf2  iteration=0 ;  \
\
 		\cf5 //\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\cf2 \
		\cf5 //******************************************************\cf2 \
		\cf5 //.................... RING ............................\cf2 \
		\cf5 //******************************************************\cf2 \
\
		\cf5 //******************** OUTPUT **************************\cf2 \
\
		output_file = \cf4 "/tmp/output.au"\cf2  ;\
\
		\cf5 // The following may be used to copy a formatted\cf2 \
		\cf5 // sound file into the output location listed above.   \cf2 \
\
		\cf5 //cp ~/32bit.BLANK.au $output_file\cf2 \
\
		\cf5 //...............AUTO-PLAY OF OUTPUT FILE...............\cf2 \
		auto_output_sound_file_play__repetitions = 0 ;\
		\cf5 //(-2 = interactive: Play once, then prompts for more.)\cf2 \
		\cf5 //(-1 = interactive: Prompt for each play of file.)\cf2 \
		\cf5 //( 0 = off)\cf2 \
		\cf5 //( 1 or greater = Auto-repeat for specified repetitions.)\cf2 \
\
		\cf5 //................ RESCALE .............................\cf2 \
\
		rescale_level_in_decibels = -0 ;\
		\cf5 //(-96 to 0 dB. )\cf2 \
		\cf5 //(Set to 1 to rescale to peak of input file.)\cf2 \
		\cf5 //(Set to 2 to bypass rescale.)\cf2 \
\
\
		\cf5 //******************** INPUT ***************************\cf2 \
\
		input_file = \cf4 "~/voice.NF.aiff"\cf2  ;\
\
\
		\cf5 //........ BEGIN/END TIMES .............................\cf2 \
\
		begintime = 0 ;\
		endtime = 0 ;\
		\cf5 // (0 end time defaults to end of file)\cf2 \
\
		\cf5 //======================================================\cf2 \
		\cf5 //*** ANALYSIS PARAMETERS ******************************\cf2 \
\
		fft_length = 4096 ;\
		window_type = 16 ;\
		windowsize = 0 ;\
\
		\cf5 //( Use larger FFT sizes for ring routines.)\cf2 \
\
		frames_per_second = 200 ;\
\
		\cf5 //======================================================\cf2 \
		\cf5 //*** RESYNTHESIS PARAMETERS ***************************\cf2 \
\
\
		\cf5 //........... OUTPUT CHANNEL(S) .......................\cf2 \
\
		output_channel = 0 ;\
		\cf5 // (channels are numbered from 1-maximum)\cf2 \
		\cf5 // (0 = all channels)\cf2 \
\
		\cf5 //.............OSCIL THRESHOLD ........................\cf2 \
\
		oscillator_resynthesis_threshold_in_db = -96 ;\
\
		\cf5 //****************** MODIFICATIONS *********************\cf2 \
\
		\cf5 //.................. TIME ..............................\cf2 \
\
		time_expansion_contraction_factor = 1 ;\
\
		\cf5 //.........  MASTER DECIBELS (source + reverb) .........\cf2 \
\
		master_gain_in_decibels = 0 ;\
\
		\cf5 //******************************************************\cf2 \
		\cf5 //.......... SOURCE PARAMETERS .........................\cf2 \
		\cf5 //******************************************************\cf2 \
\
		\cf5 //.................. DECIBELS ..........................\cf2 \
\
		source_gain_in_decibels = -100 ;\
\
		\cf5 //.................. PITCH .............................\cf2 \
\
		source_frequency_shift = 0 ;\
		source_pitch_transposition_in_semitones = 0 ;\
\
		\cf5 //******************************************************\cf2 \
		\cf5 //.......... REVERB PARAMETERS ........................\cf2 \
		\cf5 //******************************************************\cf2 \
\
		\cf5 //...REVERB OUTPUT CONTROLS ** y(n) ***...............\cf2 \
\
		\cf5 //.................. DECIBELS ..........................\cf2 \
		reverb_gain_in_decibels = 0 ;\
\
		\cf5 //.................. PITCH .............................\cf2 \
		reverb_frequency_shift = 0 ;\
		reverb_pitch_transposition_in_semitones = 0 ;\
\
\
		\cf5 //This control is for tuning the FFT analysis/synthesis \cf2 \
		\cf5 // freq. \cf2 \
		\cf5 //-----------------------------------------------------\cf2 \
		\cf5 //........... OUTPUT EQ .............................. \cf2 \
		reverb_output_low_shelf_eq_gain_in_decibels = 0 ;\
		reverb_output_low_shelf_eq_frequency = 1000 ;\
		reverb_output_high_shelf_eq_gain_in_decibels = -0 ;\
		reverb_output_high_shelf_eq_frequency = 4000 ;\
		\cf5 //-----------------------------------------------------\cf2 \
\
		\cf5 //-----------------------------------------------------\cf2 \
		\cf5 //-------------> RANDOM DEVIATION CONTROLS <----------\cf2 \
		\cf5 //......... RANDOM  FREQUENCY DEVIATION.................\cf2 \
		reverb_random_freq_deviation_in_hz = 50 ;\
		\cf5 //( This is the maximum deviation added to the bin)\cf2 \
		reverb_random_deviation_response_time_in_seconds = 0.01 ;\
\
		\cf5 //(this low pass filters the change, slowing it; .1 is good.)\cf2 \
\
		\cf5 //......... RANDOM  AMPLITUDE DEVIATION...............\cf2 \
\
		reverb_random_amplitude_deviation_floor_in_decibels = -0 ;\
		\cf5 //(0 (none) to -96 (full range) )\cf2 \
		reverb_random_amplitude_deviation__response_time_in_seconds = 0.01 ;\
		\cf5 //(this low pass filters the change, slowing it.)\cf2 \
		\cf5 //-----------------------------------------------------\cf2 \
\
		\cf5 //=====================================================\cf2 \
\
		\cf5 //...REVERB INPUT CONTROLS ** x(n) ***..................\cf2 \
\
		\cf5 //........... INPUT THRESHOLD CONTROLS .................\cf2 \
\
		reverb_threshold_in_db = -30 ;\
		reverb_threshold_passmode__above_1_below_0 = 1 ;\
\
		\cf5 // The source bins are input into the reverb path depending\cf2 \
		\cf5 //  on whether they lie above or below the threshold. The\cf2 \
		\cf5 //  passmode determines whether to pass bins above or below \cf2 \
		\cf5 // the threshold. For harmonic sounds,  passing above -20\cf2 \
		\cf5 //  works well. For some noisy sounds,  passing stuff below\cf2 \
		\cf5 // -20 can be interesting. \cf2 \
\
		\cf5 //------------------------------------------------------\cf2 \
		\cf5 //*** REVERB INPUT x(n) ENVELOPE RESPONSE\cf2 \
\
		reverb_input_envelope_attack_time_in_seconds = 0.0 ;\
		reverb_input_envelope_release_time_in_seconds = 0.0 ;\
\
		\cf5 //-----------------------------------------------------\cf2 \
		\cf5 //........... INPUT EQ .............................. \cf2 \
		reverb_input_low_shelf_eq_gain_in_decibels = -0 ;\
		reverb_input_low_shelf_eq_frequency = 500 ;\
		reverb_input_high_shelf_eq_gain_in_decibels = -0 ;\
		reverb_input_high_shelf_eq_frequency = 1000 ;\
		\cf5 //-----------------------------------------------------\cf2 \
\
		\cf5 //=====================================================\cf2 \
		\cf5 //...REVERB FEEDBACK CONTROLS ** y(n-1) ***............\cf2 \
\
		reverb_decay_time_in_seconds = 4 ;\
		\cf5 //gen4 -L1000 0 1 2    1 12 > /tmp/rtime\cf2 \
		\cf5 //( This is a master decay time control.)\cf2 \
\
		\cf5 //...................................................... \cf2 \
\
		reverb_feedback_eq_decay_time_in_seconds = 4 ;\
		\cf5 //( This controls the decay into the feedback eq.)\cf2 \
\
\
		\cf5 //-----------------------------------------------------\cf2 \
		\cf5 //........... FEEDBACK_EQ .............................. \cf2 \
		reverb_feedback_low_shelf_eq_gain_in_decibels = -0 ;\
		reverb_feedback_low_shelf_eq_frequency = 100 ;\
		reverb_feedback_high_shelf_eq_gain_in_decibels = -0 ;\
		reverb_feedback_high_shelf_eq_frequency = 500 ;\
		\cf5 //-----------------------------------------------------\cf2 \
\
		\cf5 //............BALANCE...................................\cf2 \
		reverb_feedback_eq_signal_balance_gain_limiter_in_db = 0 ;\
		\cf5 //(0 to 96dB, 0 = balancing off full limiting), \cf2 \
		\cf5 //               96 = balance full-on (no limiting)\cf2 \
\
		\cf5 //******************************************************\cf2 \
		\cf5 //********** AMPLITUDE STATISTICS ********************** \cf2 \
		print_amplitude_statistics_0_no__1_yes = 1 ;\
		amplitude_statistics_time_interval = 1 ;\
\
\
		\cf5 //======================================================\cf2 \
		\cf5 //========= SCRATCH SPACE ==============================\cf2 \
		\cf5 //======================================================\cf2 \
\
\
\
\
		\cf5 //====================================================\cf2 \
 		\cf5 //\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\'95\cf2 \
\
	\} ;\
	\cf5 //*********************************************************************************************************\cf2 \
	\cf5 //************* COMMAND LINE SETUP -- OFFICE USE ONLY ***************************************\cf2 \
	\cf5 //************* DO NOT CHANGE ANYTHING IN THE FUNCTION BELOW THIS LINE ***************\cf2 \
	\cf5 //*********************************************************************************************************\cf2 \
\
	iterations.do(\{\cf3 arg\cf2  n; \
		setRoutineVariables.value( iteration: n ) ; \
\
		\cf5 // ******** UNIX COMMAND LINE 0 ***********\cf2 \
		addCommand.value(\
\
					\cf5 // * ROUTINE * \cf2 \
			\cf4 "ring "\cf2  ++ \cf4 " "\cf2  ++\
\
				\cf5 // * FLAGS * \cf2 \
			 \cf4 " -w"\cf2  ++ print_amplitude_statistics_0_no__1_yes ++  \cf4 " -S"\cf2  ++ source_gain_in_decibels ++ \
			 \cf4 " -V"\cf2  ++ reverb_threshold_passmode__above_1_below_0 ++  \cf4 " -e"\cf2  ++ endtime ++ \
			 \cf4 " -T"\cf2  ++ reverb_feedback_eq_decay_time_in_seconds ++  \cf4 " -m"\cf2  ++ reverb_feedback_high_shelf_eq_frequency ++ \
			 \cf4 " -="\cf2  ++ rescale_level_in_decibels ++  \cf4 " -H"\cf2  ++ reverb_frequency_shift ++ \
			 \cf4 " -Z"\cf2  ++ reverb_decay_time_in_seconds ++  \cf4 " -X"\cf2  ++ reverb_feedback_low_shelf_eq_gain_in_decibels ++ \
			 \cf4 " -p"\cf2  ++ source_pitch_transposition_in_semitones ++  \cf4 " -W"\cf2  ++ window_type ++ \
			 \cf4 " -f"\cf2  ++ source_frequency_shift ++  \cf4 " -A"\cf2  ++ master_gain_in_decibels ++  \cf4 " -N"\cf2  ++ fft_length ++ \
			 \cf4 " -l"\cf2  ++ reverb_input_envelope_attack_time_in_seconds ++  \cf4 " -E"\cf2  ++ reverb_feedback_eq_signal_balance_gain_limiter_in_db ++ \
			 \cf4 " -s"\cf2  ++ reverb_output_low_shelf_eq_frequency ++  \cf4 " -t"\cf2  ++ oscillator_resynthesis_threshold_in_db ++ \
			 \cf4 " -O"\cf2  ++ reverb_input_low_shelf_eq_gain_in_decibels ++  \cf4 " -C"\cf2  ++ output_channel ++ \
			 \cf4 " -Q"\cf2  ++ reverb_feedback_high_shelf_eq_gain_in_decibels ++  \cf4 " -b"\cf2  ++ begintime ++ \
			 \cf4 " -I"\cf2  ++ time_expansion_contraction_factor ++  \cf4 " -D"\cf2  ++ frames_per_second ++ \
			 \cf4 " -U"\cf2  ++ reverb_feedback_low_shelf_eq_frequency ++  \cf4 " -Y"\cf2  ++ reverb_input_high_shelf_eq_gain_in_decibels ++ \
			 \cf4 " -k"\cf2  ++ reverb_output_low_shelf_eq_gain_in_decibels ++  \cf4 " -P"\cf2  ++ reverb_pitch_transposition_in_semitones ++ \
			 \cf4 " -z"\cf2  ++ reverb_threshold_in_db ++  \cf4 " -c"\cf2  ++ reverb_output_high_shelf_eq_gain_in_decibels ++ \
			 \cf4 " -n"\cf2  ++ reverb_input_high_shelf_eq_frequency ++  \cf4 " -M"\cf2  ++ windowsize ++  \cf4 " -j"\cf2  ++ reverb_random_freq_deviation_in_hz ++ \
			 \cf4 " -F"\cf2  ++ reverb_gain_in_decibels ++  \cf4 " -d"\cf2  ++ reverb_input_low_shelf_eq_frequency ++ \
			 \cf4 " -_"\cf2  ++ auto_output_sound_file_play__repetitions ++  \cf4 " -K"\cf2  ++ reverb_random_deviation_response_time_in_seconds ++ \
			 \cf4 " -q"\cf2  ++ reverb_random_amplitude_deviation__response_time_in_seconds ++  \cf4 " -G"\cf2  ++ reverb_output_high_shelf_eq_frequency ++ \
			 \cf4 " -L"\cf2  ++ reverb_input_envelope_release_time_in_seconds ++  \cf4 " -i"\cf2  ++ amplitude_statistics_time_interval ++ \
			 \cf4 " -x"\cf2  ++ reverb_random_amplitude_deviation_floor_in_decibels\
\
				\cf5 // * INPUT/OUTPUT FILES * \cf2 \
			 ++  \cf4 " "\cf2   ++ input_file ++ \cf4 "  "\cf2  ++  \cf4 " "\cf2   ++ output_file\
\
		) ;\
 \
\
\
	\}) ;\
\} ;\
\
\
\
\cf5 /////////////////////////////////////////////////////////////////////////////////////\cf2 \
\cf5 //         ---------- FUNCTION CALLS  ----------            \cf2 \
\cf5 /////////////////////////////////////////////////////////////////////////////////////\cf2 \
\
\
\cf5 // *** CLEAR SCREEN ***\cf2 \
\cf3 Document\cf2 .listener.string=\cf4 ""\cf2 ; \
\
ring.value( iterations: 1 ) ; \
\
runCommands.value ;\
\
\
\cf4 "..........DONE!"\cf2  \
)\
}