#ifndef TYPES_H
#define TYPES_H

namespace CoverTreeTypes {

#ifdef DOUBLE
    typedef double REAL;
#define REAL_MAX DBL_MAX
#define COPY dcopy
#define AXPY daxpy
#define GESVD dgesvd
#define NRM2 dnrm2
#define FFTW_PLAN   fftw_plan
#define FFTW_COMPLEX fftw_complex
#define FFTW_EXECUTE_DFT_C2R fftw_execute_dft_c2r
#define FFTW_EXECUTE_DFT_R2C fftw_execute_dft_r2c
#define FFTW_PLAN_DFT_R2C_2D fftw_plan_dft_r2c_2d
#define FFTW_PLAN_DFT_C2R_2D fftw_plan_dft_c2r_2d
#define FFTW_EXECUTE fftw_execute
#define FFTW_DESTROY_PLAN fftw_destroy_plan
#define FFTW_MALLOC fftw_malloc
#endif

#ifdef FLOAT
    typedef float REAL;
#define REAL_MAX FLT_MAX
#define COPY scopy
#define AXPY saxpy
#define GESVD sgesvd
#define NRM2 snrm2
#define FFTW_PLAN   fftwf_plan
#define FFTW_COMPLEX fftwf_complex
#define FFTW_EXECUTE_DFT_C2R fftwf_execute_dft_c2r
#define FFTW_EXECUTE_DFT_R2C fftwf_execute_dft_r2c
#define FFTW_PLAN_DFT_R2C_2D fftwf_plan_dft_r2c_2d
#define FFTW_PLAN_DFT_C2R_2D fftwf_plan_dft_c2r_2d
#define FFTW_EXECUTE fftwf_execute
#define FFTW_DESTROY_PLAN fftwf_destroy_plan
#define FFTW_MALLOC fftwf_malloc
#endif
    
typedef unsigned int INDEX;
#define INDEX_MAX UINT_MAX
}

#endif
    


