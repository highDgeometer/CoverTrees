//Compile command: mex -O -DDOUBLE findwithin.C ThreadsWithCounter.C IDLList.C IDLListNode.C Vector.C Cover.C findWithin.C Point.C CoverNode.C EnlargeData.C FindWithinData.C -lpthread

/* findwithin take five arguments:
 The first argument is what was returned by covertree
 The second argument is the array that was passed as the second argument to
 covertree
 The third argument is a struct whose first field is "distances"
 of class double
 and whose second field is "numlevels" of class int32.
 both are either 1x1 or NYx1 where NY is computed from the fourth argument
 as below.
 The fourth argument is the array of search vectors.
 The fifth argument, of class int32, is the number of threads (0 for serial).
 
 findwithin returns a struct whose first member is "pi" of class int32
 and is NYx2; whose second members "indices" is of class int32 and is
 totalfound x 1; and whose third member "distances" is of class double
 and is totalfound x 1. The (i,1) entry of pi is the number found
 correspoinding the j'th search vector and the (i,2) entry of indices is the
 offset in found of the indices of the corresponding found vectors with
 respect to the second argument. The distances corresponding to a given
 query point are sorted in nondecreasing order.
 */

#include <math.h>
#include "mex.h"
#include "Cover.H"
#include "Vector.H"
#include "ThreadsWithCounter.H"
#include "FindWithinData.H"
#include "Distances.H"
#include "covertree_MEX.H"


const char* fnames_in[]={
    "theta",
    "outparams",
    "radii",
    "levels",
    "ncallstogetdist"
};

const char* within_in[]={
    "distances",
    "numlevels"
};

const char* fnames_out[]={
    "pi",
    "indices",
    "distances"
};


int dim=0;
REAL theta=0.0;
REAL mu=0.0;
mwSize dims[2];
int ndims=2;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    Distance_Mode DISTANCE_FCN;
    VectorClassNames VECTOR_CLASS;
    
#ifdef DOUBLE
    char mxIsClassName[] = "double";
#else
    char mxIsClassName [] = "single";
