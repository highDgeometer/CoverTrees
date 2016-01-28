#ifndef IDLLIST_H
#define IDLLIST_H 
#include "IDLListNode.H"

//the instantiation will fail unless T is derived from IDLListNode
template<class T> class IDLList {
private:
  IDLListNode* fi;
  IDLListNode* la;
  int count;
public:
  IDLList() : fi(0), la(0), count(0) {;}
  ~IDLList() {}
  int getCount() const { return count; }
  bool empty() const { return count==0; }
  bool notEmpty() const { return count!=0; }
  
  T* first() const { return (T*)(fi); }  
  T* last() const { return (T*)(la); }
  T* next(const T* p) const {
    return (T*)(p->next());
  }
  T* prev(const T *p) const {
    return (T*)(p->prev());
  }
  
  void prepend(T*);
  void append(T*);
  void remove(T*);

  void reset() { fi=0; la=0; count=0; }
  void printOn(ostream& os = cerr) const;
};

//template<class T> ostream& operator<<(ostream&,const IDLList<T> &);

#endif
