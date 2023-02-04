#include  <iostream>
#include  <fstream>
#include  <string>
#include <list>


using namespace std;

std::list<uint64_t> read_file_int(string filename)
{

  std::list< uint64_t > data;

  string line;
  int value;
  ifstream file (filename);

  if(file.is_open())
  {

     while(!file.eof())
     {

        getline(file, line);

        try
        {

          value = stoi(line);

        }
        catch (std::invalid_argument const& ex)
        {

        }

        //cout << line << endl;

        //fprintf(stderr, " \n");

        data.push_back( (uint64_t) value );

     }

        file.close();

  }

  return data;

}

std::list<double> read_file_double(string filename)
{

  std::list< double > data;

  string line;
  double value;
  ifstream file (filename);

  if(file.is_open())
  {

     while(!file.eof())
     {

        getline(file, line);

        try
        {

          value = stod(line);

        }
        catch (std::invalid_argument const& ex)
        {

        }

        //cout << line << endl;

        //fprintf(stderr, " \n");

        data.push_back( (double) value );

     }

        file.close();

  }

  return data;

}
