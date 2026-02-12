#include "file.h"

#include <vector>

char *GetFileFilters(FileFilter *filters, int numOfFilters)
{
    std::string filtersStr = "";
    for(int filterIndex = 0; filterIndex < numOfFilters; filterIndex++)
    {
        filtersStr += filters[filterIndex].name + '\0' + filters[filterIndex].extension + '\0';
    }
    filtersStr += "All Files\0*.*\0";
    return (char *)filtersStr.c_str();
}

std::string OpenFileDialog(FileFilter *filters, int numOfFilters)
{
    char fileName[MAX_PATH] = "";

    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = GetFileFilters(filters, numOfFilters);
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if(GetOpenFileNameA(&ofn))
        return std::string(fileName);
    return "";
}

std::string SaveFileDialog(FileFilter *filters, int numOfFilters)
{
    char fileName[MAX_PATH] = "";

    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = GetFileFilters(filters, numOfFilters);
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if(GetSaveFileNameA(&ofn))
        return std::string(fileName);
    return "";
}