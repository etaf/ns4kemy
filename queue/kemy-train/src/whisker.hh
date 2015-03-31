#ifndef WHISKER_HH
#define WHISKER_HH

#include <string>
#include <vector>

#include "memoryrange.hh"
#include "../protobufs/dna.pb.h"

class Whisker {
private:
  unsigned int _generation;

    int _drop_f;
    MemoryRange _domain;
    //bool _improved;
public:
  Whisker( const Whisker & other );
  Whisker( const int s_drop_f, const MemoryRange & s_domain );

  Whisker( const MemoryRange & s_domain ) : Whisker( get_optimizer().drop_f.default_value,
						     s_domain ) {}

  void use( void ) const { _domain.use(); }
  void reset_count( void ) const { _domain.reset_count(); }
  unsigned int count( void ) const { return _domain.count(); }

  bool combine_other(const Whisker& other, bool trace);
  const unsigned int & generation( void ) const { return _generation; }
  bool drop_f()const{return _drop_f;}

  //void set_improved(bool improved){ _improved = improved;}
  //bool improved()const{return _improved;}

  const MemoryRange & domain( void ) const { return _domain; }

  std::vector< Whisker > next_generation( void ) const;

  void promote( const unsigned int generation );

  std::string str( const unsigned int total=1 ) const;

  std::vector< Whisker > bisect( void ) const;

  void demote( const unsigned int generation ) { _generation = generation; }

  KemyBuffers::Whisker DNA( void ) const;
  Whisker( const KemyBuffers::Whisker & dna );

  void round( void );

  bool operator==( const Whisker & other ) const { return (_drop_f == other._drop_f)  && (_domain == other._domain); }

  friend size_t hash_value( const Whisker & whisker );

  template < typename T >
  struct OptimizationSetting
  {
    T min_value; /* the smallest the value can be */
    T max_value; /* the biggest */

    T min_change; /* the smallest change to the value in an optimization exploration step */
    T max_change; /* the biggest change */

    T multiplier; /* we will explore multiples of the min_change until we hit the max_change */
    /* the multiplier defines which multiple (e.g. 1, 2, 4, 8... or 1, 3, 9, 27... ) */

    T default_value;

    std::vector< T > alternatives( const T & value ) const;
    bool eligible_value( const T & value ) const;

    KemyBuffers::OptimizationSetting DNA( void ) const
    {
      KemyBuffers::OptimizationSetting ret;

      ret.set_min_value( min_value );
      ret.set_max_value( max_value );
      ret.set_min_change( min_change );
      ret.set_max_change( max_change );
      ret.set_multiplier( multiplier );
      ret.set_default_value( default_value );

      return ret;
    }
  };

  struct OptimizationSettings
  {
    OptimizationSetting< int > drop_f;

    KemyBuffers::OptimizationSettings DNA( void ) const
    {
      KemyBuffers::OptimizationSettings ret;

      ret.mutable_drop_f()->CopyFrom( drop_f.DNA() );

      return ret;
    }
  };

  static const OptimizationSettings & get_optimizer( void ) {
    static OptimizationSettings default_settings {
      { 0,  1, -1,1, 1, 0 }, /* drop_f */
    };
    return default_settings;
  }
};

#endif
