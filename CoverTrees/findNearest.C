//#include "mex.h"
#include "Cover.H"
#include "FindNearestData.H"
#include "MemoryDebugger.H"

#include <vector>
#include <algorithm>

extern int dim;
extern REAL theta;
extern REAL mu;


void Cover::findNearest(const Point* point,vector<DescendNodePtr>& vdn,
                        int k,int& K,int L,Cover::DescendList& descend_list,const Point** ptarr,REAL* distances) {
    
    if(k<=0)
        return;
    REAL r=point->getDist(root->getPoint());
    for(int l=minlevel;l<=L;l++) {          //will look for children of level m-1. MM: changed l<L to l<=L
        descend_list.descendForFindNearest(vdn,r,k);
    }
    
    int n=descend_list.getCount();
    
    sort(&vdn[0],&vdn[0]+n);
    
    K= n<k ? n : k;
    
    for(int i=0;i<K;i++) {
        ptarr[i]=vdn[i].getPtr()->getCoverNode()->getPoint();
        distances[i]=vdn[i].getPtr()->getDist();
    }
    
    for(int i=K;i<k;i++) {
        ptarr[i]=0;
        distances[i]=-1.0;
    }
}

void Cover::DescendList::descendForFindNearest(vector<DescendNodePtr>& vdn,REAL& r,int k) {    
    int n=0;
    REAL muradius=mu*theta*radius;
    
    // append to T_{j+1} from P^{-1}[T_j] and update min_dist
    DescendNode* descendnode=first();
    vdn[0]=DescendNodePtr(descendnode);
    DLPtrListNode<CoverNode>* childnode=0;
    while(descendnode) {
        vdn[n++]=DescendNodePtr(descendnode);
        childnode=descendnode->getCurrentChild();
        DescendNode* nextdescendnode=next(descendnode);
        while((childnode) && (((childnode->getPtr()->getLevel())==level+1) || (childnode->getPtr()->getLevel())==INT_MAX)) {
            DLPtrListNode<CoverNode>* nextchildnode=childnode->next();
            REAL dist=point->getDist(childnode->getPtr()->getPoint());
            if(dist< muradius+r) {
                DescendNode* newdescendnode=new(descendnode_seglist.getPtr())
                DescendNode(childnode->getPtr(),childnode->getPtr()->getChildren()->first(),dist);
                prepend(newdescendnode);
                vdn[n++]=DescendNodePtr(newdescendnode);
            }
            childnode=nextchildnode;
        }
        descendnode->setCurrentChild(childnode);
        descendnode=nextdescendnode;
    }
    
    sort(&vdn[0],&vdn[0]+n);
    if(n<k) {
        r=vdn[n-1].getDist();
    } else {
        r=vdn[k-1].getDist();
    }
    
    descendnode=first();
    while(descendnode) {
        childnode=descendnode->getCurrentChild();
        DescendNode* nextdescendnode=next(descendnode);
        if( (muradius+r>0) && (descendnode->getDist())>=muradius+r) {
            remove(descendnode);
        }
        descendnode=nextdescendnode;
    }
    
    
    level++;
    radius*=theta;
    
}

// for findNearest with threads

void* i(void* p) {
    FindNearestArg* parg=(FindNearestArg*)p;
    //int tid=parg->getTid();
    Cover* pcover=parg->getCover();
    int N=pcover->getCount();
    vector<DescendNodePtr> vdn(N);
    ThreadsWithCounter* threads=parg->getThreads();
    FindNearestData* findnearestdata=parg->getFindNearestData();
    int k=findnearestdata->getNumberToFind();
    int L=findnearestdata->getMaxLevelPresent();
    int K;
    const Point** ptarr=findnearestdata->getPtArr();
    //REAL* distances=findnearestdata->getDist();
    //threads->semStartWait(tid);
    Cover::DescendList descend_list;
    while(1) {
        threads->Lock();
        int counter=threads->getCounter(); // how many left to insert
        threads->decrementCounter();
        threads->unLock();
        if(counter<=0)
            break;
        counter--;
        const Point* point=findnearestdata->getPoint(counter);
        REAL* distances=findnearestdata->getDist();
        //int index=findnearestdata->getIndex(point);
        descend_list.reset(pcover,point,0,true);
        pcover->findNearest(point,vdn,k,K,L,descend_list,ptarr+k*counter,distances+k*counter);
    }
    //threads->semEndPost(tid);   MM: Is this needed?
    //threads->semStartWait(tid);
    
    return 0;
}


