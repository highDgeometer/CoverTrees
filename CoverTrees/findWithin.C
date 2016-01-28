#include "Cover.H"
#include "FindWithinData.H"

#ifdef MEM_DEBUG
#include "MemoryDebugger.H"
#endif

extern int dim;
extern REAL theta;
extern REAL mu;

void Cover::findWithin(const Point* point,REAL r,int numfindlevels,
                       Cover::DescendList& descend_list) {
    
    if(numfindlevels<=0)
        return;
    if(numfindlevels>numlevels) {
        numfindlevels=numlevels;
    }
    int findlevel=minlevel+numfindlevels;
    for(int l=minlevel;l<findlevel;l++) {  //will look for children of level m-1
        descend_list.descendForFindWithin(r);
    }
    //can't call prune here
    DescendNode* node=descend_list.first();
    while(node) {
        DescendNode* nextnode=descend_list.next(node);
        if((node->getDist())>=r) {
            descend_list.remove(node);
        }
        node=nextnode;
    }
    
}

void Cover::DescendList::descendForFindWithin(REAL r) {
    
    REAL muradius=mu*theta*radius;
    // append to S_{j+1} from P^{-1}[S_j] and update min_dist
    DescendNode* descendnode=first();
    DLPtrListNode<CoverNode>* childnode=0;
    while(descendnode) {
        childnode=descendnode->getCurrentChild();
        DescendNode* nextdescendnode=next(descendnode);
        while((childnode) && ((childnode->getPtr()->getLevel())==level+1)) {
            DLPtrListNode<CoverNode>* nextchildnode=childnode->next();
            REAL dist=point->getDist(childnode->getPtr()->getPoint());
            if(dist< muradius+r) {
                DescendNode* newdescendnode=new(descendnode_seglist.getPtr())
                DescendNode(childnode->getPtr(),
                            childnode->getPtr()->getChildren()->first(),dist);
                prepend(newdescendnode);
            }
            childnode=nextchildnode;
        }
        descendnode->setCurrentChild(childnode);
        REAL dist=descendnode->getDist();
        if(dist>=muradius+r) {
            remove(descendnode);
        }
        descendnode=nextdescendnode;
    }
    
    level++;
    radius*=theta;
    
}

// for findWithin with threads

void* h(void* p) {
    FindWithinArg* parg=(FindWithinArg*)p;
//    int tid=parg->getTid();
    Cover* pcover=parg->getCover();
    ThreadsWithCounter* threads=parg->getThreads();
    FindWithinData* findwithindata=parg->getFindWithinData();
//    threads->semStartWait(tid);
    //Cover::DescendList descend_list;
    while(1) {
        threads->Lock();
        int counter=threads->getCounter(); // how many left to insert
        threads->decrementCounter();
        threads->unLock();
        if(counter<=0)
            break;
        counter--;
        const Point* point=findwithindata->getPoint(counter);
        Cover::DescendList& descend_list=findwithindata->getDescendList(counter);
        descend_list.reset(pcover,point);
        pcover->findWithin(point,
                           findwithindata->getDist(counter),
                           findwithindata->getNumFindLevels(counter),
                           descend_list);
    }
//    threads->semEndPost(tid);
//    threads->semStartWait(tid);
    
    return 0;
}


int Cover::findWithin(Points& points,FindWithinData& findwithindata, DescendList* descendlists) {
    
    ThreadsWithCounter* threads=findwithindata.getThreads();
    int NTHREADS=threads->getNThreads();
    
    if(NTHREADS==0) {
        const Point* point=points.next();
        int i=0;
        while(point) {
            descendlists[i].reset(this,point);
            findWithin(point,findwithindata.getDist(i),findwithindata.getNumFindLevels(i),descendlists[i]);
            /*
             bool test=checkFindWithin(point,r[i],numfindlevels[i],descendlists[i]);
             if(test==false) {
             cout << i << " failed" << endl;
             }
             */
            point=points.next();
            i++;
        }
    } else {
        unsigned long N=points.getCount();
        threads->setCounter((unsigned)N);
        threads->initializeSemaphores();
        
#ifdef MEM_DEBUG
        void** args                     = OPERATOR_NEW_BRACKET(void*,NTHREADS);
        FindWithinArg* findwithinargs   = OPERATOR_NEW_BRACKET(FindWithinArg,NTHREADS);
#else
        void** args                     = new void*[NTHREADS];
        FindWithinArg* findwithinargs   = new FindWithinArg[NTHREADS];
#endif
        
        for(int t=0;t<NTHREADS;t++) {
            args[t]=findwithinargs+t;
            new(args[t]) FindWithinArg(t,threads,this,&findwithindata);
        }
        
        threads->create(h,args);
        
//        threads->semEndWait();                                                  // MM: TBD: should this be here?
        
        threads->join();
        
        delete [] findwithinargs;
        delete [] args;
    }
    
    INDEX N = points.getCount();
    INDEX totalfound=0;
    for(INDEX i=0;i<N;i++) {
        totalfound+=descendlists[i].getCount();
    }
    return totalfound;
}

