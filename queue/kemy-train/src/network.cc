#include "network.hh"
#include<unistd.h>
#include<sstream>
#include<cmath>
#include "whiskertree.hh"
#include<boost/thread/thread.hpp>
#include<string>
#include<sstream>
#include<fcntl.h>
void Network::run_simulation(WhiskerTree & _whiskers, bool trace, unsigned int seed_run)
{
    const unsigned int BUFFSIZE = 256;
    //write whiskers to file
    char whiskers_file[BUFFSIZE];
    char current_dir[BUFFSIZE];

    char* tmp = getcwd(current_dir,sizeof(current_dir));
    if(!tmp){
        perror("error get cwd");
    }
    std::stringstream ss;
    ss<<boost::this_thread::get_id();
    sprintf(whiskers_file,"%s/../jrats/%s",current_dir,ss.str().c_str() );

    int fd = open( whiskers_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
    auto kemycc = _whiskers.DNA();
    if ( not kemycc.SerializeToFileDescriptor( fd ) ) {
        fprintf( stderr, "Could not serialize KemyCC.\n" );
        exit( 1 );
    }
    if ( ::close( fd ) < 0 ) {
        perror( "close" );
        exit( 1 );
    }

    //system call ns run_simulation.tcl
    //
    //change to tcl dir
  /*  char tcl_dir[BUFFSIZE];*/
    //sprintf(tcl_dir,"%s/../tcl",current_dir);
    //if(chdir(tcl_dir) == -1){
        //perror("change  directory to tcl error");
        //exit(1);
    /*}*/
    char buf[1024];
    sprintf(buf,"WHISKERS=%s ./run-simulation.tcl  -nsrc %d -bw %.2f -delay %.2f -run %u",
            whiskers_file,
            _config._num_senders,
            _config._bottle_bw,
            _config._bottle_single_delay,
            seed_run
            );
    if(trace){
        strcat(buf," -trace4split true");
    }
    //puts(buf);
    int ret_code = system(buf);
    if( ret_code !=0 ) {
        fprintf(stderr,"error when call:\n%s\n",buf);
        fprintf(stderr,"system return code:%d\n",ret_code);
        fprintf(stderr,"whiskers_file:%s\n whiskertree:%s\n",whiskers_file, _whiskers.str().c_str());
        exit(1);
    }

    //read whiskers tree
    sprintf(buf,"%s.out",whiskers_file);
    fd = open( buf, O_RDONLY );
    if ( fd < 0 ) {
        perror( "open" );
        exit( 1 );
    }

    KemyBuffers::WhiskerTree tree;
    if ( !tree.ParseFromFileDescriptor( fd ) ) {
        fprintf( stderr, "Could not parse %s.\n", buf );
        exit( 1 );
    }

    _whiskers = WhiskerTree( tree );
    if ( close( fd ) < 0 ) {
        perror( "close" );
        exit( 1 );
    }

    // read utility
    sprintf(buf,"%s.utility",whiskers_file);
    FILE* fp = fopen(buf,"r");
    double tp=0,del=0;
    int cnt = 0;
    while(fgets(buf,sizeof(buf),fp)){
        std::stringstream ss(buf);
        std::string tmp;
        ss>>tmp;
        if(tmp != "tp="){
            perror("read utility error");
            exit(1);
        }
        ss>>tmp;
        tp += stod(tmp);
        ss>>tmp;
        ss>>tmp;
        if(tmp!="del="){
            perror("read utility error");
        }
        ss>>tmp;
        del += stod(tmp);
        ++cnt;
    }
    if(cnt){
        _utility = log2(tp) - log2(del);
    }
    fclose(fp);

/*    if(chdir(current_dir) == -1){*/
        //perror("change  directory to tcl error");
        //exit(1);
    /*}*/

//rm the tmp file
    sprintf(buf,"rm %s*",whiskers_file);
    if( system(buf)!=0 ) {
        perror("error when call rm");
        exit(1);
    }

}
