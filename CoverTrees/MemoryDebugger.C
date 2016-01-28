#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <vector>
#ifdef __GNUC__
//#include <new.h>
#endif
#ifdef MEM_DEBUG
#include "MemoryDebugger.H"
#include "jtIDLList.C"

extern "C" {
  void F77_NAME(test_types)();
}

//buffer_type MemoryNode::beforeval=0x7ff0123456789abc;
//buffer_type MemoryNode::afterval =0x7ffedcba98765432;
buffer_type MemoryNode::beforeval=0x12345678;
buffer_type MemoryNode::afterval=0x87654321;
char MemoryNode::mallocval=0xEE;
size_t MemoryNode::buffer_bytes=0U;
size_t MemoryNode::buffer_size=0U;

bool MemoryDebugger::inited = false;
int MemoryDebugger::count=0;
size_t MemoryDebugger::buffer_size=1U;
long MemoryDebugger::numalloc=0;
long MemoryDebugger::maxalloc=0;
jtIDLList<MemoryNode>* MemoryDebugger::alloclist=0;

// The following should be selected to give maximum probability that
// pointers loaded with these values will cause an obvious crash. On
// Unix machines, a large value will cause a segment fault.
// MALLOCVAL is the value to set malloc'd data to.
char MemoryDebugger::badval=0x7A;

void MemoryNode::create(size_t n,void *d) {
  CHECK_POSITIVE(buffer_size)
//first we have storage for this
  char *dd=static_cast<char*>(d);
  size_t nd=sizeof(MemoryNode); Align(nd);

//next, we have storage for the before buffer
  buffer_type *val=static_cast<buffer_type*>(static_cast<void*>(dd+nd));
  for (size_t i=0;i<buffer_size;i++,val++) {
    *val=beforeval;
  }
  nd+=buffer_bytes;

//next, we have storage for the user data
  data=static_cast<void*>( dd+nd );
  nbytes=n; Align(nbytes);
  memset(data,mallocval,nbytes);
  nd+=nbytes;

//finally, we have storage for the after buffer
  val=static_cast<buffer_type*>(static_cast<void*>(dd+nd));
  for (size_t i=0;i<buffer_size;i++,val++) {
    *val=afterval;
  }
}

MemoryNode::MemoryNode(size_t n,void *d) : line_number(-1) {
  create(n,d);
  file[0]='\0';
}

MemoryNode::MemoryNode(size_t n,void *d,const char *f,int l) : 
line_number(l) {
  create(n,d);
  int len=strlen(f);
  len=min(FILE_LENGTH-1,len);
  strncpy(file,f,len);
  file[len]='\0';
}

bool MemoryNode::checkBuffer() const {
  CHECK_POSITIVE(buffer_size)
  char *dd=static_cast<char*>(data);
  buffer_type *val=
    static_cast<buffer_type*>(static_cast<void*>(dd-buffer_bytes));
  for (size_t i=0;i<buffer_size;i++,val++) {
    if (*val!=beforeval) {
      cout << "Pointer " << data << " underrun" << endl;
      printOn(cout);
      return false;
    }
  } 
  val=static_cast<buffer_type*>(static_cast<void*>(dd+nbytes));
  for (size_t i=0;i<buffer_size;i++,val++) {
    if (*val!=afterval) {
      cout << "Pointer " << data << " overrun" << endl;
      printOn(cout);
      return false;
    }
  } 
  return true;
}

void MemoryNode::printOn(ostream &os) const {
  os << "MemoryNode:: nbytes=" << nbytes << ",data=" << data << endl;
  if (line_number>=0) {
    os << "\tallocated in file " << file << " at line number "
       << line_number << endl;
  }
}

MemoryDebugger::MemoryDebugger(size_t bs) { 
  if (!inited) {
    buffer_size=bs;
    CHECK_TEST(alloclist==0);
    alloclist=new jtIDLList<MemoryNode>();
    inited=true;
#ifdef DEBUG
    //F77_NAME(test_types)();
#endif
  }
}

