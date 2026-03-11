#ifndef FILE_H

#include <string>
#include <fstream>

struct FileFilter
{
    std::string name;
    std::string extension;
};

std::string OpenFileDialog(FileFilter *filters, int numOfFilters);
std::string SaveFileDialog(FileFilter *filters, int numOfFilters);

#define FILE_H
#endif