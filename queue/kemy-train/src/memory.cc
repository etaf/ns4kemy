#include <boost/functional/hash.hpp>
#include <vector>
#include <cassert>

#include "memory.hh"

using namespace std;

static const double alpha = 0.5;

/*void Memory::update(const double& tickno __attribute__((unused)),const unsigned int & qlen)*/
//{
    ////update _interval
    //_interval = now - _last_arrival;

    ////update _min_interval
    //_min_interval = std::min(_interval,_min_interval);

    ////update _last_arrival
    //_last_arrival = now;

    ////update _ewma_qlen
    //double lamda = exp(- _interval/_min_interval * 0.69);
    //_ewma_qlen = ( 1-lamda) * qlen + lamda * _ewma_qlen;

    ////update_ewma_arrival_rate
    //_ewma_arrival_rate = (1-lamda) * arriva_rate + lamda * _ewma_arrival_rate;

    ////update_ewma_depart_rate
    

//}

string Memory::str( void ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "ewma_qlen=%f, _ewma_arrival_rate=%f, _ewma_depart_rate=%f", _ewma_qlen,_ewma_arrival_rate,_ewma_depart_rate);
  return tmp;
}

const Memory & MAX_MEMORY( void )
{
  static const Memory max_memory( { 163840, 1638400, 1638400 } );
  return max_memory;
}

KemyBuffers::Memory Memory::DNA( void ) const
{
  KemyBuffers::Memory ret;
  ret.set_ewma_qlen( _ewma_qlen );
  ret.set_ewma_arrival_rate( _ewma_arrival_rate);
  ret.set_ewma_depart_rate( _ewma_depart_rate);
  return ret;
}

/* If fields are missing in the DNA, we want to wildcard the resulting rule to match anything */
#define get_val_or_default( protobuf, field, limit ) \
  ( (protobuf).has_ ## field() ? (protobuf).field() : (limit) ? 0 : 163840 )

Memory::Memory( const bool is_lower_limit, const KemyBuffers::Memory & dna )
  : _ewma_qlen( get_val_or_default( dna, ewma_qlen, is_lower_limit ) ),
    _ewma_arrival_rate( get_val_or_default( dna, ewma_arrival_rate, is_lower_limit)),
    _ewma_depart_rate( get_val_or_default( dna, ewma_depart_rate, is_lower_limit))
{
}

size_t hash_value( const Memory & mem )
{
  size_t seed = 0;
  boost::hash_combine( seed, mem._ewma_qlen );
  boost::hash_combine( seed, mem._ewma_arrival_rate);
  boost::hash_combine( seed, mem._ewma_depart_rate);
  return seed;
}