void Cover::findWithin(Points* points,REAL r,int numfindlevels,
                       int*& offsets,int*& counts,const Point**& pout) {
    INDEX N=points->getCount();
    
#ifdef MEM_DEBUG
    DescendList* descendlists=
    (DescendList*)OPERATOR_NEW_BRACKET(char,N*sizeof(DescendList));
#else
    DescendList* descendlists=new DescendList[N];
#endif
    
    for(int i=0;i<N;i++) {
        const Point* point=points->next();
        descendlists[i].reset(this,point,r);
        findWithin(point,r,numfindlevels,descendlists[i]);
    }
    
#ifdef MEM_DEBUG
    offsets=OPERATOR_NEW_BRACKET(int,N);
    counts=OPERATOR_NEW_BRACKET(int,N);
#else
    offsets=new int[N];
    counts=new int[N];
#endif
    
    offsets[0]=0;
    int total=0;
    for(int i=0;i<N;i++) {
        offsets[i]=total;
        total+=counts[i]=descendlists[i].getCount();
    }
    
#ifdef MEM_DEBUG
    pout=OPERATOR_NEW_BRACKET(const Point*,total);
#else
    pout=new const Point*[total];
#endif
    int offset=0;
    for(int i=0;i<N;i++) {
        DescendList* list=&descendlists[i];
        for(DescendNode* node=list->first();node;node=list->next(node)) {
            pout[offset++]=node->getCoverNode()->getPoint();
        }
    }
    
    delete [] descendlists;
}



void printFromFindWithin(Points* points,int* offsets,int* counts,const Point** pout,ostream& os) {
    
    cout << "printFromFindWithin" << endl;
    points->reset();
    const Point* point=points->next();
    int i=0;
    while(point) {
        point->printOn(os);
        os << endl;
        for(int j=0;j<counts[i];j++) {
            const Point* outpoint=pout[offsets[i]+j];
            os << "\t";
            outpoint->printOn(os);
            os << endl;
        }
        i++;
        point=points->next();
    }
}


void Cover::fillArrFromDescendList(int* arr,const Points* points,DescendList& descendlist) const {
    //arr must point to descendlist.getCount() integers
    int i=0;
    for(DescendNode* descendnode=descendlist.first();descendnode;
        descendnode=descendlist.next(descendnode)) {
        const Point* point=descendnode->getCoverNode()->getPoint();
        int index=points->getIndex(point);
        arr[i++]=index;
    }
}



bool Cover::checkFindWithin(const Point* p,REAL r,int numfindlevels,
                            DescendList& descend_list) {
    if(numfindlevels<=0)
        return true;
    //cout << "found:" << endl;
    bool val=true;
    //p->printOn(cout);
    //cout << " dist=" << r << endl;
    for(DescendNode* descendnode=descend_list.first();descendnode;
        descendnode=descend_list.next(descendnode)) {
        const Point* q=descendnode->getCoverNode()->getPoint();
        REAL dist=p->getDist(q);
        //cout << "\t";
        //q->printOn();
        //cout << endl;
        if(dist>=r) {
            val=false;
            cout << "\t";
            p->printOn(cout);
            cout << " ";
            q->printOn(cout);
            cout << endl;
            cout << "\tdist " << dist << " should be < " << r << endl;
        }
        //cout << endl;
    }
    
    //cout << "checking cover:" << endl;
    int n=0;
    for(DLPtrListNode<CoverNode>* node=first();node;node=next(node)) {
        CoverNode* covernode=node->getPtr();
        const Point* q=covernode->getPoint();
        int level=covernode->getLevel();
        REAL dist=p->getDist(q);
        if((dist<r)&&(level<=minlevel+numfindlevels)) {
            //cout << "\t";
            //q->printOn(cout);
            //cout << endl;
            n++;
        }
    }
    
    if(n!=descend_list.getCount()) {
        val=false;
        cout << "number within=" << n << " number found=" << descend_list.getCount() << endl;
    }
    return val;
}

