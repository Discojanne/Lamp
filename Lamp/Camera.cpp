#include "Camera.h"
#include <algorithm>

#include "Point.h"
#include "ExtraMath.h"

Camera::Camera()
{
    // set starting camera state
    //m_cameraPosition = DirectX::XMFLOAT4(8.0f, 3.0f, -8.0f, 0.0f);  // cubeman
    //m_cameraTarget = DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 0.0f);     // cubeman
    m_cameraPosition = DirectX::XMFLOAT3(15.0f, 10.0f, -60.0f); // Bridovivel
    m_lookDirection = DirectX::XMFLOAT3(15.0f, 10.0f, 0.0f);     // Bridovivel
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
    //BuildCamMatrices();
}

void Camera::BuildCamMatrices()
{
    // build projection and view matrix
    //m_cameraProjMat = DirectX::XMMatrixPerspectiveFovLH(45.0f * (DirectX::XM_PI / 180.0f), (float)m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);


    // build view matrix
    //DirectX::XMVECTOR cPos = DirectX::XMLoadFloat3(&m_cameraPosition);
    //DirectX::XMVECTOR cTarg = DirectX::XMLoadFloat3(&m_cameraTarget);
    //DirectX::XMVECTOR cUp = DirectX::XMLoadFloat3(&m_cameraUp);
    //m_cameraViewMat = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_cameraPosition), DirectX::XMLoadFloat3(&m_cameraTarget), DirectX::XMLoadFloat3(&m_cameraUp));
    //XMStoreFloat4x4(&m_cameraViewMat, tmpMat);

    //// first cube
    //m_cube1Position = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // set cube 1's position
    //DirectX::XMVECTOR posVec = XMLoadFloat4(&m_cube1Position); // create xmvector for cube1's position

    //DirectX::XMMATRIX tmpMat = DirectX::XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
    //DirectX::XMStoreFloat4x4(&m_cube1RotMat, DirectX::XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
    //DirectX::XMStoreFloat4x4(&m_cube1WorldMat, tmpMat); // store cube1's world matrix

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

void Camera::UpdateCube(float dt)
{
    //// create rotation matrices
    //DirectX::XMMATRIX rotXMat = DirectX::XMMatrixRotationX(0.5f * dt);
    //DirectX::XMMATRIX rotYMat = DirectX::XMMatrixRotationY(0.0f * dt);
    //DirectX::XMMATRIX rotZMat = DirectX::XMMatrixRotationZ(0.0f * dt);

    //// add rotation to cube1's rotation matrix and store it
    //DirectX::XMMATRIX rotMat = DirectX::XMLoadFloat4x4(&m_cube1RotMat) * rotXMat * rotYMat * rotZMat;
    //DirectX::XMStoreFloat4x4(&m_cube1RotMat, rotMat);

    //// create translation matrix for cube 1 from cube 1's position vector
    //DirectX::XMMATRIX translationMat = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat4(&m_cube1Position));

    //DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);

    //// create cube1's world matrix by first rotating the cube, then positioning the rotated cube
    //DirectX::XMMATRIX worldMat = scaleMat * rotMat * translationMat;

    //// store cube1's world matrix
    //DirectX::XMStoreFloat4x4(&m_cube1WorldMat, worldMat);

}

void Camera::Update(float dt)
{

    if (m_keysPressed.a){
        m_cameraPosition.x -= m_moveSpeed * dt;
        m_lookDirection.x -= m_moveSpeed * dt;
    }
        
    if (m_keysPressed.d){
        m_cameraPosition.x += m_moveSpeed * dt;
        m_lookDirection.x += m_moveSpeed * dt;
    }
        
    if (m_keysPressed.w){
        m_cameraPosition.z += m_moveSpeed * dt;
        m_lookDirection.z += m_moveSpeed * dt;
    }
    
    if (m_keysPressed.s){
        m_cameraPosition.z -= m_moveSpeed * dt;
        m_lookDirection.z -= m_moveSpeed * dt;
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
