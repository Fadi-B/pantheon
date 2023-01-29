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

#include <Eigen/Dense>
#include "kalman_filter.hh"

class CollectorManager
{

public:

    static const int WINDOW_LENGTH = 1; /* Will start off with 1 window length for prototyping */

    typedef Eigen::Matrix<double, KF::DIM, 1> Vector;
    typedef Eigen::Matrix<double, WINDOW_LENGTH, KF::DIM> Matrix;

    static double getCurrentTime( std::chrono::high_resolution_clock::time_point &start_time )
    {
        using namespace std::chrono;
        high_resolution_clock::time_point cur_time = high_resolution_clock::now();

        return duration_cast<duration<double>>(cur_time - start_time).count()*1000;
    }

    CollectorManager(double collectInterval, uint16_t window_length) /* in ms */
    {

        COLLECT_INTERVAL = collectInterval;

        _start_time_point = chrono::high_resolution_clock::now();

        double current_time = getCurrentTime(_start_time_point);
        _collect_time = current_time + COLLECT_INTERVAL;

        /* Initializing all the collectors */
        _packet_collector = new PacketCollector(COLLECT_INTERVAL);
	    _rtt_grad_collector = new RTTGradCollector(COLLECT_INTERVAL);
        _queuing_delay_collector = new QueuingDelayCollector(COLLECT_INTERVAL);
        _inter_arrival_collector = new InterArrivalTimeCollector(COLLECT_INTERVAL);

        /* Adding all of them to our collector set */
        collectors.push_back(_packet_collector);
        collectors.push_back(_rtt_grad_collector);
        collectors.push_back(_queuing_delay_collector);
        collectors.push_back(_inter_arrival_collector);

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

    /**
     * @brief Get the Congestion Signals History o
     * 
     * @param window_length 
     * @return Matrix 
     */
    Matrix getCongestionSignalsHistory()
    {

        Matrix congestion_history;

        congestion_history(0, KF::iBias) = 1;

        /* Note: Collectors will be looped over in the order
         *
         *        1. Packet Collector
         *        2. RTT Gradient Collector
         *        3. Queuing Delay Collector
         *        4. Inter Arrival Collector
         * 
         * Note: I have done this in the order that I was designing the state-space model in python
         * 
         */

        uint16_t column_index = 1; /* Starts at 1 since we have already intialized the bias column */

        for (std::list<Collector*>::iterator itr=collectors.begin(); itr!=collectors.end(); itr++)
        {

            std::list< double > data = (*itr)->getData();

            uint16_t row_index = 0;

            /* We are reverse iterating as we want to look back on the past to do our forecast */
            for (std::list<double>::reverse_iterator revitr=data.rbegin(); revitr!=data.rend(); revitr++ )
            {

                auto obj = *revitr;

                if (row_index < WINDOW_LENGTH)
                {

                    congestion_history(row_index, column_index) = obj;

                }
                else
                {
                    break;
                }

                row_index = row_index + 1;
                
            }

            column_index = column_index + 1;

        }

        return congestion_history;

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
