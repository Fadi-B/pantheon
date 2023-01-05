#include <time.h>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>

class NoiseGenerator
{

public:

    static const boost::variate_generator<boost::mt19937, boost::normal_distribution<>> generator;

    NoiseGenerator(const double noise_std)
    {

        generator(boost::mt19937(time(0)), boost::normal_distribution<>(0, noise_std));
       
    }

    double get_noise()
    {
        return generator();
    }


private:

    const double noise_std;

};