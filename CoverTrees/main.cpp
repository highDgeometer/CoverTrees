#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <sys/times.h>
#include <sys/param.h>

#include "Cover.H"
#include "EnlargeData.H"
#include "FindWithinData.H"
#include "FindNearestData.H"
#include "ThreadsWithCounter.H"
#include "Vector.H"
#include "Timer.H"
#include "TimeUtils.h"


#ifdef MEM_DEBUG
#include "MemoryDebugger.H"
#endif

#define DOFINDNEAREST   false
#define DOFINDWITHIN    false


using namespace std;
using namespace CoverTreeTypes;

ThreadsWithCounter* threads=0;

//#define DEBUG

// These need to be global as they're used by Cover
REAL theta=0.0;
REAL mu=0.0;


// Other variables
REAL pForBlob = 1;                          // p in the p-norm of the blobs being generated
int dimAmbient = 0;
REAL sigma = 0.1;                          // factors in front of uniform [-0.5,0.5] noise
bool Precompute = true;                     // Precompute things when vectors/images are being created to spped up distance computations later


// Arguments:
// 1) dimension
// 2) number of points
// 3) theta
// 4) numlevels
// 5) NTHREADS
// 6) BLOCKSIZE
// 7) DISTANCE_FCN;


extern TimeList globalTimeList;

#define DATASET     0


