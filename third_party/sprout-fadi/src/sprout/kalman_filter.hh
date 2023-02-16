#pragma once

#include <Eigen/Dense>
#include "assert.h"
#include <stdio.h>

//#include "noise_generator.hh"

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

	 //Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
	 //std::cerr << "\n F \n" << F.format(CleanFmt);

	    /* Checking correct dimensions */
	    //assert(  );
        //fprintf(stderr, "Pre Pred: %f \n", _mean(iBand, 0));
        const Vector new_mean = F * _mean;
        //new_mean(iBand, 0) = 1;
        const Matrix newP = F * _cov * F.transpose() + Q;

        _cov = newP;
        _mean = new_mean;

    }


    void update(Vector measurement, Matrix measurementVar)
    {

	Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");

	//fprintf(stderr, "Meas Update: %f \n", measurement(iBand, 0));
        //fprintf(stderr, "Pre Update: %f \n", _mean(iBand, 0));

	//std::cerr << "\n Pre Update \n" << _mean.format(CleanFmt);

        const Vector y = innovation(measurement);

	//fprintf(stderr, "Innovation: %f \n", y(iBand, 0));

        const Matrix K = kalman_gain(measurementVar);

	//fprintf(stderr, "Kalman Gain: %f \n", K(iBand, 0));

	std::cerr << "\n Pre-Cov \n" << _cov.format(CleanFmt) << "\n";

        Vector new_mean = _mean + K * y;
        Matrix new_cov = (Matrix::Identity() - K * H) * _cov;

	std::cerr << "\n Kalman Gain \n" << K.format(CleanFmt);
	std::cerr << "\n Post-Cov \n" << new_cov.format(CleanFmt) << "\n";

        //fprintf(stderr, "Post Update: %f \n", _mean(iBand, 0));

	new_mean[iBias] = 1; // Bias

	// Max function does not seem to work so will be doing it manually
	double throughput = new_mean[iBand];

	if (throughput < 0)
	{
	  new_mean[iBand] = 0;
	}

	new_mean[iRTTG] = measurement[iRTTG]; 		  	// RTT Gradient
	new_mean[iQueueDelay] = measurement[iQueueDelay];	// Queuing Delay
	new_mean[iInterArrival] = measurement[iInterArrival];   // Inter arrival time

        _cov = new_cov;
        _mean = new_mean;

	//std::cerr << "\n New Mean \n" << _mean.format(CleanFmt);

        return;

    }

    Vector innovation(Vector measurement)
    {
        Vector innov = measurement - H * _mean;

	Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
        //std::string sep = "\n----------------------------------------\n";

        //std::cerr << "\n Innov \n" << innov.format(CleanFmt);
        //std::cerr << "\n Meas. \n" << measurement.format(CleanFmt);
	//std::cerr << "\n Mean \n" << _mean.format(CleanFmt);

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
/*
	Matrix N;

	for (int i = 0; i < DIM; i++)
	{

	  for (int j = 0; j < DIM; j++)
	  {

	    double noise = (*gen).get_noise(generator);

	    N(i, j) = noise;

	  }

	}

	// Add small noise to ensure invertible
	S = S + N;
*/
        //printf("SIZE: %d\n", (_cov * H.transpose()).size());

	/* We will compute the kalman gain without explicitly
	 * inverting the S matrix as this can produce issues if S is not fully invertible
	 * due to numerical precision etc.
	 *
	 * Will solve for x in S.T*x = H*P.T as this will give K.T
	 *
	 * From which K can be easily obtained by transposing the result
	 *
	 */
	Matrix A = S.transpose();

	Matrix x = A.colPivHouseholderQr().solve(H*(_cov.transpose())); // computes A^-1 * b
	Matrix K_solv = x.transpose();

        //Matrix K_solv = _cov * H.transpose() * S.inverse();

	Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
        //std::string sep = "\n----------------------------------------\n";

        //std::cerr << "\n S \n" << S.format(CleanFmt);
	//std::cerr << "\n S Inv \n" << S.inverse().format(CleanFmt);
        //std::cerr << "\n Cov. \n" << _cov.format(CleanFmt);
        //std::cerr << "\n K \n" << K.format(CleanFmt);
	//std::cerr << "\n K Solv \n" << K_solv.format(CleanFmt);

        return K_solv;
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

/* Creating the Gaussian Noise generator */
    //NoiseGenerator * gen = new NoiseGenerator(0, 0, 0.01); //seed, mean, std
    //NoiseGenerator::Generator generator = (*gen).get_generator();

};
