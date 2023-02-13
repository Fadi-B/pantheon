#ifndef QUEUING_DELAY_COLLECTOR_HPP
#define QUEUING_DELAY_COLLECTOR_HPP

#include <list>
#include "data_collector.hh"
//#include "filewriter.hh"

using namespace std;

class QueuingDelayCollector : public Collector
{

public:

    using Collector::TICK_TIME;
    using Collector::data;
    using Collector::writer;

    /* Curerntly does not have an impact */
    static constexpr double EWMA_WEIGHT = 1.0;

    QueuingDelayCollector(double tick_time)
    :Collector(tick_time)
    {

        /* Dummy initialization - Update function will do the true updating */
        MIN_RTT = 0;

    }

    Type getType() const override
    {

        return Type::QueueDelay;

    }

    /* NOTE: I am skeptical against what would happen if we get the odd super large RTTs
      
       Does not seem to have appeared yet, but keep an eye as we might have to add an RTT check
    */
    void update(double RTT, double min_rtt)
    {

        helper_data.push_back(RTT);

        /* MIN_RTT can change depending on the time scale we are interested in */
        MIN_RTT = min_rtt;

        return;

    }

    double compute()
    {

        double sum = 0;

        for (auto it = helper_data.begin(); it != helper_data.end(); it++)
        {

	        auto obj = *it;
        
            sum = sum + obj;

        }

        /* Assign it min RTT by default */
        double RTT = MIN_RTT;

        uint16_t size = helper_data.size();

        if ( size > 0)
        {
            /* RTT for this tick will be considered as the average */
            RTT = sum / size;

        }

        /* Be careful here since technically the only reason it works is because EWMA WEIGHT is 1*/
        double queuing_delay = (1 - EWMA_WEIGHT) * queuing_delay + ( EWMA_WEIGHT * RTT - MIN_RTT); 

        data.push_back(queuing_delay);

	    return queuing_delay;

    }

    void resetHelperData()
    {

        helper_data.clear();

    }

    void resetAll()
    {
        helper_data.clear();
        data.clear();

	data.push_back(TICK_TIME);
    }

    std::list< double > getHelperData() 
    {
        return helper_data;
    }

    void saveData(std::list<double> &data)
    {

        writer.write_to_file("queue_delay_data.csv", data);

    }

private:

    std::list< double > helper_data;

    double MIN_RTT;

};


#endif