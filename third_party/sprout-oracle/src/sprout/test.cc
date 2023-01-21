#include "filewriter.hh"


int main() 
{

    std::list<double> li;

    li.push_back(2.3);
    li.push_back(4.5);

    write_to_file("fadi.txt", li);

    return 0;

}