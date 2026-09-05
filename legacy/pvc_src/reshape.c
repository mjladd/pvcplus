#include "globals.h"
#include "time.h"

// THINGS TO ADD:
//	PROCESS SIGNAL ACCORDING TO FEEDBACK FILTER COEFFICIENTS STORED IN FILE


#define QUICK_PLOT_X 120
#define QUICK_PLOT_Y 30

#define WIDTH 132

#define re_fvec( name, size )\
if ( ( name = (float *) realloc( name, size * sizeof(float) ) ) == NULL) {\
    fprintf( stderr, "Insufficient memory\n" );\
    exit( -1 );\
}
#define re_ivec( name, size )\
if ( ( name = (int *) realloc( name, size * sizeof(int) ) ) == NULL) {\
    fprintf( stderr, "Insufficient memory\n" );\
    exit( -1 );\
}

void usage() ; 
void pd( int i ) ;

float makeFuncProp(
	int computationLevel,
	int numberOfValues,
	int numberOfWindows,
	int windowLengths[],
	int window,
	int i,
	int nn
) ; 


void singleOperator( 
	char ch,  
	float value[], 
	float scratchValue[],
	int numberOfValues, 
	struct func *operator,
	float flagValue,
	int silentflag
) ; 


void mapIntoNewBoundaries( 
	float value[], 
	int numberOfValues, 
	struct func *lowbound,	
	struct func *hibound,
	int *lowBoundFlag,
	int *hiBoundFlag
);

void lowpassFilter( 
	float value[],
	int numberOfValues, 
	struct func *feedcu, 
	struct func *feedcd, 
	int *positiveFeedFlag,
	int *negativeFeedFlag 
) ; 

int reshapeDebugFlag=0 ; 

