#pragma once
#ifndef COLLECTOR_MANAGER_HPP
#define COLLECTOR_MANAGER_HPP

#include <list>
#include <chrono>

#include "data_collector.hh"
#include "rtt_grad_collector.hh"
#include "packet_collector.hh"
#include "inter_arrival_time_collector.hh"
#include "queuing_delay_collector.hh"


class CollectorManager
{

public:

    static double getCurrentTime( std::chrono::high_resolution_clock::time_point &start_time )
    {
        using namespace std::chrono;
        high_resolution_clock::time_point cur_time = high_resolution_clock::now();

        return duration_cast<duration<double>>(cur_time - start_time).count()*1000;
    }

    CollectorManager(double collectInterval) /* in ms */
    {

        COLLECT_INTERVAL = collectInterval;

        _start_time_point = chrono::high_resolution_clock::now();

        double current_time = getCurrentTime(_start_time_point);
        _collect_time = current_time + COLLECT_INTERVAL;

        /* Initializing all the collectors */
	    _rtt_grad_collector = new RTTGradCollector(COLLECT_INTERVAL);
        _packet_collector = new PacketCollector(COLLECT_INTERVAL);
        _inter_arrival_collector = new InterArrivalTimeCollector(COLLECT_INTERVAL);
        _queuing_delay_collector = new QueuingDelayCollector(COLLECT_INTERVAL);

        /* Adding all of them to our collector set */
        collectors.push_back(_rtt_grad_collector);
        collectors.push_back(_packet_collector);
        collectors.push_back(_inter_arrival_collector);
        collectors.push_back(_queuing_delay_collector);

    }

    void collectData(double packet_frac, uint64_t RTT, uint64_t timestamp_received, uint64_t min_rtt, uint64_t inter_arrival_time)
    {
        double current_time = getCurrentTime(_start_time_point);

        bool collect = false;

        if (current_time >= _collect_time)
        {
            collect = true;
        }

        for (std::list<Collector*>::iterator itr=collectors.begin(); itr!=collectors.end(); itr++)
        {

	        if (collect)
	        {

        	    (*itr)->compute();
                (*itr)->resetHelperData();

                std::list< double > data = (*itr)->getData();

                (*itr)->saveData(data);
            }

            switch((*itr)->getType())
            {
       	        case Type::RTTGrad:
		        (*itr)->update(RTT, timestamp_received);
                 //fprintf(stderr, "RTTGrad: %d\n", (*itr)->getType());
		        break;
		        
                case Type::Packet:
		        (*itr)->update(packet_frac, 0);
		        //fprintf(stderr, "Packet: %f\n", packet_frac);
		        break;

                case Type::InterArrivalTime:
                (*itr)->update(inter_arrival_time, 0); /* IMPORTANT: SHOULD MOST LIKELY CHANGE THE TIMESTAMP USED*/
        	    
                break;

                case Type::QueueDelay:
                (*itr)->update(RTT, min_rtt);

                break;
                
                default:

		        //Should really throw an exception
                fprintf(stderr, "ERROR: %d \n", (*itr)->getType());
            }

        }

        /* Ensure we update the next collect time */
	    if (collect)
        {

            _collect_time = current_time + COLLECT_INTERVAL;

        }

    }


private:

    double COLLECT_INTERVAL;
    std::chrono::high_resolution_clock::time_point _start_time_point;
    double _collect_time;

    RTTGradCollector *_rtt_grad_collector;
    PacketCollector *_packet_collector;
    InterArrivalTimeCollector * _inter_arrival_collector;
    QueuingDelayCollector * _queuing_delay_collector;

    std::list<Collector*> collectors;


};

#endif
