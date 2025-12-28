#include <stdio.h>
#include <math.h>
#include "pv.h"

float randf( float min, float max )
{
return( ((float)(random()/(float)0x7fffffff)*(max-min))+min );
}
int randi( min, max ) int min, max;
{
return( ((random()%(max-min))+min ));
}
