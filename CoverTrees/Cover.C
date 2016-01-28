#include "Cover.H"
#include "DLPtrList.H"
#include <limits.h>
#ifdef MEM_DEBUG
#include "MemoryDebugger.H"
#endif
#include <cmath>
#include <float.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <fstream>
#include "Points.H"
#include "ThreadsWithCounter.H"
#include "EnlargeData.H"
#include "Distances.H"
#include "Vector.H"
#include <algorithm>

//theta and mu are set in main
extern REAL theta;
extern REAL mu;

void setMaxRadius(int minlevel,REAL& maxradius);
void setMaxRadiusAndMinLevel(REAL dist,REAL theta,REAL& maxradius,int& minlevel);
void set(REAL dist,REAL theta,int& level,REAL& radius);


void* g(void* p) {                                                                                                                                                                              // Function for building cover with multiple threads
    EnlargeArg* parg = (EnlargeArg*)p;
    int tid                     = parg->getTid();
    Cover* pcover               = parg->getCover();
    ThreadsWithCounter* threads = parg->getThreads();
    EnlargeData* enlargedata    = parg->getEnlargeData();
    
    Cover::DescendList  descend_list;
    REAL                tempradius;
    int                 templevel;
    
    threads->semStartWait(tid);
    
    while(enlargedata->getRemaining()>0) {                                                                                                                                                      // Get points one by one
        while(1) {
            threads->Lock();
            int counter = threads->getCounter();                                                                                                                                                // How many points left to insert
            int howmany = threads->decrementCounterByAtMost(512);
            threads->unLock();
            if(counter<=0)
                break;
            
            for (int i=0; i<howmany; i++) {
                PointLevelParent* plp = enlargedata->getPointLevelParent(counter-i-1);
                
                if(plp) {
                    descend_list.reset                      ( pcover,plp->getPoint() );
                    pcover->insert                          ( descend_list,plp,tempradius,templevel,parg );
                    enlargedata->incrThreadNCallsToGetDist  ( tid, descend_list.getNCallsToGetDist() );
                    enlargedata->incrThreadTimeToGetDist    ( tid, descend_list.getTimeToGetDist() );
                }
            }
        }
        threads->semEndPost(tid);
        threads->semStartWait(tid);
    }
    
    return 0;
}



Cover::Cover(const Point* point,SegList<DLPtrListNode<CoverNode> >& seglist,int inumlevels, int iminlevel) :
DLPtrList<CoverNode>(&seglist),
root(0),
numlevels(inumlevels),
minlevel(iminlevel),
maxlevel(iminlevel+inumlevels-1),
number_inserted(0),
number_deep(0),
number_duplicates(0),
covernode_seglist(seglist.getCount(),seglist.getNumber()),
dlptrlistnode_covernode_seglist(seglist.getCount(),seglist.getNumber())//,fftw_ws(0)
{
    ::setMaxRadius(minlevel,maxradius);
    
    root = newCoverNode(point,minlevel,0);
    append(root);
    
#ifdef DEBUG
    setCounts();
#endif
}


