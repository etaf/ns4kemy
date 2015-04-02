#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ratbreeder.hh"

using namespace std;

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  string output_filename;

  for ( int i = 1; i < argc; ++i ) {
      string arg( argv[ i ] );
      if ( arg.substr( 0, 3 ) == "if=" ) {
          string filename( arg.substr( 3 ) );
          int fd = open( filename.c_str(), O_RDONLY );
          if ( fd < 0 ) {
              perror( "open" );
              exit( 1 );
          }

          KemyBuffers::WhiskerTree tree;
          if ( !tree.ParseFromFileDescriptor( fd ) ) {
              fprintf( stderr, "Could not parse %s.\n", filename.c_str() );
              exit( 1 );
          }
          whiskers = WhiskerTree( tree );
          if ( close( fd ) < 0 ) {
              perror( "close" );
              exit( 1 );
          }
      } else if ( arg.substr( 0, 3 ) == "of=" ) {
          output_filename = string( arg.substr( 3 ) );
      }
  }

  ConfigRange configuration_range;
  configuration_range.bottle_bw = make_pair( 10, 20 ); // 10 Mbps to 20 Mbps
  configuration_range.bottle_single_delay = make_pair( 100, 200 ); // ms
  configuration_range.max_senders = 50;

  //configuration_range.mean_on_duration = 5000;
  //configuration_range.mean_off_duration = 5000;

  //  configuration_range.lo_only = true;

  RatBreeder breeder( configuration_range );

  unsigned int run = 0;

  printf( "#######################\n" );
  printf( "Optimizing for link packets_per_ms in [%f, %f]\n",
	  configuration_range.bottle_bw.first,
	  configuration_range.bottle_bw.second );
  printf( "Optimizing for rtt_ms in [%f, %f]\n",
	  configuration_range.bottle_single_delay.first,
	  configuration_range.bottle_single_delay.second );
  printf( "Optimizing for num_senders = 1-%d\n",
	  configuration_range.max_senders );

  printf( "Initial rules (use if=FILENAME to read from disk): %s\n", whiskers.str().c_str() );
  printf( "#######################\n" );

  if ( !output_filename.empty() ) {
    printf( "Writing to \"%s.N\".\n", output_filename.c_str() );
  } else {
    printf( "Not saving output. Use the of=FILENAME argument to save the results.\n" );
  }

  //prepare the results fold
  if(system("rm ../jrats -rf && mkdir ../jrats && rm core* -f") !=0 ){
      perror("rm ../jrats error");
      return 1;
  }

  while ( 1 ) {
    auto outcome = breeder.improve( whiskers );
    printf( "run = %u, score = %f\n", run, outcome.score );

    printf( "whiskers: %s\n", whiskers.str().c_str() );

    if ( !output_filename.empty() ) {
      char of[ 128 ];
      snprintf( of, 128, "%s.%d", output_filename.c_str(), run );
      fprintf( stderr, "Writing to \"%s\"... ", of );
      int fd = open( of, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
      if ( fd < 0 ) {
	perror( "open" );
	exit( 1 );
      }

      auto kemycc = whiskers.DNA();
      kemycc.mutable_config()->CopyFrom( configuration_range.DNA() );
      kemycc.mutable_optimizer()->CopyFrom( Whisker::get_optimizer().DNA() );

      if ( not kemycc.SerializeToFileDescriptor( fd ) ) {
	fprintf( stderr, "Could not serialize kemyCC.\n" );
	exit( 1 );
      }

      if ( close( fd ) < 0 ) {
	perror( "close" );
	exit( 1 );
      }

      fprintf( stderr, "done.\n" );
    }

    fflush( NULL );
    run++;
  }

  return 0;
}
