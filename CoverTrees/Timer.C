#include "Timer.H"

Timer::Timer() : start(0), is_on(false), time(0.0) { }
void Timer::on() {
  gettimeofday(&tim,0); 
  start=tim.tv_sec+(tim.tv_usec/1000000.0);
}

void Timer::off() {
  gettimeofday(&tim,0); 
  end=tim.tv_sec+(tim.tv_usec/1000000.0);
  time=end-start;
}

void Timer::printOn(ostream& os) {
  os << "time=" << time << endl;
}