void Cover::enlargeBy( EnlargeData& enlargedata,Points& points ) {                                                                                                                              // Add points to cover tree, both single- and multi-threaded
    
    ThreadsWithCounter* threads=enlargedata.getThreads();
    int NTHREADS=threads->getNThreads();
    if( (NTHREADS>0) && (points.getRemaining()>0) ) {                                                                                                                                           // Multi-threaded
        //new(threads->getReorderPoints()) IndexedPoint(root->getPoint(),0);
        
        if ( threads->initializeSemaphores() ) {    return;        }
        
#ifdef MEM_DEBUG
        void** args             = OPERATOR_NEW_BRACKET( void*,NTHREADS );
        EnlargeArg* enlargeargs = OPERATOR_NEW_BRACKET( EnlargeArg,NTHREADS );
#else
        void** args             = new void*[NTHREADS];
        EnlargeArg* enlargeargs = new EnlargeArg[NTHREADS];
#endif
        
        for(int t=0;t<NTHREADS;t++) {                                                                                                                                                           // Create args for different threads
            args[t]     =   enlargeargs+t;
            new(args[t])    EnlargeArg( t,threads,this,&enlargedata );
        }
        
        int counter = enlargedata.setPoints(points);
        
        threads->setCounter     ( counter );
        threads->create         ( g,args );
        threads->semStartPost   ();
        
        while(enlargedata.getRemaining()>0) {
            threads->semEndWait();                                                                                                                                                              // wait for threads to be done with insertions
            
            merge(threads,counter,enlargedata);                                                                                                                                                 // merge
            
            enlargedata.decrementRemaining( counter );
            counter = enlargedata.setPoints( points );
            threads->setCounter( counter );
            
            threads->semStartPost();                                                                                                                                                            // let the threads start again inserting
        }
        
        threads->join();                                                                                                                                                                        // let the threads finish, free memory for threads arguments
        delete [] args;
        delete [] enlargeargs;
        
    } else {                                                                                                                                                                                    // We are serial or building mergecover
        const Point* point = points.next();
        Cover::DescendList  descend_list;
        PointLevelParent*   plp = new PointLevelParent;
        REAL    tempradius;
        int     templevel;
        
        while(point) {                                                                                                                                                                          // Insert points one at a time
            descend_list.reset(this,point);
            templevel=INT_MAX;
            insert(descend_list,plp,tempradius,templevel,0);
            if(templevel<minlevel+1) { // need to adjust root
                minlevel=templevel-1;
                root->setLevel(minlevel);
                maxradius=tempradius;
            }
            CoverNode* covernode = newCoverNode(plp);
            append(covernode);
            enlargedata.incrMergeNCallsToGetDist( descend_list.getNCallsToGetDist() );
            enlargedata.incrTimeToGetDist       ( descend_list.getTimeToGetDist() );
            point = points.next();
        }
    }
#ifdef DEBUG
    setCounts();
#endif
}

void Cover::merge(ThreadsWithCounter* threads,int counter,EnlargeData& enlargedata) {                                                                                                           // Merge points inserted by different threads
    
    vector<PointLevelParent>& vplp  = enlargedata.getPointLevelParent();
    CoverNode** mergecovernodes     = enlargedata.getMergeCoverNodes();
    
    sort(&vplp[0],&vplp[0]+counter);
    
    int templevel=vplp[0].getLevel();
    if( templevel<=minlevel ) {
        for(int i=templevel;i<=minlevel;i++) {
            maxradius/=theta;
        }
        minlevel=templevel-1;
        maxlevel=minlevel+numlevels-1;
        root->setLevel(minlevel);
    }
    
    DLPtrList<CoverNode> oldchildren(&dlptrlistnode_covernode_seglist);
    oldchildren = root->children;
    resetRootChildren();
    Cover::DescendList descend_list;
    PointLevelParent plp;
    REAL tempradius;
    
    for( int i=0;i<counter;i++ ) {
        const Point* point  = vplp[i].getPoint();
        descend_list.reset(this,point);
        templevel           = INT_MAX;
        
        insert(descend_list,&plp,tempradius,templevel,0);
        
        enlargedata.incrMergeNCallsToGetDist(descend_list.getNCallsToGetDist());
        
        if( templevel<minlevel+1 ) {                                                                                                                                                            // Need to adjust root
            minlevel=templevel-1;
            root->setLevel(minlevel);
            maxradius=tempradius;
        }
        mergecovernodes[i] = newCoverNode(&plp);
        append( mergecovernodes[i] );
    }
    
    root->children.reset();
    root->children=oldchildren;
    
    for(int i=0;i<counter;i++) {
        CoverNode* covernode = mergecovernodes[i];
        covernode->children.reset();
        
        int threadlevel = vplp[i].getLevel();
        int mergelevel  = mergecovernodes[i]->getLevel();
        
        if( threadlevel>=mergelevel ) {
            covernode->setLevel(threadlevel);
            covernode->setParent(vplp[i].getParent());
        }
        covernode->getParent()->insertInChildrenAtRightLevel(covernode);
    }
}

void Cover::resetRootChildren() {
    root->children=DLPtrList<CoverNode>(&dlptrlistnode_covernode_seglist);
}

