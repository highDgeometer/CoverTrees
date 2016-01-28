#include "FastSeg.H"
//#include "Types.H"
#include <iostream>
#include "MemoryDebugger.H"

using namespace std;

template<class T> SegList<T>::Seg::Seg(unsigned int s_count,unsigned int n) :
seg_count(s_count), base(0),current(0), next(0), end(0),number(n) {
    
#ifdef MEM_DEBUG
    current=base=(T*)OPERATOR_NEW_BRACKET(char,seg_count*number*sizeof(T));
#else
    current=base=(T*)new char[seg_count*number*sizeof(T)];
#endif
    
    end=base+seg_count*number;
}

template<class T> SegList<T>::Seg::~Seg() {
    
#ifdef MEM_DEBUG
    //mem_check();
#endif
    delete [] (char*)base;
}

template<class T> T* SegList<T>::Seg::getPtr() {
    if(current<end) {
        T* temp=current;
        current+=number;
        return temp;
    } else {
        return 0;
    }
}

template<class T> void SegList<T>::Seg::printOn(ostream& os) const {
    os << "this=" << this << endl;
    os << "\tbase=" << base << " current=" << current
    << " next=" << next << " end=" << end << "number=" << number << endl;
    //for(T* t=base;t<end;t++)
    //  t->printOn(os);
}

template<class T> SegList<T>::SegList(unsigned int s_count,unsigned int n) :
seg_count(s_count), first(0),current(0), last(0), total_nbytes(0),
number(n) {
}

template<class T> SegList<T>::~SegList() {
    
    Seg* seg=first;
    while(seg) {
        Seg* sseg=seg->next;
        seg->~Seg();
        delete [] (char*)seg;
        seg=sseg;
#ifdef MEM_DEBUG
        //mem_check();
#endif
    }
#ifdef MEM_DEBUG
    //mem_check();
#endif
}

template<class T> void SegList<T>::reset() {
    
    for( Seg* s=first;s;s=s->next ) {
        s->reset();
    }
    current=first;
}



template<class T> T* SegList<T>::getPtr() {
    total_nbytes+=number*sizeof(T);
    if(current) {  //this is not empty
        T* p=current->getPtr();
        if(p) {  //current had something left, return it
            return p;
        } else { //current had nothing left; move to next
            Seg* seg=current->next;
            if(seg) {
                current=seg;
                T* p=seg->getPtr();
                if(!p)
                    cout << "Uh oh!" << endl;
                return p;
            } else { //ran out, need new seg
#ifdef MEM_DEBUG
                Seg* newseg=OPERATOR_NEW Seg(seg_count,number);
#else
                Seg* newseg=new Seg(seg_count,number);
#endif
                current->next=newseg;
                current=last=newseg;
                return newseg->getPtr();
            }
        }
    } else {//this is empty
#ifdef MEM_DEBUG
        Seg* newseg = OPERATOR_NEW Seg(seg_count,number);
#else
        Seg* newseg = new Seg(seg_count,number);
#endif        
        last = current = first = newseg;
        return current->getPtr();
    }
}

template<class T> void SegList<T>::printOn(ostream& os) const {
    os << "total_nbytes=" << total_nbytes << " sizeof(T)=" << sizeof(T) 
    << " number=" << number <<endl;
    int i=0;
    for( Seg* s=first;s;s=s->next,i++ ) {
        os << "i=" << i << endl;
        s->printOn(os);
    }
}



