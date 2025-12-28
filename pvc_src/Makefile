CFLAGS = -g -O -I/opt/local/include
MOLES = plainpv \
	readheader pvanalysis	groupdelaymaker reshape \
	envelope centroid fluxoid freqresponse filtresponsemaker \
     	chordresponsemaker peakformant filter ring twarp \
	ringfilter ringtvfilter spectwarper compander \
	tvfilter filtdeviator spectralextractor \
	chordmapperplus noisefilter \
	amptodB dBtoamp  Hztopitch pitchtoHz \
	tvfiltdeviator formantsmapper harmonizer \
	inharmonator delayfilter \
	pitchtracker convolver impulseresponse \
	irconvolver irconvolvesequencer  \
	channelmix channelcollect mixfiles \
	ratechanger specflattracker \
	roomresponsemaker roomresponsesequencer


LIBDIR = ../PVC_LIB
DESTDIR = ../bin
#LDFLAGS = -lpvoc -lm
LDFLAGS =  -lsndfile -lpvoc -L/opt/local/lib/


lib:		
		(cd ../PVC_LIB ; make clean; make ; cd ../PVC_SRC)

all:		$(MOLES)

clean:
		-rm -f *.o core $(MOLES)

install:	$(DESTDIR)
		strip $(MOLES)
		cp $(MOLES) $(DESTDIR)

formantsmapper:	 formantsmapper.o
		cc -o formantsmapper $(CFLAGS)  formantsmapper.o -L${LIBDIR} $(LDFLAGS)
formants:	 formants.o
		cc -o formants $(CFLAGS)  formants.o -L${LIBDIR} $(LDFLAGS)
transients:	 transients.o
		cc -o transients $(CFLAGS)  transients.o -L${LIBDIR} $(LDFLAGS)
plainpv:	 plainpv.o
		cc -o plainpv $(CFLAGS)  plainpv.o -L${LIBDIR} $(LDFLAGS)
harmonizer:	 harmonizer.o
		cc -o harmonizer $(CFLAGS)  harmonizer.o -L${LIBDIR} $(LDFLAGS)
inharmonator:	 inharmonator.o
		cc -o inharmonator $(CFLAGS)  inharmonator.o -L${LIBDIR} $(LDFLAGS)
chordmapperplus:	 chordmapperplus.o
		cc -o chordmapperplus $(CFLAGS)  chordmapperplus.o -L${LIBDIR} $(LDFLAGS)
chordmappersuperplus:	 chordmappersuperplus.o
		cc -o chordmappersuperplus $(CFLAGS)  chordmappersuperplus.o -L${LIBDIR} $(LDFLAGS)
freqresponse:	 freqresponse.o
		cc -o freqresponse $(CFLAGS)  freqresponse.o -L${LIBDIR} $(LDFLAGS)
filter:		 filter.o
		cc -o filter $(CFLAGS)  filter.o -L${LIBDIR} $(LDFLAGS)
compander:	compander.o
		cc -o compander $(CFLAGS)  compander.o -L${LIBDIR} $(LDFLAGS)
pvanalysis:	pvanalysis.o
		cc -o pvanalysis $(CFLAGS)  pvanalysis.o -L${LIBDIR} $(LDFLAGS)
readheader:		 readheader.o
		cc -o readheader $(CFLAGS)  readheader.o -L${LIBDIR} $(LDFLAGS)
tvfilter:		 tvfilter.o
		cc -o tvfilter $(CFLAGS)  tvfilter.o -L${LIBDIR} $(LDFLAGS)
convolver:		 convolver.o
		cc -o convolver $(CFLAGS)  convolver.o -L${LIBDIR} $(LDFLAGS)
chordresponsemaker:		 chordresponsemaker.o
		cc -o chordresponsemaker $(CFLAGS)  chordresponsemaker.o -L${LIBDIR} $(LDFLAGS)
ring:		 ring.o
		cc -o ring $(CFLAGS)  ring.o -L${LIBDIR} $(LDFLAGS)
ringfilter:		 ringfilter.o
		cc -o ringfilter $(CFLAGS)  ringfilter.o -L${LIBDIR} $(LDFLAGS)
ringtvfilter:		 ringtvfilter.o
		cc -o ringtvfilter $(CFLAGS)  ringtvfilter.o -L${LIBDIR} $(LDFLAGS)
filtdeviator:		 filtdeviator.o
		cc -o filtdeviator $(CFLAGS)  filtdeviator.o -L${LIBDIR} $(LDFLAGS)
tvfiltdeviator:		 tvfiltdeviator.o
		cc -o tvfiltdeviator $(CFLAGS)  tvfiltdeviator.o -L${LIBDIR} $(LDFLAGS)
