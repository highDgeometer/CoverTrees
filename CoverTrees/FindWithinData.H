#ifndef FINDWITHINDATA_H
#define FINDWITHINDATA_H

#include <iostream>
#include "Points.H"
#include "Cover.H"

using namespace std;

class ThreadsWithCounter;
class Cover;

class FindWithinData {
private:
  ThreadsWithCounter* threads;
  Points& points;
  REAL* pdist;
  int* pnumfindlevels;
  REAL dist;
  int numfindlevels;
  Cover::DescendList* descendlists;
public:
  FindWithinData(ThreadsWithCounter* pthreads,Points& rpoints, REAL* ppdist, int* ppnumfindlevels, Cover::DescendList* pdescendlists) :
    threads(pthreads),
    points(rpoints), 
    pdist(ppdist),pnumfindlevels(ppnumfindlevels),
    dist(-1.0), numfindlevels(0),						
    descendlists(pdescendlists) {}
    
  FindWithinData(ThreadsWithCounter* pthreads,Points& rpoints, REAL rdist, int inumfindlevels,Cover::DescendList* pdescendlists) :
      threads(pthreads),
      points(rpoints), pdist(0),pnumfindlevels(0),
      dist(rdist), numfindlevels(inumfindlevels),				
      descendlists(pdescendlists) {}
  ~FindWithinData() {}
    
  ThreadsWithCounter* getThreads() const { return threads; }
  const Point* getPoint(int i) { return points.getPoint(i); }
  REAL getDist(int i) { 
    if(pdist) {
      return pdist[i];
    } else {
      return dist;
    }
  }
  int getNumFindLevels(int i) { 
    if(pnumfindlevels) {
      return pnumfindlevels[i];
    } else {
      return numfindlevels;
    }
  }
  Cover::DescendList& getDescendList(int i) { return descendlists[i]; }
};

class FindWithinArg {
private:
  int tid;
  ThreadsWithCounter* threads;
  Cover* cover;
  FindWithinData* enlargedata;
  int counter;
public:
  FindWithinArg() : tid(-1), threads(0), cover(0), enlargedata(0), counter(-1) {} 
  FindWithinArg(int itid,ThreadsWithCounter* pthreads,Cover* pcover,
    FindWithinData* penlargedata) : tid(itid), threads(pthreads), cover(pcover),
				 enlargedata(penlargedata) {}
  ~FindWithinArg() {}
  ThreadsWithCounter* getThreads() { return threads; }
  int getTid() const { return tid; }
  Cover* getCover() { return cover; }
  FindWithinData* getFindWithinData() { return enlargedata; }
};

class DescendList;
void flattenDescendLists(int,int*,REAL*,int,int*,Points&,Cover::DescendList*);
bool checkFlattenDescendList(int,int*,int,int*,Points&,Cover::DescendList*);

bool FindWithincheckFlattenDescendList(int totalfound,int* found,int N,int* pi,Points& points, Cover::DescendList* descendlists);
void FindWithinflattenDescendLists(int totalfound,int* indices,REAL* distances,int N,int* pi,Points& points,Cover::DescendList* descendlists);

#endif