Cover::~Cover() {
    
}

void Cover::insert(DescendList& descend_list,PointLevelParent* plp, REAL& tempradius, int& templevel,EnlargeArg* parg) {
    const Point* p      = descend_list.getPoint();
    REAL dist_to_root   = descend_list.getDistToRoot();
    
    if(dist_to_root>=mu*maxradius) {                                                                                                                                                            // Far from root. Will adjust root later
        //        if(parg==0) {                             // MM: this and the next three lines were changed by me. Before the change, I was getting the new point linked to the root, but with the new point at a level higher than the root. This was happening when the second point was at large distance from the root
        //            adjustRootAndMinLevelAndMaxRadius(dist_to_root);
        //            new(plp) PointLevelParent(p,minlevel+1,root);
        //        } else {
        ::setMaxRadiusAndMinLevel   ( dist_to_root,theta,tempradius,templevel );
        plp->setPointLevelParent    ( p,templevel+1,root );
        //        }
        return;
    }
    
    if(dist_to_root==0.0) {                                                                                                                                                                     // Duplicate point
        plp->setPointLevelParent(p,INT_MAX,root);
        return;
    }
    
    int m = minlevel+numlevels-1;                                                                                                                                                               // dist_to_root>0 at this point, and also not too large
    while( (descend_list.first()) && (descend_list.getLevel()<m) ) {                                                                                                                            // since dist<mu*maxradius S_minlevel is nonempty, will enter
        CoverNode* q=descend_list.descend();
        if(q) { //duplicate
            plp->setPointLevelParent(p,INT_MAX,q);
            return;
        }
    }
    
    if( descend_list.getCount()!=0 ) {
        descend_list.prune();
    }
    
    if( descend_list.getCount()!=0 ) { // too deep
        DescendNode* descendnode    = descend_list.last();
        CoverNode*   near_node      = descendnode->getCoverNode();
        //REAL dist_to_near_node=descendnode->getDist();
        plp->setPointLevelParent(p,m+1,near_node);
    } else { // not too deep
        CoverNode* parent=descend_list.getParent();
        if(parent) {
            int new_level=descend_list.getRLevel()+1;
            plp->setPointLevelParent(p,new_level,parent);
        } else { //no parent found;need to adjust root
            if(parg==0) {
                adjustRootAndMinLevelAndMaxRadius(dist_to_root);
                plp->setPointLevelParent(p,minlevel+1,root);
            } else {
                REAL tempradius;
                int level;
                ::setMaxRadiusAndMinLevel(dist_to_root,theta,tempradius,level);
                plp->setPointLevelParent(p,level+1,root);
            }
        }
    }
    return;
}

Cover::DescendList::DescendList() :
IDLList<DescendNode>(),
point(0),
level(INT_MAX),
radius(1.0),
parent(0),
R_level(INT_MAX),
dist_to_root(0.0),
ncallstogetdist(0),
descendnode_seglist(1024*16,1)
{
}

void Cover::DescendList::reset(const Cover* cover,const Point* ppoint,REAL r, bool forcenonempty) {
    IDLList<DescendNode>::reset();
    descendnode_seglist.reset();
    
    point   = ppoint;
    level   = cover->getMinLevel();
    radius  = cover->getMaxRadius(),
    parent  = 0;
    R_level = INT_MAX;
    
    CoverNode* root = cover->getRoot();
    timetogetdist.startClock("getDist");
    dist_to_root    = point->getDist(root->getPoint());
    timetogetdist.endClock("getDist");

    ncallstogetdist = 1;
    if( dist_to_root<mu*radius+r || forcenonempty ) {                                        //MM:TBD: I don't know what forcenonempty is for. Seems to be always set to false
        DescendNode* descendnode = new(descendnode_seglist.getPtr()) DescendNode(root,root->getChildren()->first(),dist_to_root);
        append(descendnode);
    }
}

Cover::DescendList::~DescendList() {
}

void Cover::DescendList::printOn(ostream& os) {
    for(DescendNode* node=first();node;node=next(node)) {
        node->printOn(os);
    }
}

