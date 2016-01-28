#ifndef CoverTrees_Distances_h
#define CoverTrees_Distances_h

#include <cmath>
#include <complex.h>
#include "Points.H"

#ifdef _ACCELERATE_ON_
namespace accelerate {
#include <Accelerate/Accelerate.h>
}
#endif


// List of available distances.
// To add a new distance, add one item to this list, then write a wrapper and put it among the distance functions below
typedef enum
{
    EUCLIDEAN                                   = 0,
    EUCLIDEAN_ABS_VALUES                        = 1,
} Distance_Mode;

typedef struct Distance_Workspace
{
    Distance_Mode distance_mode;
    
} Distance_Workspace;


// Basic utility functions
bool init_DistanceWorkspace ( Distance_Workspace &ws, Distance_Mode dist_mode, Points *points );
void clear_DistanceWorkspace( Distance_Workspace &ws );

// Distance functions
REAL EuclideanNorm( const REAL *v, unsigned long int dim);
REAL EuclideanNormSq( const REAL *v, unsigned long int dim);
REAL EuclideanNormSq_stable( const REAL *v, unsigned long int dim);
REAL EuclideanDist( const REAL*v1, const REAL *v2, unsigned long int dim);
REAL EuclideanDistModReflections( const REAL *v1, const REAL *v2, unsigned long int dim);


// Utilities (probably should be moved elsehwere, e.g. in a new file called VectorUtils
template <class REALNUMBER> REALNUMBER max      ( const REALNUMBER* v,  unsigned long int sz, unsigned long int *max_idx = 0 );
template <class REALNUMBER> REALNUMBER maxabs   ( const REALNUMBER* v,  unsigned long int sz, unsigned long int *max_idx = 0 );
template <class REALNUMBER> void thresholdVec   ( REALNUMBER* v,        unsigned long int sz, REALNUMBER thres = 0.0 );


#endif
