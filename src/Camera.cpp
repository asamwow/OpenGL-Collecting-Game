#include "Camera.h"
#define MAXDEPTH -0.5
#define MINDEPTH -20

glm::mat4 Camera::process(double frametime, int mousex, int mousey,
                            int width, int height)
{
    float speed = 0;
    float lateralSpeed = 0;
    float yawAngle = 0;
    float pitchAngle = 0;
    float lookSensitivity = 0.08;

    if (w == 1)
        speed = 5 * frametime;
    else if (s == 1)
        speed = -5 * frametime;
    if(a == 1)
        lateralSpeed = -5 * frametime;
    else if(d == 1)
        lateralSpeed = 5 * frametime;

    if ((height - 4.0 * mousey) < 0 && targetPitch.x < 3.14159 / 2.0)
        pitchAngle += lookSensitivity * frametime * (height - 4.0 * mousey);
    else if ((height - 4.0 * mousey) > 0 && targetPitch.x > -3.14159 / 2.0)
        pitchAngle += lookSensitivity * frametime * (height - 4.0 * mousey);

    yawAngle = lookSensitivity * frametime * (width - 4.0 * mousex);
   
    targetYaw.y -= yawAngle; 
    targetPitch.x -= pitchAngle;

    if(targetPitch.x < -3.14159 / 2.0)
        targetPitch.x = -3.14159 / 2.0;
    else if(targetPitch.x > 3.14159 / 2.0)
        targetPitch.x = 3.14159 / 2.0;

    yaw += -0.1f * yaw + 0.1f * targetYaw;
    pitch += -0.1f * pitch + 0.1f * targetPitch;

    glm::mat4 Ry = glm::rotate(glm::mat4(1), yaw.y, glm::vec3(0, 1, 0));
    glm::mat4 Rx = glm::rotate(glm::mat4(1), pitch.x, glm::vec3(1, 0, 0));
    glm::vec4 dir = glm::vec4(0, 0, speed, 1);

    dir = dir * Rx * Ry;
    
    glm::vec3 lateralDir = glm::vec3(glm::vec4(0,0, lateralSpeed, 1) * 
                glm::rotate(glm::mat4(1), 3.14159f / 2.0f, glm::vec3(0,1,0))
                           *Ry);
    targetPos += lateralDir;
    targetPos += glm::vec3(dir.x, dir.y, dir.z);
    position += -0.1f * position + 0.1f * targetPos;

    if (targetPos.y > MAXDEPTH)
        targetPos.y = MAXDEPTH;
    else if (targetPos.y < MINDEPTH)
        targetPos.y = MINDEPTH;
    
    glm::mat4 T = glm::translate(glm::mat4(1), position);
    return Rx * Ry * T;
}