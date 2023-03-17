#pragma once

#include <Eigen/Dense>
#include <stdio.h>
#include "assert.h"
#include "kalman_filter.hh"

#include "packet_collector.hh"

#include <fdeep/fdeep.hpp>

#include <list>

class KFForecaster
{

public:


    //using KF::DIM;
    //using KF::Matrix;

    static const uint16_t bits = PacketCollector::MSS * PacketCollector::BYTE_SIZE;
    static const uint16_t ms_per_sec = 60; /* We should be careful with this one as we are forecasting 20ms into the future*/
    static const uint8_t iModel = 1;

    static const uint16_t NUM_TICKS = 4;


    KFForecaster(double initBandwidth, double initRTTGrad, double initQueueDelay, double initInterArrival)
    :state(initBandwidth, initRTTGrad, initQueueDelay, initInterArrival)
    {

          /* Initialize the transition model */
       	  initForecastModel();

          KF::Matrix F = KF::Matrix::Identity(KF::DIM, KF::DIM);
//        F.row(iModel) = forecastModel.transpose(); //Slightly unsure if this would work so should confirm

          /* Note: Uncertainty Matrix is initialized to identity in KF class - No need to set it */

          state.setF(F);

          /* Initialize the noise covariances*/
          initQ();
          initR();

        _start_time_point = chrono::high_resolution_clock::now();

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

        KF::Matrix F = KF::Matrix::Identity(KF::DIM, KF::DIM);

        double now = CollectorManager::getCurrentTime(_start_time_point);

        for (int i=0; i < tick_number; i++)
        {

//	    fprintf(stderr, "PRE: %f \n", state.mean()(KF::iBand));

           if (now > 3000)
           {

            F.row(iModel) = forecastModel.row(i);
            state.setF(F);

           }
            state.predict(Q);

//            state.mean()(KF::iBand) = 3.4;

//            fprintf(stderr, "Post: %f \n", state.mean()(KF::iBand));

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

       //Will not be changed anymore
       start = true;

      //clearForecast();
      //showdata(bytes_to_be_drained, 8);

    }

    void correctForecast(CollectorManager::Matrix observed)
    {

        Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");

        Eigen::Matrix<double, KF::DIM, 1> obs; //= observed.row(0).transpose();

//        std::cerr << "\n Measurement \n" << observed.format(CleanFmt) << "\n";

      	for (int i = 0; i < KF::HISTORY_SIZE; i++)
        {
	  //108
          Eigen::Matrix<double, 1, KF::STATE_SIZE> data = observed.row(i);


          for (int j = 0; j < KF::STATE_SIZE; j++)
          {

	    // Note: The +1 is to make sure we do not override bias
	    obs(i*KF::STATE_SIZE + j + 1, 0) = data(0, j);

          }

        }

       // Adding the bias
       obs(0,0) = 1;

  //     std::cerr << "\n OBS-Post \n" << obs.format(CleanFmt) << "\n";

        double b = ((ms_per_sec * 1000) * obs(KF::iBand, 0))/bits;

//       fprintf(stderr, "Correct Obs: %f \n", b);
//        fprintf(stderr, "Current Mean: %f \n", state.mean()(KF::iBand));
//	fprintf(stderr, "Correcting \n");
        state.update(obs, R);

    }

    double getForecastedBytes()
    {

	double uncertainty = state.cov()(KF::iBand, KF::iBand);
	double stddev = std::sqrt(uncertainty);

	//fprintf(stderr, "Stddev: %f \n", stddev);

        /* Stored in Mbits/s */
        double rate = state.mean()(KF::iBand); //- 2*stddev;

//        fprintf(stderr, "RATE: %f \n", rate);

        double bytes =  ((ms_per_sec * 1000) * rate) / bits;

//        fprintf(stderr, "Bytes: %f \n", bytes);

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

    KF getState()
    {
	return state;
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

    //To avoid odd behavior during start up
    bool start = false;

    std::chrono::high_resolution_clock::time_point _start_time_point;

    KF state;

    /* Noise covariances */
    KF::Matrix Q;   /* System Noise Covariance */
    KF::Matrix R;   /* Observation Noise Covariance */

    /* Forecast Model that we have learnt offline */
    Eigen::Matrix<double, 8, KF::DIM> forecastModel;

    double bytes_to_be_drained[NUM_TICKS];


    /* Initialization functions */
    void initForecastModel()
    {

        /* Note: It seems that removing terms and relearning a new model based on those performs roughly the same 
         * Most likely because model learn compensates for the previous things - might indicate that they all tell us the same thing?
         */

        double COLUMN = 0; /* Vector will just have 1 column */

        /* Will hold weights corresponding to bias, rtt gradient, queuing delay and inter arrival time */
        //double params[KF::DIM] = {-0.00120897, 1, -0.0000451813877, -0.000013251127, 0.00024678328}; //TMobile UMTS


        //double params[KF::DIM] = {0,1,0,0,0};


        //double params[8][KF::DIM] = {{0.05902253, 1, 0.00023383, -0.0010024 ,  0.00124224},
        //{-0.00639942, 1, 2.15790548e-04, 4.18576614e-05, 1.67243052e-04}, 
        //{0.01497034, 1, -0.00042156, -0.00023359,  0.00010058}, 
        //{0.01890435, 1, 5.79744307e-04, -2.43326647e-04, -6.67425321e-05}, 
        //{-0.03524784, 1, -0.00259395,  0.00034825,  0.00032649}, 
        //{-0.01470577, 1, -2.36631317e-03,  6.08377038e-05,  4.86204102e-04},
        //{0.01216091, 1, 0.00244498, -0.00011558, -0.0001529},
        //{0.01307779, 1, 2.41631104e-03, -1.45166175e-04, -5.37754775e-05}};


        double params[8][KF::DIM] = {{-0.01635259, 1, 7.62239417e-04, 7.06147579e-05, 7.52978888e-04},
        {0.02469452, 1, -6.90801984e-04, -8.74495950e-05, -1.09508919e-03},
        {-0.01206305, 1, 4.56056864e-04, 5.62361262e-05, 4.94337408e-04},
        {0.00396807, 1, 6.38296570e-05, -1.07335821e-05, -2.71770445e-05},
        {-3.27033924e-06, 1, -7.60925365e-04, -7.60435763e-07, -4.49435499e-04},
        {0.0195256, 1,0.00094799, -0.00010472, -0.00012913},
        {-2.88735809e-06, 1, -6.91335377e-04, -1.58784203e-05, -2.35765707e-04},
        {0.00452315, 1, 3.54427466e-04,  8.22688359e-05, -5.14540091e-04}};

        for (int i = 0; i < 8; i++) 
        {

        for (int j = 0; j < KF::DIM; j++)
        {

            forecastModel(i, j) = params[i][j];

        }

      }

    }

    void initQ()
    {

	Q.setZero();

        /* For now will assume everything else has noise 0 */
        Q(KF::iBand, KF::iBand) = 0.21;//0.21;

        /* Note: Should check that the other entries are 0 by default */

    }

    void initR()
    {

	R.setZero();

        //R(KF::iBand, KF::iBand) = 2;

        R(KF::iBand, KF::iBand) = 0.36;


    }


};