int main( argc, argv )
    int argc ; char *argv[] ;
{
int i,j,  k, l, m, numberOfSplineValues, numberOfScrollingPlotValues  ;
  int   eof = 0, obank = 0,  sflag = 0;
 float P = 1.0;
  FILE *fopen(), *fp, *adata ;
char ch, chm1, chm2 ;

float lowlimitsave, hilimitsave ; 
int lowlimitsaveflag=0, hilimitsaveflag=0 ; 

SF_INFO outputSFinfo ; 

time_t rawtime;


int processedOutputFlag=0, statsprintflag=0 ; 

int numberOfValues=0, newNumberOfValues, lowBoundFlag=0, hiBoundFlag=0 ; 
int positiveFeedFlag=0, negativeFeedFlag=0 ; 
int *scratchWindowOrder=NULL, *scratchWindowBeginIndices=NULL, *scratchWindowLengths=NULL ; 

int quickploty=0 ; 

int *quickplot=NULL, *quickplotwindowmarkers=NULL ; 

float oldval ; 

float arraylow, arrayhi, arrayrange, arrayaverage, arraysum, arraymedian, arraymode, 
	arraystandarddeviation, arraybegin, arrayend, arraymiddle ; 
int arraylength, steps, stepstotake ; 

int beginValNumberingFlag=0, endValNumberingFlag=0, numberOfReshapeFunctionValues ; 

float threshold, *randomWindowSegment=NULL, funcprop ; 

int soundfileWriteFlag=0, *indices=NULL ;

float  temp,  temp2, newaverage, newsum,  temp3, temp4,    old_temp=0. ,  n=0.,  
    N,  newhi,  newlow,  newdiff, dummyVal ;  

float low, hi, range, average, median, mode, standarddeviation, sum, begin, end, middle ; 

float randomWindowDivisionFactor, *windowAverages=NULL, *scratchWindowAverages=NULL  ; 
int windowSize, windowSizeNow, window, windowBeginIndex, *scratchWindowAveragesIndices=NULL ; 
float duplicationFactor ; 
int lowIndex, hiIndex ; 
float peakbinamp = 0.,  avgbinamp=0.,  peakamp, peakenvamp=0. ;
int   frame_count=0,  flag=1,  ASCIIoutflag=-1,  ASCIIinflag=0,  silentflag=1 ; 
float   IR,  dur=0.,   t=0.;
char tempstring[ STRING_SIZE ], inputScratchFileName[ STRING_SIZE ], csplineScratchFileName[ STRING_SIZE ], csplineCommand[ STRING_SIZE ] ;
char reshapeFunctionFilename[ STRING_SIZE ] ; 
 
FILE *fscratch=NULL,  *data=NULL,  *scalefile=NULL, *reshapefile=NULL, *inputScratchFile=NULL, *csplineScratchFile=NULL ; 
char scratch[ STRING_SIZE ], values[ STRING_SIZE ],   scalefilename[ STRING_SIZE ] = "", *user  ; 
int itty,  invflag=0,  diffmode=0 ; 
float olddiffval,  *value=NULL, *scratchValue=NULL ; 

int scalesize=0,  reshapefilesize, scalefileflag=0,  scaleOriginFlag=0, ii ; 

int lowflag=0,  hiflag=0,  lastflag=0,  numberflag=0, oversampi,  bflag=0  ; 

float vcount=0.,  out_index ; 
int histogramNumberOfBins=100   ;

int   histogramflag=0 ; 

float *histogramBins=NULL ; 

float frac, realIndex ; 

int startFlagindex, endFlagindex, diffType ; 
int *windowBeginIndices=NULL, *windowLengths=NULL, markerCount, windowLength, numberOfWindows=0, 
	computationLevel=0, newNumberOfWindows ; 
float realWindowLength ; 


int nn= 0 ;
int nv = 0 ;  
int firstv=0,  lastv=-1,  lastvflag=0 ; 

float filev,  oldfilev,  oldoutputval=0. ; 
float scalesum=0 ; 

float sortFlag=0. ; 
int notdone ; 

int lowc=0,  hic=0 ; 
int fd,  fd2 ; 
float temporigin ;
//float   *scale=NULL ; 
float   scale[1000] ; 


float ampthresh ; 

float feedcum=1.,  oldVal ; 
float feedcdm=1. ; 
int transCode=-1; 

int find_data_type( ) ; 
int sumflag=0 ; 

float pitch,  reffreq=0.,  log_of_2,  midC ; 

struct func sortIndex ;

struct func windowDivisionFactor ; 

struct  func  dummyStructure ; 

struct func reshapeFunction ; 


// LOW  LIMIT
struct  func  lowlimit ; 

//  HIGH  LIMIT
struct  func  hilimit ; 


// SHAPE INDEX
struct  func  shape ; 

// LOW  BOUND
struct  func  lowbound ; 

//  HIGH  BOUND
struct  func  hibound ; 


//  MULTIPLY
struct  func  multiply ; 

//  DIVIDE
struct  func  divide ; 

//  ADD
struct  func  adder ; 

//  INVERSE NUMERATOR
struct  func  inverse ; 

//  SCALE ORIGIN
struct  func  origin ; 

//  SCALE QUANTIZATION
struct  func  force ; 


//  RESAMPLE PROPORTION
struct  func  ninc ; 


//  SCRATCH FLOAT FILE
struct  func  scratch_file ; 

//  TIME WARP VALUE
struct  func  time_warp ; 

//  OVERSAMPLING
struct  func  oversamp ; 

// FEEDBACK UP
struct  func  feedcu ; 

// FEEDBACK DOWN
struct  func  feedcd ; 



//*****************INITIALIZE


sortIndex.L = 1. ; sortIndex.n = 1. ; sortIndex.A[ 0 ] = 0. ; 

windowDivisionFactor.L = 1. ; windowDivisionFactor.n = 1. ; windowDivisionFactor.A[ 0 ] = 0. ; 

reshapeFunction.L = 1. ; reshapeFunction.n = 1. ; reshapeFunction.A[ 0 ] = 0. ; 


dummyStructure.L = 1. ; dummyStructure.n = 1. ; dummyStructure.A[ 0 ] = 0. ; 

// LOW  LIMIT
lowlimit.L = 1. ; lowlimit.n = 1. ; lowlimit.A[ 0 ] = 0. ; 

//  HIGH  LIMIT
hilimit.L = 1. ; hilimit.n = 1. ; hilimit.A[ 0 ] = 0. ; 


// SHAPE INDEX
shape.L = 1. ; shape.n = 1. ; shape.A[ 0 ] = 0. ; 


// LOW  BOUND
lowbound.L = 1. ; lowbound.n = 0. ; lowbound.A[ 0 ] = 0. ; 

//  HIGH  BOUND
hibound.L = 1. ; hibound.n = 0. ; hibound.A[ 0 ] = 0. ; 

//    MULTIPLY
multiply.L = 1. ; multiply.n = 1. ; multiply.A[ 0 ] = 1. ; 

//  ADD
adder.L = 1. ; adder.n = 1. ; adder.A[ 0 ] = 0. ; 

//    DIVIDE
divide.L = 1. ; divide.n = 1. ; divide.A[ 0 ] = 1. ; 

//    INVERSE NUMERATOR
inverse.L = 1. ; inverse.n = 1. ; inverse.A[ 0 ] = 1. ; 

//    SCALE ORIGIN
origin.L = 1. ; origin.n = 1. ; origin.A[ 0 ] = 0. ; 

//    SCALE QUANTIZATION
force.L = 1. ; force.n = 1. ; force.A[ 0 ] = 1. ; 

//  RESAMPLE PROPORTION
ninc.L = 1. ; ninc.n = 1. ; ninc.A[ 0 ] = 1. ; 

//  SCRATCH FLOAT FILE
scratch_file.L = 1. ; scratch_file.n = 1. ; scratch_file.A[ 0 ] = 0. ; 

//  TIME WARP VALUE
time_warp.L = 1. ; time_warp.n = 1. ; time_warp.A[ 0 ] = 0. ; 

//  OVERSAMPLING
oversamp.L = 1. ; oversamp.n = 1. ; oversamp.A[ 0 ] = 1. ; 


// FEEDBACK UP
feedcu.L = 1. ; feedcu.n = 1. ; feedcu.A[ 0 ] = 0. ; 

// FEEDBACK DOWN
feedcd.L = 1. ; feedcd.n = 1. ; feedcd.A[ 0 ] = 0. ; 


srand(time(NULL)); 


log_of_2 = (float) log10( (double) 2.0 ) ; 
midC = (float) (220.* pow(2., (3./12.))) ;

ampthresh = (float) pow( 10.,  (double) (-96. / 20.) ) ;
			




//arg_index++ ; 

//startFlagindex = arg_index ; 

    while( (ch= crack( argc, argv, 
	"A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|a|b|c|d|e|f|j|k|l|m|n|o|p|r|s|t|u|w|x|y|z", 0  )) 
		!= CRACK_DONE_FLAG ) {	// g h i q v 
		processedOutputFlag = 1 ; 
		if( ch == 'z' ) silentflag = 0 ;
}


itty = isatty(0);		/* 0 == stdin */


// GET NAME OF USER
user = getlogin(); 


//******************

	
// IF DATA FROM INPUT FILE, THEN OPEN
if( itty ){
	// FROM FILE


	if( argc == arg_index ){
		fprintf( stderr,  "\nNO INPUT OR INPUT FILE.\n\n" ) ;
		usage();
	} ; 

	if( (data = fopen( argv[ arg_index ], "r")) == NULL ){
		fprintf( stderr, "\n%s <-- NO SUCH FILE (input data file). BYE.\n\n",  argv[ arg_index ] ) ; 
		exit(EXIT_FAILURE); 
	}else{
		fclose( data ) ; 
        	// TEST TO SEE IF ASCII OR BINARY FLOAT.
		ASCIIinflag = FileTest_ASCIIorBinaryFloat ( argv[ arg_index ], silentflag ); 
		data = fopen( argv[ arg_index ], "r") ;  
		if( silentflag ) fprintf( stderr, "\nINPUT FILE: %s",  argv[ arg_index ] ) ; 
    }
}else{
	// FROM PIPE: WRITE TO SCRATCH FILE, CLOSE AND THEN RE-OPEN WITH "data" POINTER. 
    data = stdin ; 

    sprintf( inputScratchFileName, "/tmp/inputScratchFile.%s.%d", user, (int)(rand()) ) ;
    while ( (inputScratchFile = fopen(inputScratchFileName, "r")) ) {
        fclose(inputScratchFile);
    		sprintf( inputScratchFileName, "/tmp/inputScratchFile.%s.%d", user, (int)(rand()) ) ;    
	} ;
    fclose(inputScratchFile);
	
	filesToRemove( inputScratchFileName, 0 ); 
    inputScratchFile = fopen( inputScratchFileName, "w" ); 
	rewind( inputScratchFile ); 

   k = 0 ; 
    while( fread( &ch, sizeof(char), 1, data ) != 0 ){
 		fwrite( &ch, sizeof(char), 1, inputScratchFile ); k++ ; 
    } ; 
    fclose( inputScratchFile ) ; 

    // TEST TO SEE IF ASCII OR BINARY FLOAT.
    ASCIIinflag = FileTest_ASCIIorBinaryFloat ( inputScratchFileName, silentflag ); 
	// OPEN SCRATCH FILE WITH data FILE POINTER, SAME AS STDIN SET ABOVE.
    data = fopen( inputScratchFileName, "r") ; 
} ; 

if( ASCIIoutflag == -1) {
	// USE FORMAT OF INPUT
	ASCIIoutflag = ASCIIinflag ; 
} ; 


 
// MAKE /tmp ENVELOPE SCRATCH SPACE
// MAKE UNIQUE NAME
    
// OPEN IT

//sprintf( scratch, "/tmp/envelope.%s.%d", user, (int)(rand()) ) ;
//filesToRemove( scratch, 0 );
//fscratch = fopen( scratch, "w" ); 

sprintf( scratch, "/tmp/envelope.%s.%d", user, (int)(rand()) ) ;
while ( (fscratch = fopen( scratch, "r")) ) {
	fclose(fscratch);
//	prs( scratch, "FAILED scratch" ) ; 
    	sprintf( scratch, "/tmp/envelope.%s.%d", user, (int)(rand()) ) ;    
} ;
fclose(fscratch);
//prs( scratch, "SCRATCH FILE NAME FOR PIPED IN VALUES" ) ; 
fscratch = fopen( scratch, "w" ); 
rewind( fscratch ); 
filesToRemove( scratch, 0 );



// 
	
while ( !eof ) {

	if( ! ASCIIinflag ){

		if( fread( &temp, sizeof(float) , 1, data ) == 0 ){
	     	eof = 1 ; 
		}else{
			numberOfValues += 1 ;
			fwrite( &temp, sizeof(float), 1, fscratch );
		}
	}else{
		if( fscanf( data, " %f" , &temp ) == EOF ){
	     	eof = 1 ; 
		} else {
			numberOfValues += 1 ; 			    
			fwrite( &temp, sizeof(float), 1, fscratch );
	    	}
	}
} ; 


fclose( data ) ; 

fclose( fscratch ) ; 
    
if( numberOfValues < 2. ){
	prf( temp, "FILE VALUE" ) ; 
	fprintf(stderr," \t\t\tCANNOT RESHAPE ONE INPUT VALUE. BYE.\n\n" ) ; 
	exit(EXIT_FAILURE); 
}


  
// READ INPUT INTO ARRAY SPACE
// OPEN SCRATCH FILE NEW WAY
fscratch = fopen( scratch, "r" ); 

 
 
 
re_fvec( value, numberOfValues ) ;  re_fvec( scratchValue, numberOfValues ) ;
re_ivec( windowBeginIndices, 1 ); re_ivec( windowLengths, 1 ) ;  

windowBeginIndices[0] = 0 ; windowLengths[0] = numberOfValues ; numberOfWindows = 1 ; computationLevel = 0 ; 


for(i = 0; i < numberOfValues; i++ ) {
	fread( &value[i], sizeof(float), 1, fscratch );   
	if( reshapeDebugFlag  == 1 ) prf( value[i], "value[i]" ) ; 
}; 
fclose( fscratch ); 



if( silentflag )prbanner( "RESHAPE", WIDTH ) ;
if( silentflag )prline( WIDTH, "*" ) ; 

if( silentflag ) prbanner( "INPUT FUNCTION STATISTICS ", WIDTH ) ;
findArrayStats( value, &numberOfValues, 
		&low, &hi, &range, &average, &median, &mode, &standarddeviation, &sum, &begin, &end, &middle, 
			silentflag ) ; 


// FLAG FOR SETTING UP WINDOW MARKERS; Values > 0 AND <= 1 treated as proportion of total number in set ;
// values > 1 as count on window size ; Functions?
// Windowing set to ON with marker setting; VALUES PROCESSED BY WINDOW SETS WITH FUNCTIONS COMPUTED ACCORDING TO
// WINDOW_INDEX / TOTAL_WINDOWS.   

// FLAG FOR TURNING WINDOWING OFF--VALUES PROCESSED AS ON SET WITH FUNCTIONS COMPUTED CONTINUOUSLY ACCORDING TO INDEX POSITION.

chm1 = 'a' ; 
chm2 = 'a' ; 

arg_index = 0 ; 
while( (ch= crack( argc, argv, " A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|a|b|c|d|e|f||h|j|k|l|m|n|o|p|r|s|t|u|w|x|y|z", 0  )) != CRACK_DONE_FLAG ) {
	statsprintflag = 1 ;
	strcpy(tempstring, arg_option);
	if( silentflag ) fprintf( stderr, "\n\n-%c%s ", ch, tempstring ) ;  

	switch(ch) {

		case 'U':		if( silentflag )fprintf( stderr, "WRITING BINARY VALUES TO FILE" ); 
					strcpy(tempstring, arg_option); fixTildeInFilename( tempstring );
					adata = fopen( tempstring, "w" ) ; 
					fwrite( &value[0], sizeof(float), numberOfValues, adata );
					fclose( adata ) ; 
					statsprintflag = 0 ; 
					break ; 

		case 'e':		if( silentflag )fprintf( stderr, "SEEDING RANDOM GENERATOR" );
					srand( crackfloat( arg_option, ch )  ); statsprintflag = 0 ; 
					break ; 

		case 'l':		computationLevel = (int) crackfloat( arg_option, ch ) ; // 0, 1, or 2
					if( computationLevel == 0 ){ 
						if( silentflag ) fprintf( stderr, "INDEXING FUNCTIONS/PROCESSING ACROSS FULL VALUE RANGE ");
					}else if( computationLevel == 1 ){ 
						if( silentflag )fprintf( stderr, "INDEXING FUNCTIONS/PROCESSING AT WINDOW VALUE RANGE");
					}else if( computationLevel == 2 ){ 
						if( silentflag )fprintf( stderr, "INDEXING FUNCTIONS/PROCESSING BY WINDOW BLOCKS");
					}else {
						prt( "\n\nERROR: ILLEGAL WINDOWING FLAG.\n\n. . . BYE.\n\n") ; 
						exit( EXIT_FAILURE ) ; 
					} ; 
					statsprintflag = 0 ;
					break ; 
			 

		case 'L':		if( silentflag )fprintf( stderr, "SETTING UP WINDOW DIVISIONS: " ); 
					strcpy(tempstring, arg_option);
					windowDivisionFactor.fp = crackstring( tempstring, &windowDivisionFactor );
						  
						for(l = 0; l < 2; l++){
							i = 0 ; markerCount = 0 ; realIndex = 0. ; 
							while( (int) realIndex < (numberOfValues - 1)){
								windowDivisionFactor.A[ 0 ] = 
									fval( &windowDivisionFactor, (float) (numberOfValues - 1.), (float) i );
								if(windowDivisionFactor.A[ 0 ] <= 0.){
									prt("\n\nERROR: ILLEGAL WINDOW DIVISION FACTOR.\n\n. . . BYE\n\n" ) ; exit( EXIT_FAILURE ) ; 
								} ; 
								if( windowDivisionFactor.A[ 0 ] <= 1. )
									realWindowLength = (windowDivisionFactor.A[ 0 ] * (float) numberOfValues) ; 
								else
									realWindowLength = windowDivisionFactor.A[ 0 ] ; 
								if( realWindowLength > (float) numberOfValues ) realWindowLength = (float) numberOfValues ;  
								
								if( realWindowLength > ((float)(numberOfValues) - realIndex) ){ 
									realWindowLength = (float)(numberOfValues) - realIndex ; 
								}else{
									if(l == 1){
										windowBeginIndices[markerCount] = (int) realIndex  ; 
									} ; 
									markerCount++ ;  
								} ;
								realIndex += realWindowLength ; 
							} ;
							if(l == 0 ){
								//if( windowBeginIndices != NULL ) free( windowBeginIndices ) ; 
								re_ivec( windowBeginIndices, markerCount ); 
								//if( windowLengths != NULL ) free( windowLengths ) ; 
								re_ivec( windowLengths, markerCount ) ;
								numberOfWindows = markerCount ; 
							} ;   
						} ; 
						for(window = 0 ; window < (numberOfWindows - 1) ; window++ ) 
							windowLengths[ window ] = windowBeginIndices[window + 1] - windowBeginIndices[ window ] ;
						windowLengths[ numberOfWindows - 1 ] = 
							numberOfValues - windowBeginIndices[ numberOfWindows - 1 ] ;

						if( reshapeDebugFlag  == 1  )for(i = 0; i < numberOfWindows; i++){
							fprintf( stderr, "\n%d: windowBeginIndices = %d, windowLengths = %d", 
								i, windowBeginIndices[i], windowLengths[i] ) ;   
						} ; 
						break ;  

		case 'K':		if( silentflag )fprintf( stderr, "RANDOMIZING ORDER ");
					if( computationLevel == 0){
							if( silentflag )fprintf( stderr, " AT COMPUTATION LEVEL 0" ) ; 
							//if( randomWindowSegment != NULL ) free( randomWindowSegment ) ; 
							re_fvec( randomWindowSegment, numberOfValues ) ;   
							for( k = 0; k < numberOfValues; k++) randomWindowSegment[k] = value[k] ;  
							for( k = 0, windowSizeNow = numberOfValues; k < numberOfValues; k++, windowSizeNow--){
								l = rand() % windowSizeNow ; 
								scratchValue[k] =  randomWindowSegment[l] ;
								for(nn = l + 1; nn < windowSizeNow; nn++) 
									randomWindowSegment[nn - 1] = randomWindowSegment[nn] ; 
							} ; 
						for(i = 0; i < numberOfValues; i++) value[i] = scratchValue[i] ;
						break ;   
					} ; 					
					if( computationLevel == 1){
						if( silentflag )fprintf( stderr, " AT COMPUTATION LEVEL 1" ) ; 
						for( window = 0; window < numberOfWindows ; window++ ){
							windowSize = windowLengths[ window ]  ; windowBeginIndex = windowBeginIndices[ window ] ;  
							//if( randomWindowSegment != NULL ) free( randomWindowSegment ) ; 
							re_fvec( randomWindowSegment, windowSize ) ;   

							for( k = 0; k < windowSize; k++) randomWindowSegment[k] = value[windowBeginIndex + k] ;  
							for( k = 0, windowSizeNow = windowSize; k < windowSize; k++, windowSizeNow--){
								l = rand() % windowSizeNow ; 
								scratchValue[windowBeginIndex + k] =  randomWindowSegment[l] ;
								for(nn = l + 1; nn < windowSizeNow; nn++) 
									randomWindowSegment[nn - 1] = randomWindowSegment[nn] ; 
							} ; 
						} ; 
						for(i = 0; i < numberOfValues; i++) value[i] = scratchValue[i] ;
						break ;   
					} ; 					

					if( computationLevel == 2){
						if( silentflag )fprintf( stderr, " AT COMPUTATION LEVEL 2" ) ; 
						// if( randomWindowSegment != NULL ) free( randomWindowSegment ) ; 
						re_fvec( randomWindowSegment, numberOfWindows ) ;
						//if( scratchWindowOrder != NULL ) free( scratchWindowOrder ) ; 
						re_ivec( scratchWindowOrder, numberOfWindows ) ; 
						//if( scratchWindowBeginIndices != NULL ) free( scratchWindowBeginIndices ) ; 
						re_ivec( scratchWindowBeginIndices, numberOfWindows ) ; 
						//if( scratchWindowLengths != NULL ) free( scratchWindowLengths ) ; 
						re_ivec( scratchWindowLengths, numberOfWindows ) ;    
						for( k = 0; k < numberOfWindows; k++) randomWindowSegment[k] = (float) k ;  
						for( k = 0, windowSizeNow = numberOfWindows; k < numberOfWindows; k++, windowSizeNow--){
							l = rand() % windowSizeNow ; pri( l, "l" ) ; 
							scratchWindowOrder[k] =  (int) randomWindowSegment[l] ;
							for(nn = l + 1; nn < windowSizeNow; nn++) 
								randomWindowSegment[nn - 1] = randomWindowSegment[nn] ; 
						} ;
						if( silentflag ) prt( "" ) ; 
						 if( silentflag ) for(i = 0; i < numberOfWindows; i++ )pri( scratchWindowOrder[i], "order" ) ; 
						m = 0 ; 
						for(window = 0; window < numberOfWindows; window++){
							i = scratchWindowOrder[ window ] ; 
							windowBeginIndex = windowBeginIndices[ i ] ; windowLength = windowLengths[ i ] ; 
							for(k = m, nn = windowBeginIndex; k < (m + windowLength) ; k++, nn++ ){
								scratchValue[ k ] = value[ nn ] ; 
							} ;  
							scratchWindowBeginIndices[ window ] = m ; scratchWindowLengths[ window ] = windowLength ; 
							m += windowLength ; 
						} ; 					
						for(i = 0; i < numberOfValues; i++) value[i] = scratchValue[i] ;
						for(window = 0; window < numberOfWindows; window++) {
							windowBeginIndices[window] = scratchWindowBeginIndices[window] ;
							windowLengths[window] = scratchWindowLengths[window] ;
						} ; 
						if( reshapeDebugFlag  == 1  )for(i = 0; i < numberOfWindows; i++){
							fprintf( stderr, "\n%d: windowBeginIndices = %d, windowLengths = %d", 
								i, windowBeginIndices[i], windowLengths[i] ) ;   
						} ; 

						break ;   
					} ; 					

		case 'k':		if( silentflag )fprintf( stderr, "DUPLICATING: "); 
					duplicationFactor = crackfloat( arg_option, ch ) ;
					if( duplicationFactor <= 1.){
						prt("\n\nILLEGAL DUPLICATION FACTOR\n\n. . . . BYE.\n\n" ) ; 
						exit(EXIT_FAILURE); 
					} ; 
					newNumberOfValues = (int) ( (float) numberOfValues * duplicationFactor ) ; 
					re_fvec( scratchValue, newNumberOfValues  ) ;
					i = 0; k = 0 ;  
					while( i < newNumberOfValues ){ 
						scratchValue[i] = value[k] ; i++ ; k++ ; if(k == numberOfValues) k = 0 ; 					
					} ; 

					newNumberOfWindows = numberOfWindows * (int)ceil( duplicationFactor ); 
					//if( scratchWindowBeginIndices != NULL ) free( scratchWindowBeginIndices ) ; 
					re_ivec( scratchWindowBeginIndices, newNumberOfWindows ) ; 
					newNumberOfWindows = 0; 
					for(i = 0; i < (int)ceil( duplicationFactor ); i++){
						for(window = 0 ; window < numberOfWindows; window++ ){
							l = windowBeginIndices[window] + (i * numberOfValues ) ;
							if( l < newNumberOfValues ){
								scratchWindowBeginIndices[ newNumberOfWindows ] = l ;  
								newNumberOfWindows++ ;  
							}else break ; 
						} ; 
					}; 
					numberOfWindows = newNumberOfWindows ; 
 					//if( windowBeginIndices != NULL ) free( windowBeginIndices ) ; 
					re_ivec( windowBeginIndices, numberOfWindows ) ; 
					//if( windowLengths != NULL ) free( windowLengths ) ; 
					re_ivec( windowLengths, numberOfWindows ) ; 
					for(window = 0 ; window < numberOfWindows ; window++ ) 
						windowBeginIndices[ window ] = scratchWindowBeginIndices[ window ] ;
					for(window = 0 ; window < (numberOfWindows - 1) ; window++ ) 
						windowLengths[ window ] = windowBeginIndices[ window + 1] - windowBeginIndices[ window ] ;
					windowLengths[ numberOfWindows - 1 ] = 
						newNumberOfValues - windowBeginIndices[ numberOfWindows - 1 ] ;
   
					numberOfValues = newNumberOfValues ; 
 					re_fvec( value, newNumberOfValues ); 
					for(i = 0; i < newNumberOfValues; i++) value[i] = scratchValue[i] ; 
					if( reshapeDebugFlag  == 1  )for(i = 0; i < numberOfWindows; i++){
							fprintf( stderr, "\n%d: windowBeginIndices = %d, windowLengths = %d", 
								i, windowBeginIndices[i], windowLengths[i] ) ;   
					} ; 
					break ; 

		case 'J':		if( silentflag )fprintf( stderr, "SCROLLING PLOT: "); 
					numberOfScrollingPlotValues = (int) crackfloat( arg_option, ch ) ;
					if( numberOfScrollingPlotValues == 0 ) temp = (float) QUICK_PLOT_Y ;
					else temp = (float) numberOfValues / (float) numberOfScrollingPlotValues ;  
					findArrayStats( value, &numberOfValues, 
							&low, &hi, &range, &average, &median, &mode, &standarddeviation, &sum, &begin, &end, &middle, 0 ) ;
					for(realIndex = 0; realIndex <= (float) (numberOfValues - 1); realIndex += temp ){	
						if( silentflag ) prt("" ) ; 
						if( silentflag ) 
							for(k = 0; k < (int)(((float) WIDTH * ((value[(int) (realIndex + 0.5)] - low) / range)) + 1.) ; k++ ) 
							fprintf( stderr, "*" ) ; 		
					} ;
					statsprintflag = 0 ;
					break ;  


		case 'j':		if( silentflag ) fprintf( stderr, "HORIZONTAL PLOT: "); 
					quickploty = (int) crackfloat( arg_option, ch ) ;
					if( (quickploty == 0) || (quickploty == 1) ) quickploty = QUICK_PLOT_Y ;
					//if( quickplot != NULL ) free( quickplot ) ; 
					re_ivec( quickplot, QUICK_PLOT_X * quickploty ) ; 
					//if( quickplotwindowmarkers != NULL ) free( quickplotwindowmarkers ) ; 
					re_ivec( quickplotwindowmarkers, QUICK_PLOT_X );
					for( i = 0; i <  QUICK_PLOT_X; i++ ) quickplotwindowmarkers[i] = 0 ; 
					findArrayStats( value, &numberOfValues, 
							&low, &hi, &range, &average, &median, &mode, &standarddeviation, &sum, &begin, &end, &middle, 0 ) ;
					for(i = 0; i < QUICK_PLOT_X; i++ ) for(k = 0; k < quickploty; k++)quickplot[(i * quickploty) + k] = 0  ;   
					for(i = 0; i < QUICK_PLOT_X; i++){	
						l =  (int)(float) (numberOfValues - 1) * (float) i / (float) (QUICK_PLOT_X - 1) ;
						m = (float) (quickploty - 1) * (value[l] - low) / range ; 
						for(k = 0; k < m ; k++ ) quickplot[(i * quickploty) + k] = 1 ; 		
					} ;

					for(window = 0; window < numberOfWindows ; window++ ){
							i = 
							(int) ((float) QUICK_PLOT_X  * ( (float) windowBeginIndices[window] / (float) numberOfValues )) ;  
							l =  (int)(float) (numberOfValues - 1) * (float) i / (float) (QUICK_PLOT_X - 1) ;
							m = (float) (quickploty - 1) * (value[l] - low) / range ; 
							for(k = 0; k < m ; k++ ) quickplot[(i * quickploty) + k] = -1 ;
							quickplotwindowmarkers[i] = -1 ;  
					} ; 

					prf( hi, "MAXIMUM" );
					if( silentflag ) for(k = quickploty - 1; k >= 0; k--) {
						for(i = 0; i < QUICK_PLOT_X; i++ ) 
							if(quickplot[(i * quickploty) + k] == 1 ) fprintf( stderr, "*" )  ;
							else if (quickplot[(i * quickploty) + k] == -1 )fprintf( stderr, "|" )  ;
							else fprintf( stderr, " " ) ;    
						prt(""); 
					} ;
					if( silentflag ) for(i = 0; i < QUICK_PLOT_X; i++ ){ 
							if( quickplotwindowmarkers[i] == -1 ) fprintf( stderr, "|" )  ; else fprintf( stderr, "-" )  ; 
					} ;
					if( silentflag ) prt("");

					if( silentflag ) fprintf( stderr, "MINIMUM = %f", low );
					statsprintflag = 0 ; 
					break ;  

		case 'o':		if( silentflag ) fprintf( stderr, "ADDING SOUNDFILE OUTPUT: "); 
					strcpy(ofile, arg_option);
					fixTildeInFilename( ofile ); 
					if( silentflag ) fprintf( stderr, "%s", ofile ) ;

					// GET THE FILE FORMAT FROM THE SPECIFIED OUTPUT FILE AND LOG SUCCESS.
					// IF FILE DOES NOT EXIST, LOG FAILURE TO GET FORMAT, WHICH WILL CAUSE
					// INPUT SOUND FILE FORMAT TO BE USED INSTEAD. 
				
					// OPEN OUTPUT SOUND FILE IN READ/WRITE MODE; GET FORMAT. 
					if (! (outfile = sf_open (ofile, SFM_READ, &outputSFinfo )))
					{   
						// PRE-EXISTING SOUND FILE NOT FOUND.  USE INPUT FILE FORMAT INSTEAD. 
						if( silentflag ) fprintf (stderr, "\n\nFORMAT-SUPPLYING OUTPUT SOUND FILE %s  WAS NOT FOUND.\n", ofile ) ;
						bannero() ;
 				
					}else{
						// PRE-EXISTING FILE FOUND: SET OUTPUT FORMAT AND RATE TO MATCH OUTPUT FILE.
						oformat = outputSFinfo.format ;
					} ;

					// CLOSE IT. 

					sf_close( outfile ) ;     

				    // SET NUMBER OF CHANNELS.

					outputSFinfo.channels = 1 ;

				    if( ! ( sf_format_check (&outputSFinfo) ) )
				    {
				        fprintf( stderr, "\nBEFORE CREATE OF OUTPUT FILE: INVALID SOUND FILE FORMAT\n" ) ; 
				    } ; 

				    if (! (outfile = sf_open (ofile, SFM_WRITE, &outputSFinfo )))
				    {   printf ("1. Not able to open output file %s.\n", ofile ) ;
					    bannero() ;
				           puts (sf_strerror (NULL)) ;
				           exit(EXIT_FAILURE) ;
				    } ;

				    prbanner( "OUTPUT SOUNDFILE",  WIDTH  ) ; 
				    prs( ofile,  "OUTPUT FILE: FILENAME " );
				    pri( osr,  "OUTPUT FILE: SAMPLE RATE" ) ; 
				    pri( outputSFinfo.channels,  "OUTPUT FILE: NUMBER OF CHANNELS" ) ; 

				    // POSITION AT BEGINNING.
				    sf_count_t frames = 0 ; 
				    n = sf_command(outfile, SFC_FILE_TRUNCATE, &frames, sizeof (frames)) ; 

					// WRITE TO SOUND FILE
					sf_write_float( outfile, value, numberOfValues  ) ;
	
					sf_close( outfile ) ; 

					statsprintflag = 0 ;   
					break ;  
		case 'a':		if( silentflag ) fprintf( stderr, "ABSOLUTE VALUE . . . " ) ; 
					for(i = 0; i < numberOfValues; i++)value[i] = fabs( value[i] ); 					
					break ; 

		case 'u':   	if( silentflag ) fprintf( stderr, "REVERSE . . ." ) ; 
					re_fvec( scratchValue, numberOfValues ) ;
					if( computationLevel == 0 ){
						for(i = 0, j = numberOfValues - 1; i < numberOfValues; i++, j--) scratchValue[i] = value[j] ; 

						// if( scratchWindowLengths != NULL ) free( scratchWindowLengths ) ; 
						re_ivec( scratchWindowLengths, numberOfWindows ) ; 
						for(i = 0, k = numberOfWindows - 1; i < numberOfWindows; i++, k--)
							scratchWindowLengths[ i ] = windowLengths[ k ] ; 
						nn = 0 ; 
						for(i = 0; i < numberOfWindows ; i++){
							windowLengths[i] = scratchWindowLengths[ i ] ; 
							windowBeginIndices[i] = nn ; 
							nn += windowLengths[i] ; 
						} ; 
					}else if( computationLevel == 1 ){
						for(window = 0; window < numberOfWindows; window++){
							for(i = windowBeginIndices[window], k = windowBeginIndices[window] + windowLengths[window] - 1; 
								i < windowBeginIndices[window] + windowLengths[window]; 
									i++, k--) scratchValue[i] = value[k] ;  
						} ; 						

					}else{
						nn = 0 ; 
						for(window = 0, k = numberOfWindows - 1; window < numberOfWindows; window++, k--){
							windowBeginIndices[window] = nn ; 
							for(i = windowBeginIndices[k]; i < (windowBeginIndices[k] + windowLengths[k]); i++ ){
								scratchValue[nn] = value[i] ; nn++ ; 
							} ;  
							windowLengths[window] = nn - windowBeginIndices[window] ; 
						} ; 
					} ; 

					for(i = 0; i < numberOfValues; i++){
						value[i] = scratchValue[i] ; 
						if( reshapeDebugFlag  == 1 ) prf( value[i], "REVERSED VALUE" ) ;
					} ; 
					break; 
		case 'c':   	if( silentflag ) fprintf( stderr, "CUBIC SPLINE FUNCTION RESAMPLING . . ." ) ; 
					numberOfSplineValues = (int) crackfloat( arg_option, ch ) ;
					if( numberOfSplineValues == 0 ){
						prt( "\nMISSING RESAMPLE LENGTH.\n . . . . BYE.\n\n" ) ; 
						exit(EXIT_FAILURE);
					}else{
				    		sprintf( csplineScratchFileName, "/tmp/csplineScratchFileName.%s.%d", user, (int)(rand()) ) ;
    						filesToRemove( csplineScratchFileName, 0 );
						csplineScratchFile = fopen( csplineScratchFileName, "w" ); 
    						for(i = 0; i < numberOfValues; i++) 
							fprintf( csplineScratchFile, "%d %f ", i, value[i] ) ;
    						fclose( csplineScratchFile ) ;
						sprintf( tempstring, "/tmp/csplineOutput.%s.%d", user, (int)(rand()) ) ;
						filesToRemove( tempstring, 0 );
						snprintf( csplineCommand, sizeof(csplineCommand), "cspline %d `cat %s` > %s",
							numberOfSplineValues, csplineScratchFileName, tempstring ) ;
						system( csplineCommand ) ; 
						fscratch  = fopen( tempstring, "r" ) ;

						for(i = 0; i < numberOfWindows; i++)
							windowBeginIndices[i] = 
							(float) numberOfSplineValues * ((float) windowBeginIndices[i] / (float) numberOfValues) ; 
						for(i = 0; i < numberOfWindows - 1; i++)
							windowLengths[i] = windowBeginIndices[i + 1] - windowBeginIndices[i] ; 
						windowLengths[numberOfWindows - 1] = 
							numberOfSplineValues - windowBeginIndices[numberOfWindows - 1] ; 

						numberOfValues = numberOfSplineValues ;   


						 re_fvec( value, numberOfValues ) ; 
						 re_fvec( scratchValue, numberOfValues ) ; 


						for(i = 0; i < numberOfValues; i++)fread( &value[i], sizeof(float), 1, fscratch );
						fclose( fscratch ) ;    


					}; 
					break ; 

		case 'R':		if( silentflag ) fprintf( stderr, "RESAMPLING . . . " ) ; 
					strcpy(tempstring, arg_option);
					ninc.fp = crackstring( tempstring, &ninc );
					newNumberOfValues = 0 ; n = 0 ;  
					while( n <= (numberOfValues - 1) ){
						ninc.A[ 0 ] = fval( &ninc, (float) (numberOfValues - 1.), (float) i );
						if( ninc.A[ 0 ] < 0. ){
							pri( ninc.A[ 0 ], "\nRESAMPLE PROPORTION" ) ;
							prt( "\n----> ERROR: ILLEGAL RESAMPLE PROPORTION (-R); MUST BE >= 0\n. . . .BYE.\n\n");  
							exit(EXIT_FAILURE); 
						};  
	    					n += (1. / ninc.A[ 0 ]) ; newNumberOfValues++ ; 
					} ; 
					re_fvec( scratchValue, newNumberOfValues ) ; 

					k = 0; n = 0; 	
					while( n <= (numberOfValues - 1) ){
						ninc.A[ 0 ] = fval( &ninc, (float) (numberOfValues - 1.), (float) i );
						if( ninc.A[ 0 ] < 0. ){
							pri( ninc.A[ 0 ], "\nRESAMPLE PROPORTION" ) ;
							prt( "\n----> ERROR: ILLEGAL RESAMPLE PROPORTION (-R); MUST BE >= 0\n. . . .BYE.\n\n");  
							exit(EXIT_FAILURE); 
						};  
						if( ninc.A[ 0 ] > 1 ){
							// INTERPOLATE
							i = (int) floor( n ) ; j = i + 1 ; if( j > (numberOfValues - 1)) j = numberOfValues - 1 ;
							frac = n - (float) i ;
							 scratchValue[k] = value[i] + (frac * (value[j] - value[i])) ;
						}else{
							// ROUND INDEX
							i = (int)(n + 0.5) ; if(i > (numberOfValues - 1)) i = numberOfValues - 1 ; 
							scratchValue[k] = value[ i ] ; 
						} ;  
						k++ ; n += (1. / ninc.A[ 0 ]) ;
					} ; 


					for(i = 0; i < numberOfWindows; i++)
							windowBeginIndices[i] = 
							(float) newNumberOfValues * ((float) windowBeginIndices[i] / (float) numberOfValues) ; 
					for(i = 0; i < numberOfWindows - 1; i++)
							windowLengths[i] = windowBeginIndices[i + 1] - windowBeginIndices[i] ; 
					windowLengths[numberOfWindows - 1] = 
							newNumberOfValues - windowBeginIndices[numberOfWindows - 1] ; 


					numberOfValues = newNumberOfValues ;  
					re_fvec( value, numberOfValues ) ;
					for(i = 0; i < numberOfValues; i++){
						value[i] = scratchValue[i] ; 
						if( reshapeDebugFlag  == 1 ) prf( value[i], "RE-SAMPLED VALUE" ) ; 
					}; 
					if( ninc.n != 1. ) fclose( ninc.fp );
					break;

					


		case 'V':   	if( silentflag ) fprintf( stderr, "OVER-SAMPLING . . ." ) ; 
					strcpy(tempstring, arg_option);
					oversamp.fp = crackstring( tempstring,&oversamp );
					newNumberOfValues = 0 ; 
					for(i = 0; i < numberOfValues; i++){
						oversamp.A[ 0 ] = fval( &oversamp, (float) (numberOfValues - 1.), (float) i );
	    					oversampi = (int) (oversamp.A[ 0 ] + .5) ;
						if( oversampi < 1 ){
							pri( oversampi, "\nOVER SAMPLING AMOUNT" ) ;
							prt( "\n----> ERROR: ILLEGAL OVERSAMPLING AMOUNT (-V); MUST BE >= 1\n. . . .BYE.\n\n");  
							exit(EXIT_FAILURE); 
						};  
						newNumberOfValues += oversampi ; 
					} ; 
					re_fvec( scratchValue, newNumberOfValues ) ; 

					k = 0; 	
					for(i = 0; i < numberOfValues; i++){
						oversamp.A[ 0 ] = fval( &oversamp, (float) (numberOfValues - 1.), (float) i );
	    					oversampi = (int) (oversamp.A[ 0 ] + .5) ;
						if( oversampi < 1 ){
							pri( oversampi, "\nOVER SAMPLING AMOUNT" ) ;
							prt( "\n----> ERROR: ILLEGAL OVERSAMPLING AMOUNT (-V); MUST BE >= 1\n. . . .BYE.\n\n");  
							exit(EXIT_FAILURE); 
						};
  
						for(j = 0; j < oversampi; j++, k++ )scratchValue[k] = value[i] ;  
					} ;

					for(i = 0; i < numberOfWindows; i++)
							windowBeginIndices[i] = 
							(float) newNumberOfValues * ((float) windowBeginIndices[i] / (float) numberOfValues) ; 
					for(i = 0; i < numberOfWindows - 1; i++)
							windowLengths[i] = windowBeginIndices[i + 1] - windowBeginIndices[i] ; 
					windowLengths[numberOfWindows - 1] = 
							newNumberOfValues - windowBeginIndices[numberOfWindows - 1] ; 



					numberOfValues = newNumberOfValues ;  
					re_fvec( value, numberOfValues ) ;
					for(i = 0; i < numberOfValues; i++){
						value[i] = scratchValue[i] ; 
						if( reshapeDebugFlag  == 1 ) prf( value[i], "OVER-SAMPLED VALUE" ) ; 
					};
					if( oversamp.n != 1. ) fclose( oversamp.fp );
					break;

		case 'S':   	strcpy( scalefilename, arg_option);
					scalefileflag = 1 ; 
					if( scalefileflag != 0 ){
						if( (scalefile = fopen( scalefilename, "r")) == NULL ){
	    						fprintf( stderr, "\n%s <-- NO SUCH FILE (scalefile). BYE.\n\n",  scalefilename ) ; 
	    						exit(EXIT_FAILURE); 
						} ; 
						k = 0 ; 
						while( fscanf( scalefile,  " %f ",  &temp ) != EOF ) k++ ;
						scalesize = k + 1 ;  			
						rewind( scalefile ) ;    

    						k = 1 ;
    						scale[0] = 0. ;  
    						while( fscanf( scalefile,  " %f ",  &scale[ k ] ) != EOF ){
							scale[ k ] += scale[k - 1] ; k++ ; 			
    						} ;
				    		scalesum = scale[scalesize - 1] ;
						fclose( scalefile ) ;  
					}    
					break;

		case 'O':   	if( silentflag ) fprintf( stderr, "SETTING SCALE ORIGIN . . . " ) ; 
					strcpy(tempstring, arg_option);
					origin.fp = crackstring( tempstring, &origin );
					scaleOriginFlag = 1 ; 
					if( origin.n != 1. ) fclose( origin.fp );
					break;
	    
		case 'Q':   	if( silentflag ) fprintf( stderr, "QUANTIZING INTO SCALE . . . " ) ; 
					strcpy(tempstring, arg_option);
					if( scalefileflag == 0 ){
						prt( "\n SCALE FILE (-S) MISSING.\n. . . . BYE.\n\n" ); 
						exit( EXIT_FAILURE ); 
					}else if( scaleOriginFlag == 0 ){
						prt( "\nSCALE FILE ORIGIN (-O) UNSPECIFIED.\n. . . . BYE.\n\n" ); 
						exit( EXIT_FAILURE ); 
					}else{
						force.fp = crackstring( tempstring, &force );
						
						i = 0 ; 
						for(window = 0; window < numberOfWindows; window++){
							
							for(nn = 0 ; nn < windowLengths[window]; nn++ ){

								funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ; 

								origin.A[ 0 ] = fval( &origin, 1., funcprop );
		    						temporigin = origin.A[ 0 ]  ; 
								force.A[ 0 ] = fval( &force, 1., funcprop );

		    						while( temporigin > value[i] ) temporigin -= scalesum ;
		    						while( (value[i] - temporigin) >= scalesum ) temporigin += scalesum ; 
		    						temp2 = (float) ((int) ((value[i] - temporigin) / scalesum)) ;
		    						// PARTIAL SCALE ABOVE TONIC
		    						temp3 = value[i] - temporigin + (scalesum * temp2) ; 
		    						// STEP THROUGH SCALE AND FIND POSITION
		    						k = 0 ; 
		    						while( temp3 >= scale[k] )k++ ;
								temp4 = (temp3 -  scale[k - 1]) / (scale[k] - scale[k - 1]) ; 
								if( temp4 < .5 ) value[i] -= (force.A[ 0 ] * (temp3 - scale[k - 1])) ; 
		    						else value[i] += (force.A[ 0 ] * (scale[k] - temp3)) ;
								if( reshapeDebugFlag  == 1 ) prf( value[i], "SCALE-MODIFIED VALUE" ) ; 
								i++ ; 
							} ;

						} ; 
						if( force.n != 1. ) fclose( force.fp ); 
						break;
	
					} ; 

		case 'P':   	if( silentflag ) fprintf( stderr, "SETTING REFERENCE FREQUENCY FOR TRANSLATION . . . " ) ; 
					pitch = crackfloat( arg_option, ch ) ;
    					if( pitch < 0. ){
						prf( pitch, "\nREFERENCE FREQUENCY OR OCT.PITCHCLASS PITCH" ) ; 
						prt( "---ERROR: ILLEGAL REFERENCE FREQUENCY OR PITCH\n. . . . BYE.\n\n" ); 
						exit( EXIT_FAILURE ) ; 	
					}else if( pitch <= 12.0 ){
						// CONVERT TO FREQ
    		    				temp = (float) ((int) pitch ) ; // INTEGER PART
		     			temp =  (( 12. * (temp - 8.)) + (100. * (pitch - temp)) ) / 12. ;
		    				reffreq = midC * pow( 2., (double) temp ) ; 
    					}else{
						// IT's A FREQ
						reffreq = pitch ; 
    					} ; 
					break;

		case 't':		 
					transCode = (int) crackfloat( arg_option, ch ) ;
					if(transCode == 0){ // AMP TO DB
						if( silentflag ) fprintf( stderr, "TRANSLATING VALUES INTO DECIBELS FROM AMPLITUDE . . . " ) ;
						for(i = 0; i < numberOfValues; i++){
							 value[i] = amp_to_dB( value[i] );
							if( reshapeDebugFlag  == 1 ) prf( value[i], "DECIBELS VALUE" ) ;
						}; 
 					}else if(transCode == 1){ // DB TO AMP
						if( silentflag ) fprintf( stderr, "TRANSLATING VALUES INTO AMPLITUDE FROM DECIBELS. . . " ) ;
						for(i = 0; i < numberOfValues; i++){
							 value[i] = dB_to_amp( value[i] );
							if( reshapeDebugFlag  == 1 ) prf( value[i], "AMPLITUDE VALUE" ) ;
						}; 
 					}else if(transCode == 2){ // FREQ TO SEMITONES OF DEVIATION
						if( silentflag ) fprintf( stderr, "TRANSLATING VALUES INTO SEMITONES OF DEVIATION FROM FREQUENCY. . . " ) ;
						if( reffreq != 0. ){						
							for(i = 0; i < numberOfValues; i++){
								value[i] = value[i] / reffreq ; 
								value[i] = 12. *  (float) (((float) log10( (double) value[i])) / log_of_2 ) ;  
								if( reshapeDebugFlag == 1 ) prf( value[i], "SEMITONES OF DEVIATION VALUE" ) ;
							};
 						}else{
							prt( "----> ERROR: REFERENCE FREQUENCY (-P) MUST BE PREVIOUSLY SET FOR TRANSLATION CODES 2 AND 3."); 
							prt( ". . . . BYE.\n\n" )  ; 
						} ; 
  					}else if(transCode == 3){ // FREQ TO SEMITONES OF CORRECTION
						if( silentflag ) fprintf( stderr, "TRANSLATING VALUES INTO SEMITONES OF CORRECTION FROM FREQUENCY. . . " ) ;
						if( reffreq != 0. ){						
							for(i = 0; i < numberOfValues; i++){
								value[i] = value[i] / reffreq ; 
								value[i] = 12. *  (float) (((float) log10( (double) value[i])) / log_of_2 ) ;  
								value[i] = value[i] * -1. ;
								if( reshapeDebugFlag == 1 ) prf( value[i], "SEMITONES OF CORRECTION VALUE" ) ;
							};
 						}else{
							prt( "----> ERROR: REFERENCE FREQUENCY (-P) MUST BE PREVIOUSLY SET FOR TRANSLATION CODES 2 AND 3."); 
							prt( ". . . . BYE.\n\n" )  ; 
						} ; 
 					}else if(transCode == 4){ // TRUNCATE TO INTEGER
						if( silentflag ) fprintf( stderr, "TRANSLATING VALUES INTO TRUNCATED INTEGERS. . . " ) ;
						for(i = 0; i < numberOfValues; i++){
							 value[i] = floor( value[i] ) ;
							if( reshapeDebugFlag  == 1 ) prf( value[i], "TRUNCATE TO INTEGER VALUE" ) ;
						}; 
 					}else if(transCode == 5){ // ROUND TO INTEGER
						if( silentflag ) fprintf( stderr, "TRANSLATING VALUES INTO ROUNDED INTEGERS. . . " ) ;
						for(i = 0; i < numberOfValues; i++){
							 value[i] = rint( value[i] );
							if( reshapeDebugFlag  == 1 ) prf( value[i], "ROUND TO INTEGER VALUE" ) ;
						}; 
 					}else if(transCode == 6){ // HZ TO MIDI
						if( silentflag ) fprintf( stderr, "TRANSLATING VALUES FROM HZ INTO MIDI. . . " ) ;
						for(i = 0; i < numberOfValues; i++){
							 value[i] = Hz_to_MIDI( value[i] );
							if( reshapeDebugFlag  == 1 ) prf( value[i], "HZ INTO MIDI" ) ;
						}; 
 					}else if(transCode == 7){ // MIDI TO HZ
						if( silentflag ) fprintf( stderr, "TRANSLATING VALUES FROM MIDI INTO HZ . . . " ) ;
						for(i = 0; i < numberOfValues; i++){
							 value[i] = MIDI_to_Hz( value[i] );
							if( reshapeDebugFlag  == 1 ) prf( value[i], "MIDI INTO HZ" ) ;
						}; 
 					}else{
    						if( silentflag ) fprintf( stderr, "\nILLEGAL TRANSLATION CODE.\n\n" ) ; 
    						exit(EXIT_FAILURE) ; 
					};
					break ;  

		case 'C':   	if( silentflag ) fprintf( stderr, "SORTING VALUES . . . " ) ;
					strcpy(tempstring, arg_option);
					sortIndex.fp = crackstring( tempstring, &sortIndex );

					if( computationLevel == 0 ){

						sortIndex.A[ 0 ] = fval( &sortIndex, 1., 1. ) ; 
						if( (sortIndex.A[ 0 ] >= -1.) &&  (sortIndex.A[ 0 ] <= 1.) ){
							for(l = 0; l < 2; l++){
								for(i = 0; i < numberOfValues; i++ ) scratchValue[i] = value[i] ;
								notdone = 1 ; ii = 0 ; steps = 0 ;
								while( notdone ){ 
									notdone = 0 ; 
	    								for( i = ii ; i  < (numberOfValues - 1); i += 2 ){
										if( (l == 1) && (steps >= stepstotake) ) {
											notdone = 0 ; break ; 
										}else{								
											if( sortIndex.A[ 0 ] > 0. ){
		    										// ASCENDING
		    										if( scratchValue[i] > scratchValue[i + 1] ){
													// SWITCH
													temp = scratchValue[i] ; scratchValue[i] = scratchValue[i + 1] ; 
													scratchValue[i + 1] = temp ; 
													notdone = 1 ; steps++ ;  
		    										}
											}else if( sortIndex.A[ 0 ] < 0.){
		    										// DESCENDING
		    										if( scratchValue[i] < scratchValue[i + 1] ){
													// SWITCH
													temp = scratchValue[i] ; scratchValue[i] = scratchValue[i + 1] ; 
													scratchValue[i + 1] = temp ; 
													notdone = 1 ; steps++ ; 
		    										}
											}
										} ; 				

	    								}
	    								if( ii == 0 ) ii = 1 ; else ii = 0 ; 
								} ; 
								stepstotake = (int)( (float) steps * fabs(sortIndex.A[ 0 ]) ) ; 
							} ;

						}else{
							prt( "------> ERROR: -C SORT INDEX MUST BE BETWEEN 1 (ASCENDING) AND -1 (DESCENDING)" ); 
							prt( "\n\n. . . .BYE\n\n" ) ; 
							exit( EXIT_FAILURE ) ; 
						} ; 
					}else if(computationLevel == 1){
						// SORT VALUES WITHIN WINOWS

						for(window = 0; window < numberOfWindows; window++){
							funcprop = numberOfWindows > 1 ? (float) window / (float) (numberOfWindows - 1) : 1. ; 
							sortIndex.A[ 0 ] = fval( &sortIndex, 1., funcprop ) ; 										

							for(l = 0; l < 2; l++){
								for(nn = 0, i = windowBeginIndices[window] ; 
									nn < windowLengths[window]; nn++, i++ ) scratchValue[i] = value[i] ;
								notdone = 1 ; ii = 0 ; steps = 0 ;
								while( notdone ){

									notdone = 0 ; 
	    								for( i = ii, k = windowBeginIndices[window] + ii ; 
										i  < (windowLengths[window] - 1); i += 2, k += 2 ){
										if( (l == 1) && (steps >= stepstotake) ) {
												notdone = 0 ; break ; 
										}else{

											if( sortIndex.A[ 0 ] > 0. ){
		    										// ASCENDING
		    										if( scratchValue[k] > scratchValue[k + 1] ){
													// SWITCH
													temp = scratchValue[k] ; scratchValue[k] = scratchValue[k + 1] ; 
													scratchValue[k + 1] = temp ; 
													notdone = 1 ; steps++ ;  
			    									}
											}else if( sortIndex.A[ 0 ] < 0.){
			    									// DESCENDING
			    									if( scratchValue[k] < scratchValue[k + 1] ){
													// SWITCH
													temp = scratchValue[k] ; scratchValue[k] = scratchValue[k + 1] ; 
													scratchValue[k + 1] = temp ; 
													notdone = 1 ; steps++ ; 
			    									}
											}
										} ; 				

		    							}
	    								if( ii == 0 ) ii = 1 ; else ii = 0 ; 
								} ; 
								stepstotake = (int)((float) steps * fabs( sortIndex.A[ 0 ] ) ) ; 
							} ;
						} ; 

					}else{
						//if( windowAverages != NULL ) free( windowAverages ) ; 
						re_fvec( windowAverages, numberOfWindows ) ; 
						for(i = 0; i < numberOfWindows; i++) windowAverages[i] = 0. ; 
						for(window = 0; window < numberOfWindows; window++){
							for(i = 0, k = windowBeginIndices[window]; i < windowLengths[window] ; i++, k++) 
									windowAverages[window] += value[k] ; 
							windowAverages[window] /= (float) windowLengths[window] ; 
						} ; 
						//if( scratchWindowAverages != NULL ) free( scratchWindowAverages ) ; 
						re_fvec( scratchWindowAverages, numberOfWindows ) ; 
						//if( scratchWindowAveragesIndices != NULL ) free( scratchWindowAveragesIndices ) ; 
						re_ivec( scratchWindowAveragesIndices, numberOfWindows ) ; 

						sortIndex.A[ 0 ] = fval( &sortIndex, 1., 1. ) ; 										

						for(l = 0; l < 2; l++){
							for(i = 0; i < numberOfWindows; i++ ){
								scratchWindowAverages[i] = windowAverages[i] ;
								scratchWindowAveragesIndices[i] = i ; 
							} ; 

							notdone = 1 ; ii = 0 ; steps = 0 ;
							while( notdone ){

								notdone = 0 ; 
	    							for( k = ii; k  < (numberOfWindows - 1); k += 2 ){
									if( (l == 1) && (steps >= stepstotake) ) {
										notdone = 0 ; break ; 
									}else{

										if( sortIndex.A[ 0 ] > 0. ){
		    									// ASCENDING
		    									if( scratchWindowAverages[k] > scratchWindowAverages[k + 1] ){
												// SWITCH
												temp = scratchWindowAverages[k] ; 
												scratchWindowAverages[k] = scratchWindowAverages[k + 1] ; 
												scratchWindowAverages[k + 1] = temp ; 

												nn = scratchWindowAveragesIndices[k] ; 
												scratchWindowAveragesIndices[k] = scratchWindowAveragesIndices[k + 1] ; 
												scratchWindowAveragesIndices[k + 1] = nn ; 

												notdone = 1 ; steps++ ;  
			    								}
										}else if( sortIndex.A[ 0 ] < 0.){
			    								// DESCENDING
			    								if( scratchWindowAverages[k] < scratchWindowAverages[k + 1] ){
												// SWITCH
												temp = scratchWindowAverages[k] ; 
												scratchWindowAverages[k] = scratchWindowAverages[k + 1] ; 
												scratchWindowAverages[k + 1] = temp ; 

												nn = scratchWindowAveragesIndices[k] ; 
												scratchWindowAveragesIndices[k] = scratchWindowAveragesIndices[k + 1] ; 
												scratchWindowAveragesIndices[k + 1] = nn ; 

												notdone = 1 ; steps++ ; 
		    									}
										}
									} ; 				

	    							}
    								if( ii == 0 ) ii = 1 ; else ii = 0 ; 
							} ; 
							stepstotake = (int)((float) steps * fabs( sortIndex.A[ 0 ] ) ) ; 
						} ;

						//if( scratchWindowBeginIndices != NULL ) free( scratchWindowBeginIndices ) ; 
						re_ivec( scratchWindowBeginIndices, numberOfWindows ) ; 
						//if( scratchWindowLengths != NULL ) free( scratchWindowLengths ) ; 
						re_ivec( scratchWindowLengths, numberOfWindows ) ; 

						k = 0 ; 
						for(window = 0; window < numberOfWindows; window++ ){
							scratchWindowBeginIndices[window] = k ;
							m = scratchWindowAveragesIndices[window] ; 
							scratchWindowLengths[window] = windowLengths[m] ;  
							for( i = 0, nn = windowBeginIndices[m]; i < windowLengths[m]; i++, nn++ ){
								scratchValue[k] = value[nn] ; k++ ; 
							} ;
						} ; 
						for(i = 0; i < numberOfWindows; i++){
							windowBeginIndices[i] = scratchWindowBeginIndices[i] ; 
							windowLengths[i] = scratchWindowLengths[i] ; 
						} ; 

					} ; 

					for(i = 0; i < numberOfValues; i++ ) value[i] = scratchValue[i] ; 
					if( reshapeDebugFlag  == 1 )for(i = 1; i < numberOfValues; i++) prf( value[i], "SORTED VALUE" ) ;

					break;

		case 's':   	if( silentflag ) fprintf( stderr, "INTEGRATING VALUES . . . " ) ; 

					if(computationLevel == 0){
						for(i = 1; i < numberOfValues; i++) {
							value[i] = value[i - 1] + value[i] ;
							if( reshapeDebugFlag  == 1 ) prf( value[i], "INTEGRATION VALUE" ) ;
						} ;

					}else if(computationLevel == 1){
						for(window = 0; window < numberOfWindows; window++){
							for(k = 1, i = 1 + windowBeginIndices[window]; k < windowLengths[window]; k++, i++) {
								value[i] = value[i - 1] + value[i] ;
								if( reshapeDebugFlag  == 1 ) prf( value[i], "INTEGRATION VALUE" ) ;
							} ;
						} ; 

					}else{
						prt("COMPUTATION LEVEL 2 (-l2) NOT AVAILABLE.\n\n. . . BYE." ) ; 
						exit( EXIT_FAILURE ) ; 
					} ; 
					break;

		case 'd':   	if( silentflag ) fprintf( stderr, "DIFFERENTIATING VALUES . . . " ) ;
					diffType = crackfloat( arg_option, ch ) ;
					singleOperator( ch, value, scratchValue, numberOfValues, &dummyStructure, diffType, silentflag ); 
					break;

		case 'w':   	if( silentflag ) fprintf( stderr, "WARPING TIME . . . " ) ; // ???
					strcpy(tempstring, arg_option);
					time_warp.fp = crackstring( tempstring, &time_warp );
					singleOperator( ch, value, scratchValue, numberOfValues, &time_warp, dummyVal, silentflag ); 
					if( time_warp.n != 1. ) fclose( time_warp.fp );
					break;



		case 'Z':		if( silentflag ) prt("MEDIAN CENTERED SYMMETRICAL VALUE WARP . . . " ) ; 
					strcpy(tempstring, arg_option);
					shape.fp = crackstring( tempstring, &shape );
					findArrayStats( value, &numberOfValues, 
							&low, &hi, &range, &average, &median,  &mode, &standarddeviation, &sum, &begin, &end, &middle, 0 ) ;
					i = 0; 
					if( range != 0.0 ) for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							shape.A[ 0 ] = fval( &shape, 1., funcprop );
							if( value[i] < median )
								value[i] = median - curve( 0., median - low, 
											( median - value[i] ) / (median - low), shape.A[ 0 ] ) ; 
							else
								value[i] = median + curve( 0., hi - median, 
											( value[i] - median ) / (hi - median), shape.A[ 0 ] ) ; 
							i++ ; 
						} ;
					} ; 
					if( shape.n != 1. ) fclose( shape.fp );
					break;

		case 'G':		if( silentflag ) prt("MID-POINT CENTERED SYMMETRICAL VALUE WARP . . . " ) ; 
					strcpy(tempstring, arg_option);
					shape.fp = crackstring( tempstring, &shape );

					findArrayStats( value, &numberOfValues, 
							&low, &hi, &range, &average, &median,  &mode, &standarddeviation, &sum, &begin, &end, &middle, 0 ) ;
					i = 0; 
					if( range != 0. ) for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							shape.A[ 0 ] = fval( &shape, 1., funcprop );

							temp = 0.5 * range ; temp2 = low + temp ;  
							if( value[i] < temp2 ) 
								value[i] = temp2 - curve( 0., temp, 
											( temp2 - value[i] ) / temp, shape.A[ 0 ] ) ; 
							else
								value[i] = temp2 + curve( 0., temp, 
											( value[i] - temp2 ) / temp, shape.A[ 0 ] ) ; 

							i++ ; 
						} ;
					} ; 

					if( shape.n != 1. ) fclose( shape.fp );
					break;

		case 'W':   	if( silentflag ) prt(" ASYMMETRICAL VALUE WARP . . . " ) ; // HERE
					strcpy(tempstring, arg_option);
					shape.fp = crackstring( tempstring, &shape ); 
					findArrayStats( value, &numberOfValues, 
							&low, &hi, &range, &average, &median,  &mode, &standarddeviation, &sum, &begin, &end, &middle, 0 ) ;
					i = 0; 
					if( (lowlimitsaveflag == 1) && (hilimitsaveflag == 1) ){
						low = lowlimitsave ; hi = hilimitsave ; 
						if( silentflag ) 
							fprintf( stderr, "\n==> USING SAVED LOW (%f) AND HIGH (%f) LIMIT VALUES AS WARPING BOUNDARIES. <==\n",
						 low, hi ) ;  
					} ; 
	
					if( range != 0.0 ) for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							shape.A[ 0 ] = fval( &shape, 1., funcprop );
							if( low != hi ) value[i] = curve( low, hi, (value[i] - low) / (hi - low), shape.A[ 0 ] ) ; 

							i++ ;
						} ;
					} ; 
					if( shape.n != 1. ) fclose( shape.fp );

					break;

		case 'I':   	if( silentflag ) fprintf( stderr, "INVERSE DIVISION . . . " ) ; 
					strcpy(tempstring, arg_option);
					inverse.fp = crackstring( tempstring, &inverse );
					i = 0; 
					for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							inverse.A[ 0 ] = fval( &inverse, 1., funcprop );
							value[i] = inverse.A[ 0 ] / value[i] ;
							i++ ; 
						} ;
					} ; 
					if( inverse.n != 1. ) fclose( inverse.fp );
					break;

		case 'E':   	if( silentflag ) fprintf( stderr, "TRANSPOSING MEAN TO TARGET . . . " ) ; 
					strcpy(tempstring, arg_option);
					multiply.fp = crackstring( tempstring, &multiply );
					findArrayStats( value, &numberOfValues, 
							&low, &hi, &range, &average, &median,  &mode, &standarddeviation, &sum, &begin, &end, &middle, 0 ) ;
					i = 0; 
					for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							multiply.A[ 0 ] = fval( &multiply, 1., funcprop );

							value[i] = value[i]  + (multiply.A[ 0 ] - average) ; 

							i++ ; 
						} ;
					} ; 
					if( multiply.n != 1. ) fclose( multiply.fp );
					break;



		case 'M':   	if( silentflag ) fprintf( stderr, "MULTIPLYING . . . " ) ; 
					strcpy(tempstring, arg_option);
					multiply.fp = crackstring( tempstring, &multiply );
					i = 0; 
					for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							multiply.A[ 0 ] = fval( &multiply, 1., funcprop );
							value[i] *= multiply.A[ 0 ] ; 
							i++ ; 
						} ;
					} ; 
					if( multiply.n != 1. ) fclose( multiply.fp );
					break;

		case 'm':   	if( silentflag ) fprintf( stderr, "SCALER MULTIPLY AROUND MEAN . . . " ) ; 
					strcpy(tempstring, arg_option);
					multiply.fp = crackstring( tempstring, &multiply );
					findArrayStats( value, &numberOfValues, 
							&low, &hi, &range, &average, &median,  &mode, &standarddeviation, &sum, &begin, &end, &middle, 0 ) ;
					i = 0; 
					for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							multiply.A[ 0 ] = fval( &multiply, 1., funcprop );

							value[i] = ((value[i] - average) * multiply.A[ 0 ]) + average ; 

							i++ ; 
						} ;
					} ; 
					if( multiply.n != 1. ) fclose( multiply.fp );
					break;
		case 'n':   	if( silentflag ) fprintf( stderr, "NORMALIZE . . . " ) ; 
					strcpy(tempstring, arg_option);
					multiply.fp = crackstring( tempstring, &multiply );
					singleOperator( ch, value, scratchValue, numberOfValues, &multiply, dummyVal, silentflag ); 
					if( multiply.n != 1. ) fclose( multiply.fp );
					break;


		case 'D':   	if( silentflag ) fprintf( stderr, "DIVIDING VALUES . . . " ) ; 
					strcpy(tempstring, arg_option);
					divide.fp = crackstring( tempstring, &divide );

					i = 0; 
					for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							divide.A[ 0 ] = fval( &divide, 1., funcprop );
							value[i] = value[i] / divide.A[ 0 ] ;  
							i++ ; 
						} ;
					} ; 

					if( divide.n != 1. ) fclose( divide.fp );
					break;

		case 'T':   	if( silentflag ) fprintf( stderr, "ADDING . . . " ) ; 
					strcpy(tempstring, arg_option);
					adder.fp = crackstring( tempstring, &adder );

					i = 0; 
					for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							adder.A[ 0 ] = fval( &adder, 1., funcprop );
							value[i] = value[i] + adder.A[ 0 ] ;  
							i++ ; 
						} ;
					} ; 

					if( adder.n != 1. ) fclose( adder.fp );
					break;

		case 'y':   	if( silentflag ) fprintf( stderr, "LOW LIMITING . . . " ) ; 
					strcpy(tempstring, arg_option); 
					lowlimit.fp = crackstring( tempstring, &lowlimit ) ;

					i = 0; 
					for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							lowlimit.A[ 0 ] = fval( &lowlimit, 1., funcprop );
							if( value[i] < lowlimit.A[ 0 ] ) value[i] = lowlimit.A[ 0 ] ; 
							i++ ; 
						} ;
					} ; 

					if( lowlimit.n == 1. ) {
						lowlimitsave = lowlimit.A[ 0 ] ; lowlimitsaveflag = 1 ; 
					} ;
					if( lowlimit.n != 1. ) fclose( lowlimit.fp );
					break ; 

		case 'Y':   	if( silentflag ) fprintf( stderr, "HIGH LIMITING . . . " ) ; 
					strcpy(tempstring, arg_option);
					hilimit.fp = crackstring( tempstring, &hilimit ) ;
					i = 0; 
					for(window = 0; window < numberOfWindows; window++){
						for(nn = 0; nn < windowLengths[window]; nn++){
							funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
							hilimit.A[ 0 ] = fval( &hilimit, 1., funcprop );
							if( value[i] > hilimit.A[ 0 ] ) value[i] = hilimit.A[ 0 ] ; 
							i++ ; 
						} ;
					} ; 
					if( hilimit.n == 1. ) {
						hilimitsave = hilimit.A[ 0 ] ; hilimitsaveflag = 1 ; 
					} ;
					if( hilimit.n != 1. ) fclose( hilimit.fp );
					break ; 

		case 'b':		if( silentflag ) fprintf( stderr, "SETTING LOW REMAP BOUNDARY . . . " ) ; 
					strcpy(tempstring, arg_option);
					lowbound.fp = crackstring( tempstring, &lowbound );
					lowBoundFlag = 1 ;
					if( hiBoundFlag == 0 ){
						if( ( (arg_index + 1) >= argc) || 
							(  ( *argv[ arg_index + 1 ] != '-' ) && ( *(argv[ arg_index + 1 ] + 1) != 'B' )  )  
						){
							prt( "\n=======> ERROR: -b and -B FLAGS MUST BE SPECIFIED TOGETHER.\n\n . . . . BYE.\n\n" ) ;  
							exit( EXIT_FAILURE ) ;
						} ; 
					}else{
						if( silentflag ) prt( "\nREMAPPING . . . " ) ; 
						findArrayStats( value, &numberOfValues, 
							&low, &hi, &range, &average, &median,  &mode, &standarddeviation, &sum, &begin, &end, &middle, 0 ) ;
						i = 0; 
						for(window = 0; window < numberOfWindows; window++){
							for(nn = 0; nn < windowLengths[window]; nn++){
								funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
								lowbound.A[ 0 ] = fval( &lowbound, 1., funcprop );
								hibound.A[ 0 ] = fval( &hibound, 1., funcprop );
								if( range == 0.0 ){
									value[i] = lowbound.A[ 0 ] ; 
								}else{
									value[i] = lowbound.A[ 0 ] + 
										( (hibound.A[ 0 ] - lowbound.A[ 0 ]) * ((value[i] - low) / range) ) ; 
								} ; 
								i++ ; 
							} ;
						} ; 
						lowBoundFlag = 0; hiBoundFlag = 0 ; 
						if( lowbound.n != 1. ) fclose( lowbound.fp );	
						if( hibound.n != 1. ) fclose( hibound.fp );

					} ; 
					break ; 


		case 'B':   	if( silentflag ) fprintf( stderr, "SETTING HIGH BOUNDARY . . . " ) ; 
					strcpy(tempstring, arg_option);
					hibound.fp = crackstring( tempstring, &hibound );
					hiBoundFlag = 1 ;
					if( lowBoundFlag == 0 ){
						if( ( (arg_index + 1) >= argc) || 
							(  ( *argv[ arg_index + 1 ] != '-' ) && ( *(argv[ arg_index + 1 ] + 1) != 'b' )  )  
						){
							prt( "\n=======> ERROR: -b and -B FLAGS MUST BE SPECIFIED TOGETHER.\n\n . . . . BYE.\n\n" ) ;  
							exit( EXIT_FAILURE ) ;
						} ; 
					}else{
						if( silentflag ) prt( "\nREMAPPING . . . " ) ; 
						findArrayStats( value, &numberOfValues, 
							&low, &hi, &range, &average, &median,  &mode, &standarddeviation, &sum, &begin, &end, &middle, 0 ) ; 
						i = 0; 
						for(window = 0; window < numberOfWindows; window++){
							for(nn = 0; nn < windowLengths[window]; nn++){
								funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
								lowbound.A[ 0 ] = fval( &lowbound, 1., funcprop );
								hibound.A[ 0 ] = fval( &hibound, 1., funcprop );
								if( range == 0.0 ){
									value[i] = lowbound.A[ 0 ] ; 
								}else{
									value[i] = lowbound.A[ 0 ] + 
										( (hibound.A[ 0 ] - lowbound.A[ 0 ]) * ((value[i] - low) / range) ) ; 
								} ; 
								i++ ; 
							} ;
						} ; 
						lowBoundFlag = 0; hiBoundFlag = 0 ; 
						if( lowbound.n != 1. ) fclose( lowbound.fp );	
						if( hibound.n != 1. ) fclose( hibound.fp );

					} ; 
					break ; 


		case 'f':   	if( silentflag ) fprintf( stderr, "SETTING ASCENDING LOWPASS FILTER COEFFICIENT . . . " ) ; 
					strcpy(tempstring, arg_option);
					feedcu.fp = crackstring( tempstring, &feedcu );
					positiveFeedFlag = 1 ; 
					if( negativeFeedFlag == 0 ){
						if( ( (arg_index + 1) >= argc) || 
							(  ( *argv[ arg_index + 1 ] != '-' ) && ( *(argv[ arg_index + 1 ] + 1) != 'F' )  )  
						){
							prt( "\n=======> ERROR: -f and -F FLAGS MUST BE SPECIFIED TOGETHER.\n\n . . . . BYE.\n\n" ) ;  
							exit( EXIT_FAILURE ) ;
						} ; 
					}else{
						if( silentflag ) prt( "\nLOW PASS FILTERING . . . " ) ; 

						i = 0; oldVal = value[0] ;
						for(window = 0; window < numberOfWindows; window++){
							for(nn = 0; nn < windowLengths[window]; nn++){
								funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
								
								// LOWPASS SMOOTH
								if( value[i] > oldVal ){
									// RISING SIGNAL
									feedcu.A[ 0 ] = fval( &feedcu, 1., funcprop );
									feedcum = 1. - feedcu.A[ 0 ] ; 
		     						value[i] = feedcum * value[i] + feedcu.A[ 0 ] * oldVal ;
								}else{
									// FALLING SIGNAL
									feedcd.A[ 0 ] = fval( &feedcd, 1., funcprop );
									feedcdm = 1. - feedcd.A[ 0 ]  ; 
		     						value[i] = feedcdm * value[i] + feedcd.A[ 0 ] * oldVal ;
								}
								oldVal = value[i] ; 

								i++ ; 
							} ;
						} ; 

						if( feedcu.n != 1. ) fclose( feedcu.fp ); 
						if( feedcd.n != 1. ) fclose( feedcd.fp );
						positiveFeedFlag = 0; negativeFeedFlag = 0 ; 
					} ; 
					
					break ; 

		case 'F':   	if( silentflag ) fprintf( stderr, "SETTING DESCENDING LOWPASS FILTER COEFFICIENT . . . " ) ; 
					strcpy(tempstring, arg_option);
					feedcd.fp = crackstring( tempstring, &feedcd );
					negativeFeedFlag = 1 ; 
					if( positiveFeedFlag == 0 ){
						if( ( (arg_index + 1) >= argc) || 
							(  ( *argv[ arg_index + 1 ] != '-' ) && ( *(argv[ arg_index + 1 ] + 1) != 'f' )  )  
						){
							prt( "\n=======> ERROR: -f and -F FLAGS MUST BE SPECIFIED TOGETHER.\n\n . . . . BYE.\n\n" ) ;  
							exit( EXIT_FAILURE ) ;
						} ; 
					}else{
						if( silentflag ) prt( "\nLOW PASS FILTERING . . . " ) ; 

						i = 0; oldVal = value[0] ;
						for(window = 0; window < numberOfWindows; window++){
							for(nn = 0; nn < windowLengths[window]; nn++){
								funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
								
								// LOWPASS SMOOTH
								if( value[i] > oldVal ){
									// RISING SIGNAL
									feedcu.A[ 0 ] = fval( &feedcu, 1., funcprop );
									feedcum = 1. - feedcu.A[ 0 ] ; 
		     						value[i] = feedcum * value[i] + feedcu.A[ 0 ] * oldVal ;
								}else{
									// FALLING SIGNAL
									feedcd.A[ 0 ] = fval( &feedcd, 1., funcprop );
									feedcdm = 1. - feedcd.A[ 0 ]  ; 
		     						value[i] = feedcdm * value[i] + feedcd.A[ 0 ] * oldVal ;
								}
								oldVal = value[i] ; 

								i++ ; 
							} ;
						} ; 

						if( feedcu.n != 1. ) fclose( feedcu.fp ); 
						if( feedcd.n != 1. ) fclose( feedcd.fp );
						positiveFeedFlag = 0; negativeFeedFlag = 0 ; 
					} ; 
					break ; 

		case 'x':   	if( silentflag ) fprintf( stderr, "SETTING NEW BEGIN INDEX . . . " ) ;
					strcpy(tempstring, arg_option);
					firstv = (int) crackfloat( arg_option, ch ) - 1 ;
					if( firstv < 0){
						pri( firstv + 1, "FIRST VALUE OUT OF RANGE" ) ; 
						pri( numberOfValues, "NUMBER OF VALUES" ) ; 
						prt( " . . . . BYE.\n\n" ) ; exit( EXIT_FAILURE ) ;
					} ; 
					beginValNumberingFlag = 1 ; 
					if( endValNumberingFlag == 0 ){
						if( ( (arg_index + 1) >= argc) || 
							(  ( *argv[ arg_index + 1 ] != '-' ) && ( *(argv[ arg_index + 1 ] + 1) != 'X' )  )  
						){
							prt( "\n=======> ERROR: -x and -X FLAGS MUST BE SPECIFIED TOGETHER.\n\n . . . . BYE.\n\n" ) ;  
							exit( EXIT_FAILURE ) ;
						} ; 
					}else{
						if( silentflag ) prt( "\nEXTRACTING NEW VALUES SEGMENT . . . " ) ; 
						for(k = firstv, i = 0; k <= lastv; k++, i++){
							value[i] = value[k] ; if( reshapeDebugFlag == 1 ) prf( value[i], "SEGMENT SELECTED VALUE" ) ;
						} ; 
						numberOfValues =  lastv - firstv + 1 ; 
 						beginValNumberingFlag = 0 ; endValNumberingFlag = 0 ;
					} ; 
					break ; 
		case 'X':   	if( silentflag ) fprintf( stderr, "SETTING NEW END INDEX . . . " ) ;
					strcpy(tempstring, arg_option);
					lastv = (int) crackfloat( arg_option, ch ) - 1 ;
					endValNumberingFlag = 1 ;
					if( lastv >=  numberOfValues){
						pri( lastv + 1, "LAST VALUE OUT OF RANGE" ) ; 
						pri( numberOfValues, "NUMBER OF VALUES" ) ; 
						prt( " . . . . BYE.\n\n" ) ; exit( EXIT_FAILURE ) ;
					} ; 
					if( beginValNumberingFlag == 0 ){
						if( ( (arg_index + 1) >= argc) || 
							(  ( *argv[ arg_index + 1 ] != '-' ) && ( *(argv[ arg_index + 1 ] + 1) != 'x' )  )  
						){
							prt( "\n=======> ERROR: -x and -X FLAGS MUST BE SPECIFIED TOGETHER.\n\n . . . . BYE.\n\n" ) ;  
							exit( EXIT_FAILURE ) ;
						} ; 
					}else{
						if( silentflag ) prt( "EXTRACTING NEW VALUES SEGMENT . . . " ) ; 
						for(k = firstv, i = 0; k <= lastv; k++, i++){
							value[i] = value[k] ; if( reshapeDebugFlag == 1 ) prf( value[i], "SEGMENT SELECTED VALUE" ) ;
						} ; 
						numberOfValues =  lastv - firstv + 1 ; 
 						beginValNumberingFlag = 0 ; endValNumberingFlag = 0 ;
					} ; 
					break ; 

		case 'r':		if( silentflag ) fprintf( stderr, "INPUT MAPPED THROUGH FUNCTION . . ." ) ;
					strcpy(tempstring, arg_option);
					reshapeFunction.fp = crackstring( tempstring, &reshapeFunction );
					findArrayStats( value, &numberOfValues, 
						&arraylow, &arrayhi, &arrayrange, &arrayaverage, &arraymedian,  &arraymode, &arraystandarddeviation, 
								&arraysum, &arraybegin, &arrayend, &arraymiddle, 0 ) ; 

					for(i = 0; i < numberOfValues; i++){
						reshapeFunction.A[ 0 ] = fval( &reshapeFunction, 1., (value[i] - arraylow) / arrayrange ); 
						value[i] = reshapeFunction.A[ 0 ] ; 
						if( reshapeDebugFlag  == 1 ) prf( value[i], "RE-MAPPED/SHAPED VALUE" ) ;
					};
					if( reshapeFunction.n != 1. ) fclose( reshapeFunction.fp );  
					break ; 


		case 'p':		if( silentflag ) fprintf( stderr, "FUNCTION MAPPED THROUGH INPUT . . ." ) ;
					strcpy(tempstring, arg_option);
					reshapeFunction.fp = crackstring( tempstring, &reshapeFunction );
					funcStats( &reshapeFunction, 
						&arraylow, &arrayhi, &arrayaverage, &arraylength, &arraymedian ) ; 
					if( (arraylow < 0.) || (arrayhi > 1.) ){
						prt( "\n\nINPUT VALUE ADDRESS FUNCTION MUST BE BETWEEN 0-1.\n\n . . .  BYE\n" ) ;
						exit( EXIT_FAILURE ) ;   
					}else{
						re_fvec( scratchValue, numberOfValues ) ;
						i = 0; 
						for(window = 0; window < numberOfWindows; window++){
							for(nn = 0; nn < windowLengths[window]; nn++){
								funcprop = makeFuncProp( computationLevel, numberOfValues, numberOfWindows,
											windowLengths, window, i, nn ) ;
								reshapeFunction.A[ 0 ] = fval( &reshapeFunction, 1., funcprop );

								realIndex = (float) numberOfValues * (reshapeFunction.A[ 0 ] - arraylow) / (arrayhi - arraylow) ;
								lowIndex = (int) floor( realIndex ) ; hiIndex = (int) ceil( realIndex ) ;
								if( hiIndex >= numberOfValues ) hiIndex = numberOfValues - 1 ;
								frac = realIndex - (float) lowIndex  ;   
								scratchValue[i] = value[lowIndex] + (frac * (value[hiIndex] - value[lowIndex])) ;

								i++ ; 
							} ;
						} ; 
						for(i = 0; i < numberOfValues; i++) {
							value[i] = scratchValue[i] ; 
							if( reshapeDebugFlag  == 1 ) prf( value[i], "RE-MAPPED/SHAPED VALUE" ) ;
						}; 


						if( reshapeFunction.n != 1. ) fclose( reshapeFunction.fp );  
						break ; 
					} ; 

		case 'N':   	if( silentflag ) fprintf( stderr, "OUTPUT WILL BE NUMBERED . . . " ) ; 
					numberflag = 1 ;
					statsprintflag = 0 ;   
					break ;  

		case 'H':   	if( silentflag ) fprintf( stderr, "OUTPUT WILL BE HISTOGRAM" ) ; 
				  	temp = (float)(int) crackfloat( arg_option, ch ) ;
					if( reshapeDebugFlag  == 1 )prf( temp, "temp" ) ; 
					if( temp > 0. ) histogramNumberOfBins = (int) temp ; 
					if( reshapeDebugFlag  == 1 )pri( histogramNumberOfBins, "histogramNumberOfBins" ) ;  
					histogramflag = 1 ;
					statsprintflag = 0 ;   
					break;


		case 'A':   	ASCIIoutflag = (int) crackfloat( arg_option, ch ) ;
					statsprintflag = 0 ;
					break;
		case 'z':   	silentflag = 0 ;
					statsprintflag = 0 ;
					break;
	}


