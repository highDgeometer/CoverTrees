#ifndef FAST_SEG_H
#define FAST_SEG_H

#include <iostream>

using namespace std;

template<class T> class SegList {
private:
    class Seg {
    private:
        unsigned int seg_count;
        int number;        
        T* base;
        T* current;
        T* end;
    public:
        Seg* next;
        Seg(unsigned int,unsigned int = 1);
        ~Seg();
        
        unsigned int getNumber()    const  { return number; }
        T* getPtr();
        void reset() { current=base; }
        void printOn(ostream&) const;
    };
    unsigned int seg_count;
    unsigned int total_nbytes;
    Seg* first;
    Seg* current;
    Seg* last;
    unsigned int number;
public:
    SegList(unsigned int,unsigned int = 1);
    ~SegList();
    unsigned int getNumber()    const   { return number; }
    unsigned int getCount()     const   { return seg_count; }
    T* getPtr();
    void reset();
    void printOn(ostream&) const;
};


#endif
