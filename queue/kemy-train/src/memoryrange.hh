#ifndef MEMORYRANGE_HH
#define MEMORYRANGE_HH

/*#include <boost/accumulators/accumulators.hpp>*/
//#include <boost/accumulators/statistics/stats.hpp>
/*#include <boost/accumulators/statistics/median.hpp>*/
#include <vector>
#include <string>

#include "memory.hh"
#include "../protobufs/dna.pb.h"

class MemoryRange {
private:
  Memory _lower, _upper;

/*  mutable std::vector< boost::accumulators::accumulator_set< Memory::DataType,*/
								 //boost::accumulators::stats<
								   /*boost::accumulators::tag::median > > > _acc;*/
  mutable std::vector< std::vector<double> > _arrs;
  mutable unsigned int _count;
  mutable std::vector< double > _medians;
  double median(  std::vector<double>* )const;
public:
  MemoryRange( const Memory & s_lower, const Memory & s_upper )
    : _lower( s_lower ), _upper( s_upper ), _arrs( Memory::datasize ), _count( 0 ), _medians(Memory::datasize,-10000000)
  {}

  std::vector< MemoryRange > bisect( void ) const;
  Memory range_median( void ) const;

  bool contains( const Memory & query ) const;

  void use( void ) const { _count++;}
  unsigned int count( void ) const { return _count; }
  void reset_count( void ) const { _count = 0; _arrs.clear(); _arrs.resize(Memory::datasize); _medians.clear(); _medians.resize(Memory::datasize,-10000000); }
  void combine_other(const MemoryRange& other, bool trace);
  void track( const Memory & query ) const;

  bool operator==( const MemoryRange & other ) const;

  std::string str( void ) const;

  KemyBuffers::MemoryRange DNA( void ) const;
  MemoryRange( const KemyBuffers::MemoryRange & dna );

  friend size_t hash_value( const MemoryRange & mr );
};

#endif
