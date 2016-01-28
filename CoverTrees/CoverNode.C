#include "CoverNode.H"
#include <assert.h>

CoverNode::CoverNode(const Point* ppoint, int ilevel, CoverNode* pparent, SegList<DLPtrListNode<CoverNode> >* child_seglist) :
point(ppoint),
level(ilevel),
parent(pparent),
children(child_seglist) {
    if(parent) {
#ifdef DEBUG
        assert(level>parent->getLevel());
#endif
        parent->insertInChildrenAtRightLevel(this);
    }
}

DLPtrListNode<CoverNode>*  CoverNode::insertInChildrenAtRightLevel( CoverNode* child ) {
    int childlevel=child->getLevel();
#ifdef DEBUG
    assert(childlevel>level);
#endif
    DLPtrListNode<CoverNode>* childnode = children.first();
    if(!childnode) { //no children
        return children.prepend(child);
    } else {
        for( ;(childnode!=0) && (childnode->getPtr()->getLevel()<childlevel);
            childnode=children.next(childnode)) {    }
        if(!childnode) { //at end
            return children.append(child);
        } else {
            return children.insertBefore(childnode,child);
        }
    }
}


void CoverNode::printOn(ostream& os) const {
    os << "covernode " << " level=" << level << endl;
    point->printOn(os);
    if(parent) {
        os << " parent ";
        parent->getPoint()->printOn(os);
    }
    os << endl;
    for( DLPtrListNode<CoverNode>* childnode=children.first(); childnode; childnode=children.next(childnode)) {
        os << "\tchild " << " level=" << childnode->getPtr()->getLevel() << " ";
        childnode->getPtr()->getPoint()->printOn(os);
        os << endl;
    }
}