//	if(silentflag && statsprintflag)
	findArrayStats( value, &numberOfValues, &arraylow, &arrayhi, 
							&arrayrange, &arrayaverage, &arraymedian,  &arraymode, &arraystandarddeviation, 
								&arraysum, &arraybegin, &arrayend, &arraymiddle, silentflag ) ; 

	chm2 = chm1 ; chm1 = ch ; 

	if( (chm1 != 'y') && (chm2 != 'y') ){
		lowlimitsaveflag = 0 ; 
	}; 
	if( (chm1 != 'Y') && (chm2 != 'Y') ){
		hilimitsaveflag = 0 ; 
	}; 

//	fprintf(stderr, "\n\nchm2: %c chm1: %c\n", chm2, chm1 ) ; 
	// HERE
}

rewind( stdout ) ; 


if( histogramflag ){
	// HISTOGRAM

	if( silentflag ) prt( "HISTOGRAM OUTPUT FUNCTION" ) ; 

	re_fvec( histogramBins, histogramNumberOfBins ) ;
	for(i = 0; i <  histogramNumberOfBins; i++) histogramBins[i] = 0. ; 

	findArrayStats( value, &numberOfValues, 
		&arraylow, &arrayhi, &arrayrange, &arrayaverage, &arraymedian,  &arraymode, &arraystandarddeviation,
				&arraysum, &arraybegin, &arrayend, &arraymiddle, 0 ) ; 

//prf( arraylow, "arraylow" ) ; 
//prf( arrayhi, "arrayhi"); 

	temp = arrayrange / (float) histogramNumberOfBins ; 

	threshold = arraylow + temp ; 
	k = 0 ; sortFlag=1 ; ch = 'C' ; 
	singleOperator( ch, value, scratchValue, numberOfValues, &dummyStructure, sortFlag, silentflag );
	for(i = 0; i < numberOfValues; i++){
		if( value[i] <= threshold ){ 
			histogramBins[k] += 1. ; 
		} else {
			prf( threshold, "threshold" ); 
			threshold += temp ; k++ ; histogramBins[k] += 1. ; 
		}; 
	}; 

	re_fvec( value, histogramNumberOfBins ) ; 
	numberOfValues = histogramNumberOfBins ; 
	for(i = 0; i < numberOfValues; i++)value[i] = histogramBins[i] ; 

} ; 

	
	nn = 0 ;