MemoryDebugger::~MemoryDebugger() {
  if (inited) {
    if (count!=0) {
#ifndef ALPHA
      cout << "in MemoryDebugger::~MemoryDebugger count = " << count 
           << endl;
      printOn(cout);
      cout << "count=" << count << " should be zero" << endl;
      //CHECK_TEST(count == 0);
#else
      register MemoryNode *mn=alloclist->first();
      bool trouble=false;
      for ( ; mn; mn = alloclist->next(mn)) {
	if (mn->size()!=1024) {
	  trouble=true;
	  cout << "Unfreed pointer: ";
	  mn->printOn(cout);
	}
      }
      CHECK_TEST(!trouble);
#endif
    }
  }
  inited = false;
  if (alloclist!=0) delete alloclist; alloclist=0;
  buffer_size=0;
}

void* MemoryDebugger::malloc(size_t n) {
  if (inited) {
    size_t sz=MemoryNode::allocationSize(n,buffer_size);
    void *data=::malloc(sz);
    if (data) {
      MemoryNode *mn=new(data) MemoryNode(n,data);
      alloclist->append(mn);
      count++;
//    bytes allocated may be more than requested because of alignment:
      numalloc += mn->size();
      if (numalloc > maxalloc) maxalloc = numalloc;
      return mn->ptr();
    } else {
      cout << 
          "\tMemoryDebugger::malloc failed to allocate pointer of size "
           << n << endl;
      cout << "\tsz = " << sz << endl;
      assert(data);
    } 
    return data;
  } else {
    return ::malloc(n);
  }
}

void* MemoryDebugger::malloc(size_t n,const char *f,int l) {
  if (inited) {
    size_t sz=MemoryNode::allocationSize(n,buffer_size);
    void *data=::malloc(sz);
    if (data) {
      MemoryNode *mn=new(data) MemoryNode(n,data,f,l);
      alloclist->append(mn);
      count++;
//    bytes allocated may be more than requested because of alignment:
      numalloc += mn->size();
      if (numalloc > maxalloc) maxalloc = numalloc;
      return mn->ptr();
    } else {
      cout << "\tMemoryDebugger::malloc failed to alloc ptr of size "
           << n << endl;
      cout << "\tsz = " << sz << endl;
      assert(data);
    } 
    return data;
  } else return ::malloc(n);
}

void MemoryDebugger::free(void *ptr) {
//Linux OS 7.1 ostrostream::~ostrstream can call operator delete[] 
//  for a zero pointer
  if (!ptr) return;
  if (inited) {
    count--;
    if (count < 0) {
      cout << "More frees than allocs";
      assert(count>=0);
    }

    MemoryNode *mn = convert(ptr);
    assert(mn->checkBuffer());

    size_t bytes=mn->size();
    numalloc -= bytes;
    if (numalloc < 0) {
      cout << "freeing more bytes than allocated" << endl;
      cout << "numalloc = " << numalloc << " nbytes = " << bytes <<endl;
      CHECK_NONNEGATIVE(numalloc)
    }
    alloclist->remove(mn);

//  Stomp on the freed storage to help detect references
//  after the storage was freed.
//  mn->printOn will give segmentation fault after this
    memset(mn,badval,mn->allocationSize(bytes,buffer_size));
    ::free(static_cast<void *>(mn));
  } else {
    ::free(ptr);
  }
}

void MemoryDebugger::check() {
  if (inited) {
    register MemoryNode *mn=alloclist->first();
    for ( ; mn; mn = alloclist->next(mn)) {
      assert(mn->checkBuffer());
    }
  }
}

void MemoryDebugger::checkPtr(void *p) {
  if (p!=0 && inited) {
    MemoryNode *mn=convert(p);
    mn->printOn(cout);
    assert(mn->checkBuffer());
  }
}

void MemoryDebugger::printOn(ostream &os) {
  os << "MemoryDebugger: inited = " << inited << endl;
  os << "\tcount = " << count << endl;
  os << "\tbuffer_size = " << buffer_size << endl;
  os << "\tnumalloc = " << numalloc << endl;
  os << "\tmaxalloc = " << maxalloc << endl;
  os << "\tbadval = " << badval << endl;
  register MemoryNode *mn=alloclist->first();
  for ( ; mn; mn = alloclist->next(mn)) {
    mn->printOn(os);
  }
}