void Cover::DescendList::printOnForFindNearest(ostream& os) {
    DescendNode* node=first();
    for( ;node;node=next(node)) {
        node->getCoverNode()->getPoint()->printOn(cout);
        cout << " dist=" << node->getDist() << endl;
    }
}

void Cover::DescendList::prune() {
    DescendNode* node=first();
    while(node) {
        DescendNode* nextnode=next(node);
        if((node->getDist())>=radius) {
            remove(node);
        }
        node=nextnode;
    }
}

void Cover::DescendList::prune(REAL r) {
    DescendNode* node=first();
    while(node) {
        DescendNode* nextnode=next(node);
        if((node->getDist())>=r) {
            remove(node);
        }
        node=nextnode;
    }
}

CoverNode* Cover::DescendList::descend() {
    REAL muradius       = mu*theta*radius;
    bool parent_found   = false;                                                                                                                                                                // true=parent at level not found
    // append to S_{j+1} from P^{-1}[S_j] and update min_dist
    DescendNode* descendnode            = first();
    DLPtrListNode<CoverNode>* childnode = 0;
    
    while(descendnode) {
        childnode                       = descendnode->getCurrentChild();
        DescendNode* nextdescendnode    = next(descendnode);
        while((childnode) && ((childnode->getPtr()->getLevel())==level+1)) {
            DLPtrListNode<CoverNode>* nextchildnode = childnode->next();
            timetogetdist.startClock("getDist");
            REAL dist                               = point->getDist(childnode->getPtr()->getPoint());
            timetogetdist.endClock("getDist");
            ncallstogetdist++;
            if( dist==0.0 ) //duplicate
                return childnode->getPtr();
            if( dist<muradius ) {
                DescendNode* newdescendnode         = new(descendnode_seglist.getPtr()) DescendNode(childnode->getPtr(),childnode->getPtr()->getChildren()->first(),dist);
                prepend(newdescendnode);
            }
            childnode=nextchildnode;
        }
        descendnode->setCurrentChild(childnode);
        REAL dist = descendnode->getDist();
        if(dist==0.0) {
            return descendnode->getCoverNode();
        }
        if (!parent_found && (dist<radius)){                                                                                                                                                    // found a possible parent
            parent=descendnode->getCoverNode();
            R_level=level;
            parent_found=true;                                                                                                                                                                  // don't look for parents anymore at this level
        }
        if(dist>=muradius) {
            remove(descendnode);
        }
        descendnode=nextdescendnode;
    }
    
    level++;
    radius*=theta;
    
    return 0;
    
}



bool Cover::checkParents(Points*,const REAL* radii,ostream& os) const {
    bool val=true;
    int maxlevel=minlevel+numlevels-1;
    for(DLPtrListNode<CoverNode>* node=first();node;node=next(node)) {
        CoverNode* covernode=node->getPtr();
        const Point* point=covernode->getPoint();
        int l=covernode->getLevel();
        if(l<=maxlevel) {
            CoverNode* parent=covernode->getParent();
            if(covernode!=root) {
                if(parent==0) {
                    os << "no parent ";
                    point->printOn(os);
                    val=false;
                } else {
                    int pl=parent->getLevel();
                    if(l<pl) {
                        os << "level=" << l
                        << " should be less than " << "parent level=" << pl << endl;
                        val=false;
                    }
                    const Point* parentpoint=parent->getPoint();
                    REAL dist=point->getDist(parentpoint);
                    if(dist>=radii[l-1-minlevel]) {
                        os << "dist=" << dist << " from child to parent should be < "
                        << radii[l-1-minlevel] << endl;
                        os << "\t";
                        point->printOn(os);
                        os << "\t";
                        parent->printOn(os);
                    }
                }
            }
        }
    }
    
    return val;
}