twarp:		 twarp.o
		cc -o twarp $(CFLAGS)  twarp.o -L${LIBDIR} $(LDFLAGS)
pitchtracker:		 pitchtracker.o
		cc -o pitchtracker $(CFLAGS)  pitchtracker.o -L${LIBDIR} $(LDFLAGS)
newpitchtracker:		 newpitchtracker.o
		cc -o newpitchtracker $(CFLAGS)  newpitchtracker.o -L${LIBDIR} $(LDFLAGS)
noisefilter:		 noisefilter.o
		cc -o noisefilter $(CFLAGS)  noisefilter.o -L${LIBDIR} $(LDFLAGS)
spectralextractor:	spectralextractor.o
		cc -o spectralextractor $(CFLAGS)  spectralextractor.o -L${LIBDIR} $(LDFLAGS)
envelope:		 envelope.o
		cc -o envelope $(CFLAGS)  envelope.o -L${LIBDIR} $(LDFLAGS)
reshape:		 reshape.o
		cc -o reshape $(CFLAGS)  reshape.o -L${LIBDIR} $(LDFLAGS)
centroid:		 centroid.o
		cc -o centroid $(CFLAGS)  centroid.o -L${LIBDIR} $(LDFLAGS)
peakformant:		 peakformant.o
		cc -o peakformant $(CFLAGS)  peakformant.o -L${LIBDIR} $(LDFLAGS)
fluxoid:		 fluxoid.o
		cc -o fluxoid $(CFLAGS)  fluxoid.o -L${LIBDIR} $(LDFLAGS)
spectwarper:		 spectwarper.o
		cc -o spectwarper $(CFLAGS)  spectwarper.o -L${LIBDIR} $(LDFLAGS)
filtresponsemaker:	filtresponsemaker.o
		cc -o filtresponsemaker $(CFLAGS)  filtresponsemaker.o -L${LIBDIR} $(LDFLAGS)
delayfilter:   		delayfilter.o
		cc -o delayfilter $(CFLAGS)  delayfilter.o -L${LIBDIR} $(LDFLAGS)
groupdelaymaker:   	groupdelaymaker.o
		cc -o groupdelaymaker $(CFLAGS)  groupdelaymaker.o -L${LIBDIR} $(LDFLAGS)

amptodB:		 amptodB.o
		cc -o amptodB $(CFLAGS)  amptodB.o -L${LIBDIR} $(LDFLAGS)
dBtoamp:		 dBtoamp.o
		cc -o dBtoamp $(CFLAGS)  dBtoamp.o -L${LIBDIR} $(LDFLAGS)
Hztopitch:		 Hztopitch.o
		cc -o Hztopitch $(CFLAGS)  Hztopitch.o -L${LIBDIR} $(LDFLAGS)
pitchtoHz:		 pitchtoHz.o
		cc -o pitchtoHz $(CFLAGS)  pitchtoHz.o -L${LIBDIR} $(LDFLAGS)

impulseresponse:     impulseresponse.o
		cc -o impulseresponse $(CFLAGS)  impulseresponse.o -L${LIBDIR} $(LDFLAGS)
irconvolver:     irconvolver.o
		cc -o irconvolver $(CFLAGS)  irconvolver.o -L${LIBDIR} $(LDFLAGS)
irconvolvesequencer:     irconvolvesequencer.o
		cc -o irconvolvesequencer $(CFLAGS)  irconvolvesequencer.o -L${LIBDIR} $(LDFLAGS)
channelmix:     channelmix.o
		cc -o channelmix $(CFLAGS)  channelmix.o -L${LIBDIR} $(LDFLAGS)
channelcollect:     channelcollect.o
		cc -o channelcollect $(CFLAGS)  channelcollect.o -L${LIBDIR} $(LDFLAGS)
mixfiles:     		mixfiles.o
		cc -o mixfiles $(CFLAGS)  mixfiles.o -L${LIBDIR} $(LDFLAGS)
ratechanger:		ratechanger.o
		cc -o ratechanger $(CFLAGS)  ratechanger.o -L${LIBDIR} $(LDFLAGS)
specflattracker:		specflattracker.o
		cc -o specflattracker $(CFLAGS)  specflattracker.o -L${LIBDIR} $(LDFLAGS)
roomresponsemaker:		roomresponsemaker.o
		cc -o roomresponsemaker $(CFLAGS)  roomresponsemaker.o -L${LIBDIR} $(LDFLAGS)
roomresponsesequencer:		roomresponsesequencer.o
		cc -o roomresponsesequencer $(CFLAGS)  roomresponsesequencer.o -L${LIBDIR} $(LDFLAGS)

