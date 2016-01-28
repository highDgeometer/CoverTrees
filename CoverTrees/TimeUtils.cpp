//
//  TimeUtils.cpp
//
// Currently for OS X
//

#include "TimeUtils.h"
#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

using namespace std;


TimeList::~TimeList() {
    // times is destroyed automatically and so are the objects in the list
}

void TimeList::startClock( string Tag )  {
    list<TimeToken>::iterator *list_iter = FindTimeToken( Tag );
    
    if( list_iter==NULL )    {
        TimeToken newTimeToken;
#ifdef __APPLE__
        newTimeToken.clockstart = mach_absolute_time();
#else
        newTimeToken.clockstart = 0;
#endif
        newTimeToken.Tag.assign( Tag );
        newTimeToken.sec = 0.0;
        
        times.push_back(newTimeToken);
    } else {
#ifdef __APPLE__
        (*list_iter)->clockstart = mach_absolute_time();
#else
        (*list_iter)->clockstart = 0;
#endif
        free( list_iter );
    }
}


double TimeList::endClock( string Tag ) {
#ifdef __APPLE__
    uint64_t clockend = mach_absolute_time();
#else
    uint64_t clockend = 0;
#endif

    for(list<TimeToken>::iterator list_iter = times.begin(); list_iter != times.end(); list_iter++) {
        if( Tag.compare( list_iter->Tag )==0 ) {
            list_iter->sec = list_iter->sec + subtractTimes( clockend, list_iter->clockstart);
            return list_iter->sec;
        }
    }
    
    return 0.0;
}


// Finds a TimeToken by tag
list<TimeToken>::iterator *TimeList::FindTimeToken( string Tag ) {
    list<TimeToken>::iterator *list_iter = (list<TimeToken>::iterator *)malloc( sizeof(list<TimeToken>::iterator) );
    
    for(*list_iter = times.begin(); *list_iter != times.end(); (*list_iter)++)
        if( Tag.compare( (*list_iter)->Tag )==0 )
            return list_iter;
    
    free( list_iter );
    
    return NULL;
}


bool TimeList::addTimeToken( TimeToken *timeToken)  {                                                                                                                                           // Adds a TimeToken to the TimeList, if it already exists it adds the times
    list<TimeToken>::iterator *list_iter = FindTimeToken( timeToken->Tag );
    bool found = (list_iter!=nullptr);
    
    if( !found )    {
        TimeToken newTimeToken = *timeToken;
        times.push_back(newTimeToken);
    }  else {
        (*list_iter)->sec += timeToken->sec;
        free( list_iter );
    }
    
    return found;
}

void TimeList::Merge( TimeList *timeList )  {                                                                                                                                                   // Merge with other TimeList, adding new clocks that did not exist in this TimeList, and for existing clocks adding the time from timeList
    list<TimeToken>::iterator list_iter, *list_iter_lookup;
    TimeToken *newTimeToken;
    
    for(list_iter = times.begin(); list_iter != times.end(); list_iter++) {
        list_iter_lookup = FindTimeToken( list_iter->Tag );
        if ( list_iter_lookup==nullptr ) {
            newTimeToken    = (TimeToken*)malloc(sizeof(TimeToken));
            *newTimeToken   = *list_iter;
            addTimeToken(newTimeToken);
        } else {
            list_iter->sec += (*list_iter_lookup)->sec;
            free( list_iter_lookup );
        }
    }
}

double TimeList::getTime( string Tag )  {
    double sec = 0.0;
    list<TimeToken>::iterator *list_iter = FindTimeToken( Tag );
    
    if ( list_iter!=nullptr) {
        sec = (*list_iter)->sec;
        free( list_iter );
    }
    return sec;
}



ostream &operator<<( ostream &out, TimeList &timeList ) {
    for(list<TimeToken>::iterator list_iter = timeList.times.begin(); list_iter != timeList.times.end(); list_iter++) {
        cout << list_iter->Tag << ":" << list_iter->sec << "\n";
    }
    
    return out;
}


// Raw mach_absolute_times going in, difference in seconds out
double subtractTimes( uint64_t endTime, uint64_t startTime )
{
#ifdef __APPLE__
    uint64_t difference = endTime - startTime;
    static double conversion = 0.0;
    
    if( conversion == 0.0 )
    {
        mach_timebase_info_data_t info;
        kern_return_t err = mach_timebase_info( &info );
        //Convert the timebase into seconds
        if( err == 0  )
            conversion = 1e-9 * (double) info.numer / (double) info.denom;
    }
    
    return conversion * (double) difference;
#else
    return 0;
#endif
}


// Raw mach_absolute_times going in, difference in seconds out
double addTimes( uint64_t Time1, uint64_t Time2 )
{
#ifdef __APPLE__
    uint64_t sum = Time1 + Time2;
    static double conversion = 0.0;
    
    if( conversion == 0.0 )
    {
        mach_timebase_info_data_t info;
        kern_return_t err = mach_timebase_info( &info );
        //Convert the timebase into seconds
        if( err == 0  )
            conversion = 1e-9 * (double) info.numer / (double) info.denom;
    }
    
    return conversion * (double) sum;
#else
    return 0;
#endif
}
