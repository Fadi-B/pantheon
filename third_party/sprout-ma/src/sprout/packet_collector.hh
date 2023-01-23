#ifndef PACKET_COLLECTOR_HPP
#define PACKET_COLLECTOR_HPP

#include <list>
#include "data_collector.hh"
//#include "filewriter.hh"

using namespace std;

class PacketCollector : public Collector
{

public:

    using Collector::TICK_TIME;
    using Collector::data;
    using Collector::writer;

    static const int MSS = 1500;     /* Bytes */
    static const int BYTE_SIZE = 8;  /* Bits */

    PacketCollector(double tick_time)
    :Collector(tick_time)
    {

    }

    Type getType() const override
    {

        return Type::Packet;

    }

    void update(double packets, double arg2 __attribute((unused)))
    {

        helper_data = helper_data + packets;
        return;

    }

    double compute()
    {

        //fprintf(stderr, "Helper2: %f \n", helper_data);

        int bits = MSS * BYTE_SIZE;

        double total_bits = helper_data * bits;

        double bits_per_sec = total_bits / (TICK_TIME*1000);

        data.push_back(bits_per_sec);

	return bits_per_sec;

    }

    void resetHelperData()
    {

        helper_data = 0;

    }

    void resetAll()
    {
        helper_data = 0;
        data.clear();

	data.push_back(TICK_TIME);
    }

    double getHelperData() 
    {
        return helper_data;
    }

    void saveData(std::list<double> &data)
    {

        writer.write_to_file("throughput_data.csv", data);

    }

private:

    double helper_data;

//    std::list< double > dataf;

//    double TICK_TIME;

};


#endif
