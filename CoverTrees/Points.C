#include "Points.H"

/*
void Points::printOn(ostream& os) const {
  const Point* point=pop();
  while(point) {
    point->printOn(os);
    point=pop();
  }
}
*/

/*
  Points::Points(int NN, int* f, int* m, REAL* d) : N(NN), found(f), marks(m),
  marked(0), dist(d) {}

void Points::resetForInsertion() {
 for(int j=0;j<marked;j++) {
    found[marks[j]]=0;
  }
  marked=0;
}

REAL Points::getDistForInsertion(int i, int j) {
 if(found[j]) {
    return dist[j];
  }
 //dist_ctr++;
  REAL d=getDist(i,j);
  found[j]=1;
  dist[j]=d;
  marks[marked++]=j;
  return d;
}  

*/
