#ifndef FINDNEARESTDATA_H
#define FINDNEARESTDATA_H

#include <iostream>
#include "Points.H"
#include "Cover.H"

using namespace std;

class ThreadsWithCounter;
class Cover;

class FindNearestData {
private:
  ThreadsWithCounter* threads;
  Points& points;
  int k;
  int maxlevelpresent;
  const Point** ptarr;
  REAL* distances;
public:
  FindNearestData(ThreadsWithCounter* pthreads,Points& rpoints, int ik,int il,const Point** pptarr,REAL* pdistances) :
    threads(pthreads),
    points(rpoints), 
    k(ik),
    maxlevelpresent(il),
    ptarr(pptarr),
    distances(pdistances) {}
  ~FindNearestData() {}
  ThreadsWithCounter* getThreads() const { return threads; }
  const Point* getPoint(INDEX i) { return points.getPoint(i); }
  int getNumberToFind() const { return k; }  
  int getMaxLevelPresent() const { return maxlevelpresent; }
  const Point** getPtArr() const {
    return ptarr;
  }
  REAL* getDist() const { return distances; }
};

class FindNearestArg {
private:
  int tid;
  ThreadsWithCounter* threads;
  Cover* cover;
  FindNearestData* nearestdata;
  int counter;
public:
  FindNearestArg() : tid(-1), threads(0), cover(0), nearestdata(0), counter(-1) {} 
  FindNearestArg(int itid,ThreadsWithCounter* pthreads,Cover* pcover,
    FindNearestData* pnearestdata) : tid(itid), threads(pthreads), cover(pcover), nearestdata(pnearestdata) {}
  ~FindNearestArg() {}
  ThreadsWithCounter* getThreads() { return threads; }
  int getTid() const { return tid; }
  Cover* getCover() { return cover; }
  FindNearestData* getFindNearestData() { return nearestdata; }
};

#endif