//	fprintf( stderr, "\nnumberOfValues: %d", numberOfValues ) ; 
//	fprintf( stderr, "\nnumberflag: %d", numberflag ) ; 

	while ( nn < numberOfValues ){
//		fprintf( stderr, "\nnn: %d", nn ) ; 
		temp = value[ nn ] ; 
    
    
		// OUTPUT THE VALUE AS FLOATS OR ASCII
		if( ASCIIoutflag == 1 ){
//			fprintf( stderr, "nn: %d", nn ) ; 
    
			// ASCII
			if( nn == 0 )
			    if( silentflag ) prt( "(OUTPUT ON)\n" ) ; 

			// OUTPUT A FLOAT
			if( numberflag == 1 ) fprintf( stdout,  "%d\t",  nn ) ; 
//			fprintf( stderr, "\nOUTPUTING ASCII: %f", temp ) ;
			fprintf( stdout,  "%f\n",  temp ) ; 
    
		}else if( isatty(1) != 1 ){
    
			if( nn == 0 )
			    if( silentflag )prt( "(OUTPUT ON)\n" ) ; 
			
			// FLOATS TO REDIRECTED STDOUT
			out_index = (float) nn ; 
			if( numberflag == 1 ) fwrite( &out_index, sizeof(float), 1, stdout ); 
//			fprintf( stderr, "\nOUTPUTING FLOAT TO STDOUT: %f", temp ) ;

			fwrite( &temp, sizeof(float), 1, stdout );
		}else{
			if( nn == 0 ) if( silentflag ) prt( "\n(2. OUTPUT OFF)\n" ) ; 
		}
    
		nn++ ; 
	}
	fseek( stdout, 0, 0 ) ; 
	fflush(stdout);


