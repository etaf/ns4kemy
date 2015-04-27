#ifndef CONFIG_RANGE_HH
#define CONFIG_RANGE_HH

#include "dna.pb.h"

class ConfigRange
{
public:
  std::pair< double, double > bottle_bw { 1, 2 };
  std::pair< double, double > bottle_single_delay { 50, 100 };
  unsigned int min_senders { 1 };
  unsigned int max_senders { 16 };

  KemyBuffers::ConfigRange DNA( void ) const;
};

#endif  // CONFIG_RANGE_HH
