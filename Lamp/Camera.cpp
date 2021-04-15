#include "Camera.h"
#include <algorithm>

#include "Point.h"
#include "ExtraMath.h"

Camera::Camera()
{
    // set starting camera state
    //m_cameraPosition = DirectX::XMFLOAT4(8.0f, 3.0f, -8.0f, 0.0f);  // cubeman
    //m_cameraTarget = DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f);     // cubeman
    m_cameraPosition = DirectX::XMFLOAT3(6.0f, 10.0f, 30.0f); // Bridovivel
    m_lookDirection = DirectX::XMFLOAT3(6.0f, 10.0f, 0.0f);     // Bridovivel
    m_cameraUp = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

    m_moveSpeed = 20.0f;
    m_keysPressed = {};
}

Camera::~Camera()
{
}

void Camera::init(int w, int h)
{
    m_windowWidth = w;
    m_windowHeight = h;
}


DirectX::XMMATRIX Camera::GenerateWVP(DirectX::XMMATRIX worldMatrixOfObject)
{
    DirectX::XMMATRIX viewMat = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_cameraPosition), DirectX::XMLoadFloat3(&m_lookDirection), DirectX::XMLoadFloat3(&m_cameraUp));
    
    // swapped x and y to make it work temporarly
    viewMat = viewMat * DirectX::XMMatrixRotationX(m_rotY) * DirectX::XMMatrixRotationY(m_rotX);

    DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(45.0f * (DirectX::XM_PI / 180.0f), (float)m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f); // load projection matrix
    DirectX::XMMATRIX wvpMat = worldMatrixOfObject * viewMat * projMat; // create wvp matrix
    return wvpMat;
}

void Camera::Update(float dt)
{

    if (m_keysPressed.a){
        m_cameraPosition.x += m_moveSpeed * dt;
        m_lookDirection.x += m_moveSpeed * dt;
    }
        
    if (m_keysPressed.d){
        m_cameraPosition.x -= m_moveSpeed * dt;
        m_lookDirection.x -= m_moveSpeed * dt;
    }
        
    if (m_keysPressed.w){
        m_cameraPosition.z -= m_moveSpeed * dt;
        m_lookDirection.z -= m_moveSpeed * dt;
    }
    
    if (m_keysPressed.s){
        m_cameraPosition.z += m_moveSpeed * dt;
        m_lookDirection.z += m_moveSpeed * dt;
    }

    if (m_keysPressed.left) {
        m_rotX += DirectX::XM_PIDIV2 * dt;
    }
    if (m_keysPressed.right) {
        m_rotX -= DirectX::XM_PIDIV2 * dt;
    }


}

void Camera::OnKeyDown(int key)
{
    switch (key)
    {
    case 'W':
        m_keysPressed.w = true;
        break;
    case 'A':
        m_keysPressed.a = true;
        break;
    case 'S':
        m_keysPressed.s = true;
        break;
    case 'D':
        m_keysPressed.d = true;
        break;
    case 0x25://left
        m_keysPressed.left = true;
        break;
    case 0x27://right
        m_keysPressed.right = true;
        break;
    case 0x26://up
        m_keysPressed.up = true;
        break;
    case 0x28://down
        m_keysPressed.down = true;
        break;
    }
}

void Camera::OnKeyUp(int key)
{
    switch (key)
    {
    case 'W':
        m_keysPressed.w = false;
        break;
    case 'A':
        m_keysPressed.a = false;
        break;
    case 'S':
        m_keysPressed.s = false;
        break;
    case 'D':
        m_keysPressed.d = false;
        break;
    case 0x25:
        m_keysPressed.left = false;
        break;
    case 0x27:
        m_keysPressed.right = false;
        break;
    case 0x26:
        m_keysPressed.up = false;
        break;
    case 0x28:
        m_keysPressed.down = false;
        break;
    }
}

void Camera::OnMouseMove(const Point& delta_mouse_pos)
{
    //m_rotX -= delta_mouse_pos.m_x * 0.001;
    //m_rotY -= delta_mouse_pos.m_y * 0.001;
}
