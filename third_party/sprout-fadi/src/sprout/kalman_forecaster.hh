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


    //using KF::DIM;
    //using KF::Matrix;

    static const uint16_t bits = 1500 * 8;
    static const uint16_t ms_per_sec = 60; /* We should be careful with this one as we are forecasting 20ms into the future*/
    static const uint8_t iModel = 1;

    static const uint16_t NUM_TICKS = 8;


    KFForecaster(double initBandwidth, double initRTTGrad, double initQueueDelay, double initInterArrival)
    :state(initBandwidth, initRTTGrad, initQueueDelay, initInterArrival)
    {

        /* Initialize the transition model */
        initForecastModel();

        KF::Matrix F = KF::Matrix::Identity(KF::DIM, KF::DIM);
        F.row(iModel) = forecastModel.transpose(); //Slightly unsure if this would work so should confirm

        /* Note: Uncertainty Matrix is initialized to identity in KF class - No need to set it */

        state.setF(F);

        /* Initialize the noise covariances*/
        initQ();
        initR();

    }

    /* For debugging */
    void showdata (double *data,  int n) {
  	int i; 

	if (1) {
    	fprintf (stderr, "Bytes to be drained");
    	for (i=0; i<n; i++ ){
    	 fprintf (stderr, " %f", data[i]);
    	}
   	 fprintf(stderr, "\n");
  	}
    }

    void forecast(uint8_t tick_number)
    {

        for (int i=0; i < tick_number; i++)
        {

	    //fprintf(stderr, "Given Number: %d \n", tick_number);

            state.predict(Q);

            //fprintf(stderr, "Forecast Prediction: %f \n", state.mean()(KF::iBand));

            if (i == 0)
            {

                //fprintf(stderr, "Forecast Prediction: %f \n", state.mean()(KF::iBand));

                bytes_to_be_drained[i] = getForecastedBytes();

            }
            else
            {

                //double cum_drained = *(bytes_to_be_drained.rbegin()) + getForecastedBytes();

                bytes_to_be_drained[i] = bytes_to_be_drained[i - 1] + getForecastedBytes();

            }

        }

      //clearForecast();
      //showdata(bytes_to_be_drained, 8);

    }

    void correctForecast(CollectorManager::Matrix observed)
    {

        Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");

        Eigen::Matrix<double, KF::DIM, 1> obs; //= observed.row(0).transpose();

//        std::cerr << "\n OBS-PRE \n" << obs.format(CleanFmt) << "\n";

      	for (int i = 0; i < KF::HISTORY_SIZE; i++)
        {
	  //108
          Eigen::Matrix<double, 1, KF::STATE_SIZE> data = observed.row(i);


          for (int j = 0; j < KF::STATE_SIZE; j++)
          {

	    obs(i*KF::STATE_SIZE + j, 0) = data(0, j);

          }

        }

       std::cerr << "\n OBS-Post \n" << obs.format(CleanFmt) << "\n";

        double b = ((ms_per_sec * 1000) * observed(KF::iBand, 0))/bits;

//        fprintf(stderr, "Correct Obs: %f \n", b/*observed(KF::iBand, 0)*/);
//        fprintf(stderr, "Current Mean: %f \n", state.mean()(KF::iBand));
	fprintf(stderr, "Correcting \n");
        state.update(obs, R);

    }

    double getForecastedBytes()
    {

	double uncertainty = state.cov()(KF::iBand, KF::iBand);
	double stddev = std::sqrt(uncertainty);

	//fprintf(stderr, "Stddev: %f \n", stddev);

        /* Stored in Mbits/s */
        double rate = state.mean()(KF::iBand); //- 2*stddev;

        double bytes =  ((ms_per_sec * 1000) * rate) / bits;

        return bytes;

    }

    double * getBytesToBeDrained()
    {
        return bytes_to_be_drained;
    }

    void clearForecast()
    {

        for (int i = 0; i < NUM_TICKS; i++)

	    {

	        bytes_to_be_drained[i] = 0;

	    }

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

    double bytes_to_be_drained[NUM_TICKS];


    /* Initialization functions */
    void initForecastModel()
    {

        double COLUMN = 0; /* Vector will just have 1 column */

        /* Will hold weights corresponding to bias, rtt gradient, queuing delay and inter arrival time */
        double params[KF::DIM] = {0.01622525, 1, -0.004615, -0.000017466, -0.0011150}; //TMobile UMTS

	//double params[KF::DIM] = {0.40928865, 1, -0.3889949 , -0.0033164 , -0.01104911}; //TMobile-LTE

        for (int i = 0; i < KF::DIM; i++)
        {

            forecastModel(i, COLUMN) = params[i];

        }

    }

    void initQ()
    {

	Q.setZero();

        /* For now will assume everything else has noise 0 */
        Q(KF::iBand, KF::iBand) = 0.21;

        /* Note: Should check that the other entries are 0 by default */

    }

    void initR()
    {

	R.setZero();

        //R(KF::iBand, KF::iBand) = 2;

        R(KF::iBand, KF::iBand) = 0.35873197;


    }


};

