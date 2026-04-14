#ifndef DEBUG_H

#include <string>

inline void StartProfiling(bool reset = false);
inline void EndProfiling(const std::string& extraInfo = "");

#define DEBUG_H
#endif