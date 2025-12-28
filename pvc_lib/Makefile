CFLAGS  = -g -O -I/opt/local/include
#-lsndfile 
MOLES   = libpvoc
LIB = libpvoc.a
DESTDIR = ../src
LDFLAGS = -lm -L/opt/local/lib/
BINARIES =  convert.o convert_nounwrap.o crack.o fft.o fold.o leanconvert.o \
leanunconvert.o makewindows.o noscbank.o overlapadd.o \
fileio.o \
shiftin.o shiftout.o unconvert.o getthresh.o space.o \
rands.o \
stripheader.o curve.o fval.o \
crackstring.o crackstring_bin_only.o crackstring_noprint.o fillfunc.o eq.o eq2.o \
spectmagwarp.o spectmagwarp2.o normalize.o \
noscbank2.o noscbank3.o compress.o pr.o \
smoothspec.o tprintspec.o \
RIfindpeak.o noscbank_extra.o invertresponse.o \
miscellania.o tprintspec_groupdelay.o smooth.o CartesianSmooth.o smoothfreqs.o smoothnew.o \
smooth_amp_change.o smooth_zero.o \
find_peak_amp.o compand.o make_freqresponse.o \
threshold_limit.o spectmagwarp_inharm.o find_centroid.o find_fluxoid.o \
cut_data_lines.o eq3.o eq4.o AnalyzePartials.o \
fixTildeInFilename.o FileTest_ASCIIorBinaryFloat.o findFreqOfPeakFormant.o \
funcStats.o makeInterpolatedFilterFrame.o makeFilterFrame.o getFormants.o \
writeSpectrumPlotFile.o NormalizeToPeaksOfSpectrumInBand.o \
findFilterTimeAndConstrainByWindow.o  \
normalizeLoopAmplitudes.o makeLoopSmoothTime.o cutDC.o OPPC_to_Hz.o  Hz_to_OPPC.o \
windows.o findFuncMinMaxAvg.o \
getGlobalFunctionValues.o \
setupToneBankOrPartialFunctionValues.o crackfloat.o autoplay.o filesToRemove.o \
printUnixCommandLine.o findArrayStats.o findFuncStats.o Hz_to_MIDI.o findMode.o sortArray.o \
phaselock.o find_spectralflatness.o






#readin.o openfile.o mapp.o

all:    $(MOLES)

clean:
	-rm -f a.out *.o libpvoc.a


libpvoc:      $(BINARIES)
	-ld -x -r  $(BINARIES)
	-ar r $(LIB) $(BINARIES)
	-ranlib -s $(LIB)

