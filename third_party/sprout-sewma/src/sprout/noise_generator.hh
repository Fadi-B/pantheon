#include <time.h>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>

/**
 * @brief This class generated gaussian noise
 * 
 */
class NoiseGenerator
{

public:

    /* typedef for the variate_generator object that will be used */
    typedef boost::variate_generator<boost::mt19937, boost::normal_distribution<>> Generator;

    /**
     * @brief This is the constructor and will initialize relevant variables
     * 
     *        Note: The seed argument is currently not used as we want to ensure
     *              true randomness (even in between runs) so we use time(0) as
     *              the seed
     * 
     * @param seed - The seed for the generator object
     * @param mean - The mean of the gaussian distribution
     * @param std  - The standard deviation of the gaussian distribution
     */
    NoiseGenerator(int seed, double mean, double std)
    {

     _seed = time(0); /* Using time(0) to ensure randomness even during restart */
     _mean = mean;
     _std = std;

    }

    /**
     * @brief This instantiates a generator object
     * 
     *        Note: The same generator object should be used throughout
     *              for each experiement as otherwise you will not get random numbers
     * 
     * @return Generator object
     */
    Generator get_generator()
    {

     return  Generator(boost::mt19937(_seed),
               boost::normal_distribution<>());

    }

    /**
     * @brief This gets the gaussian noise generated
     * 
     * @param noise_generator - The reference to the Generator object
     * @return double - the gaussian noise
     */
    double get_noise(Generator &noise_generator)
    {

      double noise = _std*noise_generator() + _mean;

      return noise;

    }


private:

  int _seed;
  double _mean;
  double _std;

};
