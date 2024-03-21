#pragma once

#include <glm/glm.hpp>

enum class WCameraMovement {
    FORWARD,
    BACKWARD,
    UP,
    DOWN,
    WORLD_FORWARD,
    WORLD_BACKWARD,
    WORLD_UP,
    WORLD_DOWN,
    RIGHT,
    LEFT,
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

class WCamera {
   public:
    WCamera();
    inline static WCamera New() { return WCamera(); }

    WCamera &withPosition(glm::vec3 position);
    WCamera &withFront(glm::vec3 front);

    void processCameraMovement(WCameraMovement direction, float dt);
    void processMouseMovement(float xOffset, float yOffset, bool constrain = true, float constrainValue = 89.9f);
    void processMouseScroll(float yOffset, float zoomMax = 45.0f, float zoomMin = 1.0f);

    void setWorldUp(glm::vec3 worldUp);
    void setMovementSpeed(float speed);
    void setMouseSensitivity(float sensitivity);

    float getZoom() const;

    glm::vec3 getPosition() const;
    glm::vec3 getFront() const;
    glm::vec3 getUp() const;
    glm::vec3 getWorldUp() const;

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect, float near = 0.0001f, float far = 1000.0f) const;

   private:
    glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_Right;
    glm::vec3 m_Up;
    glm::vec3 m_WorldUp;

    float m_Yaw;
    float m_Pitch;

    float m_MovementSpeed;
    float m_MouseSensitivity;
    float m_Zoom;

    void updateCameraVectors();
};

class WCameraManager {
   public:
    WCameraManager(float lastX,
                   float lastY)
        : m_FirstMouse(true),
          m_LastXPosition(lastX),
          m_LastYPosition(lastY),
          m_Far(1000.0f),
          m_Near(0.0001f),
          m_Camera(WCamera::New()) {}

    void processCameraMovement(WCameraMovement direction, float dt);
    void processMouseMovement(float xPos, float yPos, bool constrain = true, float constrainValue = 89.9f);
    void processMouseScroll(float yOffset, float zoomMax = 45.0f, float zoomMin = 1.0f);

    void setWorldUp(glm::vec3 worldUp);
    void setMovementSpeed(float speed);
    void setMouseSensitivity(float sensitivity);
    void setFarNear(float far, float near);
    void setFar(float far);
    void setNear(float near);
    void setFirstMouse(bool firstMouse = true);

    const WCamera &getCamera() const;
    float getFar() const;
    float getNear() const;

    glm::mat4 getProjectionMatrix(float aspect) const;
    glm::mat4 getViewMatrix() const;

   private:
    WCamera m_Camera;

    bool m_FirstMouse;

    float m_LastXPosition;
    float m_LastYPosition;
    float m_Far;
    float m_Near;
};