#include "NIDLList.H"
#include <limits.h>
#include <stdlib.h>
#ifdef MEM_DEBUG
#include "MemoryDebugger.H"
#endif
#include <assert.h>

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template<class T> ostream& operator<<(ostream& strm ,
const NIDLListNode<T> &node) {
  strm << "NIDLListNode: &node = " << &node
       << " bk = " << node.prev() 
       << " fd = " << node.next(); 
  return strm;
}

template<class T> void NIDLListNode<T>::printOn(ostream &os) const {
  os << "NIDLListNode: this =" << this 
     << " pr=" << pr << " ne =" << ne << endl; 
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template<class T> NIDLListNode<T>* NIDLList<T>::prepend(T* item) {
  NIDLListNode<T>* pn = new(pseglist->getPtr()) NIDLListNode<T>(item);
  if(fi==0) {
    fi=la=pn;
    return pn;
  }
  pn->ne=fi;
  fi->pr=pn;
  fi=pn;
  count--;
  return pn;
}

template<class T> NIDLListNode<T>* NIDLList<T>::append(T* item) {
  NIDLListNode<T> *pn = new(pseglist->getPtr()) NIDLListNode<T>(item);
  if(fi==0) {
    fi=la=pn;
    return pn;
  }
  pn->pr=la;
  la->ne=pn;
  la=pn;
  count++;
  return pn;
}


template<class T> NIDLListNode<T>* NIDLListNode<T>::remove() {
  if(pr==0) { //this if first
    ne->pr=0;
    ne=0;
    pr=0;
    return this;
  }

  if(ne==0) { //this is last
    pr->ne=0;
    ne=0;
    pr=0;
    return this;
  }

  pr->ne=ne;
  ne->pr=pr;
  ne=0;
  pr=0;
  return this;
}

template<class T> T NIDLList<T>::remove(NIDLListNode<T> *p) {
  if(p==fi) {
    fi=p->ne;
    p->ne->pr=0;
    p->ne=0;
    p->pr=0;
    return p->get();
  }

  if(p==la) {
    la=p->pr;
    p->pr->ne=0;
    p->ne=0;
    p->pr=0;
    return p->get();
  }

  p->pr->ne=p->ne;
  p->ne->pr=p->pr;
  p->ne=0;
  p->pr=0;
  return p->get();
}

template<class T> ostream& operator<<(ostream& strm,
const NIDLList<T> &list) {
  NIDLListNode<T>* p = list.first();
  if (p) {
    for( int l=0; p; p=list.ne(p), l++ ){
      strm << l << " : " << *p << "\n";
    }
  } else strm << "list is empty" << endl;
  return strm;
}


  template<class T> void NIDLList<T>::printOn(ostream& os) const {
    int l=0;
    os << "NIDLList<T>: " << this << endl;
    for (NIDLListNode<T> *p = first();p;p=next(p), l++) {
      //os << l << " : " << p << " p.t=" << p->get() << endl;
    }
    os << "\n";
}
