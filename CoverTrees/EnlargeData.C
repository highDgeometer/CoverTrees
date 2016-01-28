#include "EnlargeData.H"
#include "CoverNode.H"
#include "ThreadsWithCounter.H"

EnlargeData::EnlargeData(ThreadsWithCounter* pthreads,int iBLOCKSIZE,int iremaining) :
threads(pthreads),BLOCKSIZE(iBLOCKSIZE), remaining(iremaining), vplp(iBLOCKSIZE),
vthreadncallstogetdist(pthreads->getNThreads()), vthreadtimegetdist(pthreads->getNThreads()),
mergencallstogetdist(0), mergecovernodes(0) {
    
    int NTHREADS=threads->getNThreads();
    if( NTHREADS>0 ) {
        mergecovernodes = new CoverNode*[BLOCKSIZE];        
        for( int i=0;i<NTHREADS; i++ ) {    vthreadncallstogetdist[i]=0;        }
    } else {
        timetogetdist = 0.0;
    }
}

EnlargeData::~EnlargeData() {
    if(mergecovernodes) {
        delete [] mergecovernodes;
    }
}

int EnlargeData::setPoints(Points& points) {
    const Point*    point   = 0;
    int             counter = 0;
    
    for( int u=0; u<BLOCKSIZE; u++ ) {
        point   = points.next();
        if( point ) counter++;
        vplp[u].setPoint( point );
    }
    return counter;
}

long int EnlargeData::getThreadNCallsToGetDist() const {
    long int    total    = 0;
    int         NTHREADS = threads->getNThreads();
    
    for( int t=0; t<NTHREADS; t++ ) {
        total += vthreadncallstogetdist[t];
    }
    return total;
}

double EnlargeData::getThreadTimeToGetDist() const {
    double    total    = 0;
    int       NTHREADS = threads->getNThreads();
    
    for( int t=0; t<NTHREADS; t++ ) {
        total += vthreadtimegetdist[t];
    }
    return total;
}

void EnlargeData::printNCalls(ostream& os) const {
    cout << "mergencallstogetdist="         << mergencallstogetdist         << endl;
    cout << "total threadsncallstogetdist=" << getThreadNCallsToGetDist();
}

void EnlargeData::printTimes(ostream& os) {
    double total = 0;
    if( threads->getNThreads()>0 )  {
        total = getThreadTimeToGetDist();
    } else {
        total = timetogetdist;
    }
        
    cout << "EnlargeData time to compute distances: " << total << endl;
}
