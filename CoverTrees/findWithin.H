#ifndef FINDWITHIN_H
#define FINDWITHIN_H

#include "Cover.H"

class FindWithinData {
private:
  REAL* rarr;
  int* numfindlevelsarr;
  REAL r;
  int numfindlevels;
public:
  FindWithinData(REAL* pr,int* pnumfindlevels) : rarr(pr), 
    pnumfindlevels(numfindlevelsarr), r(-1.0), numfindlevels(0) {}
  FindWithinData(REAL rr,int inumfindlevels) : rarr(0), pnumfindlevels(0),
    r(rr), numfindlevels(inumfindlevels) {}
  ~FindWithinData() {}
  REAL getRadius(int i) { 
    if(rarr) {
      return rarr[i];
    } else {
      return r;
    }
  }
  int getNumFindLevels(int i) { 
    if(rarr) {
      return numfindlevelsarr[i];
    } else {
      return numfindlevels;
    }
  }
};

#endif
