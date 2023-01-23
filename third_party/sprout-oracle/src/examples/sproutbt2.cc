#include <unistd.h>
#include <string>
#include <assert.h>
#include <list>
#include <iostream>

#include "sproutconn.h"
#include "select.h"

#include "file_reader.hh"
#include <list>

using namespace std;
using namespace Network;

int main( int argc, char *argv[] )
{
  char *ip;
  int port;

  Network::SproutConnection *net;

  bool server = true;

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

  fallback_interval = 50;
  int new_rate_interval = 250;

  uint64_t time_of_next_transmission = timestamp() + fallback_interval;
  uint64_t time_of_rate_adjustment = timestamp() + new_rate_interval;

  fprintf( stderr, "Looping...\n" );

//  uint64_t test = 5000 + timestamp();

  std::list<uint64_t> oracle;

  oracle = read_file("oracle50.txt");
  auto it = oracle.begin();// + 61; /* +61 offset to start at 3s*/

  if (it == oracle.end()) {fprintf(stderr, "MUHAHAHHAHHAHAHAHA");}

//  for (int i = 0; i < 1; i++) {
//  it++;
// }


  int bytes;
  //fprintf(stderr, "To Send: %d \n", bytes);

  /* loop */
  while ( 1 ) {
    int bytes_to_send = net->window_size();

    if ( server ) {
      bytes_to_send = 0;
    }

    //fprintf(stderr, "To Send: %d \n", bytes);

//    if (test <= timestamp()) {bytes_to_send = 750;}


    /* actually send, maybe */
    if ( /*( bytes_to_send > 0 ) &&*/ ( time_of_next_transmission <= timestamp() ) ) {

      bytes = *it;

      bytes_to_send = std::max(bytes - 50, 0);

      fprintf(stderr, "To Send: %d \n", bytes);
      /* How much to send in this tick */
      //bytes_to_send = (bytes / new_rate_interval) * fallback_interval;

      if (it != oracle.end())
      {
        //int v = 1;
        it++;
      }
      else
      {
        fprintf(stderr, "Iterator Ended \n");
      }

      do {
	int this_packet_size = std::min( 1400, bytes_to_send );
	bytes_to_send -= this_packet_size;
	assert( bytes_to_send >= 0 );

	string garbage( this_packet_size, 'x' );

	int time_to_next = 0;
	if ( bytes_to_send == 0 ) {
	  time_to_next = fallback_interval;
	}

	net->send( garbage, time_to_next );
      } while ( bytes_to_send > 0 );

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
