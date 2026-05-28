#ifndef DEBUG_H

#include <string>

inline void StartProfiling(bool reset = false);
inline void EndProfiling(const std::string& extraInfo = "");
void SaveFramebufferContents(Framebuffer fbo);

#define DEBUG_H
#endif