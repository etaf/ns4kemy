#ifndef KEMY_QUEUE_HH
#define KEMY_QUEUE_HH

#include <queue>
#include "packet.hh"
#include "whiskertree.hh"
#include "memory.hh"
class KemyQueue
{
    private:
        WhiskerTree&  _whiskers;
        Memory _memory;
        std::queue< Packet > _queue;
        unsigned int _the_window;
        bool _track;

    public:
        KemyQueue( WhiskerTree &  s_whiskers,const bool s_track=false) :
        _whiskers(s_whiskers),
        _memory(),
        _queue(),
        _the_window(0),
        _track(s_track)
        {};

        void  enque(Packet p,const double& tickno);
        void  deque();
        bool  empty(){return _queue.empty(); } 
        Packet front(){return _queue.front();}
};
#endif