void Cover::findNearest(Points& querypoints,FindNearestData& findnearestdata,
                        const Point** ptarr,REAL* distances) {
    
    ThreadsWithCounter* threads=findnearestdata.getThreads();
    int NTHREADS=threads->getNThreads();
    
    if(NTHREADS==0) {
        int N=getCount();
        vector<DescendNodePtr> vdn(N);
        int k=findnearestdata.getNumberToFind();
        Cover::DescendList descendlist;
        const Point* point=querypoints.next();
        int K;
        int L=getMaxLevelPresent();
        int i=0;
        while(point) {
            descendlist.reset(this,point,0,true);
            findNearest(point,vdn,k,K,L,descendlist,ptarr+i*k,distances+i*k);
            i++;
            /*
             bool test=checkFindNearest(point,r[i],numfindlevels[i],descendlists[i]);
             if(test==false) {
             cout << i << " failed" << endl;
             }
             */
            point=querypoints.next();
        }
    } else {
        int N=querypoints.getCount();
        threads->setCounter(N);
        threads->initializeSemaphores();
        
#ifdef MEM_DEBUG
        void** args=(void**)OPERATOR_NEW_BRACKET(char,NTHREADS*sizeof(void*));
#else
        void** args=new void*[NTHREADS];
#endif
        
#ifdef MEM_DEBUG
        FindNearestArg* findnearestargs=(FindNearestArg*)
        OPERATOR_NEW_BRACKET(char,NTHREADS*sizeof(FindNearestArg));
#else
        FindNearestArg* findnearestargs=new FindNearestArg[NTHREADS];
#endif
        
        for(int t=0;t<NTHREADS;t++) {
            args[t]=findnearestargs+t;
            new(args[t]) FindNearestArg(t,threads,this,&findnearestdata);
        }
        
        threads->create(i,args);
        
        //threads->semEndWait(); //MM: Is this needed?
        
        threads->join();
        
        delete [] findnearestargs;
        delete [] args;
    }
}

void Cover::printFindNearest(Points& querypoints, const Point** ptarr,int k,ostream& os) {
    
    querypoints.reset();
    int N=querypoints.getCount();
    for(int i=0;i<N;i++) {
        const Point* querypoint=ptarr[(k+1)*i];
        querypoint->printOn(os);
        os << endl;
        for(int j=1;j<k+1;j++) {
            const Point* foundpoint=ptarr[i*(k+1)+j];
            os << "\t ";
            if(foundpoint) {
                foundpoint->printOn(os);
                os << endl;
            }
        }
    }
}

bool Cover::checkFindNearest(Points& querypoints,const Point** ptarr,int k) {
    
    bool val=true;
    
    int N=getCount();
    int M=querypoints.getCount();
    vector<REAL> distances(N);
    REAL dist;
    for(int i=0;i<M;i++) {
        const Point* point=ptarr[i*(k+1)];
        //point->printOn(cout);
        int j=0;
        for(DLPtrListNode<CoverNode>* node=first();node;node=next(node)) {
            distances[j]=point->getDist(node->getPtr()->getPoint());
            j++;
        }
        sort(&distances[0],&distances[0]+k); //MM should be k+1?
        dist=distances[k-1];
        for(j=1;j<k+1;j++) {
            REAL d=point->getDist(ptarr[i*(k+1)+j]);
            if(d>dist) {
                val=false;
                cout << "d=" << d << " should be <= dist=" << dist << endl;
            }
        }
    }
    return val;
}


