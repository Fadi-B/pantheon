#include "RTTCollector.hh"
#include<tuple>

using namespace std;

RTTCollector::RTTCollector()
{

}

void RTTCollector::update(double RTT, double receptionTime) 
{

    data.push_back( tuple<double, double>(RTT, receptionTime) );

    return;

}

double RTTCollector::computeRTTGradient()
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
    RTTCollector::RTTGrads.push_back(slope);

    return slope;

}

void RTTCollector::resetData()
{

    RTTCollector::data.clear();

}

void RTTCollector::resetAll()
{
    RTTCollector::data.clear();
    RTTCollector::RTTGrads.clear();
}
/**
int main()
{

    //std::tuple <int, double> F(1, 3.14);
    //fprintf(stderr, "%d", std::get<0>(F));

    RTTCollector fadi = RTTCollector();

    fadi.update(2,1);
    fadi.update(6,3);
    fadi.update(8,4);
    fadi.update(20,10);

    fadi.resetAll();

    double slope = fadi.computeRTTGradient();
    fprintf(stderr, "%f", slope);

    return 0;
}
*/
