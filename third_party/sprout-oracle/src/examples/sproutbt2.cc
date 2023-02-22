#include <unistd.h>
#include <string>
#include <assert.h>
#include <list>
#include <iostream>

#include "sproutconn.h"
#include "select.h"

//#include "file_reader.hh"
#include <list>

#include <chrono>
#include <cmath>

using namespace std;
using namespace Network;


int main( int argc, char *argv[] )
{
  char *ip;
  int port;

  Network::SproutConnection *net;

  //std::chrono::high_resolution_clock::time_point _start_time_point( chrono::high_resolution_clock::now() );

  bool server = true;
  bool start = true;

  if ( argc > 2 ) {
    /* client */

    server = false;

    ip = argv[ 1 ];
    port = atoi( argv[ 2 ] );

    net = new Network::SproutConnection( "4h/Td1v//4jkYhqhLGgegw", ip, port );
  } else if ( argc == 2 ) {
    net = new Network::SproutConnection( NULL, argv[ 1 ] );

    printf( "Listening on port: %d\n", net->port() );
  } else {
    net = new Network::SproutConnection( NULL, NULL );

    cout << "Listening on port: " << net->port() << endl;
  }

  Select &sel = Select::get_instance();
  sel.add_fd( net->fd() );

  /*const*/ int fallback_interval = server ? 20 : 50;

  /* wait to get attached */
  if ( server ) {
    while ( 1 ) {
      int active_fds = sel.select( -1 );
      if ( active_fds < 0 ) {
	perror( "select" );
	exit( 1 );
      }

      if ( sel.read( net->fd() ) ) {
	net->recv();
      }

      if ( net->get_has_remote_addr() ) {
	break;
      }
    }
  }

  /* We are using a minimum RTT of 60ms
   *
   * We will assume that we are given best throughput to use
   * for each RTT every RTT
   *
   * However, we will space things out every 10ms (or 20ms)
   *   -> No, I do not think that is a good idea.
   */
  //fallback_interval = 60;
  int new_rate_interval = 60;

  //uint64_t time_of_next_transmission = timestamp() + fallback_interval;
  //uint64_t time_of_rate_adjustment = timestamp() + new_rate_interval;

  fprintf( stderr, "Looping...\n" );

  /* Loading the oracle */

  std::ifstream sizeF("oracle_size.txt");

  uint64_t SIZE = 0;

  // Read first line as it represents size of oracle data
  if (sizeF) {

    sizeF >> SIZE;

  }

  int oracle[SIZE];

  string line;
  int value;
  ifstream file ("oracle.txt");

  int counter = 0;

 if(file.is_open())
  {

     while(!file.eof())
     {

        getline(file, line);

        try
        {

           value = stoi(line);

        }
        catch (std::invalid_argument const& ex)
        {

        }

//        cout << line << endl;

//        fprintf(stderr, " \n");

        oracle[ counter ] = ((int) value);

        //fprintf(stderr, "FIRST ENTRY: %d \n", data[0]);

        counter = counter + 1;

     }
    //fprintf(stderr, "OUT: %d \n", 3);

        file.close();

  }

  std::chrono::high_resolution_clock::time_point _start_time_point( chrono::high_resolution_clock::now() );
  double cur = CollectorManager::getCurrentTime(_start_time_point);
  fprintf(stderr, "CUR: %f \n", cur);
  fallback_interval = oracle[((int)cur)];

  uint64_t time_of_next_transmission = timestamp() + fallback_interval; //We want to send instantly
  uint64_t time_of_rate_adjustment = timestamp() + new_rate_interval;

  fprintf(stderr, "Fallback Interval: %d \n", fallback_interval);

  /* Make sure it is initialized so we do not miss first iteration */
  int bytes;
  int rounded;

  /* loop */
  while ( 1 ) {
    int bytes_to_send = net->window_size();

    if ( server ) {
      bytes_to_send = 0;
    }

    /* actually send, maybe */
    if ( /*( bytes_to_send > 0 ) &&*/ ( time_of_next_transmission <= timestamp() ) ) {

      if (start)
      {

        double cur = CollectorManager::getCurrentTime(_start_time_point) / 1000;
	fprintf(stderr, "Sender Start Time: %f \n", cur);
        start = false;

      }

      cur = CollectorManager::getCurrentTime(_start_time_point);
      rounded = (int)ceil(cur / fallback_interval);

      //if (server) {fprintf(stderr, "Server Time: %f Rounded: %d \n", cur, rounded);} else {fprintf(stderr, "Client Time: %f Rounded: %d \n", cur, rounded);}
      //fprintf(stderr, "Rounded: %d \n", rounded);
      bytes = oracle[rounded - 1];

      bytes_to_send = std::max(bytes - 20, 0);

      //rounded = rounded + 1;

      //fprintf(stderr, "To Send: %d \n", bytes);

      /* How much to send in this tick */
     // bytes_to_send = (bytes / new_rate_interval) * fallback_interval;
      //bytes_to_send = std::max(bytes_to_send - 20, 0);

//      if (bytes_to_send == 0) {fprintf(stderr, "Zero \n");}

     if ( bytes_to_send != 0)
      {

      do {
	int this_packet_size = std::min( 1400, bytes_to_send );
	bytes_to_send -= this_packet_size;
	assert( bytes_to_send >= 0 );

	/* This generates the data of required size since 'x' is 1 byte */
	/* IMPORTANT: IF NOT DATA THEN SIZE WILL JUST BE SIZE OF FORECASTPACKET - 64 bytes */
	string garbage( this_packet_size, 'x' );

	int time_to_next = 0;
	if ( bytes_to_send == 0 ) {
	  time_to_next = fallback_interval;
	}

	net->send( garbage, time_to_next );
      } while ( bytes_to_send > 0 );

     }

      time_of_next_transmission = std::max( timestamp() + fallback_interval,
					    time_of_next_transmission );

    }

    /* wait */
    int wait_time = time_of_next_transmission - timestamp();
    if ( wait_time < 0 ) {
      wait_time = 0;
    } else if ( wait_time > 10 ) {
      wait_time = 10;
    }

    int active_fds = sel.select( wait_time );
    if ( active_fds < 0 ) {
      perror( "select" );
      exit( 1 );
    }

    /* receive */
    if ( sel.read( net->fd() ) ) {
      string packet( net->recv() );
    }
  }
}