bool Cover::checkChildren(Points* points,const REAL* radii,ostream& os) const {
    int maxlevel=minlevel+numlevels-1;
    bool val=true;
    for(DLPtrListNode<CoverNode>* node=first();node;node=next(node)) {
        CoverNode* covernode=node->getPtr();
        int l=covernode->getLevel();
        if(l<=maxlevel) {
            const Point* p=covernode->getPoint();
            DLPtrList<CoverNode>* children=covernode->getChildren();
            for(DLPtrListNode<CoverNode>* childnode=children->first();childnode;
                childnode=children->next(childnode)) {
                int cl=childnode->getPtr()->getLevel();
                if(!(cl>l)) {
                    os << "cl=" << cl << " should be > " << "l=" << l << endl;
                    val=false;
                }
                DLPtrListNode<CoverNode>* nextchildnode=children->next(childnode);
                if(nextchildnode) {
                    int nextcl=nextchildnode->getPtr()->getLevel();
                    if(nextcl<cl) {
                        val=false;
                        os << "nextcl " << cl << " < cl " << l << endl;
                    }
                }
                const Point* c=childnode->getPtr()->getPoint();
                REAL dist=p->getDist(c);
                if((cl<=maxlevel) && !(dist<radii[cl-minlevel-1])) {
                    val=false;
                    os << "dist " << dist << " from child to parent is >= "
                    << radii[cl-minlevel-1] << endl;
                    cout << "\t";
                    c->printOn(os);
                    cout << "\t";
                    p->printOn(os);
                }
            }
        }
    }
    
    return val;
}

bool Cover::checkDistances(Points* points,const REAL* radii,ostream& os) const {
    bool val=true;
    int maxlevel=minlevel+numlevels-1;
    
    DLPtrList<CoverNode> list((SegList<DLPtrListNode<CoverNode> >*)&dlptrlistnode_covernode_seglist);
    
    for(DLPtrListNode<CoverNode>* node=first();node;node=next(node)) {
        CoverNode* covernode=node->getPtr();
        int level=covernode->getLevel();
        if(level<=maxlevel) {
            list.append(covernode);
        }
    }
    
    for(DLPtrListNode<CoverNode>* node=list.first();node;
        node=list.next(node)) {
        //cout << "i=" << i++ << endl;
        for(DLPtrListNode<CoverNode>* othernode=list.next(node);othernode;
            othernode=list.next(othernode)) {
            int level=node->getPtr()->getLevel();
            int otherlevel=othernode->getPtr()->getLevel();
            int ml= (otherlevel>level) ? otherlevel : level;
            if(ml<=maxlevel) {
                const Point* point=node->getPtr()->getPoint();
                const Point* otherpoint=othernode->getPtr()->getPoint();
                REAL dist=point->getDist(otherpoint);
                REAL radius=radii[ml-minlevel];
                if(dist<radius) {
                    val=false;
                    os << "\t";
                    os << endl << points->getIndex(point) << " level=" << level<< "\t";
                    point->printOn(os);
                    os << endl;
                    os << "\t";
                    os << endl << points->getIndex(otherpoint) << " level=" << otherlevel<< "\t";
                    otherpoint->printOn(os);
                    os << endl;
                    
                    os << "dist=" << dist << " should be >=" << radius << endl;
                }
            }
        }
    }
    return val;
}

//void Cover::check(Points* points,ostream& os) const {
//    REAL radii[numlevels];
//    radii[0]=maxradius;
//    for(int i=1;i<numlevels;i++) {
//        radii[i]=radii[i-1]*theta;
//    }
//
//    bool val=checkDistances(points,radii,os);
//    if(val) {
//        os << " checked distances passed" << endl;
//    } else {
//        os << " checked distances failed" << endl;
//    }
//
//    val=checkParents(points,radii,os);
//    if(val) {
//        os << " checked parents passed" << endl;
//    } else {
//        os << " checked parents failed" << endl;
//    }
//
//    val=checkChildren(points,radii,os);
//    if(val) {
//        os << " checked children passed" << endl;
//    } else {
//        os << " checked children failed" << endl;
//    }
//}


void Cover::setCounts() {
    number_inserted=0;
    number_deep=0;
    number_duplicates=0;
    
    int maxlevel=minlevel+numlevels-1;
    
    for( DLPtrListNode<CoverNode>* node=first(); node; node=next(node) ) {
        CoverNode* covernode    = node->getPtr();
        int level               = covernode->getLevel();
        if( level<=maxlevel ) {
            number_inserted++;
        } else if( level<INT_MAX ) {
            number_deep++;
        } else if( level==INT_MAX ) {
            number_duplicates++;
        }
    }
}

