#ifndef DEBUG_H

#include <string>

inline void StartProfiling(bool reset = false);
inline void EndProfiling(const std::string& extraInfo = "");
void SaveFramebufferContents(Framebuffer fbo);

void CreateShadowVolumeLines(Line *lines, GLuint shader);
void UpdateShadowVolumeLines(Line *lines, glm::mat4 lightViewProj);

#define DEBUG_H
#endif