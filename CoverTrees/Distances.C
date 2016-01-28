#include <assert.h>
#include <sys/param.h>
#include <algorithm>

#ifdef _EIGEN_
#include <Eigen/Dense>
#endif

#include "Distances.H"
#include "Vector.H"
#include "MemoryDebugger.H"

//#define DEBUG_VECTOR
//#define CHECK_DISTANCE
//#define _ACCELERATE_ON_



#ifdef _ACCELERATE_ON_
float EuclideanDist_accelerate(const float *x, const float *y, unsigned int count);
#endif



REAL EuclideanNorm( const REAL *v, unsigned long int dim)  {
    return sqrt(EuclideanNormSq(v,dim));
}

REAL EuclideanNormSq( const REAL *v, unsigned long int dim)  {
    REAL normsq=0.0;
#if defined(_ACCELERATE_ON_)
#ifdef DOUBLE
    accelerate::vDSP_dotprD( (const double *)v,1,(const double *)v,1,&normsq,dim );
#else
    accelerate::vDSP_dotpr( (const float *)v,1,(const float *)v,1,&normsq,dim );
#endif
#elif defined( _EIGEN_ )
    using namespace Eigen;
#ifdef DOUBLE
    Map<RowVectorXd> vec((double*)v,dim);
#else
    Map<RowVectorXf> vec((float*)v,dim);
#endif
    normsq = vec.squaredNorm();
#else
    for(int k=0;k<dim;k++) {
        normsq+=v[k]*v[k];
    }
#endif
    return normsq;
}

REAL EuclideanNormSq_stable( const REAL *v, unsigned long int dim)  {
    REAL normsq=0.0;
    REAL max_v = maxabs( v, dim );
    REAL tmp_v;
    
    REAL *w = new REAL[dim];
    memcpy(w, v, dim*sizeof(REAL));
    
    std::sort(w, w+dim);
    
    for(int k=0;k<dim;k++) {
        tmp_v  = w[k]/max_v;
        normsq+=tmp_v*tmp_v;
    }
    
    normsq *= max_v*max_v;
    
    delete [] w;
    
    return normsq;
}


REAL EuclideanDist( const REAL*v1, const REAL *v2, unsigned long int dim) {
    REAL distsq;
#if defined(_ACCELERATE_ON_)
#ifdef DOUBLE
    accelerate::vDSP_distancesqD( (const double *)v1,1,(const double *)v2,1,&distsq,dim );
#else
    accelerate::vDSP_distancesq( (const float *)v1,1,(const float *)v2,1,&distsq,dim );
#endif
    return sqrt(distsq);
#elif defined(_EIGEN_)
    using namespace Eigen;
#ifdef DOUBLE
    Map<RowVectorXd> vec1( (double*)v1,dim);
    Map<RowVectorXd> vec2( (double*)v2,dim);
    RowVectorXd v = vec1 - vec2;
#else
    Map<RowVectorXf> vec1( (float*)v1,dim);
    Map<RowVectorXf> vec2( (float*)v2,dim);
    RowVectorXf v = vec1 - vec2;
#endif
    
    return v.norm();
#else
    REAL diff=0.0;
    for(unsigned int k=0, distsq = 0.0;k<dim;k++) {
        diff    = v1[k]-v2[k];
        distsq += diff*diff;
    }
    return sqrt(distsq);
#endif
}


REAL EuclideanDistModReflections( const REAL *v1, const REAL *v2, unsigned long int dim) {
    REAL distsq=0.0, distsqrefl=0.0;
#if defined(_ACCELERATE_ON_)
    distsq = EuclideanDist( v1, v2, dim );
    REAL *v3 = (REAL*)malloc( sizeof(REAL) * dim );
    REAL f = -1.0;
#if defined(DOUBLE)
    accelerate::vDSP_vsmulD( v1, 1, &f, v3,1, dim );
#else
    accelerate::vDSP_vsmul( v1, 1, &f, v3,1, dim );
#endif
    distsqrefl = EuclideanDist( v3, v2, dim );
    free( v3 );
#elif defined(_EIGEN_)
    using namespace Eigen;
#ifdef DOUBLE
    Map<RowVectorXd> vec1( (double*)v1,dim);
    Map<RowVectorXd> vec2( (double*)v2,dim);
    RowVectorXd vd1 = vec1 - vec2;
    RowVectorXd vd2 = vec1 + vec2;
#else
    Map<RowVectorXf> vec1( (float*)v1,dim);
    Map<RowVectorXf> vec2( (float*)v2,dim);
    RowVectorXf vd1 = vec1 - vec2;
    RowVectorXf vd2 = vec1 + vec2;
#endif
    
    distsq      = vd1.norm();
    distsqrefl  = vd2.norm();
#else
    REAL diff;
    for(int k=0;k<dim;k++) {
        diff=v1[k]-v2[k];
        distsq+=diff*diff;
    }
    distsq = sqrt(distsq);
    for(int k=0;k<dim;k++) {                                            
        diff=v1[k]+v2[k];
        distsqrefl+=diff*diff;
    }
    distsqrefl = sqrt(distsqrefl);
#endif
    return (REAL)(distsq < distsqrefl ? distsq : distsqrefl);
}


template <class REALNUMBER>
REALNUMBER max ( const REALNUMBER* v, unsigned long int sz, unsigned long int *max_idx )  {
    REAL curmax = 0;            // MM:TBD: should put here minimum REAL VALUE POSSIBLE
    
    if (max_idx!=0) {
        for( unsigned long int i=0; i<sz; i++)
            if( v[i] > curmax ) {
                curmax = v[i];
                *max_idx = i;
            }
    }
    else    {
        for( unsigned long int i=0; i<sz; i++)
            if( v[i] > curmax )
                curmax = v[i];
    }
    return curmax;
}

template double max<double> ( const double* v, unsigned long int sz, unsigned long int *max_idx );
template float max<float> ( const float* v, unsigned long int sz, unsigned long int *max_idx );

template <class REALNUMBER>
REALNUMBER maxabs ( const REALNUMBER* v, unsigned long int sz, unsigned long int *max_idx )  {
    REALNUMBER curmax = 0;
    if (max_idx!=0) {
        for( unsigned long int i=0; i<sz; i++) {
            REALNUMBER vabs = abs(v[i]);
            if( vabs > curmax )     {
                curmax = vabs;
                *max_idx = i;
            }
        }
    }
    else    {
        for( unsigned long int i=0; i<sz; i++) {
            REALNUMBER vabs = abs(v[i]);
            if( vabs > curmax )     {
                curmax = vabs;
            }
        }
    }
    return curmax;
}

template float maxabs<float> ( const float* v, unsigned long int sz, unsigned long int *max_idx );
template double maxabs<double> ( const double* v, unsigned long int sz, unsigned long int *max_idx );


template <class REALNUMBER>
void thresholdVec( REALNUMBER* v, unsigned long int sz, REALNUMBER thres )  {
    if( thres>0.0 ) {
        for( unsigned long int i=0; i<sz; i++ ) {
            if (abs(v[i])<thres) {
                v[i] = 0;
            }
        }
    }
}

template void thresholdVec<float>( float* v, unsigned long int sz, float thres );
template void thresholdVec<double>( double* v, unsigned long int sz, double thres );


