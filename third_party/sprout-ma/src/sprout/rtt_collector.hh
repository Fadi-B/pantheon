#ifndef RTTCOLLECTOR_HPP
#define RTTCOLLECTOR_HPP

#include <list>
#include<tuple>
//#include "boost/tuple/tuple.hpp"

using namespace std;

class RTTCollector
{

public:

    static const int RTT_INDEX = 0;
    static const int RECEPTION_INDEX = 1;

    inline RTTCollector(double tick_time)
    {

	TICK_TIME = tick_time;
	RTTGrads.push_back(tick_time);

    }

    void update(double RTT, double receptionTime) 
    {

        data.push_back( tuple<double, double>(RTT, receptionTime) );

        return;

    }

    double computeRTTGradient()
    {

	/* Under the assumption that RTTGrad remains unchanged when not enough data observed */
        if (data.empty() || data.size() == 1) {

	    return 0;

        }

        int size = data.size();

        double sumX = 0;
        double sumX_squared = 0;
        double sumY = 0;
        double sumXY = 0;

        for (auto it = data.begin(); it != data.end(); it++)
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

	fprintf(stderr, "Start \n");

        if (1) {

        for (auto it = data.begin(); it != data.end(); it++)
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

        /* Calculating slope of linear fit */

	/* IMPORTANT: Will return NAN if no data observed or no unique solution */
        double slope = (size*sumXY - sumX*sumY) / (size*sumX_squared - sumX*sumX);

        /* Update observed slopes */
        RTTGrads.push_back(slope);

        return slope;

    }

    void resetData()
    {

        data.clear();

    }

    void resetAll()
    {
        data.clear();
        RTTGrads.clear();

	RTTGrads.push_back(TICK_TIME);
    }

    std::list< std::tuple<double, double> > getData() 
    {
        return data;
    }

    std::list< double > getRTTGrads() 
    {
        return RTTGrads;
    }

private:

    std::list< std::tuple<double, double> > data;
    std::list< double > RTTGrads;

    double TICK_TIME;

};


#endif
