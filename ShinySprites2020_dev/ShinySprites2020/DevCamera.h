#ifndef DEVCAMERA_H
#define DEVCAMERA_H

#include "pch.h"



enum class CAM_DIRECTION 
{
    CAM_FORWARD,
    CAM_BACK,
    CAM_LEFT,
    CAM_RIGHT
};




class DevCamera
{
public:

    DevCamera(glm::vec3 initialPosition) : 
        m_front(glm::vec3(-4.0f, 0.0f, -1.0f))
    ,   m_cameraUp(glm::vec3(0.0f, 1.0f, 0.0f))
    ,   m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f))
    ,   m_cameraPos(initialPosition)
    ,   m_fovy(glm::radians(45.f))
    ,   m_yaw(glm::radians(-90.f))
    ,   m_pitch(glm::radians(0.f))
    ,   m_speed(2.5f) 
    {
        update();
    }






    void yawCamera(int p_i, float deltaTime)
    {
        m_yaw += p_i * deltaTime * 10.f * XM_PI / 180.f;

        update(); 
    }



    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(m_cameraPos, m_cameraPos + m_front, m_cameraUp);
    }





    void ProcessKeyboard(CAM_DIRECTION direction, float deltaTime) // TODO:  add page up page down;
    {
        float velocity = m_speed * deltaTime;

        if (direction == CAM_DIRECTION::CAM_FORWARD)
            m_cameraPos += m_front * velocity;

        if (direction == CAM_DIRECTION::CAM_BACK)
            m_cameraPos -= m_front * velocity;

        if (direction == CAM_DIRECTION::CAM_LEFT)
            m_cameraPos -= m_cameraRight * velocity;

        if (direction == CAM_DIRECTION::CAM_RIGHT)
            m_cameraPos += m_cameraRight * velocity;
    }



    void ProcessMouseScroll(float yoffset)
    {
        if ((m_cameraPos.z <= 5.f) || (m_cameraPos.z >= 15.f))
        {
            return; // limit to range 15 to 5; 
        }

        m_cameraPos -= yoffset * m_front; 

        std::cout << "depth = " << lrintf(m_cameraPos.z) << std::endl;
    }

public:
    glm::vec3 m_cameraPos;
    glm::vec3 m_front;
    glm::vec3 m_cameraUp;
    glm::vec3 m_cameraRight;
    glm::vec3 m_worldUp;
    float m_yaw;
    float m_pitch;
    float m_speed;
    float m_fovy;

private:

    void update()
    {
        glm::vec3 tmpFront;
        tmpFront.x = cos(m_yaw) * cos(m_pitch);
        tmpFront.y = sin(m_pitch);
        tmpFront.z = sin(m_yaw) * cos(m_pitch);

        m_front = glm::normalize(tmpFront);

        m_cameraRight = glm::normalize(glm::cross(m_front, m_worldUp));  

        m_cameraUp    = glm::normalize(glm::cross(m_cameraRight, m_front));
    }
};
#endif // DEVCAMERA_H
