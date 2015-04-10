#ifndef CONFIG_RANGE_HH
#define CONFIG_RANGE_HH

#include "dna.pb.h"

class ConfigRange
{
public:
  std::pair< double, double > bottle_bw { 1, 2 };
  std::pair< double, double > bottle_single_delay { 100, 200 };
  unsigned int min_senders { 5 };
  unsigned int max_senders { 32 };
  bool lo_only { false };

  KemyBuffers::ConfigRange DNA( void ) const;
};

#endif  // CONFIG_RANGE_HH
