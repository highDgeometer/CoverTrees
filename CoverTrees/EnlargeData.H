#ifndef ENLARGEDATA_H
#define ENLARGEDATA_H

#include <vector>
//#include "CoverNode.H"
#include <iostream>
#include "Points.H"
#include "TimeUtils.h"

using namespace std;

class ThreadsWithCounter;
class PointLevelParent;
class CoverNode;
class Cover;

class EnlargeData {
private:
    ThreadsWithCounter*         threads;
    int                         BLOCKSIZE;
    int                         remaining;
    vector<PointLevelParent>    vplp;
    vector<long int>            vthreadncallstogetdist;
    vector<double>              vthreadtimegetdist;

    long int                    mergencallstogetdist;
    vector<double>              mergetimegetdist;
    CoverNode**                 mergecovernodes;
    
    //TimeList timings;
    //Points& points;
    
public:
    
    double                      timetogetdist;
    
    EnlargeData                                             (ThreadsWithCounter* pthreads,int iBLOCKSIZE,int remaining);
    ~EnlargeData                                            ();
    
    int                         setPoints                   (Points& points);                                                                                                                   // Seetters
    
    ThreadsWithCounter*         getThreads                  () const                    { return threads; }                                                                                     // Getters
    int                         getBlockSize                () const                    { return BLOCKSIZE; }
    int                         getRemaining                () const                    { return remaining; }
    CoverNode**                 getMergeCoverNodes          ()                          { return mergecovernodes; }
    PointLevelParent*           getPointLevelParent         (int i)                     { return &vplp[i]; }
    vector<PointLevelParent>&   getPointLevelParent         ()                          { return vplp; }
    long int                    getMergeNCallsToGetDist     () const                    { return mergencallstogetdist; }
    long int                    getThreadNCallsToGetDist    () const;
    double                      getThreadTimeToGetDist      () const;
    
    void                        decrementRemaining          (int counter)               { remaining-=counter;                   }
    
    void                        incrThreadNCallsToGetDist   (int t,long int ncalls)     { vthreadncallstogetdist[t]+=ncalls;    }
    void                        incrThreadTimeToGetDist     (int t,double time)         { vthreadtimegetdist[t]+=time;          }
    void                        incrTimeToGetDist           (double time)               { timetogetdist+=time;                  }
    void                        incrMergeNCallsToGetDist    (long int ncalls)           { mergencallstogetdist+=ncalls;         }
    void                        printNCalls                 (ostream& os = cout)    const;
    void                        printTimes                  (ostream& os = cout);
};

class EnlargeArg {
private:
    int                         tid;
    ThreadsWithCounter*         threads;
    Cover*                      cover;
    EnlargeData*                enlargedata;
    int                         counter;
public:
    EnlargeArg                                              () : tid(-1), threads(0), cover(0), enlargedata(0), counter(-1)
                                                                                        {}
    EnlargeArg                                              ( int itid,ThreadsWithCounter* pthreads,Cover* pcover,EnlargeData* penlargedata) :
                                                                tid(itid), threads(pthreads), cover(pcover),enlargedata(penlargedata)
                                                                                        {}
    ~EnlargeArg                                             ()                          {}
    
    ThreadsWithCounter* getThreads                          ()                          { return threads; }                                                                                     // Getters
    int getTid                                              () const                    { return tid; }
    Cover* getCover                                         ()                          { return cover; }
    EnlargeData* getEnlargeData                             ()                          { return enlargedata; }
};


#endif