#endif
    
    // Check for proper number of arguments.
    if (nrhs != 5)                                                                          { mexErrMsgTxt("Five inputs required."); }
    
    // Check the first argument, whic is what was returned by covertree
    int nelements=mxGetNumberOfFields(prhs[0]);
    if(nelements!=5)                                                                        { mexErrMsgTxt("First argument should have five fields."); }
    
    // Get first field of first input
    const mxArray* tmp=mxGetField(prhs[0],0,fnames_in[0]);
    if(!mxIsClass(tmp,mxIsClassName))                                                       { mexErrMsgTxt("First field of first input must be of correct real type\n"); }
    REAL* ptheta=(REAL*)mxGetData(tmp);
    theta=*ptheta;
    mu=1.0/(1.0-theta);
    
    // Get second field of first input
    tmp=mxGetField(prhs[0],0,fnames_in[1]);
    if(!mxIsClass(tmp,"int32"))                                                             { mexErrMsgTxt("Second field of first input must be int32\n"); }
    
    // Get dimensions of second field of first input
    mwSize ndims_in=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_in=mxGetDimensions(tmp);
    bool val=(ndims_in==2)&&(dims_in[0]==1)&&(dims_in[1]==9);
    if(!val)                                                                                { mexErrMsgTxt("Second field of first input has bad size\n"); }
    
    int* outparams  = (int*)mxGetData(tmp);
    int minlevel    = outparams[1];
    int numlevels   = outparams[2];
    DISTANCE_FCN    = (Distance_Mode)(outparams[7]);
    VECTOR_CLASS    = (VectorClassNames)(outparams[8]);
    
    tmp=mxGetField(prhs[0],0,fnames_in[3]);
    if(!mxIsClass(tmp,"int32"))                                                             { mexErrMsgTxt("Fourth field of first input should be int32\n"); }
    mwSize ndims_indices=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_indices=mxGetDimensions(tmp);
    if(ndims_indices!=2) {
        mexErrMsgTxt("Fourth field of first input has bad size\n");
    }
    int* q=(int*)mxGetData(tmp);
    
    // Get dimensions of second field of input
    tmp=prhs[1];
    if(!mxIsClass(tmp,mxIsClassName))                                                       { mexErrMsgTxt("Second input must be of correct real type\n"); }
    mwSize ndims_X=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_X=mxGetDimensions(tmp);
    
    int NX=dims_X[ndims_in-1];
    int d=dims_indices[0];
    if(d!=NX)                                                                               { mexErrMsgTxt("Mismatch between first and second inputs\n"); }
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
    CoverIndices coverindices(theta,numlevels,minlevel,NX,q);
    SegList<DLPtrListNode<CoverNode> > seglist(1024);
    Cover cover(*vectorsX,seglist,coverindices);
    
    // The third argument,   withinradius, numfindlevels
    int nfields = mxGetNumberOfFields(prhs[2]);
    if(nfields!=2)                                                                          { mexErrMsgTxt("Third argument should have two fields."); }
    
    tmp=mxGetField(prhs[2],0,within_in[0]);
    if(!mxIsClass(tmp,mxIsClassName))                                                       { mexErrMsgTxt("First field of third argument must be of correct real type\n"); }
    mwSize ndims_radius=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_radius=mxGetDimensions(tmp);
    if(ndims_radius!=2)                                                                     { mexErrMsgTxt("First field of third argument should have 2 dims\n"); }
    if(dims_radius[1]!=1)                                                                   { mexErrMsgTxt("First field of third argument should have dims[1]=1\n"); }
    REAL* pwithinradius=(REAL*)mxGetData(tmp);
    
    tmp=mxGetField(prhs[2],0,within_in[1]);
    if(!mxIsClass(tmp,"int32"))                                                             { mexErrMsgTxt("Third field of fourth argument must be int32\n"); }
    mwSize ndims_findlevels=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_findlevels=mxGetDimensions(tmp);
    if(ndims_findlevels!=2)                                                                 { mexErrMsgTxt("Second field of third argument should have 2 dims\n"); }
    if(dims_findlevels[1]!=1)                                                               { mexErrMsgTxt("Second field of third argument should have dims[1]=1\n"); }
    int* pnumfindlevels=(int*)mxGetData(tmp);
    if(dims_radius[1]!=dims_findlevels[1])                                                  { mexErrMsgTxt("Size mismatch in third argument\n"); }
    
    // Construct vectors Y
    tmp=prhs[3];
    mwSize ndims_Y=mxGetNumberOfDimensions(tmp);
    const mwSize* dims_Y=mxGetDimensions(tmp);
    if(!mxIsClass(tmp,mxIsClassName))                                                       { mexErrMsgTxt("Fourth argument should be of correct real type\n"); }
    int NY=dims_Y[ndims_Y-1];
    int dimY=1;
    for(int i=0;i<ndims_Y-1;i++) {
        dimY*=dims_Y[i];
    }
    if(dimY!=dim)                                                                           { mexErrMsgTxt("Dimension mismatch\n"); }
    
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
    
    /* Check that third and fourth arguments are compatible */
    if(dims_radius[1]!=1) {
        if((dims_radius[1]!=NY)||(dims_findlevels[1]!=NY))                                  { mexErrMsgTxt("Mismatch between third and fourth arguments\n"); }
    }
    
    Cover::DescendList* descendlists=new Cover::DescendList[NY];
    
    // Get number of threads and create them
    tmp=prhs[4];
    ndims_in=mxGetNumberOfDimensions(tmp);
    dims_in=mxGetDimensions(tmp);
    if(!mxIsClass(tmp,"int32"))                                                             { mexErrMsgTxt("Fifth argument should be int32\n"); }
    int NTHREADS=*(int*)mxGetData(tmp);
    ThreadsWithCounter threads(NTHREADS);
    
    INDEX totalfound=0;
    if(dims_radius[0]==1) {
        FindWithinData findwithindata(&threads,*vectorsY,*pwithinradius,*pnumfindlevels,descendlists);
        totalfound=cover.findWithin(*vectorsY,findwithindata,descendlists);
    } else {
        FindWithinData findwithindata(&threads,*vectorsY,pwithinradius,pnumfindlevels,descendlists);
        totalfound=cover.findWithin(*vectorsY,findwithindata,descendlists);
    }
    
    vectorsY->reset();
    
    // Create matrix for the return argument.
    plhs[0] = mxCreateStructMatrix(1, 1, 3, fnames_out);
    
    mxArray* fout;
    
    dims[0]=NY;
    dims[1]=2;
    fout =mxCreateNumericArray(ndims,dims,mxINT32_CLASS,mxREAL);
    int* pi=(int*)mxGetData(fout);  //numfound and offsets
    mxSetField(plhs[0],0,fnames_out[0],fout);
    
    dims[0]=totalfound;
    dims[1]=1;
    fout =mxCreateNumericArray(ndims,dims,mxINT32_CLASS,mxREAL);
    int* indices=(int*)mxGetData(fout);
    mxSetField(plhs[0],0,fnames_out[1], fout);
    
    dims[0]=totalfound;
    dims[1]=1;
#ifdef DOUBLE
    fout =mxCreateNumericArray(ndims,dims,mxDOUBLE_CLASS,mxREAL);
#else
    fout =mxCreateNumericArray(ndims,dims,mxSINGLE_CLASS,mxREAL);
#endif
    REAL* distances=(REAL*)mxGetData(fout);
    mxSetField(plhs[0],0,fnames_out[2], fout);
    
    FindWithinflattenDescendLists(totalfound,indices,distances,NY,pi,*vectorsX,descendlists);
    //bool test=checkFlattenDescendList(totalfound,indices,NY,pi,vectorsX,descendlists);
    
    delete [] descendlists;
    delete vectorsY;
    delete vectorsX;
    
}

#include "FastSeg.C"
template class SegList<DLPtrListNode<CoverNode> >;
