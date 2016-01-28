#ifndef COVER_H
#define COVER_H
#include <iostream>
#include <vector>

#include "DLPtrList.H"
#include "Points.H"
#include "FastSeg.H"
#include "CoverNode.H"
#include <float.h>
#include <limits.h>
#include "NIDLList.H"
#include "ThreadsWithCounter.H"
#include "TimeUtils.h"

using namespace std;

class Levels;
class DescendNode;
class Cover;
class CoverIndices;
class EnlargeData;
class FindWithinData;
class FindNearestData;

class Cover : public DLPtrList<CoverNode> {
private:
    CoverNode* root;
    
    int minlevel;
    int maxlevel;
    int numlevels;
    
    REAL maxradius; //this is theta^minlevel
    
    int NTHREADS;
    int BLOCKSIZE;
    
    int number_inserted;
    int number_deep;
    int number_duplicates;
    
    long int threadncallstogetdist;
    long int mergecallstogetdist;
    
    SegList<CoverNode> covernode_seglist;
    
    SegList<DLPtrListNode<CoverNode> > dlptrlistnode_covernode_seglist;
    
    
    CoverNode* newCoverNode(const Point* ppoint,int ilevel,CoverNode* pparent) {
        return new(covernode_seglist.getPtr()) CoverNode(ppoint,ilevel,pparent,&dlptrlistnode_covernode_seglist);
    }
    
    CoverNode* newCoverNode(PointLevelParent* plp) {
        return new(covernode_seglist.getPtr()) CoverNode( plp->getPoint(),plp->getLevel(),plp->getParent(),&dlptrlistnode_covernode_seglist );
    }
    
    
    CoverNode* newCoverNode(PointLevelParent* plp,CoverNode* covernode) {
        new(covernode) CoverNode(plp->getPoint(),plp->getLevel(),
                                 plp->getParent(),&dlptrlistnode_covernode_seglist);
        return covernode;
    }
    
    DLPtrListNode<CoverNode>* makeNewCoverNodeAndReturnChildInParent(
                                                                     const Point* ppoint,int ilevel,CoverNode* pparent);
    
    int getCoarserLevel(int) const;
    void adjustRootAndMinLevelAndMaxRadius(REAL);
    
    int setPoints(Points&,ThreadsWithCounter*,EnlargeData*);
    
public:
    
    class DescendList : public IDLList<DescendNode> {
    private:
        const Point* point;
        int level;
        REAL radius; //Not to be confused with Cover::maxradius
        
        CoverNode* parent;
        REAL dist_to_root;
        int R_level;
        
        long int ncallstogetdist;
        TimeList timetogetdist;        
        
        SegList<DescendNode> descendnode_seglist;
        DescendNode* newDescendNode();
        
    public:
        DescendList();
        ~DescendList();
        
        void            reset           ( const Cover*,const Point*,REAL = 0.0, bool forcenonempty = false);
        DescendNode*    newDescendNode  ( CoverNode*,DLPtrListNode<CoverNode>* current_child, REAL dist);
        void prune();
        void prune(REAL);
        CoverNode* descend();
        void appendTo(DLPtrList<Point>& pointlist) const;
        void descendForFindWithin(REAL r);
        void descendForFindNearest(vector<DescendNodePtr>& vdn,REAL& r,int k);
        void printOn(ostream& = cout);
        void printOnForFindNearest(ostream& = cout);
        
        
        const Point*    getPoint        () const { return point; }
        int getLevel                    () const { return level; }
        int getRLevel                   () const { return R_level; }
        REAL getRadius                  () const { return radius; }
        CoverNode* getParent            () const { return parent; }
        REAL getDistToRoot              () const { return dist_to_root; }
        long int getNCallsToGetDist     () const { return ncallstogetdist; }
        double getTimeToGetDist         ();        
    };
    
    Cover(const Point* point,SegList<DLPtrListNode<CoverNode> >& seglist,int numlevels, int minlevel = 0 );
    Cover(Points&,SegList<DLPtrListNode<CoverNode> >& seglist,const CoverIndices&);
    Cover(const Point* point,SegList<DLPtrListNode<CoverNode> >& seglist, int inumlevels,int minlevel,int counter,vector<PointLevelParent>& vplp,CoverNode* covernodes);
    void enlargeBy(EnlargeData&,Points&);
    Cover(Points*,CoverIndices&);
    ~Cover();
    