if( processedOutputFlag == 1 ){
	if( silentflag ) prbanner( "OUTPUT FUNCTION", WIDTH ) ;    
	findArrayStats( value, &numberOfValues, 
			&arraylow, &arrayhi, &arrayrange, &arrayaverage, &arraymedian,  &arraymode, &arraystandarddeviation, 
					&arraysum, &arraybegin, &arrayend, &arraymiddle, silentflag ) ; 
    
    	if( silentflag ) prline( WIDTH, "=" ) ; 
	if( silentflag ) prt( "\n" ) ; 
}; 

//system( filesToRemove ) ;
filesToRemove( NULL, 1 );

exit(EXIT_SUCCESS) ; 

}



void usage()
{
    fprintf(stderr, "%s",
	"reshape:  function reshaper\n"
	"reshape   [flags] <input_file>  (or)\n"
	"   |  reshape [flags] \n\n"
	"   input file: header-less series of ASCII or binary floating-point numbers;\n"
	"       file format is detected automatically.\n\n"  
	"   Using the specified operations, reshape processes the sequence of input values \n"
	"	and outputs the final values to the standard out; operations are applied in the order given.\n\n"
	"	    (Bracketed values indicate defaults.)\n"
	"	    (Parameters with (func) can be controlled with function files.)\n\n" 
	"	    LIMITS\n"
	"   			Limit values if outside of the specified limits.\n"
	"	y:	lower limit [no limiting]\n"
	"	Y:	upper limit [no limiting]\n"
	"	x:	first numbered value (1 through total) [1]\n"
	"	X:	last numbered value [total]\n"
	"	w:  INDEX FOR EXPONENTIAL TIME FUNCTION  (func) [0]\n"
	"   		- Nonlinearize time axis and access value from the new stream of values.\n"
	"		   index of 0: linear time function -- no change.\n"
	"		   indices > 0 produce increasingly accelerating time functions \n"
	"		   indices < 0 produce increasingly deccelerating time functions \n"
	"	R:  	RESAMPLE PROPORTION (func) [1.]\n"
	"   		- Resample input values, adding through interpolation and subtracting through decimation. \n"
	"		    Number of output values = R * old_number.\n"
	"		    Values > 1 invoke interpolation. Values < 1 cause sampling decimation.\n"
	"	V: 	OVERSAMPLE OUTPUT PROPORTION: Increase output by duplicating values\n"
	"   		- Oversample output by integer amount. \n\n"
	"		    -V times; -V will be truncated to integer. (func) [1.]\n"
	"	c:	CSPLINE RESAMPLE LENGTH\n"
	"		- Resample the input values to the specified length using cubic spline interpolation.\n"
	"	    NEW RANGE\n"
	"   		- Fit the values into a new range.\n"
	"	k:	CYCLE DUPLICATION FACTOR\n"
	"		- Factor specifies number of appended function sets to add, including fractional set at end.\n" 
	"	e:	SEED RANDOM NUMBER GENERATOR\n"
	"		- Sets sequence for subsequent calls for random numbers.\n"
	"	K:	SCRAMBLE\n"
	"		- Function order is randomized.\n" 
	"	b:	new range BASE value (func) [old lower boundary]\n"
	"	B:	new range PEAK value (func) [old upper boundary]\n"
	"		    (You must use both -b and -B)\n"
	"	r:  IDENTITY MAPPING FUNCTION: INPUT MAPPED THROUGH IDENTITY (-p reversed)\n"
	"		- Input values are mapped through (i.e. index) identity function; ordinate (y) range of input function\n"
	"			is mapped to abcissa (x) range of mapping function--i.e. min/max of input to begin/end of\n"
	"			mapping function.\n"
	"	p:  IDENTITY MAPPING FUNCTION: IDENTITY MAPPED THROUGH INPUT (-r reversed)\n"
	"		- Identity function is mapped through input values; ordinate (y) range of identity function\n"
	"			is mapped to abcissa (x) range of input values--i.e. min/max of identity function to begin/end of\n"
	"			input values.\n"
	"	W:  WARP INDEX FOR ASYMMETRICAL DISTRIBUTION SHAPING OF OUTPUT VALUES (func) [0]\n"
	"   		- Warp the values toward the upper or lower limits of value range based on index.\n"
	"		   Indices > 0 warp toward lower limit (-b), indices < 0 warp toward upper limit. 0 = no warp.\n"
	"		   Warp limits may be specified directly using the -y and -Y flags; limits must appear \n"
	"		   immediately before the -W flag (.e.g -y1 -Y12 -W3 ). Limit flags function normally,\n"
	"		   limiting value set before their use in warping.\n"
	"	Z:  WARP INDEX FOR MEAN CENTERED SYMMETRICAL DISTRIBUTION SHAPING OF OUTPUT VALUES (func) [0]\n"
	"		- Warp the values toward the mean or the boundaries.\n"
	"		   Indices > 0 warp toward mean; indices < 0 warp toward boundaries. 0 = no warp.\n"
	"	G:  WARP INDEX FOR MID-POINT CENTERED SYMMETRICAL DISTRIBUTION SHAPING OF OUTPUT VALUES (func) [0]\n"
	"		- Warp the values toward either the mid/halfway point or the boundaries.\n"
	"		   Indices > 0 warp toward mean; indices < 0 warp toward boundaries. 0 = no warp.\n"
	"	M:  MULTIPLIER (func) [1.]\n"
	"   		- Amplify or attenuate the values by the multiplier.\n"
	"	m:	SCALE BY MULTIPLIER AROUND MEAN (func)\n"
	"	n:	NORMALIZE \n"
	"	D:	DIVISOR (func) [1.]\n"
	"		- Divide the values by the divisor--non-zero divisors only.\n"
	"		    value = value / D\n"
	"	T:  ADDER (func) [0.]\n"
	"     	- Transpose the value by the adder.\n"
	"		   	value = value + A\n"
	"	E:	TRANSPOSE VALUES BY THE DIFFERENCE OF MEAN FROM TARGET. (func)\n"
	"	I:  	INVERSE DIVISION  [off]\n"
	"   		- Divide the dividend by the values--non-zero values only.\n"
	"		    value = I / value\n"
	"	a:	ABSOLUTE VALUE\n"
	"	d:  DIFFERENCE   [0]\n"
	"		- Replace values with the difference between the current and previous value (differentiate).\n"
	"		    (value[n] = value[n] - value[n - 1])\n"
	"		    0 = off,  1 = difference, 2 = absolute value of difference\n"
	"	s:  SUM y(n) = y(n - 1) + x(n) [off]\n"
	"   		Replace values with the left-to-right (first-in-to-last-in) cumulative sum (integrate).\n" 
	"	   	SCALE:\n"
	"		- Quantize the values using the scale filter.\n"
	"	S:	scale FILE: text file of scale intervals \n"
	"	O:	scale ORIGIN (func) [0.]\n"
	"	Q:	scale QUANTIZATION: 0 = no quantization by scale\n"
	"		    1 = full scale quantization (func) [1.]\n"
	"	    COEFFICIENT(S) for lowpass smoothing filter [0.]\n"
	"   		- Smooth the values with a lowpass filter.\n"
	"	f:	COEFFICIENT for positive slope signal (func) [0.]\n"
	"	F:	COEFFICIENT for negative slope signal (func) [0.]\n"
	"		    Where c = feedback coefficient\n"
	"		    y(n) = (1. - c) * x(n) + c * y(n - 1)\n"
	"		    If x(n) > y(n-1) then c = \"-f\",  else c = \"-F\"\n"
	"	u:	RETROGRADE ORDER of values\n"
	"	t:  TRANSLATION CODE:[no translation]\n"
	"		- Translate the values according to the specified code.\n"
	"		    	0 = amplitude to decibels\n"
	"		    	1 = decibels to amplitude\n"
	"		    	2 = frequency to semitones of deviation\n"
	"				-P reference freq/pitch required with -t2.\n"
	"		    	3 = frequency to semitones of correction\n"
	"				-P reference freq/pitch required with -t3.\n"
	"		    	4 = truncate to integer\n"
	"		    	5 = round to integer\n"
	"			6 = Hertz to MIDI\n" 
	"			7 = MIDI to Hertz\n" 
	"	l:	PROCESSING MODE/LEVEL - function indexing and processing level\n"
	"		  0: individual values, indexed/processed across full range\n"
	"		  1:	individual values, indexed/processed across window range\n"
	"		  2: windows, indexed/processed series of window blocks, treated as units\n"
	"	L:	WINDOW LENGTHS for processing in number of values (func)\n"
	"	P:	REFERENCE frequency or pitch\n"
	"		    Values <= 12.0 will be interpreted as octave.pitchclass notation.\n"
	"	C: 	INDEXABLE ODD-EVEN SORT:\n"
	"   		- Sort values in either ascending order (1.), descending order (-1.), or some\n"
	"			degree of either ascending or descending as proportionally indexed by the \n"
	"			flag +/- sign and magnitude (0-1).\n"
	"	j:	HORIZONTAL (SAMPLED) ASCII TERMINAL PLOT \n"
	"		- X-axis is horizontal; number of lines for y-axis may be set through flag (i.e. -j20).\n"
	"			May be used multiple times at process sequence points.\n" 
	"	J:	VERTICAL (SAMPLED) ASCII TERMINAL PLOT\n"
	"		- X-axis is vertical; number of lines for x-axis may be set through flag (i.e. -J20)\n"
	"			May be used multiple times at process sequence points.\n" 
	"	U:	INTERIM BINARY OUTPUT FILE\n"
	"		- Write current state of values to binary file.\n"
	"	o:	INTERIM OUTPUT SOUND FILE\n"
	"		- Write current state of values to one channel sound file; file must exist and be\n"
	"			formatted as a sound file.\n" 
	"	OUTPUT:\n"
	"	H: 	HISTOGRAM: number of bins [ 100 ]\n"
	"		- Output a histogram. \n\n"
	"	N:	NUMBER OUTPUT--i.e. add x indices--using integers starting from 0.\n"
	"	A:	FORCE OUTPUT FORMAT 1 = ASCII, 0 = binary floats [default: output format same as input]\n"
	"	z:	silent statisitics/error output\n"
	);
    exit(EXIT_SUCCESS);
}

