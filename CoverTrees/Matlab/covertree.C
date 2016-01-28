//mex -g -DDOUBLE covertree.C ThreadsWithCounter.C IDLList.C IDLListNode.C Vector.C Cover.C Point.C CoverNode.C EnlargeData.C Timer.C -lpthread

/*
 On OS/X, I had to run
 mex -setup
 from the MATLAB command line first. This creates the file
 ~/.matlab/mexopts.sh
 I then modified this file, in particular I set
 CC='gcc'
 in order to use gcc and not a specified version of gcc (as in the mexopts.sh file created by MATLAB) and
 SDKROOT='/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/'
 instead of the one created by MATLAB as things have moved with the latest version of Xcode (and Matlab also does not seem to properly recognized the latest version of OS X).
 */

/*
 outparams[0]=vectors.getIndex(cover.getRoot()->getPoint());
 outparams[1]=cover.getMinLevel();
 outparams[2]=cover.getNumLevels();
 outparams[3]=cover.getCount();
 outparams[4]=cover.getNumberInserted();
 outparams[5]=cover.getNumberDeep();
 outparams[6]=cover.getNumberDuplicates();
 outparams[7]=DISTANCE_FCN;
 */

#include "mex.h"
#include <math.h>
#include "Cover.H"
#include "string.h"
#include "Vector.H"
#include "ThreadsWithCounter.H"
#include "EnlargeData.H"
#include "Timer.H"
#include "covertree_MEX.H"

int    dim=0;
REAL theta=0.0;
REAL mu=0.0;

mwSize dims[2];
int ndims=2;

const char* fnames_in[]={
    "theta",
    "numlevels",
    "minlevel",
    "NTHREADS",
    "BLOCKSIZE",
    "distancefcn",
    "classname"
};

const char* fnames_out[]={
    "theta",
    "outparams",
    "radii",
    "levels",
    "ncallstogetdist",
    "distancefcn"
};


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    Distance_Mode DISTANCE_FCN = EUCLIDEAN;
    VectorClassNames VECTOR_CLASS;

#ifdef DOUBLE
    char mxIsClassName[] = "double";
#else
    char mxIsClassName [] = "single";
#endif

    /* Check for proper number of arguments. */
    if (nrhs != 2)                                                  { mexErrMsgTxt("Two inputs required."); }
    
    int nfields = mxGetNumberOfFields(prhs[0]);
    if(nfields<5)                                                   { mexErrMsgTxt("First input must have at least 5 fields."); }
    
    // Get theta
    const mxArray* tmp=mxGetField(prhs[0],0,fnames_in[0]);
    if(!mxIsClass(tmp,mxIsClassName))                                { mexErrMsgTxt("input.theta must be double\n"); }
    REAL* ptheta=(REAL*)mxGetData(tmp);
    theta=*ptheta;
    bool val=(theta>0.0)&&(theta<1.0);
    if(!val)                                                        { mexErrMsgTxt("Bad theta\n"); }
    mu=1.0/(1.0-theta);
    
    // Get outparams
    
    tmp=mxGetField(prhs[0],0,fnames_in[1]);
    if(!mxIsClass(tmp,"int32"))                                     { mexErrMsgTxt("input.numlevels must be int32\n"); }
    int* pnumlevels=(int*)mxGetData(tmp);
    int numlevels=(int)*pnumlevels;
    
    // Get minlevel
    tmp=mxGetField(prhs[0],0,fnames_in[2]);
    if(!mxIsClass(tmp,"int32"))                                     { mexErrMsgTxt("input.minlevel must be int32\n"); }
    int* pminlevel=(int*)mxGetData(tmp);
    int minlevel=(int)*pminlevel;
    
    // Get NTHREADS
    tmp=mxGetField(prhs[0],0,fnames_in[3]);
    if(!mxIsClass(tmp,"int32"))                                     { mexErrMsgTxt("input.NTHREADS must be int32\n"); }
    int* pNTHREADS=(int*)mxGetData(tmp);
    int NTHREADS=(int)*pNTHREADS;
    
    // Get BLOCKSIZE
    tmp=mxGetField(prhs[0],0,fnames_in[4]);
    if(!mxIsClass(tmp,"int32"))                                     { mexErrMsgTxt("input.BLOCKSIZE must be int32\n"); }
    int* pBLOCKSIZE=(int*)mxGetData(tmp);
    int BLOCKSIZE=(int)*pBLOCKSIZE;
    
    // Get distancefcn
    tmp=mxGetField(prhs[0],0,fnames_in[5]);
    if(tmp==NULL)   {
        DISTANCE_FCN = EUCLIDEAN;
    }
    else {
        if(!mxIsClass(tmp,"int32")) {
            mexErrMsgTxt("input.distancefcn must be int32\n");
        }
        else    {
            int *pDISTANCE_FCN = (int*)mxGetData(tmp);
            DISTANCE_FCN = (Distance_Mode)(*pDISTANCE_FCN);
        }
    }
    // Get classname
    tmp=mxGetField(prhs[0],0,fnames_in[6]);
    if(tmp==NULL)   {
        DISTANCE_FCN = EUCLIDEAN;
    }
    else {
        if(!mxIsClass(tmp,"int32")) {
            mexErrMsgTxt("input.classname must be int32\n");
        }
        else    {
            int *pVECTOR_CLASS = (int*)mxGetData(tmp);
            VECTOR_CLASS = (VectorClassNames)(*pVECTOR_CLASS);
        }
    }
    
    //mexPrintf("DISTANCE_FCN=%d\n",DISTANCE_FCN);
    
    /* Get dimensions of second field of input */
    
    tmp=prhs[1];
    mwSize ndims_in=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_in=mxGetDimensions(tmp);
    if(!mxIsClass(tmp,mxIsClassName)) {
        mexErrMsgTxt("Second input must be double\n");
    }
    
    
