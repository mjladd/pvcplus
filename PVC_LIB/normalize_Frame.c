    
	//  NORMALIZE AND REPLACE INTO CHANNEL.
		// FIND NORMALIZATION FACTOR.
	tempChannelAmpSum = 0. ; 	
	for( i = 0; i < N; i+= 2 ){
		tempChannelAmpSum += channel[i] ;
	} ; 
		// NORMALIZE AND TRANSFER
	if( (tempChannelAmpSum > 0.0) && (frameNormalizationAmpLimit != 1.0 ) ){
		if( Normalize_to__Input_Sound_0__Filter_1 == 0 ){
			normalizationAmp = channelAmpSum / tempChannelAmpSum ;
		}else {
			normalizationAmp = filterChannelAmpSum / tempChannelAmpSum ;
		} ; 

		if( normalizationAmp > frameNormalizationAmpLimit )
			normalizationAmp = frameNormalizationAmpLimit ; 
		for( i = 0; i < N; i+= 2 )
			channel[i] = channel[i] * normalizationAmp ; 	
	} ; 

