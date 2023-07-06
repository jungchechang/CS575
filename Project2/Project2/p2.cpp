#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>


int	NowYear;		// 2023 - 2028
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// rye grass height in inches
int	    NowNumRabbits;		// number of rabbits in the current population
int     NowNumWolf;     // number of my agent in the current population

const float RYEGRASS_GROWS_PER_MONTH =		20.0;
const float ONE_RABBITS_EATS_PER_MONTH =	 1.0;

const float AVG_PRECIP_PER_MONTH =	       12.0;	// average
const float AMP_PRECIP_PER_MONTH =		4.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				60.0;
const float MIDPRECIP =				14.0;

unsigned int seed = 0;

float
Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}

float
Sqr( float x )
{
        return x*x;
}

void
Watcher()
{
    while( NowYear < 2029 )
    {
        // DoneComputing barrier:
        #pragma omp barrier
        
        // DoneAssigning barrier:
        #pragma omp barrier
        
        float TempC = (5./9.) * (NowTemp - 32);
        float PrecipCm = NowPrecip * 2.54;
        
        printf("%d, %2d, %6.2f, %6.2f, %6.2f, %2d, %2d\n", NowYear, NowMonth+1, TempC, PrecipCm, NowHeight*2.54, NowNumRabbits, NowNumWolf);

        
        float ang = ( 30.*(float)NowMonth + 15. ) * ( M_PI / 180. );
        
        float temp = AVG_TEMP - AMP_TEMP * cos( ang );
        NowTemp = temp + Ranf( &seed, -RANDOM_PRECIP, RANDOM_PRECIP );
        
        float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
        NowPrecip = precip + Ranf( &seed, -RANDOM_PRECIP, RANDOM_PRECIP );
        if( NowPrecip < 0. )
            NowPrecip = 0.;
        
        NowMonth++;
        if( NowMonth > 11 ){

            NowYear++;
            NowMonth = 0;
        }

        // DonePrinting barrier:
        #pragma omp barrier
    }
}

void
Rabbits()
{
    while( NowYear < 2029 )
    {
        int nextNumRabbits = NowNumRabbits;
        int carryingCapacity = (int)( NowHeight );
        int wolf = NowNumWolf;
        if( nextNumRabbits < carryingCapacity )
            nextNumRabbits++;
        else
                if( nextNumRabbits > carryingCapacity )
                    nextNumRabbits--;

        if (wolf > NowNumRabbits){
            nextNumRabbits--;
        }

        if( nextNumRabbits < 0 )
            nextNumRabbits = 0;

        // DoneComputing barrier:
        #pragma omp barrier
        
        NowNumRabbits = nextNumRabbits;

        // DoneAssigning barrier:
        #pragma omp barrier
        
        // DonePrinting barrier:
        #pragma omp barrier
    }
}

void
RyeGrass()
{
    while( NowYear < 2029 )
    {
        float nextHeight = NowHeight;
        float tempFactor = exp(   -Sqr(  ( NowTemp - MIDTEMP ) / 10.  )   );
        float precipFactor = exp(   -Sqr(  ( NowPrecip - MIDPRECIP ) / 10.  )   );
        
        nextHeight += tempFactor * precipFactor * RYEGRASS_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumRabbits * ONE_RABBITS_EATS_PER_MONTH;

        if( nextHeight < 0. )
            nextHeight = 0.;
        
        // DoneComputing barrier:
        #pragma omp barrier
        
        NowHeight = nextHeight;
        
        // DoneAssigning barrier:
        #pragma omp barrier
        
        // DonePrinting barrier:
        #pragma omp barrier
    }
}

void
Wolf()
{
    while( NowYear < 2029 )
    {
        int nextNumWolf = NowNumWolf;
        
        if(NowNumWolf > NowNumRabbits)
        {
            nextNumWolf-=2;
        }
        else if(NowNumWolf < NowNumRabbits)
        {
            nextNumWolf++;
        }

        if(nextNumWolf < 0)
        {
            nextNumWolf = 0;
        }
        
        // DoneComputing barrier:
        #pragma omp barrier
        
        NowNumWolf = nextNumWolf;
        
        // DoneAssigning barrier:
        #pragma omp barrier
        
        // DonePrinting barrier:
        #pragma omp barrier
    }
}


int
main( int argc, char *argv[] )
{
    #ifdef _OPENMP
	//fprintf( stderr, "OpenMP is supported -- version = %d\n", _OPENMP );
    #else
        fprintf( stderr, "No OpenMP support!\n" );
        return 1;
    #endif


    // starting date and time:
    NowMonth =    0;
    NowYear  = 2023;
    // starting state:
    NowNumRabbits = 1;
    NowNumWolf = 1;
    NowHeight =  5.;
    
    float ang = ( 30.*(float)NowMonth + 15. ) * ( M_PI / 180. );
    
    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf( &seed, -RANDOM_PRECIP, RANDOM_PRECIP );
    
    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf( &seed, -RANDOM_PRECIP, RANDOM_PRECIP );
    if( NowPrecip < 0. )
        NowPrecip = 0.;
    
    omp_set_num_threads( 4 );
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            Rabbits();
        }
        #pragma omp section
        {
            RyeGrass();
        }
        #pragma omp section
        {
            Watcher();
        }
        #pragma omp section
        {
            Wolf();
        }
    }
    
    return 0;
}