int Cover::getMaxLevelPresent() const {
    int l=minlevel;
    for(DLPtrListNode<CoverNode>* node=first();node;node=next(node)) {
        int L=node->getPtr()->getLevel();
        if(L>l && L<INT_MAX)
            l=L;
    }
    return l;
}

void Cover::printCounts(ostream& os) const {
    os << "minlevel=" << minlevel << " numlevels=" << numlevels
    << " maxlevel=" << maxlevel << endl;
    
    os << "number_inserted=" << number_inserted << endl;
    os << "number_deep=" << number_deep << endl;
    os << "number_duplicates=" << number_duplicates << endl;
    
    os << "number_inserted+number_deep+number_duplicates=" <<
    number_inserted+number_deep+number_duplicates << endl;
    os << "count=" << getCount() << endl;
}

void Cover::printOn(ostream& os) const {
    os << "printing cover count=" << getCount() << endl;
    //printDiagnostics(os);
    for(DLPtrListNode<CoverNode>* node=first();node;node=next(node)) {
        CoverNode* covernode=node->getPtr();
        covernode->printOn(os);
    }
}

CoverIndices::CoverIndices(const Cover* cover,const Points* points,int* base) :                             // MM:TBD: need to copy points to this->points??????
sz(sizeof(REAL)),
mytheta(theta),
numlevels(cover->getNumLevels()),
minlevel(cover->getMinLevel()),
count(cover->getCount()),
levels(base),
parents(0),
numchildren(0),
childoffsets(0),
children(0) {
    
    if(base==0) {
        levels=new int[5*count];
        may_delete=true;
    } else {
        may_delete=false;
    }
    
    parents=levels+count;
    numchildren=parents+count;
    childoffsets=numchildren+count;
    children=childoffsets+count;
    
    //    int i=0;
    int childoffset=0;
    
    for(DLPtrListNode<CoverNode>* node=cover->first();node;node=
        cover->next(node)) {
        CoverNode* covernode=node->getPtr();
        const Point* point=covernode->getPoint();
        unsigned int index=points->getIndex(point);
        levels[index]=covernode->getLevel();
        if(covernode==cover->getRoot()) {
            parents[index]=-1;
        } else {
            CoverNode* parent=covernode->getParent();
            if(parent) {
                parents[index]=points->getIndex(parent->getPoint());
            }
        }
        DLPtrList<CoverNode>* childlist=covernode->getChildren();
        numchildren[index]=childlist->getCount();
    }
    
    int N=cover->getCount();
    childoffset=0;
    for(int i=0;i<N;i++) {
        childoffsets[i]=childoffset;
        childoffset+=numchildren[i];
    }
    
    for(DLPtrListNode<CoverNode>* node=cover->first();node;node=
        cover->next(node)) {
        CoverNode* covernode=node->getPtr();
        const Point* point=covernode->getPoint();
        int index=points->getIndex(point);
        DLPtrList<CoverNode>* childlist=covernode->getChildren();
        int childoffset=childoffsets[index];
        for(DLPtrListNode<CoverNode>* childnode=childlist->first();childnode;childnode=childlist->next(childnode)) {
            children[childoffset++]=points->getIndex(childnode->getPtr()->getPoint());
        }
    }
}

CoverIndices::CoverIndices(REAL rtheta,int inumlevels,int iminlevel, int icount, int* base) :
sz(sizeof(double)),
mytheta(rtheta),
numlevels(inumlevels),
minlevel(iminlevel),
count(icount),
levels(base),
parents(0),
numchildren(0),
childoffsets(0),
children(0) {
    
    assert(base!=0);
    assert(sz==sizeof(REAL));
    may_delete=false;
    
    parents=levels+count;
    numchildren=parents+count;
    childoffsets=numchildren+count;
    children=childoffsets+count;
}



CoverIndices::~CoverIndices() {
    if(may_delete) {
        delete [] levels;
    }
}

