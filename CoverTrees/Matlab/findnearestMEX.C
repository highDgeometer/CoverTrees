//Compile command: mex -O -DDOUBLE findnearest.C ThreadsWithCounter.C IDLList.C IDLListNode.C Vector.C Cover.C Point.C CoverNode.C EnlargeData.C FindWithinData.C findNearest.C -lpthread

/* findnearest take five arguments:
 The first argument is what was returned by covertree
 The second argument is the array that was passed as the second argument to
 covertree
 The third argument is the array of query vectors
 The fourth argument the number of nearest neighbors to find
 The fifth argument, of class int32, is the number of threads (0 for serial).
 
 findnearest returns an k x M matrix of class int32 whose j-th column
 is the indices of k nearest nearest neighbors of the j-th queryvector
 */

#include <math.h>
#include "mex.h"
#include "Cover.H"
#include "Vector.H"
#include "ThreadsWithCounter.H"
#include "FindNearestData.H"
#include "Distances.H"
#include "covertree_MEX.H"

const char* fnames_in[]={
    "theta",
    "outparams",
    "radii",
    "levels",
    "ncallstogetdist",
};

const char* fnames_out[]={
    "indices",
    "distances"
};

int dim=0;
REAL theta, mu;
int ndims=2;
mwSize dims[2];

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    
    Distance_Mode DISTANCE_FCN = EUCLIDEAN;
    VectorClassNames VECTOR_CLASS;

#ifdef DOUBLE
    char mxIsClassName[] = "double";
#else
    char mxIsClassName [] = "single";
#endif
    
    // Check for proper number of arguments. */
    if (nrhs != 5)                                      { mexErrMsgTxt("Five inputs required."); }
    
    // Check the first argument, which is what was returned by covertree
    if(mxGetNumberOfFields(prhs[0])!=5)                 { mexErrMsgTxt("First argument should have 5 fields."); }
    
    // Get theta from first input
    const mxArray* tmp=mxGetField(prhs[0],0,fnames_in[0]);
    if(!mxIsClass(tmp,mxIsClassName))                        {  mexErrMsgTxt("Theta must be double\n");  }
    REAL* ptheta=(REAL*)mxGetData(tmp);
    theta = *ptheta;
    mu=1.0/(1.0-theta);
    
    // Get outparams from first input
    tmp=mxGetField(prhs[0],0,fnames_in[1]);
    if(!mxIsClass(tmp,"int32"))                         { mexErrMsgTxt("outparams must be int32\n"); }
    
    // Get dimensions of second field of first input
    mwSize ndims_in=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_in=mxGetDimensions(tmp);
    bool val=(ndims_in==2)&&(dims_in[0]==1)&&(dims_in[1]==9);
    if(!val)                                            { mexErrMsgTxt("Second field of first input has bad size\n"); }
    int* outparams  = (int*)mxGetData(tmp);
    int minlevel    = outparams[1];
    int numlevels   = outparams[2];
    DISTANCE_FCN    = (Distance_Mode)(outparams[7]);
    VECTOR_CLASS    = (VectorClassNames)(outparams[8]);
    
    tmp=mxGetField(prhs[0],0,fnames_in[3]);
    if(!mxIsClass(tmp,"int32"))                         { mexErrMsgTxt("Fourth field of first input should be int32\n"); }
    mwSize ndims_indices=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_indices=mxGetDimensions(tmp);
    if(ndims_indices!=2)                                { mexErrMsgTxt("Fourth field of first input has bad size\n"); }
    int* indices=(int*)mxGetData(tmp);
    
    /* Get dimensions of second field of input */
    tmp=prhs[1];
    if(!mxIsClass(tmp,mxIsClassName))                        { mexErrMsgTxt("Second input must be double\n"); }
    mwSize ndims_X=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_X=mxGetDimensions(tmp);
    if(!mxIsClass(tmp,mxIsClassName))                        { mexErrMsgTxt("Second input must be double\n"); }
    
    int NX=dims_X[ndims_in-1];
    int d=dims_indices[0];
    if(d!=NX)                                           { mexErrMsgTxt("Mismatch between first and second inputs\n"); }
    dim=1;
    for(int i=0;i<ndims_X-1;i++) {
        dim*=dims_X[i];
    }
    
    // Construct vectors X
    REAL* X=(REAL*)mxGetData(tmp);
    Vectors *vectorsX;
    switch (VECTOR_CLASS)   {
        case VECTORS:
            vectorsX = new Vectors(X,NX,dim,DISTANCE_FCN);
            break;
        default:
            mexErrMsgTxt("\n Invalid classname.");
            break;
    }
    
    // Re-format covertree
    CoverIndices coverindices(theta,numlevels,minlevel,NX,indices);
    SegList<DLPtrListNode<CoverNode> > seglist(1024);
    Cover cover(*vectorsX,seglist,coverindices);
    
    // Construct vectors Y
    tmp=prhs[2];
    mwSize ndims_Y=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_Y=mxGetDimensions(tmp);
    if(!mxIsClass(tmp,mxIsClassName))                        { mexErrMsgTxt("Third argument should be double\n"); }
    int NY=dims_Y[ndims_Y-1];
    int dimY=1;
    for(int i=0;i<ndims_Y-1;i++) {
        dimY*=dims_Y[i];
    }
    if(dimY!=dim)                                       { mexErrMsgTxt("Dimension mismatch\n"); }
    
    REAL* Y=(REAL*)mxGetData(tmp);
    Vectors *vectorsY;
    switch (VECTOR_CLASS)   {
        case VECTORS:
            vectorsY = new Vectors(Y,NY,dim,DISTANCE_FCN);
            break;
        default:
            mexErrMsgTxt("\n Invalid classname.");
            break;
    }
    
    // Get number of nearest neighbors to compute
    tmp=prhs[3];
    ndims_in    = mxGetNumberOfDimensions(tmp);
    dims_in     = mxGetDimensions(tmp);
    if(!mxIsClass(tmp,"int32"))                         { mexErrMsgTxt("Fifth argument should be int32\n"); }
    int* pk=(int*)mxGetData(tmp);
    int k=*pk;
    
    // Get number of threads and create them 
    tmp=prhs[4];
    ndims_in    = mxGetNumberOfDimensions(tmp);
    dims_in     = mxGetDimensions(tmp);
    if(!mxIsClass(tmp,"int32"))                         { mexErrMsgTxt("Fifth argument should be int32\n"); }
    int NTHREADS= *(int*)mxGetData(tmp);
    ThreadsWithCounter threads(NTHREADS);
    
    const Point** ptarr=new const Point*[k*NY];
    REAL* distances=new REAL[k*NY];
    
    //const Point** ptarr=(const Point**)mxMalloc((k+1)*NY*sizeof(const Point*));
    //double* distances=(double*)mxMalloc(k*NY*sizeof(double));
    
    int L=cover.getMaxLevelPresent();
    
    FindNearestData findnearestdata(&threads,*vectorsY,k,L,ptarr,distances);
    vectorsY->reset();
    cover.findNearest(*vectorsY,findnearestdata,ptarr,distances);
    
