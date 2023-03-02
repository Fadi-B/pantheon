#include <iostream>
#include <list>
#include <string>
#include <fstream>

class FileWriter
{

public:

    FileWriter()
    {

      count = 0;

    }

    void write_to_file(std::string filename, std::list<double> &data)
    {

        std::fstream file;

        /* This will by default delete the current content of the file */
//        file.open(filename, std::ios_base::app);
        file.open(filename, std::ios_base::out);

        auto it = data.begin();

	//std::advance(it, count);

        for (it; it != data.end(); it++) {

            auto obj = *it;
            file<<obj<<std::endl;

            //count = count + 1;

        }

        file.close();

        return;

    }

private:

  int count = 0;

};

