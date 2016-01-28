#ifndef NIDLLIST_H
#define NIDLLIST_H 
#include <iostream>
#include "FastSeg.H"

template<class T> class NIDLListNode {
  template<class> friend class NIDLList;
private:
  T t;
  NIDLListNode* pr;
  NIDLListNode* ne;
public:
  NIDLListNode(T tt) : t(tt), pr(0), ne(0) {}
  ~NIDLListNode() {}
  T getPtr() { return t; }
  NIDLListNode* prev() const { return pr; }
  NIDLListNode* next() const { return ne; }
  NIDLListNode* remove();

  virtual void printOn(ostream& = cerr) const ;
};
                                                                      
template<class T> ostream& operator<<(ostream&,const NIDLListNode<T>&);

template<class T> class NIDLList {
private:
  SegList<NIDLListNode<T> >* pseglist;
  NIDLListNode<T>* fi;
  NIDLListNode<T>* la;
  int count;
public:
  //NIDLList() : pseglist(0), fi(0), la(0), count(0) {}
  NIDLList(SegList<NIDLListNode<T> >* p) :  
    pseglist(p), fi(0), la(0), count(0) {}
  ~NIDLList() {}
  void reset() { fi=0; la=0; }
  bool empty() const { return count==0; }
  bool notEmpty() const { return count!=0; }
  int getCount() const { return count; }
  NIDLListNode<T>* first() const { return fi; }
  NIDLListNode<T>* last() const { return la; }
  NIDLListNode<T>* next(const NIDLListNode<T> *p) const {
    return p->ne;
  }
  NIDLListNode<T>* prev(const NIDLListNode<T> *p) const {
    return p->pr;
  }
  NIDLListNode<T>* prepend(T*);
  NIDLListNode<T>* append(T*);
  T remove(NIDLListNode<T>*);

    void printOn(ostream& os = cerr) const;
};
template<class T> ostream& operator<<(ostream&,const NIDLList<T>&);


#endif
