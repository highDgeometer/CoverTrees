#ifndef DLPTRLIST_H
#define DLPTRLIST_H 
#include <iostream>
#include "FastSeg.H"

template<class T> class DLPtrListNode {
  template<class> friend class DLPtrList;
private:
  T* t;
  DLPtrListNode* pr;
  DLPtrListNode* ne;
public:
  DLPtrListNode(T* tt) : t(tt), pr(0), ne(0) {}
  ~DLPtrListNode() {}
  T* getPtr() { return t; }
  DLPtrListNode* prev() const { return pr; }
  DLPtrListNode* next() const { return ne; }

  virtual void printOn(ostream& = cerr) const ;
};
                                                                      
template<class T> ostream& operator<<(ostream&,const DLPtrListNode<T>&);

template<class T> class DLPtrList {
private:
  SegList<DLPtrListNode<T> >* pseglist;
  DLPtrListNode<T>* fi;
  DLPtrListNode<T>* la;
  int count;
public:
  DLPtrList()                               :   pseglist(0), fi(0), la(0), count(0) {}
  DLPtrList(SegList<DLPtrListNode<T> >* p)  :   pseglist(p), fi(0), la(0), count(0) {}
  ~DLPtrList() {}
  void setSegList(SegList<DLPtrListNode<T> >* ppseglist)    { pseglist=ppseglist;   }
  void reset()                                              { fi=0; la=0; count=0;  }
  bool empty()              const                           { return count==0;      }
  bool notEmpty()           const                           { return count!=0;      }
  int getCount()            const                           { return count;         }
  DLPtrListNode<T>* first() const                           { return fi;            }
  DLPtrListNode<T>* last()  const                           { return la;            }
  DLPtrListNode<T>* next(const DLPtrListNode<T> *p) const   { return p->ne;         }
  DLPtrListNode<T>* prev(const DLPtrListNode<T> *p) const   { return p->pr;         }
  DLPtrListNode<T>* prepend(T*);
  DLPtrListNode<T>* append(T*);

  DLPtrListNode<T>* insertAfter(DLPtrListNode<T>*,T*);
  DLPtrListNode<T>* insertBefore(DLPtrListNode<T>*,T*);
  
   T* remove(DLPtrListNode<T>*);

  void printOn(ostream& os = cerr) const;

};



#endif
