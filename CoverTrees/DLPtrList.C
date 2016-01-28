#include "DLPtrList.H"
#include <limits.h>
#include <stdlib.h>
#ifdef MEM_DEBUG
#include "MemoryDebugger.H"
#endif
#include <assert.h>

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template<class T> ostream& operator<<(ostream& strm ,
                                      const DLPtrListNode<T> &node) {
    strm << "DLPtrListNode: &node = " << &node
    << " bk = " << node.prev()
    << " fd = " << node.next();
    return strm;
}

template<class T> void DLPtrListNode<T>::printOn(ostream &os) const {
    os << "DLPtrListNode: this =" << this
    << " pr=" << pr << " ne =" << ne << endl;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template<class T> DLPtrListNode<T>* DLPtrList<T>::prepend(T* item) {
    DLPtrListNode<T>* pn = new(pseglist->getPtr()) DLPtrListNode<T>(item);
    count++;
    if(fi==0) {
        fi=la=pn;
        return pn;
    }
    pn->ne=fi;
    fi->pr=pn;
    fi=pn;
    return pn;
}

template<class T> DLPtrListNode<T>* DLPtrList<T>::append(T* item) {
    DLPtrListNode<T> *pn = new(pseglist->getPtr()) DLPtrListNode<T>(item);
    count++;
    if(fi==0) {
        fi=la=pn;
        return pn;
    }
    pn->pr=la;
    la->ne=pn;
    la=pn;
    return pn;
}

template<class T> DLPtrListNode<T>* DLPtrList<T>::insertBefore(DLPtrListNode<T> *p,T *item)
{
    DLPtrListNode<T>* newnode=new(pseglist->getPtr()) DLPtrListNode<T>(item);
    if(p==fi) {
        return prepend(newnode->getPtr());
    }
    // p is not first
    count++;
    p->pr->ne=newnode;
    newnode->pr=p->pr;
    newnode->ne=p;
    p->pr=newnode;
    return newnode;
}

template<class T> DLPtrListNode<T>* DLPtrList<T>::insertAfter(DLPtrListNode<T> *p,
                                                              T *item) {
    DLPtrListNode<T>* newnode=new(pseglist->getPtr()) DLPtrListNode<T>(item);
    if(p==la) {
        return append(newnode->getPtr());
    }
    // p is not last
    count++;
    p->ne->pr=newnode;
    newnode->pr=p;
    newnode->ne=p->ne;
    p->ne=newnode;
    return newnode;
}


template<class T> ostream& operator<<( ostream& strm, const DLPtrList<T> &list ) {
    DLPtrListNode<T>* p = list.first();
    if (p) {
        for( int l=0; p; p=list.ne(p), l++ ){
            strm << l << " : " << *p << "\n";
        }
    } else strm << "list is empty" << endl;
    return strm;
}


template<class T> void DLPtrList<T>::printOn( ostream& os ) const {
    os << "DLPtrList<T>: " << this << " count=" << count << endl;
    for (DLPtrListNode<T> *p = first();p;p=next(p)) {
        p->getPtr()->printOn(os);
    }
    os << "\n";
}

template<class T> T* DLPtrList<T>::remove(DLPtrListNode<T> *p) {
    count--;
    
    if((p==fi)&&(p==la)) {
        fi=0;
        la=0;
        return p->getPtr();
    }
    
    if(p==fi) { //this if first
        fi=p->ne;
        p->ne->pr=0;
        p->ne=0;
        p->pr=0;
        return p->getPtr();
    }
    
    if(p==la) { //this is last
        la=p->pr;
        p->pr->ne=0;
        p->ne=0;
        p->pr=0;
        return p->getPtr();
    }
    
    p->pr->ne=p->ne;
    p->ne->pr=p->pr;
    p->ne=0;
    p->pr=0;
    return p->getPtr();
}