    void merge(ThreadsWithCounter*,int,EnlargeData&);
    
    int getNumberLevels() const {return numlevels; }
    
    void resetRootChildren();
    
    void setCounts();
    
    int getMaxLevelPresent() const;
    
    // Range search functions
    void descendForFindWithin(REAL);
    void findWithin             ( const Point* p,REAL r,int numfindlevels,DescendList& descend_list );
    void fillArrFromDescendList ( int*,const Points*,DescendList& descendlist ) const;
    int  findWithin             ( Points& points,FindWithinData& findwithindata,DescendList* descendlists );
    void findWithin             ( Points* points,REAL r,int numfindlevels,int*& offsets,int*& counts,const Point**& pout );
    bool checkFindWithin        ( const Point* p,REAL r,int numfindlevels,DescendList& descend_list );
    
    // Nearest neighbor functions
    void findNearest            ( const Point* point,vector<DescendNodePtr>& vdn,int k,int& K,int L,Cover::DescendList& descend_list,
                                 const Point** ptarr,REAL* distances );
    void findNearest            ( Points& points,FindNearestData& findnearestdata, const Point** ptarr,REAL* distances );
    void printFindNearest       ( Points& querypoints, const Point** ptarr,int k,ostream& = cout );
    bool checkFindNearest       ( Points& querypoints,const Point** ptarr,int k );
    
    void printCounts(ostream& os = cout) const;
    
    // getters
    int getNumberInserted()     const { return number_inserted; }
    int getNumberDeep()         const { return number_deep; }
    int getNumberDuplicates()   const { return number_duplicates; }
    REAL getMaxRadius()         const { return maxradius; }
    int getMinLevel()           const { return minlevel; }
    int getMaxLevel()           const { return minlevel+numlevels-1; }
    int getNumLevels()          const { return numlevels; }
    const REAL* getRadii()      const;
    CoverNode* getRoot()        const { return root; }
    
    CoverNode* makeNewEntry (const Point* point,int level,CoverNode* parent, EnlargeArg* parg);
    
    void insert(DescendList&,PointLevelParent*,REAL&,int&,EnlargeArg*);
        
    void printLevelCounts(ostream& = cout);
    
    // checkers
    //bool checkFindWithin(const Points*,int,REAL,int,int*,int) const;
    bool checkParents   (Points*,const REAL* radii,ostream& = cout) const;
    bool checkChildren  (Points*,const REAL* radii,ostream& = cout) const;
    bool checkDistances (Points*,const REAL* radii,ostream& = cout) const;
    void check          (Points*,ostream& = cout) const;
    void printOn        (ostream& = cout) const;
    //void printDiagnostics(ostream& = cout) const;
    
};

void printFromFindWithin(Points* points,int* offsets,int* counts,const Point** pout,ostream& = cout);

class CoverIndices {
private:
    bool may_delete;
    size_t sz;
    REAL mytheta;
    int numlevels;
    int minlevel;
    int count;
    int* levels;
    int* parents;
    int* numchildren;
    int* childoffsets;
    int* children;
public:
    //CoverIndices(const Cover* cover,const Points*);
    CoverIndices(const Cover* cover,const Points*,int* = 0);
    CoverIndices(const char* ifname);
    //this one is for Matlab
    CoverIndices(REAL rtheta,int inumlevels,int iminlevel, int icount, int* base);
    ~CoverIndices();
    void write(const char* ofname,REAL theta,const Points* points) const;
    REAL getTheta() const { return mytheta; }
    int getNumLevels() const { return numlevels; }
    int getMinLevel() const { return minlevel; }
    int getCount() const { return count; }
    const int* getBase() const { return levels; }
    int getLevel(int i) const {return levels[i];}
    int getParent(int i) const {return parents[i];}
    int getNumChildren(int i) const {return numchildren[i];}
    int getChildOffset(int i) const {return childoffsets[i];}
    int getChild(int i,int j) const { return children[childoffsets[i]+j]; }
    void printOn(ostream& os = cout) const;
};
#endif
