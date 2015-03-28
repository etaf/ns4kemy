#include <cassert>
#include <cmath>
#include <algorithm>
#include <boost/functional/hash.hpp>

#include "whisker.hh"

using namespace std;

vector< Whisker > Whisker::bisect( void ) const
{
  vector< Whisker > ret;
  for ( auto &x : _domain.bisect() ) {
    Whisker new_whisker( *this );
    new_whisker._domain = x;
    ret.push_back( new_whisker );
  }
  return ret;
}

Whisker::Whisker( const int s_drop_f, const MemoryRange & s_domain )
  : _generation( 0 ),
    _drop_f( s_drop_f ),
    _domain( s_domain ),
    _improved(false)
{
}

Whisker::Whisker( const Whisker & other )
  : _generation( other._generation ),
    _drop_f( other._drop_f ),
    _domain( other._domain ),
    _improved(other._improved)
{
}

template < typename T >
bool Whisker::OptimizationSetting< T >::eligible_value( const T & value ) const
{
  return value >= min_value and value <= max_value;
}

template < typename T >
vector< T > Whisker::OptimizationSetting< T >::alternatives( const T & value ) const
{
  assert( eligible_value( value ) );

  vector< T > ret( 1, value );

  for ( T proposed_change = min_change;
	proposed_change <= max_change;
	proposed_change *= multiplier ) {
    /* explore positive change */
    const T proposed_value_up = value + proposed_change;
    const T proposed_value_down = value - proposed_change;

    if ( eligible_value( proposed_value_up ) ) {
      ret.push_back( proposed_value_up );
    }

    if ( eligible_value( proposed_value_down ) ) {
      ret.push_back( proposed_value_down );
    }
  }

  return ret;
}

vector< Whisker > Whisker::next_generation( void ) const
{
  vector< Whisker > ret;

  Whisker new_whisker {*this};
  new_whisker._generation++;
  new_whisker._drop_f ^= 1;
  ret.push_back(new_whisker);
  return ret;
  
/*  for ( const auto & alt_window : get_optimizer().window_increment.alternatives( _window_increment ) ) {*/
    //for ( const auto & alt_multiple : get_optimizer().window_multiple.alternatives( _window_multiple ) ) {
	//Whisker new_whisker { *this };
	//new_whisker._generation++;

	//new_whisker._window_increment = alt_window;
	//new_whisker._window_multiple = alt_multiple;

	//new_whisker.round();

	//ret.push_back( new_whisker );
    //}
  //}

  /*return ret;*/
}

void Whisker::promote( const unsigned int generation )
{
  _generation = max( _generation, generation );
}

string Whisker::str( const unsigned int total ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "{%s} gen=%u usage=%.4f => (drop_f=%d)",
	    _domain.str().c_str(), _generation, double( _domain.count() ) / double( total ), _drop_f );
  return tmp;
}

KemyBuffers::Whisker Whisker::DNA( void ) const
{
  KemyBuffers::Whisker ret;
  ret.set_generation( _generation );
  ret.set_drop_f( _drop_f );
  ret.mutable_domain()->CopyFrom( _domain.DNA() );

  return ret;
}

Whisker::Whisker( const KemyBuffers::Whisker & dna )
  : _generation( dna.generation() ),
    _drop_f( dna.drop_f() ),
    _domain( dna.domain() ),
    _improved(false)
{
}

void Whisker::round( void )
{
  //_window_multiple = (1.0/10000.0) * int( 10000 * _window_multiple );
}

size_t hash_value( const Whisker & whisker )
{
  size_t seed = 0;
  //boost::hash_combine( seed, whisker._generation);
  boost::hash_combine( seed, whisker._drop_f );
  boost::hash_combine( seed, whisker._domain );

  return seed;
}

bool Whisker::combine_other(const Whisker& other, bool trace){
    if(!(_domain == other._domain)){
        fprintf(stderr,"whisker domain error:\n%s\n%s\n",_domain.str().c_str(), other._domain.str().c_str());
        return false;
    }
    _domain.combine_other(other._domain, trace);
    return true;
}