void* operator new(size_t n,const std::nothrow_t&) throw() {
  void *p=0;
  if (MemoryDebugger::active()) {
    p=MemoryDebugger::malloc(n);
#ifdef DEBUG
//  cout << "operator new creating pointer=" << p << " of size " << n 
//       << endl;
#endif
  } else {
    p=malloc(n);
    if (p==0) assert(p);
  }
  return p;
}

void* operator new(size_t n) throw(std::bad_alloc) {
  void *p=operator new(n,nothrow);
  return p;
}

// see http://www.tru64unix.compaq.com/linux/compaq_cxx/docs/uguimpl.htm
#ifdef __DECCXX
extern "C" {
//http://www.tru64unix.compaq.com/linux/compaq_cxx/docs/ugustl.htm#override_new
# ifdef __MODEL_ANSI
  void* __7__nw__FUl(size_t n)
# else // __MODEL_ARM
  void* __nw__XU1(size_t n)
# endif
  {
//  if (MemoryDebugger::active()) printf("\n\tin __7__nw__FUl n = %d\n",n);
    return operator new(n,nothrow);
  }
}
#endif

void  operator delete(void* ptr,const std::nothrow_t&) throw() {
  if (MemoryDebugger::active()) {
#ifdef DEBUG
    //cout << "operator delete freeing pointer=" << ptr << endl;
#endif
    MemoryDebugger::free(ptr);
  } else {
#ifndef ALPHA
    free(ptr);
#endif
  }
}

void  operator delete(void* ptr) throw() {
  operator delete(ptr,nothrow);
}

//this is called before the array entries are constructed
void* operator new[](size_t n,const std::nothrow_t&) throw() {
  void *p=0;
  if (MemoryDebugger::active()) {
    p=MemoryDebugger::malloc(n);
#ifdef DEBUG
//  cout << "operator new[] creating pointer=" << p << " of size " << n 
//       << endl;
#endif
  } else {
    p=malloc(n);
    if (p==0) assert(p);
  }
  return p;
}

//this is called before the array entries are constructed
void* operator new[](size_t n) throw(std::bad_alloc) {
  void *p=operator new[](n,nothrow);
  return p;
}

//this is called after the array entries are destructed
void operator delete[](void *ptr,const std::nothrow_t&) throw() {
  if (MemoryDebugger::active()) {
#ifdef DEBUG
    cout << "operator delete[] freeing pointer=" << ptr << endl;
#endif
    MemoryDebugger::free(ptr);
  } else { 
    free(ptr);
  }
}

//this is called after the array entries are destructed
void operator delete[](void *ptr) throw() {
  operator delete[](ptr,nothrow);
}

extern "C" {
void mem_check_ptr(void *p) { MemoryDebugger::checkPtr(p); }
void mem_check() { MemoryDebugger::check(); }
void F77_NAME(mem_check)() { MemoryDebugger::check(); }
}

template class jtIDLList<MemoryNode>;
#endif

void* operator new(size_t n,const char *f,int l) {
  void *p=0;
#if MEM_DEBUG
  if (MemoryDebugger::active()) {
    p=MemoryDebugger::malloc(n,f,l);
#ifdef DEBUG
//  cout << "operator new(char*,int) creating pointer = " << p 
//       << " of size " << n << endl;
#endif
  } else  
#endif
  p=malloc(n);
  return p;
}

//this is called before the array entries are constructed
void* operator new[](size_t n,const char *f,int l) {
  void *p=0;
#if MEM_DEBUG
  if (MemoryDebugger::active()) {
    p=MemoryDebugger::malloc(n,f,l);
#ifdef DEBUG
//  cout << "operator new[](char*,int) creating pointer = " << p 
//       << " of size " << n << endl;
#endif
  } else  
#endif
  p=malloc(n);
  return p;
}
