#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include "Points.H"
#include "Types.H"
#include "Distances.H"
#include "TimeUtils.h"


using namespace std;
using namespace CoverTreeTypes;

class Vectors;

// Vector is a base class for representing points with numeric coordinates.

class Vector : public Point {
    friend class Vectors;
private:
    REAL*       x;
    Vectors*    parent;
public:
    Vector(REAL* px, Vectors* iParent) : x(px), parent(iParent) {}
    ~Vector() {}
    
    int     getDim()                        const;
    REAL*   getPtr()                        const { return x; }
    REAL    getDist(const Point*)           const;
    REAL    getBlasDist(const Point*,REAL*) const;

    void    write(ofstream&) const;
    void    printOn(ostream& = cout) const;
};


// Vectors is a class representing an array of vectors

class Vectors : public Points {
    friend class Vector;
    friend class Images;
private:
    INDEX               current;
    bool                may_delete;
protected:
    Vector*             vectors;
    REAL*               X __attribute__ ((aligned));
    INDEX               N;
    INDEX               dim;
    Distance_Workspace  dist_ws;
    bool                Precompute;
    TimeList            timeList;

    virtual bool init_DistanceWorkspace( Distance_Mode dist_mode );
    virtual void clear_DistanceWorkspace( void );

public:
    Vectors(REAL* pX,INDEX iN, INDEX iDim,Distance_Mode dist_mode = EUCLIDEAN, bool iPrecompute = true);
    Vectors(const char*,int,INDEX,INDEX, Distance_Mode dist_mode = EUCLIDEAN);
    virtual ~Vectors();
    
    Vector* getVector(int i)        const { return vectors+i; }
    INDEX    getDim()               const { return dim; }
    INDEX    getCurrent()           const { return current; }
    INDEX    getCount()             const { return N; }
    INDEX    getRemaining()         const { return N-current; }
    INDEX    getIndex(const Point*) const;
    TimeList* getTimeList()               { return &timeList; }
    const   Distance_Workspace* getDistanceWorkspace()    const { return &dist_ws; }
    const   Point* getPoint(int i)  const { return vectors+i; }

    const Vector*   next();
    void            reset() { current=0; }
    
    void printOn(ostream& = cout) const;
};


#endif
