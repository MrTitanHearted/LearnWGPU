#include <WCamera.hpp>
#include <glm/gtc/matrix_transform.hpp>

WCamera::WCamera() {
    m_WorldUp = WORLD_UP;
    m_Yaw = YAW;
    m_Pitch = PITCH;
    m_MovementSpeed = SPEED;
    m_MouseSensitivity = SENSITIVITY;
    m_Zoom = ZOOM;

    updateCameraVectors();
}
WCamera& WCamera::withPosition(glm::vec3 position) {
    m_Position = position;
    return *this;
}
WCamera& WCamera::withFront(glm::vec3 front) {
    m_Front = front;
    return *this;
}
void WCamera::processCameraMovement(WCameraMovement direction, float dt) {
    float speed = m_MovementSpeed * dt;
    if (direction == WCameraMovement::FORWARD)
        m_Position += m_Front * speed;
    if (direction == WCameraMovement::BACKWARD)
        m_Position -= m_Front * speed;
    if (direction == WCameraMovement::UP)
        m_Position += m_Up * speed;
    if (direction == WCameraMovement::DOWN)
        m_Position -= m_Up * speed;
    if (direction == WCameraMovement::WORLD_FORWARD)
        m_Position += glm::normalize(glm::vec3(m_Front.x, 0.0f, m_Front.z)) * speed;
    if (direction == WCameraMovement::WORLD_BACKWARD)
        m_Position -= glm::normalize(glm::vec3(m_Front.x, 0.0f, m_Front.z)) * speed;
    if (direction == WCameraMovement::WORLD_UP)
        m_Position += m_WorldUp * speed;
    if (direction == WCameraMovement::WORLD_DOWN)
        m_Position -= m_WorldUp * speed;
    if (direction == WCameraMovement::RIGHT)
        m_Position += m_Right * speed;
    if (direction == WCameraMovement::LEFT)
        m_Position -= m_Right * speed;
}
void WCamera::processMouseMovement(float xOffset, float yOffset, bool constrain, float constrainValue) {
    xOffset *= m_MouseSensitivity;
    yOffset *= m_MouseSensitivity;

    m_Yaw += xOffset;
    m_Pitch += yOffset;

    if (constrain)
        if (m_Pitch > constrainValue)
            m_Pitch = constrainValue;
        else if (m_Pitch < -constrainValue)
            m_Pitch = -constrainValue;

    updateCameraVectors();
}
void WCamera::processMouseScroll(float yOffset, float zoomMax, float zoomMin) {
    m_Zoom -= yOffset;
    if (m_Zoom < zoomMin)
        m_Zoom = zoomMin;
    if (m_Zoom > zoomMax)
        m_Zoom = zoomMax;
}
void WCamera::setWorldUp(glm::vec3 worldUp) {
    m_WorldUp = glm::normalize(worldUp);
    updateCameraVectors();
}
void WCamera::setMovementSpeed(float speed) {
    m_MovementSpeed = speed;
}
void WCamera::setMouseSensitivity(float sensitivity) {
    m_MouseSensitivity = sensitivity;
}
float WCamera::getZoom() const {
    return m_Zoom;
}
glm::vec3 WCamera::getPosition() const {
    return m_Position;
}
glm::vec3 WCamera::getFront() const {
    return m_Front;
}
glm::vec3 WCamera::getUp() const {
    return m_Up;
}
glm::vec3 WCamera::getWorldUp() const {
    return m_WorldUp;
}
glm::mat4 WCamera::getViewMatrix() const {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}
glm::mat4 WCamera::getProjectionMatrix(float aspect, float near, float far) const {
    return glm::perspective(glm::radians(m_Zoom), aspect, near, far);
}
void WCamera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    m_Front = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

void WCameraManager::processCameraMovement(WCameraMovement direction, float dt) {
    m_Camera.processCameraMovement(direction, dt);
}
void WCameraManager::processMouseMovement(float xPos, float yPos, bool constrain, float constrainValue) {
    if (m_FirstMouse) {
        m_LastXPosition = xPos;
        m_LastYPosition = yPos;

        m_FirstMouse = false;
    }

    float xOffset = xPos - m_LastXPosition;
    float yOffset = m_LastYPosition - yPos;
    m_LastXPosition = xPos;
    m_LastYPosition = yPos;

    m_Camera.processMouseMovement(xOffset, yOffset, constrain, constrainValue);
}
void WCameraManager::processMouseScroll(float yOffset, float zoomMax, float zoomMin) {
    m_Camera.processMouseScroll(yOffset, zoomMax, zoomMin);
}
void WCameraManager::setWorldUp(glm::vec3 worldUp) {
    m_Camera.setWorldUp(worldUp);
}
void WCameraManager::setMovementSpeed(float speed) {
    m_Camera.setMovementSpeed(speed);
}
void WCameraManager::setMouseSensitivity(float sensitivity) {
    m_Camera.setMouseSensitivity(sensitivity);
}
void WCameraManager::setFarNear(float far, float near) {
    m_Far = far;
    m_Near = near;
}
void WCameraManager::setFar(float far) {
    m_Far = far;
}
void WCameraManager::setNear(float near) {
    m_Near = near;
}
void WCameraManager::setFirstMouse(bool firstMouse) {
    m_FirstMouse = firstMouse;
}
const WCamera& WCameraManager::getCamera() const {
    return m_Camera;
}
float WCameraManager::getFar() const {
    return m_Far;
}
float WCameraManager::getNear() const {
    return m_Near;
}
glm::mat4 WCameraManager::getProjectionMatrix(float aspect) const {
    return m_Camera.getProjectionMatrix(aspect, m_Near, m_Far);
}
glm::mat4 WCameraManager::getViewMatrix() const {
    return m_Camera.getViewMatrix();
}
