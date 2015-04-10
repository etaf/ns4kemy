#ifndef MEMORY_HH
#define MEMORY_HH

#include <vector>
#include <string>

//#include "packet.hh"
#include "../protobufs/dna.pb.h"

class Memory {
public:
  typedef double DataType;

private:
  double _ewma_qlen;
  double _ewma_arrival_rate;
  double _ewma_depart_rate;
public:
  Memory( const std::vector< DataType > & s_data )
    : _ewma_qlen( s_data.at( 0 ) ),
      _ewma_arrival_rate( s_data.at(1)),
      _ewma_depart_rate( s_data.at(2))
  {}

  Memory()
    :_ewma_qlen( 0 ),
    _ewma_arrival_rate( 0 ),
    _ewma_depart_rate( 0 )
  {}

  //void reset( void ) { _rec_send_ewma = _rec_rec_ewma = _rtt_ratio = _slow_rec_rec_ewma = _last_tick_sent = _last_tick_received = _min_rtt = 0; }
  void reset( void ) { _ewma_qlen  = 0; _ewma_arrival_rate = 0; _ewma_depart_rate = 0;}

  static const unsigned int datasize = 3;

  const DataType & field( unsigned int num ) const {
      switch(num){
          case 0:
              return _ewma_qlen;
          case 1:
              return _ewma_arrival_rate;
          case 2:
              return _ewma_depart_rate;
      }
      return _ewma_qlen;
  }
  DataType & mutable_field( unsigned int num )   {
      switch(num){
          case 0:
              return _ewma_qlen;
          case 1:
              return _ewma_arrival_rate;
          case 2:
              return _ewma_depart_rate;
      }
      return _ewma_qlen;
  }
/*  void packet_sent( const Packet & packet __attribute((unused)) ) {}*/
  //void packets_received( const std::vector< Packet > & packets, const unsigned int flow_id );
  /*void advance_to( const unsigned int tickno __attribute((unused)) ) {}*/

  void update_ewma_qlen(double ewma_qlen){ _ewma_qlen = ewma_qlen;};
  void update_ewma_arrival_rate(double ewma_arrival_rate){ _ewma_arrival_rate = ewma_arrival_rate;}
  void update_ewma_depart_rate(double ewma_depart_rate){ _ewma_depart_rate = ewma_depart_rate;}

  std::string str( void ) const;

  bool operator>=( const Memory & other ) const { return (_ewma_qlen >= other._ewma_qlen) && (_ewma_arrival_rate >= other._ewma_arrival_rate) && (_ewma_depart_rate >= other._ewma_depart_rate); }
  bool operator<( const Memory & other ) const { return (_ewma_qlen <  other._ewma_qlen) && (_ewma_arrival_rate < other._ewma_arrival_rate) && (_ewma_depart_rate < other._ewma_depart_rate); }
  bool operator==( const Memory & other ) const { return (_ewma_qlen ==  other._ewma_qlen) && (_ewma_arrival_rate == other._ewma_arrival_rate) && (_ewma_depart_rate == other._ewma_depart_rate); }

  KemyBuffers::Memory DNA( void ) const;
  Memory( const bool is_lower_limit, const KemyBuffers::Memory & dna );

  friend size_t hash_value( const Memory & mem );
};

extern const Memory & MAX_MEMORY( void );

#endif
