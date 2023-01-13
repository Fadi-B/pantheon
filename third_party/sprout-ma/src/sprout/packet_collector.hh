#ifndef PACKET_COLLECTOR_HPP
#define PACKET_COLLECTOR_HPP

#include <list>

using namespace std;

class PacketCollector
{

public:

    static const int MSS = 1500;     /* Bytes */
    static const int BYTE_SIZE = 8;  /* Bits */

    inline PacketCollector(double tick_time)
    {
	TICK_TIME = tick_time;
	throughput.push_back(tick_time);
    }

    void update(double packets)
    {

        data = data + packets;

    }

    double computeThroughput()
    {

        int bits = MSS * BYTE_SIZE;

        double total_bits = data * bits;

        double bits_per_sec = total_bits / (TICK_TIME*1000);

        throughput.push_back(bits_per_sec);

	return bits_per_sec;

    }

    void resetData()
    {

        data = 0;

    }

    void resetAll()
    {
        data = 0;
        throughput.clear();

	throughput.push_back(TICK_TIME);
    }

    double getData() 
    {
        return data;
    }

    std::list< double > getThroughput() 
    {
        return throughput;
    }

private:

    double data;
    std::list< double > throughput;

    double TICK_TIME;

};


#endif
