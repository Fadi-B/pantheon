#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <chrono>

#include "receiver.hh"
#include "sproutmath.pb.h"

#include "network.h"

#include "kalman_filter.hh"

using namespace std;

Receiver::Receiver()
  : _process( MAX_ARRIVAL_RATE,
	      BROWNIAN_MOTION_RATE,
	      OUTAGE_ESCAPE_RATE,
	      NUM_BINS ),
    _forecastr(),
    _time( 0 ),
    _score_time( -1 ),
    _count_this_tick( 0 ),
    _cached_forecast(),
    _recv_queue(),
    _ewma_rate_estimate( 1 ),
    _collect_time( 0 ),
    _start_time_point( chrono::high_resolution_clock::now() ),
    _MIN_RTT( 1000000 ), /* Initialize to high value */
    _prev_arrival( -1 ),
    _collector_manager(500),
    _KFforecaster(36, 0, 0, 0)
{

  double cur = CollectorManager::getCurrentTime(_start_time_point);
  _collect_time = cur + TICK_LENGTH;

  fprintf(stderr, "Start Time: %f \n", cur);
  fprintf(stderr, "Collect Time: %f \n", _collect_time);

  for ( int i = 0; i < NUM_TICKS; i++ ) {
    ProcessForecastInterval one_forecast( .001 * TICK_LENGTH,
					  _process,
					  MAX_ARRIVALS_PER_TICK,
					  i + 1 );
    _forecastr.push_back( one_forecast );
  }
}


void Receiver::advance_to( const uint64_t time )
{
  assert( time >= _time );

  while ( _time + TICK_LENGTH < time ) {

    _KFforecaster.forecast(1);

    if ( (_time >= _score_time) || (_count_this_tick > 0) ) {

       /* Currently 1 x 5 */
      CollectorManager::Matrix measurement = _collector_manager.getCongestionSignalsHistory();

      int rounded_bytes = int( _count_this_tick + 0.5 );

      if ( _count_this_tick > 0 && _count_this_tick < 1)
      {

        rounded_bytes = 1;

        /* Ensure we update the rate estimation in this case*/
        measurement(1, KF::iBand) = PacketCollector::to_bits_per_sec(rounded_bytes);

      }

      _KFforecaster.correctForecast(measurement.transpose());

      //const double alpha = 1.0;
      //_ewma_rate_estimate = (1 - alpha) * _ewma_rate_estimate + ( alpha * _count_this_tick );

      _count_this_tick = 0;

    } else {
      //      fprintf( stderr, "-SKIP-" );
    }
    _time += TICK_LENGTH;
  }
}

void Receiver::recv( const uint64_t seq, const uint16_t throwaway_window, const uint16_t time_to_next, const size_t len, uint16_t timestamp, uint16_t timestamp_reception )
{
  _count_this_tick += len / 1400.0;
  _recv_queue.recv( seq, throwaway_window, len );
  _score_time = std::max( _time + time_to_next, _score_time );

  uint16_t RTT = Network::timestamp_diff(timestamp_reception, timestamp);
  uint16_t inter_arrival_time = Network::timestamp_diff(timestamp_reception, _prev_arrival);
  /* Check if starting value since we need at least two points */
  if (_prev_arrival == -1)
  {

    _prev_arrival = timestamp_reception;

    /* To indicate that we have not obtained two samples yet */
    inter_arrival_time = -1;

  }

  /* Ensure we update the previous estimate of packet arrival */
  _prev_arrival = timestamp_reception;

  /* Updates the minimum RTT observed during recent connection */
  if (RTT < _MIN_RTT)
  {

    _MIN_RTT = RTT;

  }

  _collector_manager.collectData(len/1400, RTT, timestamp_reception, _MIN_RTT, inter_arrival_time);

}

Sprout::DeliveryForecast Receiver::forecast( void )
{
  if ( _cached_forecast.time() == _time ) {
    return _cached_forecast;
  } else {
    std::vector< int > counts;

    _process.normalize();

    _cached_forecast.set_received_or_lost_count( _recv_queue.packet_count() );
    _cached_forecast.set_time( _time );
    _cached_forecast.clear_counts();

    int tick_number = 1;

    auto iter = _KFforecaster.getBytesToBeDrained().begin();

    for ( auto it = _forecastr.begin(); it != _forecastr.end(); it++ ) {
      //_cached_forecast.add_counts( it->lower_quantile(_process, 0.05) );

      //Note: For now we have not added any uncertainty bounds
      _cached_forecast.add_counts( *iter );
      
      iter++;

    }

    /* Ensure we clear our previous forecast - Will be irrelevant for the future */
    _KFforecaster.clearForecast();

    return _cached_forecast;
  }
}

void Receiver::RecvQueue::recv( const uint64_t seq, const uint16_t throwaway_window, const int len )
{
  received_sequence_numbers.push( PacketLen( seq, len ) );
  throwaway_before = std::max( throwaway_before, seq - throwaway_window );
}

uint64_t Receiver::RecvQueue::packet_count( void )
{
  /* returns cumulative count of bytes received or lost */
  while ( !received_sequence_numbers.empty() ) {
    if ( received_sequence_numbers.top().seq < throwaway_before ) {
      received_sequence_numbers.pop();
    } else {
      break;
    }
  }

  std::priority_queue< PacketLen, std::deque<PacketLen>, PacketLen > copy( received_sequence_numbers );

  int buffer_sum = 0;
  while ( !copy.empty() ) {
    buffer_sum += copy.top().len;
    copy.pop();
  }

  return throwaway_before + buffer_sum;
}