void pd( int i ) { fprintf( stderr, "\n PRINT DEBUG POINT # %d ", i ) ; }

void mapIntoNewBoundaries( 
	float value[], 
	int numberOfValues, 
	struct func *lowbound,	
	struct func *hibound,
	int *lowBoundFlag,
	int *hiBoundFlag
){
	int n ; 
	float peak=0., base=0., range ; 

	// FIND PEAK, BASE, AND RANGE	
	for(n = 0; n < numberOfValues; n++){
		if( (n == 0) || (value[n] > peak) ) peak = value[n] ; 
		if( (n == 0) || (value[n] < base) ) base = value[n] ; 
	} ; 

	range = peak - base ; 
	
	if( range == 0. ){
		prf( base, "BASE VALUE" ) ; 
		prf( peak, "PEAK VALUE" ) ; 
		prt( "\n ERROR ======> BASE AND PEAK ARE EQUAL; NO RANGE TO REMAP.\n\n . . . .BYE.\n\n" );
		exit( EXIT_FAILURE ) ;  
	}else{
		for(n = 0; n < numberOfValues; n++){
			lowbound->A[ 0 ] = fval( lowbound, (float) (numberOfValues - 1.), (float) n );
			hibound->A[ 0 ] = fval( hibound, (float) (numberOfValues - 1.), (float) n );

			value[n] = lowbound->A[ 0 ] + 
					( 
						(hibound->A[ 0 ] - lowbound->A[ 0 ])   *   ((value[n] - base) / range)
					) ; 
			if( reshapeDebugFlag  == 1 ) prf( value[n], "REMAPPED VALUE" ) ; 

		} ; 
	} ; 
	*lowBoundFlag = 0; *hiBoundFlag = 0 ; 

};


