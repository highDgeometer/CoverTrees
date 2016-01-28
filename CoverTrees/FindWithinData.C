#include <vector>
#include "EnlargeData.H"
#include "CoverNode.H"
#include "ThreadsWithCounter.H"
#include "Cover.H"

#include <algorithm>

void FindWithinflattenDescendLists(int totalfound,int* indices,REAL* distances,int N,int* pi,Points& points,Cover::DescendList* descendlists) {
    //total found it the sum of the descendlists counts
    //found must point to totalfound ints
    //upon return found will have the indices of the points on the descendlists
    //N is the number of descendlists, one for each query point
    //pi must point to 2*N ints
    
    vector<DescendNodePtr> vect(totalfound);
    
    int* numfound=pi;
    int* offsets=pi+N;
    numfound[0]=descendlists[0].getCount();
    offsets[0]=0;
    int j=0;
    for(DescendNode* node=descendlists[0].first();node;node=descendlists[0].next(node),j++){
        new(&vect[j]) DescendNodePtr(node);
    }
    sort(&vect[0],&vect[0]+j);
    for(int k=0;k<j;k++) {
        DescendNode* dnode=vect[k].getPtr();
        indices[k]=points.getIndex(dnode->getCoverNode()->getPoint());
        distances[k]=dnode->getDist();
    }
    for(int i=1;i<N;i++) {
        numfound[i]=descendlists[i].getCount();
        offsets[i]=offsets[i-1]+numfound[i-1];
        int j=0;
        for(DescendNode* node=descendlists[i].first();node;node=descendlists[i].next(node),j++){
            new(&vect[j]) DescendNodePtr(node);
        }
        sort(&vect[0],&vect[0]+j);
        for(int k=0;k<j;k++) {
            DescendNode* dnode=vect[k].getPtr();
            indices[offsets[i]+k]=points.getIndex(dnode->getCoverNode()->getPoint());
            distances[offsets[i]+k]=dnode->getDist();
        }
    }
}

bool FindWithincheckFlattenDescendList(int totalfound,int* found,int N,int* pi,Points& points, Cover::DescendList* descendlists) {
    bool val=true;
    if(points.getCount()!=N) {
        return false;
    }
    
    int* counts=pi;
    int* offsets=pi+N;
    int j=0;    
    for(int i=0;i<N;i++) {
        if(counts[i]!=descendlists[i].getCount() ) {
            cout << "bad count" << endl;
            return false;
        }
        for(DescendNode* node=descendlists[i].first();node;node=descendlists[i].next(node),j++){
            int index=found[offsets[i]+j];
            const Point* p=points.getPoint(index);
            const Point* q=node->getCoverNode()->getPoint();
            if(p!=q) {
                cout << "i=" << i << " ";
                p->printOn(cout);
                cout << " j=" << j << " ";
                q->printOn(cout);
                cout << endl;
                val=false;
            }
        }
    }
    
    return val;
}


