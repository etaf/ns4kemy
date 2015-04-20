#include <fcntl.h>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "configrange.hh"
#include "evaluator.hh"
#include "network.hh"
#include <future>

Evaluator::Evaluator( const ConfigRange & range )
  : _prng_seed( global_PRNG()() ), /* freeze the PRNG seed for the life of this Evaluator */
    _configs()
{
  /* first load "anchors" */
  _configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.first ).set_bottle_single_delay( range.bottle_single_delay.first ).set_num_senders( range.max_senders ));

  if ( range.lo_only ) {
    return;
  }

  _configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.first ).set_bottle_single_delay( range.bottle_single_delay.first ).set_num_senders( range.max_senders ) );
/*  _configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.first ).set_bottle_single_delay( range.bottle_single_delay.second ).set_num_senders( range.max_senders ) );*/
  //_configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.second ).set_bottle_single_delay( range.bottle_single_delay.first ).set_num_senders( range.max_senders ));
  //_configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.second ).set_bottle_single_delay( range.bottle_single_delay.second ).set_num_senders( range.max_senders ));

  //_configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.first ).set_bottle_single_delay( range.bottle_single_delay.first ).set_num_senders( range.min_senders ) );
  //_configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.first ).set_bottle_single_delay( range.bottle_single_delay.second ).set_num_senders( range.min_senders ) );
  //_configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.second ).set_bottle_single_delay( range.bottle_single_delay.first ).set_num_senders( range.min_senders ));
  /*_configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.second ).set_bottle_single_delay( range.bottle_single_delay.second ).set_num_senders( range.min_senders ));*/


/*  for ( unsigned int i = (range.max_senders+1)/3*2; i <= range.max_senders; ++i)*/
  //{
  //_configs.push_back( NetConfig().set_bottle_bw( (range.bottle_bw.second + range.bottle_bw.second)/2 ).set_bottle_single_delay( (range.bottle_single_delay.first + range.bottle_single_delay.second)/2 ).set_num_senders(i) );
  /*}*/
  /* now load some random ones just for fun */

  for ( int i = 0; i < 15; i++ ) {
/*    boost::random::uniform_real_distribution<> link_speed( range.bottle_bw.first, range.bottle_bw.second );*/
    /*boost::random::uniform_real_distribution<> rtt( range.bottle_single_delay.first, range.bottle_single_delay.second );*/
    boost::random::uniform_int_distribution<> num_senders( 1, range.max_senders );

    _configs.push_back( NetConfig().set_bottle_bw( range.bottle_bw.first ).set_bottle_single_delay( range.bottle_single_delay.first ).set_num_senders( num_senders( global_PRNG() ) ) );
  }
}

/*ProblemBuffers::Problem Evaluator::DNA( const WhiskerTree & whiskers ) const*/
//{
  //ProblemBuffers::Problem ret;

  //ret.mutable_whiskers()->CopyFrom( whiskers.DNA() );

  //ProblemBuffers::ProblemSettings settings;
  //settings.set_prng_seed( _prng_seed );
  //settings.set_tick_count( TICK_COUNT );

  //ret.mutable_settings()->CopyFrom( settings );

  //for ( auto &x : _configs ) {
    //KemyBuffers::NetConfig *config = ret.add_configs();
    //*config = x.DNA();
  //}

  //return ret;
//}

/*Evaluator::Outcome Evaluator::parse_problem_and_evaluate( const ProblemBuffers::Problem & problem )*/
//{
    //std::vector<NetConfig> configs;
  //for ( const auto &x : problem.configs() ) {
    //configs.emplace_back( x );
  //}

  //WhiskerTree run_whiskers = WhiskerTree( problem.whiskers() );

  //return Evaluator::score( run_whiskers, problem.settings().prng_seed(),
			   //configs, false, problem.settings().tick_count() );
/*}*/

/*AnswerBuffers::Outcome Evaluator::Outcome::DNA( void ) const*/
//{
  //AnswerBuffers::Outcome ret;

  //for ( const auto & run : throughputs_delays ) {
    //AnswerBuffers::ThroughputsDelays *tp_del = ret.add_throughputs_delays();
    //tp_del->mutable_config()->CopyFrom( run.first.DNA() );

    //for ( const auto & x : run.second ) {
      //AnswerBuffers::SenderResults *results = tp_del->add_results();
      //results->set_throughput( x.first );
      //results->set_delay( x.second );
    //}
  //}

  //ret.set_score( score );

  //return ret;
/*}*/

/*Evaluator::Outcome::Outcome( const AnswerBuffers::Outcome & dna )*/
  //: score( dna.score() ),
    //throughputs_delays(),
    //used_whiskers()
//{
  //for ( const auto &x : dna.throughputs_delays() ) {
      //std::vector< std::pair< double, double > > tp_del;
    //for ( const auto &result : x.results() ) {
      //tp_del.emplace_back( result.throughput(), result.delay() );
    //}

    //throughputs_delays.emplace_back( NetConfig( x.config() ), tp_del );
  //}
/*}*/

Evaluator::Outcome Evaluator::score( WhiskerTree & run_whiskers,
				     const bool trace ) const
{
  return score( run_whiskers, _prng_seed, _configs, trace);
}


Evaluator::Outcome Evaluator::score( WhiskerTree & run_whiskers,
				     const unsigned int prng_seed,
				     const std::vector<NetConfig> & configs,
				     const bool trace
				     )
{
  PRNG run_prng( prng_seed );

  run_whiskers.reset_counts();

  /* run tests */
  Evaluator::Outcome the_outcome;
  the_outcome.used_whiskers = run_whiskers;

 //printf("before:\t%s\n",the_outcome.used_whiskers.str().c_str());

  std::vector<std::future<std::pair<WhiskerTree, double> > > fs;
  for (auto &x : configs){
        fs.emplace_back(std::async(std::launch::async, [] (NetConfig x_,WhiskerTree run_whiskers_,unsigned int prng_seed, bool trace_){
                        Network network1( x_ );
                        network1.run_simulation(run_whiskers_,trace_, (prng_seed % 1024));
                        return std::make_pair(run_whiskers_, network1.utility());
                    },x,run_whiskers,prng_seed, trace));
  }
  for( auto &x : fs){
      auto res = x.get();
      if(the_outcome.used_whiskers.combine_other(res.first, trace) == false){
          perror("combine_other error");
          exit(1);
      }
      the_outcome.score += res.second;
  }
  run_whiskers = the_outcome.used_whiskers;
 //printf("after:\t%s\n",the_outcome.used_whiskers.str().c_str());
  return the_outcome;
}
