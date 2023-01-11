#ifndef RTTCOLLECTOR_HPP
#define RTTCOLLECTOR_HPP

#include <list>
#include<tuple>
//#include "boost/tuple/tuple.hpp"

class RTTCollector
{

public:

    static const int RTT_INDEX = 0;
    static const int RECEPTION_INDEX = 1;

    RTTCollector();

    void update(double RTT, double receptionTime);

    double computeRTTGradient();

    void resetData();
    void resetAll();

private:

    std::list< std::tuple<double, double> > data;
    std::list< double > RTTGrads;

};


#endif
