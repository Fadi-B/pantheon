#pragma once

#include <Eigen/Dense>
#include <stdio.h>
#include "assert.h"
#include "kalman_filter.hh"

#include "packet_collector.hh"
#include <list>

class KFForecaster
{

public:


    //using namespace KF::DIM;
    //using namespace KF;

    static const uint8_t bits = 8;
    static const uint16_t ms_per_sec = 500; /* We should be careful with this one as we are forecasting 20ms into the future*/
    static const uint8_t iModel = 1;


    KFForecaster(double initBandwidth, double initRTTGrad, double initQueueDelay, double initInterArrival)
    :state(initBandwidth, initRTTGrad, initQueueDelay, initInterArrival)
    {

        /* Initialize the transition model */
        initForecastModel();

        KF::Matrix F = KF::Matrix::Identity(KF::DIM, KF::DIM);
        F.row(iModel) = forecastModel.transpose(); //Slightly unsure if this would work soshould confirm

        /* Note: Uncertainty Matrix is initialized to identity in KF class - No need to set it */

        state.setF(F);

        /* Initialize the noise covariances*/
        initQ();
        initR();

    }

    void forecast(uint8_t tick_number)
    {

        for (int i=0; i < tick_number; i++)
        {

            state.predict(Q);

            if (bytes_to_be_drained.size() == 0)
            {

                bytes_to_be_drained.push_back(getForecastedBytes());

            }
            else
            {

                uint64_t cum_drained = *(bytes_to_be_drained.rbegin()) + getForecastedBytes();

                bytes_to_be_drained.push_back(cum_drained);

            }

        }

    }

    void correctForecast(KF::Vector observed)
    {

        /* Ensure we convert the packets seen into Mbits/s as we are working with that */
        //double packets_received = observed(KF::iBand, 1);
        //observed(KF::iBand, 1) = PacketCollector::to_bits_per_sec(packets_received);

        state.update(observed, R);

    }

    uint64_t getForecastedBytes()
    {

        /* Stored in Mbits/s */
        double rate = state.mean()(KF::iBand);

        uint64_t bytes = ((ms_per_sec * 1000) * rate) / bits;

        return bytes;

    }

    std::list<uint64_t> getBytesToBeDrained()
    {
        return bytes_to_be_drained;
    }

    void clearForecast()
    {
        bytes_to_be_drained.clear();
    }

    void setQ(KF::Matrix newQ)
    {

        Q = newQ;

    }

    void setR(KF::Matrix newR)
    {

        R = newR;

    }

private:

    KF state;

    /* Noise covariances */
    KF::Matrix Q;   /* System Noise Covariance */
    KF::Matrix R;   /* Observation Noise Covariance */

    /* Forecast Model that we have learnt offline */
    KF::Vector forecastModel;

    std::list<uint64_t> bytes_to_be_drained;


    /* Initialization functions */
    void initForecastModel()
    {

        double COLUMN = 0; /* Vector will just have 1 column */

        uint8_t SIZE = 4;

        /* Will hold weights corresponding to bias, rtt gradient, queuing delay and inter arrival time */
        double params[SIZE] = {1.9736, 0.27786, -0.019446, -0.13415163};

        for (int i = 0; i < SIZE; i++)
        {

            forecastModel(i, COLUMN) = params[i];

        }

    }

    void initQ()
    {

        /* For now will assume everything else has noise 0 */
        Q(KF::iBand, KF::iBand) = 0.1;

        /* Note: Should check that the other entries are 0 by default */

    }

    void initR()
    {

        //R(KF::iBand, KF::iBand) = 2;

        R << 0.044, -0.033, 0.243, -0.025, 0.0109, 
        0.158, 1.909, -0.059, 0.0187, -0.0329,
        -0.1192, -0.0204, -0.0358, 0.06034, -0.1664,
        -0.0700, 0.1151, 0.1857, -0.1511, 0.0644,
        -0.098, -0.0856, -0.0871, -0.0422, 0.0996;


    }


};