void lowpassFilter( 
	float value[],
	int numberOfValues, 
	struct func *feedcu, 
	struct func *feedcd, 
	int *positiveFeedFlag,
	int *negativeFeedFlag 
){
	int n ; 
	float oldVal, feedcum, feedcdm ; 

	oldVal = value[0] ; 
	for(n = 0; n < numberOfValues; n++){
		// LOWPASS SMOOTH
		if( value[n] > oldVal ){
			// RISING SIGNAL
			feedcu->A[ 0 ] = fval( feedcu, (float) (numberOfValues - 1), (float) n );
			feedcum = 1. - feedcu->A[ 0 ] ; 
		     value[n] = feedcum * value[n] + feedcu->A[ 0 ] * oldVal ;
		}else{
			// FALLING SIGNAL
			feedcd->A[ 0 ] = fval( feedcd, (float) (numberOfValues - 1), (float) n );
			feedcdm = 1. - feedcd->A[ 0 ]  ; 
		     value[n] = feedcdm * value[n] + feedcd->A[ 0 ] * oldVal ;
		}
		oldVal = value[n] ; 
		if( reshapeDebugFlag  == 1 ) prf( value[n], "VALUE LOWPASSED" ) ;
	} ; 

	*positiveFeedFlag = 0; *negativeFeedFlag = 0 ; 
}; 

