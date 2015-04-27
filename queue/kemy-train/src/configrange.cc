#include "configrange.hh"

using namespace std;

static KemyBuffers::Range pair_to_range( const pair< double, double > & p )
{
  KemyBuffers::Range ret;
  ret.set_low( p.first );
  ret.set_high( p.second );
  return ret;
}

KemyBuffers::ConfigRange ConfigRange::DNA( void ) const

{
  KemyBuffers::ConfigRange ret;
  ret.mutable_bottle_bw()->CopyFrom( pair_to_range( bottle_bw ) );
  ret.mutable_bottle_single_delay()->CopyFrom( pair_to_range( bottle_single_delay ) );
  ret.mutable_num_senders()->CopyFrom( pair_to_range( make_pair( min_senders, max_senders ) ) );

  //ret.set_mean_off_duration( mean_off_duration );
  //ret.set_mean_on_duration( mean_on_duration );

  return ret;
}
