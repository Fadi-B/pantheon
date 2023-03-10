#ifndef RTT_COLLECTOR_HPP
#define RTT_COLLECTOR_HPP

#include <list>
#include <tuple>

//#include "filewriter.hh"
#include "data_collector.hh"
#include "cmath"

using namespace std;

class RTTGradCollector : public Collector
{

public:

    using Collector::TICK_TIME;
    using Collector::data;
    using Collector::writer;

    static const int RTT_INDEX = 0;
    static const int RECEPTION_INDEX = 1;

    /* Currently set to very high as we will consider all packets for now */
    static const int MAX_RTT = 1000000000; /* Default 50000 in Sprout smoothed RTT calculation */

    static constexpr double LOW_PASS_FILTER_CUT_OFF = 0.015;

    RTTGradCollector(double tick_time)
    :Collector(tick_time)
    {

    }

    Type getType() const override
    {

    	return Type::RTTGrad;

    }

    void update(double RTT, double receptionTime) 
    {

        if (RTT < MAX_RTT)
        {

            helper_data.push_back( tuple<double, double>(RTT, receptionTime) );

        }

        return;

    }

    double compute()
    {

	/* Under the assumption that RTTGrad remains unchanged when not enough data observed */
        if (helper_data.empty() || helper_data.size() == 1) {

	    return 0;

        }

        int size = helper_data.size();

        double sumX = 0;
        double sumX_squared = 0;
        double sumY = 0;
        double sumXY = 0;

        for (auto it = helper_data.begin(); it != helper_data.end(); it++)
        {

	        auto obj = *it;

            double x_coord = get<RECEPTION_INDEX>(obj);
            double y_coord = get<RTT_INDEX>(obj);

            sumX = sumX + x_coord;
            sumX_squared = sumX_squared + (x_coord * x_coord);
            sumY = sumY + y_coord;
            sumXY = sumXY + (x_coord*y_coord);

        }

        double numerator = size*sumXY - sumX*sumY;
	double denominator = size*sumX_squared - sumX*sumX;

//	fprintf(stderr, "Start \n");
/*
        if (1) {

        for (auto it = helper_data.begin(); it != helper_data.end(); it++)
        {

	    auto obj = *it;

            double x_coord = get<RECEPTION_INDEX>(obj);
            double y_coord = get<RTT_INDEX>(obj);

            fprintf(stderr, "RTT: %f STAMP: %f \n", y_coord, x_coord);

        }

        }

	if (denominator == 0) {

        fprintf(stderr, "Denominator: %f \n", denominator);
	fprintf(stderr, "Numerator: %f \n", numerator);


        }

	fprintf(stderr, "End \n");
*/
        /* Calculating slope of linear fit */

	/* IMPORTANT: Will return NAN if no data observed or no unique solution */
        double slope = (size*sumXY - sumX*sumY) / (size*sumX_squared - sumX*sumX);

        /* This is employed to avoid network jitter that can be misleading about the measurement */
        if (abs(slope) < LOW_PASS_FILTER_CUT_OFF)
        {
            slope = 0;
        }

        /* Update observed slopes */
        data.push_back(slope);

        return slope;

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

    std::list< std::tuple<double, double> > getHelperData() 
    {
        return helper_data;
    }

    void saveData(std::list<double> &data)
    {

        writer.write_to_file("rtt_grad_data.csv", data);

    }

private:

    std::list< std::tuple<double, double> > helper_data;

};


#endif
