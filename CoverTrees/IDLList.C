#include "IDLList.H"
#include <stdlib.h>

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template<class T> void IDLList<T>::prepend(T *item) {
  if(fi==0) {
    fi=item;
    la=item;
  } else {
    fi=fi->prepend(item);
  }
  count++;
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template<class T> void IDLList<T>::append(T *item) {
  if(fi==0) {
    fi=item;
    la=item;
  } else {
    la=la->append(item);
  }
  count++;
}

template<class T> void IDLList<T>::remove(T *p) {
  if(p==fi) {
    fi=p->next();
  } 
  if(p==la) {
    la=p->prev();
  } 
  p->remove();
  count--;
}

template<class T> void IDLList<T>::printOn(ostream& os) const {
  int l=0;
  os << "IDLList: " << this << endl;
  for (T *p = first();p;p=next(p), l++) {
    os << l << " : "; p->printOn(os);
  }
  os << "\n";
  os << "count=" << count << endl;
}
/*
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
template<class T> ostream& operator<<(ostream& s,const IDLList<T> &dl) {
  T *p=dl.first();
  if (p) {
    for (int l=0; p; p=dl.next(p), l++) {
      s << l << " : "; p->printOn(s); s << endl;
    }
  } else s << "list is empty" << endl;
  return s;
}
*/
