#include "IDLListNode.H"

/*
IDLListNode* IDLListNode::placeBefore(IDLListNode* n) {
ne=n; pr=n->pr; n->pr->ne=this; return n->pr=this;
}

IDLListNode* IDLListNode::placeAfter(IDLListNode* n) {
ne=n->ne; pr=n; n->ne->pr=this; return n->ne=this;
}
*/

IDLListNode* IDLListNode::remove() {
 if(pr) { //this not first
   pr->ne=ne;
 }
 if(ne) { //this is not last
   ne->pr=pr;
 }
 ne=0;
 pr=0;
 return this;
}

