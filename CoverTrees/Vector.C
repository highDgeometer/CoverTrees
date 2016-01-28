#include <assert.h>
#include "Vector.H"
#include "MemoryDebugger.H"


#define THRES_DISTANCE  1e-6

TimeList globalTimeList;

//#define DEBUG_DISTANCES

//
// Vector class implementation
//

REAL Vector::getDist(const Point* q) const {
#ifdef DEBUG
    const Vector* w=dynamic_cast<const Vector*>(q);
#else
    const Vector* w=(const Vector*)(q);
#endif
    REAL dist = 0.0;
    
    if( parent->dist_ws.distance_mode == EUCLIDEAN)    {                                                                                                // Euclidean distance
        dist = EuclideanDist(x,w->x,parent->dim);
    }
    else {
        cout << "\nUnknown distance function " << parent->dist_ws.distance_mode << endl;
        dist = -1;
    }
    
    return dist;
}



//
// Vectors class implementation
//

Vectors::Vectors(REAL* pX,INDEX iN, INDEX iDim, Distance_Mode dist_mode, bool iPrecompute) :
Points(), N(iN), dim(iDim), current(0), vectors(0), X(pX), may_delete(false), Precompute(iPrecompute) {
    
#ifdef MEM_DEBUG
    vectors=(Vector*)OPERATOR_NEW_BRACKET(char,N*sizeof(Vector));
#else
    vectors=(Vector*)new char[N*sizeof(Vector)];
#endif
    
    for(int i=0;i<N;i++) {
        new((void*)(vectors+i)) Vector(X+i*dim,this);                                       // mem_check();
    }
    
    init_DistanceWorkspace( dist_mode );
}

Vectors::Vectors(const char* filename,int start,INDEX iN, INDEX iDim, Distance_Mode dist_mode) :
Points(), N(iN), dim(iDim), current(0), vectors(0), X(0) {
#ifdef MEM_DEBUG
    X=OPERATOR_NEW_BRACKET(REAL,N);
#else
    X=new REAL[N];
#endif
    
    ifstream ifs;
    ifs.open(filename,ios::binary);
    ifs.seekg(start*dim*sizeof(REAL),ios::beg);
    long int nbytes=N*dim*sizeof(REAL);
    ifs.read((char*)X,nbytes);
    long int count=ifs.gcount();
    cout << "count=" << count << endl;
    if(N*dim*sizeof(REAL)!=count)
        assert(0);
    
#ifdef MEM_DEBUG
    vectors=(Vector*)OPERATOR_NEW_BRACKET(char,N*sizeof(Vector));
#else
    vectors=(Vector*)new char[N*sizeof(Vector)];
#endif
    
    for(int i=0;i<N;i++) {
        new((void*)(vectors+i)) Vector(X+i*dim,this);
    }
    
    init_DistanceWorkspace( dist_mode );
}

void Vector::write(ofstream& ofs) const {
    ofs.write((char*)x,parent->dim*sizeof(REAL));
}

void Vector::printOn(ostream& os) const {
    //os << "i=" << i << " x=(";
    os << "(";
    for(int j=0;j<parent->dim-1;j++) {
        os << x[j] << ",";
    }
    os << x[parent->dim-1] << ")";
}

int Vector::getDim() const  {
    if ( parent )
        return (int)(parent->dim);
    else return 0;
}


Vectors::~Vectors() {
    clear_DistanceWorkspace();
    
    delete [] (char*)vectors;
    
    if(may_delete) {
        delete [] X;
    }
}

const Vector* Vectors::next() {
    if(current==N) {
        return 0;
    } else {
        return vectors+current++;
    }
}


INDEX Vectors::getIndex(const Point* p) const {
    INDEX idx;
#ifdef DEBUG
    const Vector* v=dynamic_cast<const Vector*>(p);
#else
    const Vector* v=static_cast<const Vector*>(p);
#endif
    const REAL* x=v->x;
    
    idx = (INDEX)(x-X)/dim;
    
    if ( x<X || idx>=N )  {   idx = INDEX_MAX;   }
    
    return idx;
}


void Vectors::printOn(ostream& os) const {
    for(int i=0;i<N;i++) {
        vectors[i].printOn(os);
    }
}



bool Vectors::init_DistanceWorkspace( Distance_Mode dist_mode ) {
    
    dist_ws.distance_mode = dist_mode;
    
    switch( dist_ws.distance_mode )    {
        case EUCLIDEAN:                                                                                         // Standard Euclidean distance
            break;
        case EUCLIDEAN_ABS_VALUES:                                                                              // Euclidean modulo change of sign
            break;
        default:
            return false;
    }
    
    return true;
}

void Vectors::clear_DistanceWorkspace( void )  {
    
    switch( dist_ws.distance_mode )    {
        case EUCLIDEAN:                                                                                         // Standard Euclidean distance
            break;
        case EUCLIDEAN_ABS_VALUES:                                                                              // Euclidean modulo change of sign
            break;
        default:
            break;
    }
}

