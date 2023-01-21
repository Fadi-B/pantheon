#ifndef DATA_COLLECTOR_HPP
#define DATA_COLLECTOR_HPP

#include <list>
#include "filewriter.hh"

enum class Type {Base, RTTGrad, Packet};

class Collector
{

public:

    explicit Collector(double tick_time)
    :writer()
    {

        TICK_TIME = tick_time;
        data.push_back(TICK_TIME);

    }

    virtual ~Collector()
    {

    }

    virtual Type getType() const
    {

	return Type::Base;

    }

    virtual void update(double arg1, double arg2) {}

    virtual double compute() {}

    virtual void resetHelperData() {}

    void resetAll();

    virtual std::list< double > getData() 
    {
        return data;
    }

    virtual void saveData(std::list<double> &data) {}


protected:

    std::list< double > data;

    double TICK_TIME;

    FileWriter writer;

};


#endif
