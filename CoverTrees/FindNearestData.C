#include "EnlargeData.H"
#include "CoverNode.H"
#include "ThreadsWithCounter.H"
#include "Cover.H"



void FindNearestflattenDescendLists(int totalfound,INDEX* found,int N,int* pi,Points& points,Cover::DescendList* descendlists) {
  //total found it the sum of the descendlists counts
  //found must point to totalfound ints
  //upon return found will have the indices of the points on the descendlists
  //N is the number of descendlists
  //pi must point to 2*N ints

  int* numfound=pi;
  int* offsets=pi+N;
  numfound[0]=descendlists[0].getCount();
  offsets[0]=0;
  int j=0;
  for(DescendNode* node=descendlists[0].first();node;
      node=descendlists[0].next(node),j++){
    const Point* point=node->getCoverNode()->getPoint();
    found[j]=points.getIndex(point);
  }
  for(int i=1;i<N;i++) {
    numfound[i]=descendlists[i].getCount();
    offsets[i]=offsets[i-1]+numfound[i-1];
    int j=0;
    for(DescendNode* node=descendlists[i].first();node;
      node=descendlists[i].next(node),j++){
      const Point* point=node->getCoverNode()->getPoint();
      found[offsets[i]+j]=points.getIndex(point);
    }
  }
}
  
bool FindNearestcheckFlattenDescendList(int totalfound,INDEX* found,int N,int* pi, Points& points, Cover::DescendList* descendlists) {
  bool val=true;
  if(points.getCount()!=N) {
    return false;
  }

  int* counts=pi;
  int* offsets=pi+N;
  for(int i=0;i<N;i++) {
    if(counts[i]!=descendlists[i].getCount() ) {
      cout << "bad count" << endl;
      return false;
    }
    int j=0;
    for(DescendNode* node=descendlists[i].first();node;
      node=descendlists[i].next(node),j++){
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
	  
    
