#ifndef MEMORY_DEBUGGER_H
#define MEMORY_DEBUGGER_H

#ifdef MEM_DEBUG
#include "malloc.h"
#include <string>
#include "Align.H"
#include "Arch.H"
#include "Errors.H"
#include "jtIDLListNode.H"
#include "jtIDLList.H"
//#include "Types.H"

//   o Check values are inserted before and after the alloc'ed data
//     to detect pointer underruns and overruns.
//   o Free'd pointers are checked against alloc'ed pointers.
//   o Free'd storage is cleared to smoke out references to free'd data.
//   o The routine mem_checkptr() is provided to check an alloc'ed
//     pointer.

//typedef unsigned long long buffer_type;
//typedef unsigned long long print_type;
typedef unsigned long buffer_type;
typedef unsigned long print_type;
#define FILE_LENGTH 40

class MemoryNode : public jtIDLListNode {
  private:
    size_t nbytes;  // size of allocation
    void* data;   // allocated data
    static buffer_type beforeval;
    static buffer_type afterval;
    static char mallocval;
    static size_t buffer_bytes;
    static size_t buffer_size;
    char file[FILE_LENGTH]; // must be of fixed length avoid calling new
    int line_number;
    void create(size_t,void*);
  public:
    MemoryNode() : nbytes(0),data(0) {}
    MemoryNode(size_t,void*);
    MemoryNode(size_t,void*,const char*,int);
    static size_t allocationSize(size_t n,size_t buf_sz) {
      if (buffer_size==0U) {
        buffer_bytes=sizeof(buffer_type)*buf_sz; Align(buffer_bytes);
        buffer_size=buffer_bytes/sizeof(buffer_type);
      }
      size_t nb=bytesBeforeData();
      nb+=n; Align(nb);
      nb+=sizeof(buffer_type)*buffer_size; Align(nb); 
      return nb;
    }
    static size_t bytesBeforeData() {
      assert(buffer_size>0);
      size_t nb=sizeof(MemoryNode); Align(nb);
      nb+=sizeof(buffer_type)*buffer_size; Align(nb);
      return nb;
    }

    void* ptr() const { return data; }
    size_t size() const { return nbytes; }
    bool inBounds(const void *p) const { 
      return p>= data && p< static_cast<char*>(data)+nbytes;
    }
    bool checkBuffer() const;
    void printOn(ostream&) const;
};

class MemoryDebugger {
  private:
    static bool inited;     // TRUE if mem package is initialized
    static int count;          // # of unfreed allocs
    static size_t buffer_size; // number of beforeval's and afterval's
    static long numalloc;      // current # bytes allocated
    static long maxalloc;      // max # bytes allocated
    static jtIDLList<MemoryNode> *alloclist;
    static char badval;
    static MemoryNode* convert(void *p) {
      assert(inited);
      char *pp=reinterpret_cast<char*>(p)-MemoryNode::bytesBeforeData();
      return reinterpret_cast<MemoryNode*>(pp);
    }
  public:
  MemoryDebugger(size_t=1U /*,ostream* =0*/);
  ~MemoryDebugger();
    static bool active() { return inited; }
    static void check() ;
    static void* malloc(size_t);
    static void* malloc(size_t,const char *,int);
    static void free(void *);
    static void checkPtr(void*);
    static void printOn(ostream&);
};

extern "C" {
void mem_check_ptr(void*);
void mem_check();
void F77_NAME(mem_check)();
}
#endif

void* operator new(size_t,const char*,int);
void* operator new[](size_t,const char*,int);
#define OPERATOR_NEW new(__FILE__,__LINE__)
#define OPERATOR_NEW_BRACKET(T,n) new(__FILE__,__LINE__)T[n]
//operator new with position does not involve MemoryDebugger

#endif
