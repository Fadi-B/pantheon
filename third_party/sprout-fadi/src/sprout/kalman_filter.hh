#pragma once

#include <Eigen/Dense>
#include "assert.h"
#include <stdio.h>

class KF
{

public:

    static const int iBias = 0;
    static const int iBand = 1;
    static const int iRTTG = 2;
    static const int iQueueDelay = 3;
    static const int iInterArrival = 4;

    /* State will be [bias, Throughput, RTT Grad, Queue Delay, Inter Arrival]*/
    static const int DIM = 5;

    typedef Eigen::Matrix<double, DIM, 1> Vector;
    typedef Eigen::Matrix<double, DIM, DIM> Matrix;

    KF(double initBandwidth, double initRTTGrad, double initQueueDelay, double initInterArrival)
    {

        /* State Initialization */
	    _mean(iBias) = 1;	   //Will always be 1
        _mean(iBand) = initBandwidth; //
        _mean(iRTTG) = initRTTGrad;
        _mean(iQueueDelay) = initQueueDelay;
	    _mean(iInterArrival) = initInterArrival;

        _cov.setIdentity();

        /* Transition Matrix Initialization */
        F = Matrix::Identity(DIM, DIM);

        /* Noiseless connection between measurement and state initialization */
        H = Matrix::Identity(DIM, DIM); // For now we assume a unity connection
    }

    void predict(Matrix Q)
    {

	    /* Checking correct dimensions */
	    //assert(  );

        const Vector new_mean = F * _mean;

        const Matrix newP = F * _cov * F.transpose() + Q;

        _cov = newP;
        _mean = new_mean;

    }


    void update(Vector measurement, Matrix measurementVar)
    {

        const Vector y = innovation(measurement);

        const Matrix K = kalman_gain(measurementVar);

        Vector new_mean = _mean + K * y;
        Matrix new_cov = (Matrix::Identity() - K * H) * _cov;

	    //new_mean[iBias] = 1; // Bias

        /*
	    // Max function does not seem to work so will be doing it manually
	    double throughput = measurement[iBand];

	    if (throughput < 0)
    	{
	      new_mean[iBand] = 0;
	    }
	    else
	    {
	    new_mean[iBand] = throughput;
	    }

	    new_mean[iRTTG] = measurement[iRTTG]; // RTT Gradient
	    new_mean[iQueueDelay] = measurement[iQueueDelay]; // Queuing Delay
	    new_mean[iInterArrival] = measurement[iInterArrival]; // Inter arrival time
        */
        _cov = new_cov;
        _mean = new_mean;

        return;

    }

    Vector innovation(Vector measurement)
    {
        Vector innov = measurement - H * _mean;

        return innov;
    }

    Matrix innovation_cov(Matrix R)
    {
        Matrix S = H * _cov * H.transpose() + R;

        return S;
    }

    Matrix kalman_gain(Matrix R)
    {
        Matrix S = innovation_cov(R);

        //printf("SIZE: %d\n", (_cov * H.transpose()).size());
        Matrix K = _cov * H.transpose() * S.inverse();

        return K;
    }

    void setF(Matrix newF)
    {

        F = newF;

    }

    void setCov(Matrix newCov)
    {
        _cov = newCov;
    }

    void reset()
    {
        //TO DO
        return;

    }


    Matrix cov() const
    {
        return _cov;
    }

    Vector mean() const
    {
        return _mean;
    }

    double bandwidth() const
    {
        return _mean(iBand);
    }

    double RTTGrad() const
    {
        return _mean(iRTTG);
    }

    double delay() const
    {
        return _mean(iQueueDelay);
    }
 
private:

    /* State Space Representation */
    Vector _mean;
    Matrix _cov;

    /* Transition Matrix */
    Matrix F;

    /* Noiseless connection between measurement and state */
    Matrix H;

};