//    mexPrintf("ndims_in=%d\n",ndims_in);
    
    int N=dims_in[ndims_in-1];
    dim=1;
    for(int i=0;i<ndims_in-1;i++) {
        dim*=dims_in[i];
    }
    
//    mexPrintf("dim=%d N=%d\n",dim,N);
    
    // Create Vectors
    REAL* X=(REAL*)mxGetData(tmp);
    Vectors *vectors;
    switch (VECTOR_CLASS)   {
        case VECTORS:
            vectors = new Vectors(X,N,dim,DISTANCE_FCN);
            break;
        default:
            mexErrMsgTxt("\n Invalid classname for Vectors.");
            break;
    }
    
    if( N==1 ) {
        NTHREADS = 0;
    }
    ThreadsWithCounter threads(NTHREADS);
    
    // Construct covertree
    SegList<DLPtrListNode<CoverNode> > seglist(1024);
    const Vector* vector=(vectors->next());
    Cover cover(vector,seglist,numlevels,minlevel);
    EnlargeData enlargedata(&threads,BLOCKSIZE,vectors->getRemaining());
//    Timer timer;
//    timer.on();
    cover.enlargeBy(enlargedata,*vectors);
//    timer.off();
//    timer.printOn(cout);
    
    
    /* Create matrix for the return argument. */
    plhs[0] = mxCreateStructMatrix(1, 1, 5, fnames_out);
    
    mxArray* fout;
    
    dims[0]=1;
    dims[1]=1;
#ifdef DOUBLE
    fout =mxCreateNumericArray(ndims,dims,mxDOUBLE_CLASS,mxREAL);
#else
    fout =mxCreateNumericArray(ndims,dims,mxSINGLE_CLASS,mxREAL);
#endif
    REAL* p=(REAL*)mxGetData(fout);
    mxSetField(plhs[0],0,fnames_out[0],fout);
    p[0]=ptheta[0];
    
    dims[0]=1;
    dims[1]=9;
    fout = mxCreateNumericArray(ndims,dims,mxINT32_CLASS,mxREAL);
    int* outparams=(int*)mxGetData(fout);
    mxSetField(plhs[0],0,fnames_out[1], fout);
    
    outparams[0]=vectors->getIndex(cover.getRoot()->getPoint());
    outparams[1]=cover.getMinLevel();
    outparams[2]=cover.getNumLevels();
    outparams[3]=cover.getCount();
    outparams[4]=cover.getNumberInserted();
    outparams[5]=cover.getNumberDeep();
    outparams[6]=cover.getNumberDuplicates();
    outparams[7]=DISTANCE_FCN;
    outparams[8]=VECTOR_CLASS;
    
    dims[0]=1;
    dims[1]=numlevels;
#ifdef DOUBLE
    fout=mxCreateNumericArray(ndims,dims,mxDOUBLE_CLASS,mxREAL);
#else
    fout=mxCreateNumericArray(ndims,dims,mxSINGLE_CLASS,mxREAL);
#endif
    REAL* pradii=(REAL*)mxGetData(fout);
    pradii[0]=cover.getMaxRadius();
    for(int i=1;i<numlevels;i++) {
        pradii[i]=ptheta[0]*pradii[i-1];
    }
    mxSetField(plhs[0],0,fnames_out[2], fout);
    
    dims[0]=N;
    dims[1]=5;
    fout =mxCreateNumericArray(ndims,dims,mxINT32_CLASS,mxREAL);
    int* base=(int*)mxGetData(fout);
    CoverIndices coverindices(&cover,vectors,base);
    mxSetField(plhs[0],0,fnames_out[3], fout);
    
    dims[0]=1;
    dims[1]=2;
    fout =mxCreateNumericArray(ndims,dims,mxINT64_CLASS,mxREAL);
    long int* pncalls=(long int*)mxGetData(fout);
    mxSetField(plhs[0],0,fnames_out[4], fout);
    pncalls[0]=enlargedata.getMergeNCallsToGetDist();
    pncalls[1]=enlargedata.getThreadNCallsToGetDist();
    
    // Clean up
    delete vectors;
}

#include "FastSeg.C"
template class SegList<DLPtrListNode<CoverNode> >;
