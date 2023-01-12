#ifndef RTT_COLLECTOR_ADDON_HPP
#define RTT_COLLECTOR_ADDON_HPP

#include <list>
#include<tuple>
//#include "boost/tuple/tuple.hpp"

using namespace std;

class RTTCollector
{

public:

    static const int RTT_INDEX = 0;
    static const int RECEPTION_INDEX = 1;

    inline RTTCollector()
    {

    }

    void update(double RTT, double receptionTime) 
    {

        data.push_back( tuple<double, double>(RTT, receptionTime) );

        return;

    }

    double computeRTTGradient()
    {

        if (data.empty()) {

	    return 0;
        }

        int size = RTTCollector::data.size();

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

        /* Calculating slope of linear fit */

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

};


#endif
