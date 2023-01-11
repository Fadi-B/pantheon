#include <iostream>
#include <list>
#include <string>
#include <fstream>


void write_to_file(std::string filename, std::list<double> &data) 
{

    std::fstream file;

    /* This will by default delete the current content of the file */
    file.open(filename, std::ios_base::out);

    for (auto it = data.begin(); it != data.end(); it++) {

        auto obj = *it;
        file<<obj<<std::endl;

    }

    file.close();

    return;

}