//    for(int j=0;j<NY;j++) {
//        for(int i=0;i<k;i++){
//            const Point* foundpoint=ptarr[j*k+i];
//            mexPrintf("\n (%d,%d) %u",j,i,foundpoint);
//            if(foundpoint) {
//                mexPrintf("\n %d, %f", vectorsX->getIndex(foundpoint), distances[j*k+i] );
//            } else {
//                mexPrintf("\n -1,-1.0");
//            }
//        }
//    }
    
//    bool test=cover.checkFindNearest(vectorsY,ptarr,k);
//    for(int i=0;i<NY;i++) {
//        if(test==false) {
//            cout << "checkFindNearest failed" << endl;
//        }
//    }
    
    /* Create matrix for the return argument. */
    plhs[0] = mxCreateStructMatrix(1, 1, 2, fnames_out);
    
    mxArray* fout;
    
    dims[0]=k;
    dims[1]=NY;
    fout=mxCreateNumericArray(ndims,dims,mxINT32_CLASS,mxREAL);
    indices=(int*)mxGetData(fout);
    mxSetField(plhs[0],0,fnames_out[0], fout);

    dims[0]=k;
    dims[1]=NY;
#ifdef DOUBLE
    fout=mxCreateNumericArray(ndims,dims,mxDOUBLE_CLASS,mxREAL);
#else
    fout=mxCreateNumericArray(ndims,dims,mxSINGLE_CLASS,mxREAL);
#endif
    REAL* outdistances=(REAL*)mxGetData(fout);
    mxSetField(plhs[0],0,fnames_out[1], fout);
    
//    mexPrintf("\nDisplaying results...");
    for(int j=0;j<NY;j++) {
//        mexPrintf("\n j=%d", j);
        //const Point* querypoint=ptarr[j*(k+1)];
        //int queryindex=vectorsY.getIndex(querypoint);
        for(int i=0;i<k;i++){
//            mexPrintf("\t i=%d", i);
            const Point* foundpoint=ptarr[j*k+i];
            if(foundpoint) {
                indices[j*k+i]=vectorsX->getIndex(foundpoint);
                outdistances[j*k+i]=distances[j*k+i];
            } else {
                indices[j*k+i]=-1;
                outdistances[j*k+i]=-1.0;
            }
//            mexPrintf("\n %d %d,%f",j,indices[j*k+i],outdistances[j*k+i]);
        }
    }

    delete vectorsY;
    delete vectorsX;
    delete [] ptarr;
    delete [] distances;
    
    //mxFree(ptarr);
    //mxFree(distances);
    
}

#include "FastSeg.C"
template class SegList<DLPtrListNode<CoverNode> >;
