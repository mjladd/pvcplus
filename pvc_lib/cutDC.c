void cutDC( 
    float channel[],
    int NC,
    float cutoffFreq
){
    int i ;
    float prop ;  

    for( i = 0; i < NC; i += 2){ 
	if( channel[i + 1] < cutoffFreq ){ 
	    if( channel[i + 1] < 0. ) channel[i + 1] = 0. ; 
	    prop =  (channel[i + 1] / cutoffFreq) ; 
          
//	    prop = pow( (double) prop, (double) 4. ) ; 
          prop = prop * prop * prop * prop ; 
	    channel[ i ] *= prop ; 
	} ; 
    } ; 

} ; 
