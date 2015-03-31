#include "network.hh"
#include<unistd.h>
#include<sstream>
#include<cmath>
#include "whiskertree.hh"
#include<string>
#include<fcntl.h>
using namespace std;

int main(int argc, char** args)
{
    if(argc != 2){
        perror("args error");
        return 1;
    }
    const unsigned int BUFFSIZE = 1024;
    //write whiskers to file
    char whiskers_file[BUFFSIZE];
    char current_dir[BUFFSIZE];

    char* tmp = getcwd(current_dir,sizeof(current_dir));
    if(!tmp){
        perror("error get cwd");
    }
    std::stringstream ss;
    sprintf(whiskers_file,"%s/%s", current_dir, args[1]);

    int fd = open( whiskers_file, O_RDONLY );
    if ( fd < 0 ) {
        perror( "open" );
        exit( 1 );
    }

    KemyBuffers::WhiskerTree tree;
    if ( !tree.ParseFromFileDescriptor( fd ) ) {
        fprintf( stderr, "Could not parse %s.\n", whiskers_file);
        exit( 1 );
    }
    if ( close( fd ) < 0 ) {
        perror( "close" );
        exit( 1 );
    }
    auto _whiskers = WhiskerTree( tree );

    _whiskers.reset_counts();

    fd = open( whiskers_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
    if ( not _whiskers.DNA().SerializeToFileDescriptor( fd ) ) {
	fprintf( stderr, "Could not serialize kemyCC.\n" );
	exit( 1 );
      }
      if ( close( fd ) < 0 ) {
	perror( "close" );
	exit( 1 );
      }

    //printf("whiskers:\n%s\n",_whiskers.str().c_str());

    printf("================================================================\n");
    char buf[1024];
    sprintf(buf,"WHISKERS=%s ./run-simulation.tcl  -nsrc 32 -bw 10 -delay 100  -qtr ./debug/out.qtr -qmon ./debug/out.qmon -trace4split true",
            whiskers_file);
    //puts(buf);
    int ret_code = system(buf);
    if( ret_code !=0 ) {
        fprintf(stderr,"system return code:%d\n",ret_code);
        exit(1);
    }

    //read whiskers tree
    sprintf(buf,"%s.out",whiskers_file);
    fd = open( buf, O_RDONLY );
    if ( fd < 0 ) {
        perror( "open" );
        exit( 1 );
    }

    KemyBuffers::WhiskerTree tree_new;
    if ( !tree_new.ParseFromFileDescriptor( fd ) ) {
        fprintf( stderr, "Could not parse %s.\n", buf );
        exit( 1 );
    }

    auto _whiskers_new = WhiskerTree( tree_new );
    if ( close( fd ) < 0 ) {
        perror( "close" );
        exit( 1 );
    }

    printf("whiskers:\n%s\n",_whiskers_new.str().c_str());

    // read utility
    sprintf(buf,"%s.utility",whiskers_file);
    FILE* fp = fopen(buf,"r");
    double tp,del;
    double _utility=0;
    double on_time=0;
    int acks=0;
    int inorder=0;
    int  sender_id=0;
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
        tp = stod(tmp);
        ss>>tmp;
        ss>>tmp;
        if(tmp!="del="){
            perror("read utility error");
        }
        ss>>tmp;
        del = stod(tmp);
        ss>>tmp>>tmp;
        ss>>on_time;
        ss>>tmp>>tmp;
        ss>>acks;
        ss>>tmp>>tmp;
        ss>>inorder;
        ss>>tmp>>tmp;
        ss>>sender_id;
        _utility+=log2(tp)-log2(del);
        printf("throughput:%f\tdelay:%f\ton:%f\tacks:%d\tinorder:%d\tid:%d\n",tp,del,on_time,acks,inorder,sender_id);
        ++cnt;
    }
    if(cnt){
        _utility/=cnt;
    }
    fclose(fp);

    ret_code = system("awk '{print $5}' ./debug/out.qmon | sort -n | tail");
    if(ret_code){
        perror("error awk");
        return 1;
    }
    printf("utility: %.2f\n",_utility);
/*    if(chdir(current_dir) == -1){*/
        //perror("change  directory to tcl error");
        //exit(1);
    /*}*/
    sprintf(buf,"rm %s.out %s.utility",whiskers_file,whiskers_file);
    if( system(buf)!=0 ) {
        perror("error when call rm");
        exit(1);
    }

    return 0;
}


