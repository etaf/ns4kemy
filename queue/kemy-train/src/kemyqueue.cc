#include "kemyqueue.hh"

void KemyQueue::enque(Packet p,const double& tickno)
{
    const Whisker & current_whisker(_whiskers.use_whisker(_memory,_track));
     _the_window = current_whisker.window(_the_window);
    if( _queue.size() >= _the_window)
    {
        _memory.packet_drop(tickno,_queue.size());
    }
    else
    {
        _memory.packet_receive(tickno,_queue.size());
        _queue.push(p);
    }
}
void KemyQueue::deque()
{
    if(!_queue.empty())
    {
        _queue.pop();
    }
}
