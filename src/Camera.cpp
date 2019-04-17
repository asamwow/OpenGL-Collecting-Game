class Camera
{
public:
    glm::vec3 position, yaw, pitch, targetPos, targetYaw, targetPitch;
    int w, a, s, d, r, f;
    camera()
    {
        w = a = s = d = r = f = 0;
        position = yaw = pitch = glm::vec3(0, 0, 0);
        targetPos = targetYaw = targetPitch = glm::vec3(0, 0, 0);
    }
    glm::mat4 process(double frametime)
    {
        float speed = 0;
        float yangle=0;
        float xangle=0;
        
        if (w == 1)
            speed = 5*frametime;
        else if (s == 1)
            speed = -5*frametime;

        if (a == 1)
            yangle = -2*frametime;
        else if(d==1)
            yangle = 2*frametime;
        if (r == 1 && targetPitch.x > -3.14159 / 2.0)
            xangle = -2*frametime;
        else if(f == 1 && targetPitch.x < 3.14159 / 2.0)
            xangle = 2*frametime;
        
        targetYaw.y += yangle;
        targetPitch.x += xangle;
        
        yaw += -0.02f * yaw + 0.02f * targetYaw;
        pitch += -0.02f * pitch + 0.02f * targetPitch;

        glm::mat4 Ry = glm::rotate(glm::mat4(1), yaw.y, glm::vec3(0, 1, 0));
        glm::mat4 Rx = glm::rotate(glm::mat4(1), pitch.x, glm::vec3(1, 0, 0));
        glm::vec4 dir = glm::vec4(0, 0, speed,1);
        
        dir = dir*Rx*Ry;
        
        targetPos += glm::vec3(dir.x, dir.y, dir.z);
        position += -0.02f * position + 0.02f * targetPos;
        
        if(targetPos.y > MAXDEPTH)
            targetPos.y = MAXDEPTH;
        else if(targetPos.y < MINDEPTH)
            targetPos.y = MINDEPTH;
        glm::mat4 T = glm::translate(glm::mat4(1), position);
        return Rx*Ry*T;
    }
};