int main(int argc,char* argv[]) {
#ifdef MEM_DEBUG
    MemoryDebugger md;
#endif

    TimeList timings;
    
    int dim=atoi(argv[1]);
    int N=atoi(argv[2]);
    theta=atof(argv[3]);
    mu=1.0/(1.0-theta);
    
    int numlevels   = atoi(argv[4]);
    int NTHREADS    = atoi(argv[5]);
    int BLOCKSIZE   = atoi(argv[6]);
    
    Distance_Mode DISTANCE_FCN = EUCLIDEAN;
    if ( argc>7 ) {
        DISTANCE_FCN = (Distance_Mode)atoi(argv[7]);
    };
    
    cout << "Number of points="     << N            << endl;
    cout << "Dimension="            << dim          << endl;
    cout << "Number of threads="    << NTHREADS     << endl;
    cout << "Distance function="    << DISTANCE_FCN << endl;
    
    threads=new ThreadsWithCounter(NTHREADS);
    
    REAL *X             __attribute__ ((aligned));
    Vectors *vectors    __attribute__ ((aligned));
    
    int k = 10;                                                                                                                                                                                 // how many nearest neighbors to find
    int NY = 2;                                                                                                                                                                                 // how many points of which to find the nearest neighbors
    
    const Point** ptarr=new const Point*[k*NY];
    REAL* distances=new REAL[k*NY];
    REAL *Y=new REAL[dim*NY];
    
    Vectors *vectorsY;
    
    //srand48((unsigned)time(NULL));                // random seed
    srand48(1);
    
    timings.startClock("DataSet creation");
    switch (DATASET) {
        default:
        {
            if (dimAmbient<dim) {
                dimAmbient = dim;
            }
            X=(REAL*)calloc(dimAmbient*N, sizeof(REAL));
            
            for(int i=0;i<N;i++) {
                for (int j=0; j<dim; j++) {
                    X[i*dimAmbient+j]=(REAL)drand48();
                }
            }
            if( true ) {                                                                                                                                                                         // Introduce outlier. Assumes there are at least 3 points
                X[3*dimAmbient] = 20;
            }
            dim = dimAmbient;
            vectors = new Vectors(X,N,dim);                                                                                                                                                     // Create Vectors
            
            for(int i=0;i<dim*NY;i++) {
                Y[i]=X[i]+0.000001;
            }
            vectorsY = new Vectors(Y,NY,dim,DISTANCE_FCN,Precompute);
        }
    }
    timings.endClock("DataSet creation");

    vectorsY->getPoint(0)->getDist(vectors->getPoint(0));
    
//    vectors.getVector(10)->printOn();
    
    SegList<DLPtrListNode<CoverNode> > seglist( N,1 );
    
    // Construct covertree
    const Vector* vector    = (const Vector*)(vectors->next());
    
    Cover cover(vector,seglist,numlevels);                                                                                                                                                      // Prepare cover tree
    
    EnlargeData enlargedata(threads,BLOCKSIZE,vectors->getRemaining());                                                                                                                         // Prepare cover tree for adding points
    
    timings.startClock("Covertree");
    cover.enlargeBy(enlargedata,*vectors);                                                                                                                                                      // Add points to cover tree
    timings.endClock("Covertree");
    
    //cover.printLevelCounts();
    enlargedata.printNCalls();
    cout << "\nNDistances/N^2=" << ((double)enlargedata.getThreadNCallsToGetDist()+(double)enlargedata.getMergeNCallsToGetDist())/((double)N*(double)N) << endl;
    
//    const char* outfile=argv[7];
//    CoverIndices coverindices(&cover,&vectors);
//    coverindices.write(outfile,theta,&vectors);//cover.printOn(cout);
    
    
    /*
     cout << "ncallstogetdist=" << cover.getNCallsToGetDist() << endl;
     
     cout << "cover constructed" << endl;
     
     cover.setCounts();
     cover.printCounts(cout);
     */
    //cover.printOn(cout);
    
    /*
     for(CoverNode* node=cover.first();node;node=cover.next(node)) {
     cout << "index=" << vectors.getIndex(node->getPoint())
	 << " level=" << node->getLevel() << endl;
     }
     */
    
    //cover.check(&vectors);
    
    
    //cout << "printing cover with coords" << endl;
    //cover.printOn(cout);
    
// FindNearest
    if (DOFINDNEAREST)  {
        int L=cover.getMaxLevelPresent();
        
        ThreadsWithCounter threadss(NTHREADS);
        
        FindNearestData findnearestdata(&threadss,*vectorsY,k,L,ptarr,distances);
        vectorsY->reset();
        timings.startClock("findNearest");
        cover.findNearest(*vectorsY,findnearestdata,ptarr,distances);
        timings.endClock("findNearest");
        
        if (true) {
            cout << "\n\n FindNearest ------------------------";
            for(int j=0;j<NY;j++) {
                //cout << "j=" << j << endl;
                //const Point* querypoint=ptarr[j*(k+1)];
                //int queryindex=vectorsY.getIndex(querypoint);
                for(int i=0;i<k;i++){
                    //cout << "i=" << i << endl;
                    const Point* foundpoint=ptarr[j*k+i];
                    if(foundpoint) {
                        cout << "\n" << vectors->getIndex(foundpoint) << "," << distances[j*k+i];
                    } else {
                        cout << "\n -1" << ", -1.0";
                    }
                }
                cout << "\n";
            }
        }
    }
    
    
    // FindWithin
    if (DOFINDWITHIN)   {
        int numfindlevels=10;
        REAL r=(REAL)5.0;
        ThreadsWithCounter threadss(NTHREADS);
        Cover::DescendList* descendlists=new Cover::DescendList[NY];
        
        // range search
        FindWithinData findwithindata(&threadss,*vectorsY,r,numfindlevels,descendlists);
        vectorsY->reset();
        timings.startClock("findWithin");
        int totalfound=cover.findWithin(*vectorsY,findwithindata,descendlists);
        timings.endClock("findWithin");
        
        int *pi             = new int[NY*2];
        int *indices        = new int[totalfound];
        REAL *distances     = new REAL[totalfound];
        
        // flatten results
        FindWithinflattenDescendLists(totalfound,indices,distances,NY,pi,*vectors,descendlists);

        // display results
        if (false)   {
            cout << "\n\n FindWithin ------------------------";
            for(int j=0; j<NY; j++) {
                cout << "\n" << j << ":";
                for (int i=pi[j+NY]; i<pi[j+NY]+pi[j]; i++) {
                    cout << "(" << indices[i] << "," << distances[i] << "),";
                }
                cout << "\n";
            }
        }
        
        delete [] distances;
        delete [] indices;
        delete [] pi;
    }
    

    
    cout << "\n----------------------------------------\nTimings:" << timings << "\n";
    cout << *(vectors->getTimeList());
    cout << globalTimeList;
    enlargedata.printTimes();
    
    delete vectorsY;
    delete [] Y;
    delete vectors;
    //delete [] X;
    free( X );
    
    if(threads) {
        delete threads;
    }
    
    return 0;
    
}



#include "DLPtrList.C"
template class DLPtrListNode<CoverNode>;
#include "FastSeg.C"
template class  SegList<DLPtrListNode<CoverNode> >;
