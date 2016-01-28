#ifndef IDLLISTNODE_H
#define IDLLISTNODE_H
#include <iostream>

using namespace std;

class IDLListNode {
private:
  IDLListNode* pr;
  IDLListNode* ne;
public:
  IDLListNode() : pr(0), ne(0) {;}
  IDLListNode* prev() const { return pr; }
  IDLListNode* next() const { return ne; }
  IDLListNode* prepend(IDLListNode *pn) {
    pn->ne=this; pr=pn; return pn; 
  }
  IDLListNode* append(IDLListNode *pn) {
    pn->pr=this; ne=pn; return pn;
  }
  IDLListNode* replace(IDLListNode *n) {
    n->pr->ne=this; n->ne->pr=this; pr=n->pr; ne=n->ne; return n;
  }
  void switchBackConnectionWith(IDLListNode *n) {
    IDLListNode *t=n->pr; pr->ne=n; n->pr=pr; pr=t; t->ne=this;
  }
  IDLListNode* remove();
  IDLListNode* disconnect() {
    pr->ne=ne; ne->pr=pr; pr=0; ne=0; return this;
  }
  void reverseConnections(IDLListNode *&p,IDLListNode *&n) {
    p=ne; n=pr; ne=n; pr=p;
  }
  void switchWithPrevious() {
    if (pr!=ne) {
      IDLListNode *prpr=pr->pr; ne->pr=pr; prpr->ne=this; 
      pr->pr=this; pr->ne=ne; ne=pr; pr=prpr;
    }
  }
  void switchWithNext() {
    if (pr!=ne) {
      IDLListNode *nene=ne->ne; pr->ne=ne; nene->pr=this;
      ne->ne=this; ne->pr=pr; pr=ne; ne=nene;
    }
  }
  void switchWith(IDLListNode *n) {
    IDLListNode *b=pr; IDLListNode *f=ne; ne->pr=pr->ne=n; pr=n->pr;
    ne=n->ne; n->ne->pr=n->pr->ne=this; n->pr=b; n->ne=f;
  }
  
  virtual void printOn(ostream &os = cerr) const {
    os << "IDLListNode: this = " << this
       << " pr = " << pr << " ne = " << ne;
  }
  
};   

/*
  inline ostream& operator<<(ostream &os,const IDLListNode &node) {
  node.printOn(os); return os;}
*/

#endif
