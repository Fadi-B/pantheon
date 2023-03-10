#ifndef INTER_ARRIVAL_TIME_HPP
#define INTER_ARRIVAL_TIME_HPP

#include <list>
#include "data_collector.hh"
//#include "filewriter.hh"

using namespace std;

class InterArrivalTimeCollector : public Collector
{

public:

    using Collector::TICK_TIME;
    using Collector::data;
    using Collector::writer;

    /* Curerntly does not have an impact */
    static constexpr double EWMA_WEIGHT = 1.0;

    InterArrivalTimeCollector(double tick_time)
    :Collector(tick_time)
    {


    }

    Type getType() const override
    {

        return Type::InterArrivalTime;

    }

    void update(double inter_arrival_time, double __attribute((unused)))
    {

        /* Will use less than 0 as indication of 'invalid' and so do not include it */
        if (!(inter_arrival_time < 0))
        {

            helper_data.push_back(inter_arrival_time);

        }
        

        return;

    }

    double compute()
    {

        /* Used to compute the average */
        double sum_of_inter_arrival_times = 0;

        /* Looping until next to last to ensure we do not go out of bounds */
        for (auto it = helper_data.begin(); it != helper_data.end(); it++)
        {
            
            auto obj = *it;

            sum_of_inter_arrival_times = sum_of_inter_arrival_times + obj;

        }

        double inter_arrival_time = sum_of_inter_arrival_times / helper_data.size();

        double ewma_inter_arrival_time = (1 - EWMA_WEIGHT) * ewma_inter_arrival_time + (EWMA_WEIGHT * inter_arrival_time); 

        data.push_back(ewma_inter_arrival_time);

	    return ewma_inter_arrival_time;

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

        writer.write_to_file("inter_arrival_data.csv", data);

    }

private:

    std::list< double > helper_data;

};


#endif