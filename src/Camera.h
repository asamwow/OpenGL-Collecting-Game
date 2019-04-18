#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

class Camera
{
public:
    glm::vec3 position, yaw, pitch, targetPos, targetYaw, targetPitch;
    int w, a, s, d, r, f;
    Camera()
    {
        w = a = s = d = r = f = 0;
        position = yaw = pitch = glm::vec3(0, 0, 0);
        targetPos = targetYaw = targetPitch = glm::vec3(0, 0, 0);
    }
    glm::mat4 process(double frametime);
};