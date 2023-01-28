#pragma once


#include <Eigen/Dense>

class KF
{

public:

    static const int iB = 0;
    static const int iRTTG = 1;
    static const int iD = 2;

    static const int DIM = 3;

    typedef Eigen::Matrix<double, DIM, 1> Vector;
    typedef Eigen::Matrix<double, DIM, DIM> Matrix;

    KF(double initBandwidth, double initRTTGrad, double initDelay)
    {

        /* State Initialization */
        _mean(iB) = initBandwidth;
        _mean(iRTTG) = initRTTGrad;
        _mean(iD) = initDelay;

        _cov.setIdentity();

        /* Transition Matrix Initialization */
        F = Matrix::Identity(DIM, DIM);

        /* Noiseless connection between measurement and state initialization */
        H = Matrix::Constant(1); // SIZE: 1 x 3
    }

    void predict(double dt)
    {

        const Vector new_mean = F * _mean;

        const Matrix newP = F * _cov * F.transpose();

        _cov = newP;
        _mean = new_mean;

    }


    void update(Vector measurement, Matrix measurementVar)
    {

        const Vector y = innovation(measurement);

        const Matrix K = kalman_gain(measurementVar);

        Vector new_mean = _mean + K * y;


        Matrix new_cov = (Matrix::Identity() - K * H) * _cov;

        _cov = new_cov;
        _mean = new_mean;

        return;

    }



    Vector innovation(Vector measurement)
    {
        Vector innov = measurement - H * _mean;

        return innov; //innov;
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
        return _mean(iB);
    }

    double RTTGrad() const
    {
        return _mean(iRTTG);
    }

    double delay() const
    {
        return _mean(iD);
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