void singleOperator(
	char ch,  
	float value[],
	float scratchValue[],
	int numberOfValues, 
	struct func *operator,
	float flagValue,
	int silentflag
	
){

	int n, i, ii, notdone, steps, stepstotake=0, l ; 
	float tProp, base=0., peak=0., lowVal, highVal, fraction, realIndex, oldVal, thisVal, 
		temp, midpoint, halfofarrayrange ; 
	float arraylow, arrayhi, arrayrange, arrayaverage, arraymedian, arraymode,
				arraystandarddeviation, arraysum, arraybegin, arrayend, arraymiddle ; 

	findArrayStats( value, &numberOfValues, 
		&arraylow, &arrayhi, &arrayrange, &arrayaverage, &arraymedian,   &arraymode, &arraystandarddeviation, 
			&arraysum, &arraybegin, &arrayend, &arraymiddle, 0 ) ; 
				


	if( silentflag ) prt( "**********************" ) ; 

	switch(ch) {
			
		case 'C':for(l = 0; l < 2; l++){
					for(i = 0; i < numberOfValues; i++ ) scratchValue[i] = value[i] ;
					notdone = 1 ; ii = 0 ; steps = 0 ;
					while( notdone ){ 
						notdone = 0 ; 
	    					for( i = ii ; i  < (numberOfValues - 1); i += 2 ){
							if( (l == 1) && (steps >= stepstotake) ) {
								notdone = 0 ; break ; 
							}else{								
								if( flagValue > 0. ){
		    							// ASCENDING
		    							if( scratchValue[i] > scratchValue[i + 1] ){
										// SWITCH
										temp = scratchValue[i] ; scratchValue[i] = scratchValue[i + 1] ; 
											scratchValue[i + 1] = temp ; 
										notdone = 1 ; steps++ ;  
		    							}
								}else if( flagValue < 0.){
		    							// DESCENDING
		    							if( scratchValue[i] < scratchValue[i + 1] ){
										// SWITCH
										temp = scratchValue[i] ; scratchValue[i] = scratchValue[i + 1] ; 
											scratchValue[i + 1] = temp ; 
										notdone = 1 ; steps++ ; 
		    							}
								}
							} ; 				

	    					}
	    					if( ii == 0 ) ii = 1 ; else ii = 0 ; 
					} ; 
					stepstotake = (int)((float) steps * fabs( flagValue ) ) ; 
				} ;
				for(i = 0; i < numberOfValues; i++ ) value[i] = scratchValue[i] ; 
				if( reshapeDebugFlag  == 1 )for(n = 1; n < numberOfValues; n++) prf( value[n], "SORTED VALUE" ) ;
				break ; 


		case 's':	for(n = 1; n < numberOfValues; n++) {
					value[n] = value[n - 1] + value[n] ;
					if( reshapeDebugFlag  == 1 ) prf( value[n], "INTEGRATION VALUE" ) ;
				} ; 
				break ; 

		case 'd':	oldVal = value[0] - (value[1] - value[0]) ; 
				for(n = 0; n < numberOfValues; n++) {
					thisVal = value[n] ; 
					value[n] = thisVal - oldVal ; 
					if( flagValue == 2 ) value[n] = fabs( value[n] ); 
					oldVal = thisVal ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "DIFFERENCE VALUE" ) ;
				} ; 
				break ; 

		case 'w':	
				for(n = 0; n < numberOfValues; n++){
					tProp = (float) n / (float)(numberOfValues - 1) ; 
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					realIndex = curve(0., (float)(numberOfValues - 1), tProp, operator->A[ 0 ] ) ;
					lowVal = value[ (int) realIndex ] ; 
					highVal = value[ (int)(realIndex + .5) ] ;
					fraction = realIndex - (float)( (int)realIndex ) ;   
					scratchValue[n] = lowVal + (fraction * (highVal - lowVal)) ; 
				} ;

				for(n = 0; n < numberOfValues; n++) {
					value[n] = scratchValue[n] ; 
					if( reshapeDebugFlag == 1 ) prf( value[n], "VALUE X-WARPED" ) ;
				}; 

				break ; 


		case 'W':	if( arrayhi != arraylow ) for(n = 0; n < numberOfValues; n++){ 
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					value[n] = curve( arraylow, arrayhi, (value[n] - arraylow) / (arrayhi - arraylow), operator->A[ 0 ] ) ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "VALUE ASYMMETRICALLY Y-WARPED" ) ;
				} ;
				break ; 

		case 'Z':	if( arrayhi != arraylow ) for(n = 0; n < numberOfValues; n++){
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					if( value[n] < arraymedian ) 
						value[n] = arraymedian - curve( 0., arraymedian - arraylow, 
							( arraymedian - value[n] ) / (arraymedian - arraylow), operator->A[ 0 ] ) ; 
					else
						value[n] = arraymedian + curve( 0., arrayhi - arraymedian, 
							( value[n] - arraymedian ) / (arrayhi - arraymedian), operator->A[ 0 ] ) ; 
						
					if( reshapeDebugFlag  == 1 ) prf( value[n], "MEDIAN VALUE SYMMETRICALLY Y-WARPED" ) ;
				} ;
				break ; 

		case 'G':	if( arrayrange != 0. ) for(n = 0; n < numberOfValues; n++){  
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );
					halfofarrayrange = 0.5 * arrayrange ; midpoint = arraylow + halfofarrayrange ;  
					if( value[n] < midpoint ) 
						value[n] = midpoint - curve( 0., halfofarrayrange, 
							( midpoint - value[n] ) / halfofarrayrange, operator->A[ 0 ] ) ; 
					else
						value[n] = midpoint + curve( 0., halfofarrayrange, 
							( value[n] - midpoint ) / halfofarrayrange, operator->A[ 0 ] ) ; 
						
					if( reshapeDebugFlag  == 1 ) prf( value[n], "VALUE SYMMETRICALLY Y-WARPED" ) ;
				} ;
				break ; 






		case 'M': for(n = 0; n < numberOfValues; n++){
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					value[n] = value[n] * operator->A[ 0 ] ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "VALUE MULTIPLIED" ) ;
				} ;
				break ; 


		case 'm': for(n = 0; n < numberOfValues; n++){
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					value[n] = ((value[n] - arrayaverage) * operator->A[ 0 ]) + arrayaverage ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "VALUE SCALED AROUND MEAN" ) ;
				} ;
				break ; 

		case 'n':	if( (arrayhi - arrayaverage) > (arrayaverage - arraylow) ) temp = 1. / (arrayhi - arrayaverage) ; 
				else temp = 1. / (arrayaverage - arraylow) ; 
				for(n = 0; n < numberOfValues; n++){
					value[n] = (value[n] - arrayaverage) * temp ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "NORMALIZED VALUE" ) ;
				} ;
				break ; 

		case 'E': for(n = 0; n < numberOfValues; n++){
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					value[n] = value[n]  + (operator->A[ 0 ] - arrayaverage) ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "MEAN VALUE TRANSPOSED TO TARGET" ) ;
				} ;
				break ; 


		case 'D': for(n = 0; n < numberOfValues; n++){
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					value[n] = value[n] / operator->A[ 0 ] ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "VALUE DIVIDE" ) ;
				} ;
				break ; 

		case 'I': for(n = 0; n < numberOfValues; n++){
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					value[n] = operator->A[ 0 ] / value[n] ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "VALUE DIVIDE INVERSE" ) ;
				} ;
				break ; 

		case 'T': for(n = 0; n < numberOfValues; n++){
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					value[n] = value[n] + operator->A[ 0 ] ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "VALUE ADDED" ) ;
				} ;
				break ; 

		case 'y': for(n = 0; n < numberOfValues; n++){
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					if( value[n] < operator->A[ 0 ] ) value[n] = operator->A[ 0 ] ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "LOW LIMITED VALUE" ) ;
				} ;
				break ; 
		case 'Y': for(n = 0; n < numberOfValues; n++){
					operator->A[ 0 ] = fval( operator, (float) (numberOfValues - 1.), (float) n );			
					if( value[n] > operator->A[ 0 ] ) value[n] = operator->A[ 0 ] ; 
					if( reshapeDebugFlag  == 1 ) prf( value[n], "HIGH LIMITED VALUE" ) ;
				} ; 
				break ; 






	} ; 
};



float makeFuncProp(
	int computationLevel,
	int numberOfValues,
	int numberOfWindows,
	int windowLengths[],
	int window,
	int i,
	int nn
)
{
	float funcprop ;

	if(computationLevel == 0) 
		funcprop = numberOfValues > 1 ? 
			(float) i /  (float) (numberOfValues - 1.) : 1. ; 
	else if(computationLevel == 1) 
		funcprop = windowLengths[window] > 1 ? 
			(float) nn / (float) (windowLengths[window] - 1) : 1. ; 
	else 
		funcprop = numberOfWindows > 1 ? 
			(float) window / (float) (numberOfWindows - 1) : 1. ; 

	return( funcprop ); 
} ; 
