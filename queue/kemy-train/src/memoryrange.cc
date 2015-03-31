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
      std::vector< MemoryRange > doubled;
    for ( const auto &x : ret ) {
      auto ersatz_lower( x._lower ), ersatz_upper( x._upper );
      //ersatz_lower.mutable_field( i ) = ersatz_upper.mutable_field( i ) = median( &(_arrs[ i ]) );
      ersatz_lower.mutable_field( i ) = ersatz_upper.mutable_field( i ) = _medians[i];

      if ( x._lower == ersatz_upper ) {
	/* try range midpoint instead */
	ersatz_lower.mutable_field( i ) = ersatz_upper.mutable_field( i ) = (x._lower.field( i ) + x._upper.field( i )) / 2;
      }

      if ( x._lower == ersatz_upper ) {
	assert( !(ersatz_lower == x._upper) );
	assert( x._lower == ersatz_lower );
	/* cannot double on this axis */
	doubled.push_back( x );
    printf("cannot split domain:%s\nmedians:",str().c_str());
    for(size_t kk = 0; kk < Memory::datasize; ++kk){
        printf("%f ",_medians[kk]);
    }
    printf("\n");
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
  char tmp[ 1024 ];
  snprintf( tmp, 1024, "(\nlo=<%s>\nhi=<%s>\n),count=%d,medians=%fx%fx%f",
	    _lower.str().c_str(),
	    _upper.str().c_str() ,_count,_medians[0],_medians[1],_medians[2]);
  return tmp;
}

KemyBuffers::MemoryRange MemoryRange::DNA( void ) const
{
  KemyBuffers::MemoryRange ret;

  ret.mutable_lower()->CopyFrom( _lower.DNA() );
  ret.mutable_upper()->CopyFrom( _upper.DNA() );
  ret.set_count(_count);
  for(unsigned int i =0; i< Memory::datasize; ++i){
      if(_arrs[i].size() > 0){
          _medians[i] = median( &(_arrs[ i ]) );
      }
      else{
          //if(_count > 0) printf("not logged!!!\n");
          _medians[i] = 0;
      }
  }

  for(auto x : _medians){
    ret.add_medians(x);
  }
  //printf("setting count=%d\n",_count);
  //printf("setting arrs=%lu x %lu\n",_arrs.size(),_arrs[0].size());
  return ret;

}

MemoryRange::MemoryRange( const KemyBuffers::MemoryRange & dna )
  : _lower( true, dna.lower() ),
    _upper( false, dna.upper() ),
    _arrs(Memory::datasize),
    _count( dna.count() ),
    _medians(0)
{
    _medians.clear();
    for( const auto &m : dna.medians()){
        _medians.emplace_back(m);
    }
    if(_medians.size() !=  Memory::datasize){
        _medians.resize(Memory::datasize);
    }
    /*for(auto x : _medians){*/
        //printf("%f ",x);
    /*}*/
    //printf("\n");
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
        assert(_medians.size() == Memory::datasize && other._medians.size() == Memory::datasize);
        for(size_t i =0; i<other._medians.size();++i)
        {
            _medians[i] = 0.5 * (_medians[i]  + other._medians[i]);
        }
    }
}
