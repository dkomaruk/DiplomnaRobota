#ifndef FILE_H

#include <string>
#include <fstream>

#include <windows.h>
#include <commdlg.h>

struct FileFilter
{
    std::string name;
    std::string extension;
};

std::string OpenFileDialog(FileFilter *filters, int numOfFilters);
std::string SaveFileDialog(FileFilter *filters, int numOfFilters);

#define FILE_H
#endif