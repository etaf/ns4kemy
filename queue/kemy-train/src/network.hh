#ifndef NETWORK_HH
#define NETWORK_HH

#include <string>
#include "whiskertree.hh"
class NetConfig
{
public:
  unsigned int _num_senders;
  double _bottle_bw;
  double _bottle_single_delay;

  NetConfig( void ):
      _num_senders( 16 ),
      _bottle_bw(15),
      _bottle_single_delay(50)
  {}

  NetConfig & set_num_senders( const unsigned int n ) { _num_senders = n; return *this; }
  NetConfig & set_bottle_bw( const double bottle_bw ) { _bottle_bw = bottle_bw; return *this; }
  NetConfig & set_bottle_single_delay( const double bottle_single_delay ) { _bottle_single_delay = bottle_single_delay ; return *this; }

  std::string str( void ) const
  { char tmp[ 256 ];
    snprintf( tmp, 256, "nsrc=%d, bottle_bw=%f, bottle_single_delay=%f\n Other parameters can be found in tcl configure file\n",
	       _num_senders, _bottle_bw,_bottle_single_delay );
    return tmp;
  }
};

class Network
{
private:
  NetConfig _config;
  double _utility;
public:
  Network(const NetConfig & config ):_config(config),_utility(){}
  void run_simulation(WhiskerTree & _whiskers, bool trace, unsigned int seed_run);
  double utility(){return _utility;}
};

#endif