void CoverIndices::write(const char* ofname,REAL theta,const Points* points)
const {
    //will write to file which when read will reconstruct this
    ofstream ofs(ofname,ios::out | ios::binary);
    //writing sizeof(REAL)
    size_t sz=sizeof(REAL);
    ofs.write((char*)&sz,sizeof(size_t));
    ofs.write((char*)&theta,sizeof(REAL));
    ofs.write((char*)&numlevels,sizeof(int));
    ofs.write((char*)&minlevel,sizeof(int));
    int N=getCount();
    ofs.write((char*)&N,sizeof(int));
    //writing CoverIndices
    const int* levels=getBase();
    ofs.write((char*)levels,5*N*sizeof(int));
    ofs.close();
}




CoverIndices::CoverIndices(const char* ifname) {
    //reading sizeof(REAL)
    ifstream ifs(ifname,ios::in | ios::binary);
    ifs.read((char*)&sz,sizeof(size_t));
    if(sz!=sizeof(REAL)) {
        assert(0);
    }
    
    ifs.read((char*)&mytheta,sz);
    ifs.read((char*)&numlevels,sizeof(int));
    ifs.read((char*)&minlevel,sizeof(int));
    ifs.read((char*)&count,sizeof(int));
    
    may_delete=true;
    levels=new int[5*count];
    
    parents=levels+count;
    numchildren=parents+count;
    childoffsets=numchildren+count;
    children=childoffsets+count;
    //reading CoverIndices integer data
    ifs.read((char*)levels,5*count*sizeof(int));
    ifs.close();
}

void CoverIndices::printOn(ostream& os) const {
    os << "CoverIndices::printOn" << endl;
    os << "sz=" << sz << endl;
    os << "mytheta=" << mytheta << endl;
    os << "numlevels=" << numlevels << endl;
    os << "count=" << count << endl;
    os << endl;
    
    os << "levels" << endl;
    for(int i=0;i<count;i++) {
        os << "\tlevels[" << i << "]=" << levels[i] << endl;
    }
    
    os << "parents" << endl;
    for(int i=0;i<count;i++) {
        os << "\tparents[" << i << "]=" << parents[i] << endl;
    }
    
    os << "numchildren" << endl;
    for(int i=0;i<count;i++) {
        os << "\tnumchildren[" << i << "]=" << numchildren[i] << endl;
    }
    
    os << "childoffsets" << endl;
    for(int i=0;i<count;i++) {
        os << "\tchildoffsets[" << i << "]=" << childoffsets[i] << endl;
    }
    
    os << "children" << endl;
    for(int i=0;i<count;i++) {
        os << "\tchildren[" << i << "]=" << children[i] << endl;
    }
}

Cover::Cover(Points& points,SegList<DLPtrListNode<CoverNode> >& seglist,const CoverIndices& coverindices) :
DLPtrList<CoverNode>(&seglist),
root(0),
//theta(coverindices.getTheta()),
numlevels(coverindices.getNumLevels()),
minlevel(coverindices.getMinLevel()),
//mu(1.0/(1.0-coverindices.getTheta())),
number_inserted(0),
number_deep(0),
covernode_seglist(1024),
dlptrlistnode_covernode_seglist(1024)
//ncallstogetdist(0)
{
    
    ::setMaxRadius(minlevel,maxradius);
    maxlevel=minlevel+numlevels-1;
    int N=coverindices.getCount();
    CoverNode** covernodes=new CoverNode*[N];
    
    for(int i=0;i<N;i++) {
        const Point* point=points.getPoint(i);
        int level=coverindices.getLevel(i);
        CoverNode* covernode=newCoverNode(point,level,0);
        append(covernode);
        covernodes[i]=covernode;
    }
    
    for(int index=0;index<N;index++) {
        int level=coverindices.getLevel(index);
        CoverNode* covernode=covernodes[index];
        if(level==minlevel) {
            root=covernode;
        }
        int parentindex=coverindices.getParent(index);
        if(parentindex!=-1) {
            CoverNode* parent=covernodes[coverindices.getParent(index)];
            covernodes[index]->setParent(parent);
        }
        DLPtrList<CoverNode>* childlist=covernodes[index]->getChildren();
        int numchildren=coverindices.getNumChildren(index);
        for(int j=0;j<numchildren;j++) {
            childlist->append(covernodes[coverindices.getChild(index,j)]);
        }
    }
    
    delete [] covernodes;
}

