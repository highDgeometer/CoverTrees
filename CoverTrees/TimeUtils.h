
#ifndef __TimeUtils__
#define __TimeUtils__

#include <iostream>
#include <list>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

using namespace std;

typedef struct {
    string      Tag;
    double      sec;
    uint64_t    clockstart;
} TimeToken;

class TimeList {
    friend ostream &operator<<( ostream &out, TimeList &timeList );
private:
    list<TimeToken> times;
public:
    void    startClock      ( string Tag );                                             // Restart a clock with given tag
    double  endClock        ( string Tag );                                             // Stops a clock with given tag, and adds elapsed time since the last startClock to the corresponding <sec> field
    
    list<TimeToken>::iterator *FindTimeToken( string Tag );                             // Finds TimeToken in list
    double  getTime         ( string Tag );
    bool    addTimeToken    ( TimeToken *timeToken);                                                                                                                                            // Adds a TimeToken to the TimeList, if it already exists it adds the times
    void    Merge           ( TimeList *timeList );                                     // Merge with other TimeList, adding PreTag to the name of the clocks
    
    
    ~TimeList();
};

ostream &operator<<( ostream &out, TimeList &timeList );



double subtractTimes( uint64_t endTime, uint64_t startTime );
double addTimes( uint64_t Time1, uint64_t Time2 );

#endif /* defined(__TimeUtils__) */
