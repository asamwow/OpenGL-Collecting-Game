#ifdef __APPLE__
#include "MacIncludeGL.h"
#endif
#ifdef __linux__
#include "LinuxIncludeGL.h"
#endif

class Camera
{
public:
    glm::vec3 position, yaw, pitch, targetPos, targetYaw, targetPitch;
    int w, a, s, d, r, f;
    Camera()
    {
        w = a = s = d = r = f = 0;
        yaw = pitch = glm::vec3(0, 0, 0);
        position = glm::vec3(0, 0, 0);
        targetPos = targetYaw = targetPitch = glm::vec3(0, 0, 0);
    }
    glm::mat4 process(double frametime, int mousex, int mousey,
                        int width, int height);
};