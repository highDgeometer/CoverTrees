#ifndef POINTS_H
#define POINTS_H

#include "Types.H"
#include "Point.H"
#include <iostream>
#include <climits>

using namespace std;
using namespace CoverTrees_point;

class Points {
public:
  Points() {}
  ~Points() {}
  virtual INDEX getCount() const = 0;
  virtual const Point* next() = 0;
  virtual INDEX getIndex(const Point*) const = 0;
  virtual INDEX getCurrent() const = 0;
  virtual INDEX getRemaining() const = 0;
  virtual const Point* getPoint(int) const = 0;
  virtual void reset() = 0;
  virtual void printOn(ostream& = cout) const = 0;
};


class IndexedPoints : public Points {
private:
  const IndexedPoint* base;
  INDEX count;
  INDEX current;
public:
  IndexedPoints() : Points(), base(0),current(-1) {}
  IndexedPoints(const IndexedPoint* pbase,int icount) : Points(), base(pbase), 
  count(icount),current(0) {}
  ~IndexedPoints() {}
  INDEX getCount() const { return count; }
  INDEX getRemaining() const { return count-current; }
  INDEX getIndex(const Point* p) const {
    const IndexedPoint* q=(const IndexedPoint*)p;
      if( q<base )  {   return INDEX_MAX; }
      else { return (INDEX)(q-base); }
  }
  const Point* getPoint(int i) const { return base+i; }
  void printOn(ostream& = cout) const {}
  void reset() { current=0; }
  INDEX getCurrent() const { return current; }
  const Point* next() { 
    if(current==count) {
      return 0;
    } else {
      return base+current++; 
    }
  }
};

#endif
