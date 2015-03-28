#include <boost/functional/hash.hpp>
#include<algorithm>
#include "memoryrange.hh"
#include<iostream>
//using namespace boost::accumulators;

double MemoryRange::median(std::vector<double> * vs) const{
    int n = vs->size()/2;
    std::nth_element(vs->begin(),vs->begin()+n,vs->end());
    return vs->at(n);
}
std::vector< MemoryRange > MemoryRange::bisect( void ) const
{
    std::vector< MemoryRange > ret { *this };

  /* bisect in each axis */
  for ( unsigned int i = 0; i < Memory::datasize; i++ ) {
      assert(_arrs[i].size() );
      std::vector< MemoryRange > doubled;
    for ( const auto &x : ret ) {
      auto ersatz_lower( x._lower ), ersatz_upper( x._upper );
      ersatz_lower.mutable_field( i ) = ersatz_upper.mutable_field( i ) = median( &(_arrs[ i ]) );

      if ( x._lower == ersatz_upper ) {
	/* try range midpoint instead */
	ersatz_lower.mutable_field( i ) = ersatz_upper.mutable_field( i ) = (x._lower.field( i ) + x._upper.field( i )) / 2;
      }

      if ( x._lower == ersatz_upper ) {
	assert( !(ersatz_lower == x._upper) );
	assert( x._lower == ersatz_lower );
	/* cannot double on this axis */
	doubled.push_back( x );
      } else {
	doubled.emplace_back( x._lower, ersatz_upper );
	doubled.emplace_back( ersatz_lower, x._upper );
      }
    }

    ret = doubled;
  }

  assert( !ret.empty());

  return ret;
}

Memory MemoryRange::range_median( void ) const
{
  Memory median_data( _lower );
  for ( unsigned int i = 0; i < Memory::datasize; i++ ) {
    median_data.mutable_field( i ) = (_lower.field( i ) + _upper.field( i )) / 2;
  }
  return median_data;
}

bool MemoryRange::contains( const Memory & query ) const
{
  return (query >= _lower) && (query < _upper);
}

void MemoryRange::track( const Memory & query ) const
{
  /* log it */
  for ( unsigned int i = 0; i < Memory::datasize; i++ ) {
    _arrs[ i ].push_back( query.field( i ) );
  }
}

bool MemoryRange::operator==( const MemoryRange & other ) const
{
  return (_lower == other._lower) && (_upper == other._upper); /* ignore median estimator for now */
}

std::string MemoryRange::str( void ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "(lo=<%s>, hi=<%s>),count=%d,arrs=%ldx%ld\n",
	    _lower.str().c_str(),
	    _upper.str().c_str() ,_count,_arrs.size(),_arrs.size() > 0 ? _arrs[0].size():0);
  return tmp;
}

KemyBuffers::MemoryRange MemoryRange::DNA( void ) const
{
  KemyBuffers::MemoryRange ret;

  ret.mutable_lower()->CopyFrom( _lower.DNA() );
  ret.mutable_upper()->CopyFrom( _upper.DNA() );
  ret.set_count(_count);
  for(auto vec : _arrs){
      auto arr = ret.add_arrs();
      for(auto x : vec){
        arr->add_element(x);
      }
  }
  //printf("setting count=%d\n",_count);
  //printf("setting arrs=%lu x %lu\n",_arrs.size(),_arrs[0].size());
  return ret;

}

MemoryRange::MemoryRange( const KemyBuffers::MemoryRange & dna )
  : _lower( true, dna.lower() ),
    _upper( false, dna.upper() ),
    _arrs(Memory::datasize),
    _count( dna.count() )
{
    int i=0;
    for(const auto &arr : dna.arrs() )
    {
        std::vector<double> tmp;
        for(const auto & x : arr.element()){
            tmp.emplace_back(x);
        }
        _arrs[i] = tmp;
        ++i;
    }
    //printf("reading count=%d\n",_count);
  //printf("reading arrs=%lu x %lu\n",_arrs.size(),_arrs[0].size());
}

size_t hash_value( const MemoryRange & mr )
{
  size_t seed = 0;
  boost::hash_combine( seed, mr._lower );
  boost::hash_combine( seed, mr._upper );
  //boost::hash_combine( seed, mr._arrs  );
  //boost::hash_combine( seed, mr._count );
  return seed;
}

void MemoryRange::combine_other(const MemoryRange& other, bool trace){
    _count+=other._count;
    //printf("other_count=%d\n",other._count);
    //printf("combining %ldx%ld\n",other._arrs.size(),other._arrs.size()>0?other._arrs[0].size():0);
    if(trace){
        for(size_t i=0;i< other._arrs.size();++i){
            for(size_t j=0; j < other._arrs[i].size();++j){
                _arrs[i].emplace_back(other._arrs[i][j]);
            }
        }
    }
}
