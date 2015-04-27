#include "kemy-queue.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include<iostream>
static class KemyQueueClass : public TclClass{
    public:
        KemyQueueClass() : TclClass("Queue/KEMY"){}
        TclObject* create(int, const char*const*){
            return (new KemyQueue);
        }
}class_kemy;
KemyQueue::KemyQueue() :
        _whiskers(NULL),
        _memory(),
        _drop_f(false),
        _trace4split(false),
        _last_arrival(0),
        _last_depart(0),
        _min_arrive_interval(0),
        _min_depart_interval(0),
        _ewma_qlen(0),
        _ewma_arrival_rate(0),
        _ewma_depart_rate(0),
        qib_(true),
        _mean_pktsize(1000),
        _pkg_acc(0),
        _K(0.69)
        {
            bind("mean_pktsize_", &_mean_pktsize);  // int: mean_pktsize
            bind_bool("queue_in_bytes_", &qib_);  // boolean: q in bytes?
            const char * filename = getenv("WHISKERS");
        	if ( !filename ) {
        		fprintf( stderr, "KemyQueue: Missing WHISKERS environment variable.\n" );
        		throw 1;
        	}

        	/* open file */
        	int fd = open( filename, O_RDONLY );
        	if ( fd < 0 ) {
        		perror( "open" );
        		throw 1;
        	}

        	/* parse whisker definition */
        	KemyBuffers::WhiskerTree tree;
        	if ( !tree.ParseFromFileDescriptor( fd ) ) {
        		fprintf( stderr, "KemyQueue: Could not parse whiskers in \"%s\".\n", filename );
        		throw 1;
        	}

        	/* close file */
        	if ( ::close( fd ) < 0 ) {
        		perror( "close" );
        		throw 1;
        	}

        	/* store whiskers */
        	_whiskers = new WhiskerTree( tree );

	        q_ = new PacketQueue();			    // inner queue
            pq_ = q_;
            reset();
        }

KemyQueue::~KemyQueue(){
    delete q_;
}
void KemyQueue::reset(){
    Queue::reset();
    _ewma_arrival_rate = 0;
    _ewma_qlen = 0;
    _ewma_depart_rate = 0;
    _last_depart = 0;
    _last_arrival = 0;
    _min_arrive_interval = 0;
    _min_depart_interval = 0;
    _pkg_acc = 0;
}

//update states when enque
void KemyQueue::update_enque(Packet* p ){
	double qlen = qib_ ? q_->byteLength() : q_->length();
    hdr_cmn* hdr  = hdr_cmn::access(p);
    uint32_t packet_size  = qib_ ?  hdr->size() : 1; // in bytes
    double now = Scheduler::instance().clock();

    double lamda = 0.5;
    qlen /= (qib_?1000:1);
    _ewma_qlen = ( 1-lamda) * qlen + lamda * _ewma_qlen;
    _memory.update_ewma_qlen(_ewma_qlen);

    double interval = (now - _last_arrival)*1000; //arrive interval in ms
    if(interval<1e-9){
        //arrive at the same time
        _pkg_acc += packet_size;
        //fprintf(stderr,"did not update:");
        //cout<<interval<<endl;
        return;
    }else{
        packet_size += _pkg_acc;
        _pkg_acc = 0;
    }
    //update _min_arrive_interval
    if( _min_arrive_interval == 0 ){
        _min_arrive_interval = interval;
    }else{
        _min_arrive_interval = std::min(interval,_min_arrive_interval);
    }
    //update _last_arrival
    _last_arrival = now;
    //update _ewma_qlen
    //double lamda = exp(- interval/_min_arrive_interval * _K);

    /*std::cout<<"packt_size:"<<packet_size<<"\t interval:"<<interval<<std::endl;*/
    //update_ewma_arrival_rate
    _ewma_arrival_rate = (1-lamda) * packet_size / interval  + lamda * _ewma_arrival_rate;
    _memory.update_ewma_arrival_rate(_ewma_arrival_rate);

    //if(interval<1)std::cout<<"packet_size:"<<packet_size<<"\tinterval:"<<interval<<"\tlamda:"<<lamda<<"\t_ewma_arrival_rate:"<<_ewma_arrival_rate<<std::endl;

}
void KemyQueue::update_deque(Packet* p){
    hdr_cmn* hdr  = hdr_cmn::access(p);
    uint32_t packet_size  = qib_ ?  hdr->size() : 1;
    double now = Scheduler::instance().clock();

    double interval = (now - _last_depart)*1000;

    //update _min_depart_interval
    if( _min_depart_interval == 0){
        _min_depart_interval = interval;
    }else{
        _min_depart_interval = std::min(interval, _min_depart_interval);
    }
    //update _last_depart
    _last_depart = now;

    //update _ewma_depart_rate
    //double lamda = exp( - interval/_min_depart_interval * _K);
    double lamda = 0.5;
    _ewma_depart_rate = (1-lamda) * packet_size / interval  + lamda * _ewma_depart_rate;
    _memory.update_ewma_depart_rate(_ewma_depart_rate);

    //if(interval<1)std::cout<<"packet_size:"<<packet_size<<"\tinterval:"<<interval<<"\tlamda:"<<lamda<<"\t_ewma_depart_rate:"<<_ewma_depart_rate<<std::endl;

}
void KemyQueue::enque(Packet* p)
{
   	uint32_t qlim = qib_ ? (qlim_ * _mean_pktsize) : qlim_;
	uint32_t qlen = qib_ ? q_->byteLength() : q_->length();
    //hdr_ip* iph = hdr_ip::access(p);
    //hdr_ip::access(p);
    if (qlen  >= qlim){
        drop(p);
    }
    else{
        const Whisker & current_whisker(_whiskers->use_whisker(_memory,_trace4split));
        _drop_f = current_whisker.drop_f();
        if( _drop_f )
        {
            drop(p);
        }
        else
        {
            q_->enque(p);
        }
    }
    update_enque(p);
}
Packet* KemyQueue::deque()
{
    Packet* p = q_->deque();
    if(p != NULL){
        update_deque(p);
    }
    return (p);
}

int KemyQueue::command(int argc, const char*const* argv){
    if (argc == 2) {
        if (strcmp(argv[1], "printstats") == 0) {
            char filename[1024] = "";
            strcpy(filename,getenv("WHISKERS"));
            strcat(filename,".out");
            //printf("writing whiskers to %s...\n",filename);

            int fd = open( filename,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR  );
            if ( fd < 0 ) {
                perror( "open" );
                throw 1;
            }
            if ( not _whiskers->DNA().SerializeToFileDescriptor( fd ) ) {
                fprintf( stderr, "Could not serialize kemyCC.\n" );
                exit( 1 );
            }

            if ( ::close( fd ) < 0 ) {
                perror( "close" );
                throw 1;
            }
            return (TCL_OK);
        }else if(strcmp(argv[1],"trace4split") == 0){
            _trace4split = true;
            return (TCL_OK);
        }
    }
    return (Queue::command(argc, argv));
}
