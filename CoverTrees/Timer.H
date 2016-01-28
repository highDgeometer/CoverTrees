#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
#include <iostream>

using namespace std;

class Timer {
private:
  struct timeval tim;
  double start;  
  double end;
  bool is_on;
  double time;
public:
  Timer();
  ~Timer() {}
  void on();
  void off();
  void printOn(ostream& = cout);
};

#endif
