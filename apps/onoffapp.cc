#define ETAF_DEBUG etaf_debug
#include <cmath>
#include "exception.hh"
#include "ezio.hh"
#include "onoffapp.hh"

using namespace std;

OnOffApp::OnOffApp(string str_ontype,
                   uint32_t t_id,
                   uint32_t t_pkt_size,
                   uint32_t t_hdr_size,
                   uint32_t t_run,
                   double   on_average,
                   double   off_time_avg,
                   TcpAgent* t_tcp_handle)
    : ontype_(str_ontype == "bytes" ? BYTE_BASED :
              str_ontype == "time"  ? TIME_BASED :
              str_ontype == "flowcdf" ? EMPIRICAL:
              throw Exception("OnOffApp constructor", "Invalid string for ontype")),
      sender_id_(t_id),
      pkt_size_(t_pkt_size),
      hdr_size_(t_hdr_size),
      run_(t_run),
      start_distribution_(off_time_avg, run_),
      stop_distribution_(on_average, run_),
      emp_stop_distribution_(run_),
      tcp_handle_(t_tcp_handle),
      on_timer_(this),
      off_timer_(this)
{
  on_timer_.sched(std::max(0.1,start_distribution_.sample()));
  //fprintf(stderr,"seed_run=%u\n",run_);
}

void OnOffApp::turn_on() {
#ifdef ETAF_DEBUG
  fprintf(stderr, "%d, %f Turning on\n", sender_id_, Scheduler::instance().clock());
#endif
  if (ontype_ == BYTE_BASED) {
    current_flow_.flow_size = lround(ceil(stop_distribution_.sample()));
    //current_flow_.flow_size = std::max(current_flow_.flow_size,(uint32_t) 5);
  } else if (ontype_ == TIME_BASED) {
    //current_flow_.on_duration = stop_distribution_.sample();
    current_flow_.on_duration = std::max(1.0,stop_distribution_.sample());
  } else if (ontype_ == EMPIRICAL) {
    current_flow_.flow_size = lround(ceil(emp_stop_distribution_.sample()));
  }

  laststart_ = Scheduler::instance().clock();
  state_ = ON;

  if (ontype_ == BYTE_BASED or ontype_ == EMPIRICAL) {
    //assert(current_flow_.flow_size > 0);

    /* TODO: Handle the Vegas kludge somehow */
    tcp_handle_->advanceby(current_flow_.flow_size);

  } else if (ontype_ == TIME_BASED) {
    tcp_handle_->send(-1);
    assert(off_timer_.status() == TIMER_IDLE);
    off_timer_.sched(current_flow_.on_duration);
  }
}

void OnOffApp::recv_ack(Packet* ack) {
  /* Measure RTT and other statistics */
  stat_collector_.add_sample(ack);
}

void OnOffApp::resume(void) {
  /* If ontype is BYTE_BASED or EMPIRICAL, turn off */
  if (ontype_ == BYTE_BASED or ontype_ == EMPIRICAL) {
    //printf("%d sender is turning off\n",sender_id_);
    //turn_off();
    off_timer_.sched(0.1);
  }
}

void OnOffApp::turn_off(void) {
  if (ontype_ == TIME_BASED) {
     assert(Scheduler::instance().clock() == (laststart_ + current_flow_.on_duration));
     tcp_handle_->advanceto(0); /* Src quench kludge */
  }

  state_ = OFF;
  total_on_time_ += (Scheduler::instance().clock() - laststart_ - 0.1);

  //double off_duration = start_distribution_.sample();
  double off_duration = std::max(0.1,start_distribution_.sample());
#ifdef ETAF_DEBUG
  fprintf(stderr, "%d, %f Turning off,time used:%f\n next turning on at %f\n", sender_id_, Scheduler::instance().clock()-laststart_ -0.1 ,
          Scheduler::instance().clock(),
                  Scheduler::instance().clock() + off_duration);
#endif 
  /* Either on_timer_ is unscheduled (TIMER_IDLE) */
  /* Or we got here from on_timer_'s callback, start_send. This can happen only if pkts are sent out all at once */
/*  assert(on_timer_.status() == TIMER_IDLE or*/
        /*(on_timer_.status() == TimerHandler::TimerStatus::TIMER_HANDLING and Scheduler::instance().clock() == laststart_));*/
  if(on_timer_.status() != TIMER_IDLE){
      perror("on_timer_ is not idle before resched!");
  }
  on_timer_.sched(off_duration);
}

static class OnOffClass : public TclClass {
 public:
  OnOffClass() : TclClass("Application/OnOff") {}
  TclObject* create(int argc, const char*const* argv) {
    try {
      if (argc != 12) {
        throw Exception("Application/OnOff mirror constructor", "too few args");
      } else {
        return new OnOffApp(string(argv[4]),
                            myatoi(argv[5]),
                            myatoi(argv[6]),
                            myatoi(argv[7]),
                            myatoi(argv[8]),
                            myatod(argv[9]),
                            myatod(argv[10]),
                            reinterpret_cast<TcpAgent *>(TclObject::lookup(argv[11])));
      }
    } catch (const Exception & e) {
      e.perror();
      exit(EXIT_FAILURE);
    }
  }
} class_on_off_app;

int OnOffApp::command(int argc, const char*const* argv) {
  if (strcmp(argv[1], "stats") == 0) {
    assert(++calls_ < 2);
    total_on_time_ += (state_ == OFF) ? 0 : (Scheduler::instance().clock() - laststart_);

    char buf[1024];
    if(argc > 2){
        strcpy(buf,argv[2]);
    }
    else{
        sprintf(buf,"%s.utility",getenv("WHISKERS"));
    }
    FILE* fp = fopen(buf,"a");
    if( fp == NULL ){
        fprintf(stderr, "%s open error",buf);
    }
    stat_collector_.output_stats(fp, total_on_time_, sender_id_, pkt_size_ + hdr_size_);
    fclose(fp);
    return TCL_OK;
  }
  return Application::command(argc, argv);
}

void AppOnTimer::expire(Event* e) {
  app_->turn_on();
}

void AppOffTimer::expire(Event* e) {
  app_->turn_off();
}
