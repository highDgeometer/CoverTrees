#ifndef COVERNODE_H
#define COVERNODE_H

#include "IDLList.H"
#include "Points.H"
#include "DLPtrList.H"
#include "FastSeg.H"
#include "Types.H"
#include <iostream>
#include <limits.h>

class CoverNode {
    friend class Cover;
private:
    const Point* point;
    int level;
    CoverNode* parent;
    DLPtrList<CoverNode> children;
public:
    CoverNode() : point(0), level(0), parent(0) {}
    CoverNode(const Point* ppoint,int ilevel,CoverNode* pparent) : point(ppoint),
    level(ilevel),parent(pparent) {}
    CoverNode(const Point* ppoint,int level,CoverNode* pparent,SegList<DLPtrListNode<CoverNode> >* child_seglist);
    ~CoverNode() {}
        
    const Point* getPoint() const {return point; }
    int getLevel() const { return level; }
    void setLevel(int l) { level=l; }
    void setParent(CoverNode* pparent) { parent = pparent; }
    DLPtrList<CoverNode>* getChildren() { return &children; }
    CoverNode* getParent() { return parent; }
    void insertInChildren(CoverNode*);
    DLPtrListNode<CoverNode>* insertInChildrenAtRightLevel(CoverNode*);
    void printOn(ostream& = cout) const;
    void printOn(const Points*,ostream& = cout) const;
};

class PointLevelParent {
private:
    const Point* point;
    int level;
    CoverNode* parent;
public:
    PointLevelParent() : point(0), level(INT_MIN), parent(0) {}
    PointLevelParent(const Point* ppoint,int ilevel, CoverNode* pparent) :
    point(ppoint), level(ilevel), parent(pparent) {}
    ~PointLevelParent() {}
    const Point* getPoint() const { return point; }
    int getLevel() const { return level; }
    CoverNode* getParent() const { return parent; }
    void setPoint(const Point* ppoint) { point=ppoint; }
    void setLevel(int ilevel) {  level=ilevel; }
    void setParent(CoverNode* pparent) { parent=pparent; }
    void setPointLevelParent(const Point* ppoint,int ilevel, CoverNode* pparent)  {
        point = ppoint;    level = ilevel;    parent = pparent;  }
    
    bool operator < (const PointLevelParent& other) const {
        return level< other.level;
    }
};

class DescendNode : public IDLListNode {
private:
    CoverNode* covernode;
    DLPtrListNode<CoverNode>* current_child;
    REAL dist;
public:
    DescendNode(CoverNode* pcovernode,DLPtrListNode<CoverNode>* pcurrent_child,REAL ddist) :
                covernode(pcovernode),current_child(pcurrent_child), dist(ddist) {}
    ~DescendNode() {}
    CoverNode* getCoverNode() const { return covernode; }
    REAL getDist() { return dist; }
    void setDist(REAL ddist) { dist=ddist; }
    DLPtrListNode<CoverNode>* getCurrentChild() const { return current_child; }
    void setCurrentChild(DLPtrListNode<CoverNode>* pchild) {
        current_child=pchild; }
    void printOn(ostream& os = cout) const {
        covernode->printOn(os);
        current_child->printOn(cout);
        os << " dist=" << dist << endl;
    }
};

class DescendNodePtr {
private:
    DescendNode* pdescendnode;
public:
    DescendNodePtr() : pdescendnode(0) {}
    DescendNodePtr(DescendNode* ppdescendnode) : pdescendnode(ppdescendnode) {}
    ~DescendNodePtr() {}
    DescendNode* getPtr() const { return pdescendnode; }
    REAL getDist() const { return pdescendnode->getDist(); }
    bool operator < (const DescendNodePtr& other) const {
        return (pdescendnode->getDist())< ((other.pdescendnode)->getDist());
    }
};


#endif


