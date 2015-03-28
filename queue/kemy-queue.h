#ifndef KEMY_QUEUE_HH
#define KEMY_QUEUE_HH
#include "flags.h"
#include "random.h"
//#include "template.h"
#include "queue.h"
#include "kemy-train/src/whiskertree.hh"
#include "address.h"
class KemyQueue:public Queue
{
    public:
        KemyQueue();
        ~KemyQueue();
        void reset();

    protected:
        WhiskerTree*  _whiskers;
        Memory _memory;
        bool _drop_f; //action, drop or not
        //int _the_window;
        bool _trace4split; // if trace the Memory for split

        double _last_arrival;
        double _last_depart;
        double _min_arrive_interval;
        double _min_depart_interval;

        //states
        double _ewma_qlen;
        double _ewma_arrival_rate;
        double _ewma_depart_rate;

	    int qib_;       	/* bool: queue measured in bytes? */
        double _mean_pktsize;
        double _pkg_acc;
        PacketQueue *q_;
        void  enque(Packet* );
        void update_enque(Packet*);
        void update_deque(Packet*);
        Packet*  deque();
	    int command(int argc, const char*const* argv);
    private:
        double _K;  // ewma ajust parameter
};
#endif
