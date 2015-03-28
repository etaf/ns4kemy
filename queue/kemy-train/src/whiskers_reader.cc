#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "whiskertree.hh"
#include<iostream>

void usage(){
    printf("Usage: whiskers-reader whiskers_file\n");
    return;
}
int main( int argc, char *argv[] )
{
  if(argc < 2){
      usage();
      exit(1);
  }

  int fd = open( argv[1], O_RDONLY );
  if ( fd < 0 ) {
      perror( "open" );
      exit( 1 );
  }
  KemyBuffers::WhiskerTree tree;
  if ( !tree.ParseFromFileDescriptor( fd ) ) {
      fprintf( stderr, "Could not parse %s.\n", argv[1] );
      exit( 1 );
  }
  WhiskerTree whiskers(tree);
  std::cout<<whiskers.str()<<std::endl;
  if ( close( fd ) < 0 ) {
      perror( "close" );
      exit( 1 );
  }


  return 0;
}
