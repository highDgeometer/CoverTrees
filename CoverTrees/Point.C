#include "Points.H"

/*
void IndexedPoint::printOn(ostream& os) const {
  os << "index=" << index << "";
    if(point) {
      point->printOn(os); 
    } else {
      os << "point=" << point << endl;
    }
}
*/

/*
void Points::setMinLevelAndRadius(REAL theta,int& level,REAL& maxradius) {
  const Point* root=pop();
  const Point* point=pop();
  if(point==0) {
    level=INT_MAX;
  }

  REAL dist=0.0;
  while(point) {
    REAL new_dist=root->getDist(point);    
    if(new_dist>dist) {
      dist=new_dist;
    }
    point=pop();
  }
  
  reset();
  
  if(dist<maxradius) {
    while(dist<maxradius) {
      maxradius*=theta;
      level++;
    }
  } else {
    while(dist>maxradius) {
      level--;
      maxradius/=theta;
    }
  }
}
*/
