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
public:
  Memory( const std::vector< DataType > & s_data )
    : _ewma_qlen( s_data.at( 0 ) )
  {}

  Memory()
    :_ewma_qlen( 0 )
  {}

  //void reset( void ) { _rec_send_ewma = _rec_rec_ewma = _rtt_ratio = _slow_rec_rec_ewma = _last_tick_sent = _last_tick_received = _min_rtt = 0; }
  void reset( void ) { _ewma_qlen  = 0; }

  static const unsigned int datasize = 1;

  const DataType & field( unsigned int num ) const {
        if(num == 0)
            return _ewma_qlen;
        else{
            fprintf(stderr,"wrong num of field");
            exit(1);
        }
  }
  DataType & mutable_field( unsigned int num )   {
        if(num == 0)
            return _ewma_qlen;
        else{
            fprintf(stderr,"wrong num of field");
            exit(1);
        }
  }
/*  void packet_sent( const Packet & packet __attribute((unused)) ) {}*/
  //void packets_received( const std::vector< Packet > & packets, const unsigned int flow_id );
  /*void advance_to( const unsigned int tickno __attribute((unused)) ) {}*/

  void update_ewma_qlen(double ewma_qlen){ _ewma_qlen = ewma_qlen;};

  std::string str( void ) const;

  bool operator>=( const Memory & other ) const { return (_ewma_qlen >= other._ewma_qlen) ; }
  bool operator<( const Memory & other ) const { return (_ewma_qlen <  other._ewma_qlen) ; }
  bool operator==( const Memory & other ) const { return (_ewma_qlen ==  other._ewma_qlen) ; }

  KemyBuffers::Memory DNA( void ) const;
  Memory( const bool is_lower_limit, const KemyBuffers::Memory & dna );

  friend size_t hash_value( const Memory & mem );
};

extern const Memory & MAX_MEMORY( void );

#endif