const REAL* Cover::getRadii() const {
    REAL* radii=new REAL[numlevels];
    radii[0]=maxradius;
    for(int i=1;i<numlevels;i++) {
        radii[i]=radii[i-1]*theta;
    }
    return radii;
}

void Cover::printLevelCounts(ostream& os) {
    //    int deepcount=0;
    int levelcounts[numlevels] ;
    for(int i=0;i<numlevels;i++) {
        levelcounts[i]=0;
    }
    int maxlevel=minlevel+numlevels-1;
    for(DLPtrListNode<CoverNode>* node=first();node;node=next(node)) {
        CoverNode* covernode=node->getPtr();
        int level=covernode->getLevel();
        if(level<=maxlevel) {
            levelcounts[level-minlevel]++;
        }
    }
    
    for(int i=0;i<numlevels;i++) {
        os << "levelcounts[" << i << "]=" << levelcounts[i] << endl;
    }
    
    os << "number_inserted=" << number_inserted
    << " number_deep=" << number_deep
    << " number_duplicates=" << number_duplicates
    << endl;
    
    os << "number_inserted+number_deep+number_duplicates="
    << number_inserted+number_deep+number_duplicates
    << endl;
    
    cout << "Cover::getCount()=" << getCount() << endl;
}


void Cover::adjustRootAndMinLevelAndMaxRadius(REAL dist) {
    ::setMaxRadiusAndMinLevel(dist,theta,maxradius,minlevel);
    root->setLevel(minlevel);
    maxlevel=minlevel+numlevels-1;
}

DescendNode* Cover::DescendList::newDescendNode(CoverNode* covernode, DLPtrListNode<CoverNode>* current_child, REAL dist) {
    return new(descendnode_seglist.getPtr()) DescendNode(covernode,current_child,dist);
}


int Cover::setPoints(Points& points,ThreadsWithCounter* threads,EnlargeData* enlargedata) {
    const Point* point  = 0;
    int counter         = 0;
    vector<PointLevelParent>& vplp  = enlargedata->getPointLevelParent();
    int BLOCKSIZE                   = enlargedata->getBlockSize();
    
    for(int u=0;u<BLOCKSIZE;u++) {
        point=points.next();
        if(point) counter++;
        vplp[u].setPoint(point);
    }
    return counter;
}


void setMaxRadius(int minlevel,REAL& maxradius) {
    maxradius=1.0;
    if(minlevel>0) {
        for(int l=0;l<minlevel;l++) {
            maxradius*=theta;
        }
    } else if(minlevel<0) {
        for(int l=0;l<-minlevel;l++) {
            maxradius/=theta;
        }
    }
}

void setMaxRadiusAndMinLevel(REAL dist,REAL theta,REAL& maxradius,
                             int& minlevel) {
    int templevel=0;
    REAL tempradius=1.0;
    
    if(dist>=1.0) {
        while(dist>=tempradius) {
            tempradius/=theta;
            templevel--;
        }
    } else {
        while(tempradius*theta>dist) {
            tempradius*=theta;
            templevel++;
        }
    }
    
    maxradius=tempradius;
    minlevel=templevel;
}


void set(REAL dist,REAL theta,int& level,REAL& radius) {
    level=0;
    radius=1.0;
    if(dist<radius) {
        while(dist<radius) {
            radius*=theta;
            level++;
        }
    } else {
        while(dist>radius) {
            level--;
            radius/=theta;
        }
    }
}


double Cover::DescendList::getTimeToGetDist()  {
    return timetogetdist.getTime("getDist");
}


#include "FastSeg.C"
template class SegList<DescendNode>;

#include "IDLList.C"
template class IDLList<DescendNode>;

#include "DLPtrList.C"
template class DLPtrListNode<CoverNode>;
template class DLPtrList<CoverNode>;
