#include "Camera.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::BuildCamMatrices(int width, int height)
{
    // build projection and view matrix
    DirectX::XMMATRIX tmpMat = DirectX::XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f), (float)width / (float)height, 0.1f, 1000.0f);
    XMStoreFloat4x4(&m_cameraProjMat, tmpMat);

     // set starting camera state
    m_cameraPosition = DirectX::XMFLOAT4(0.0f, 1.0f, 15.0f, 0.0f);
    m_cameraTarget = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    m_cameraUp = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

    // build view matrix
    DirectX::XMVECTOR cPos = DirectX::XMLoadFloat4(&m_cameraPosition);
    DirectX::XMVECTOR cTarg = DirectX::XMLoadFloat4(&m_cameraTarget);
    DirectX::XMVECTOR cUp = DirectX::XMLoadFloat4(&m_cameraUp);
    tmpMat = DirectX::XMMatrixLookAtLH(cPos, cTarg, cUp);
    XMStoreFloat4x4(&m_cameraViewMat, tmpMat);

    // first cube
    m_cube1Position = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // set cube 1's position
    DirectX::XMVECTOR posVec = XMLoadFloat4(&m_cube1Position); // create xmvector for cube1's position

    tmpMat = DirectX::XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
    DirectX::XMStoreFloat4x4(&m_cube1RotMat, DirectX::XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
    DirectX::XMStoreFloat4x4(&m_cube1WorldMat, tmpMat); // store cube1's world matrix

}

DirectX::XMMATRIX Camera::GenerateWVP(DirectX::XMMATRIX worldMatrixOfObject)
{
    DirectX::XMMATRIX viewMat = DirectX::XMLoadFloat4x4(&m_cameraViewMat); // load view matrix
    DirectX::XMMATRIX projMat = DirectX::XMLoadFloat4x4(&m_cameraProjMat); // load projection matrix
    DirectX::XMMATRIX wvpMat = worldMatrixOfObject * viewMat * projMat; // create wvp matrix
    return DirectX::XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu;
}

void Camera::Update(float dt)
{
    // create rotation matrices
    DirectX::XMMATRIX rotXMat = DirectX::XMMatrixRotationX(0.0f * dt);
    DirectX::XMMATRIX rotYMat = DirectX::XMMatrixRotationY(0.5f * dt);
    DirectX::XMMATRIX rotZMat = DirectX::XMMatrixRotationZ(0.0f * dt);

    // add rotation to cube1's rotation matrix and store it
    DirectX::XMMATRIX rotMat = DirectX::XMLoadFloat4x4(&m_cube1RotMat) * rotXMat * rotYMat * rotZMat;
    DirectX::XMStoreFloat4x4(&m_cube1RotMat, rotMat);

    // create translation matrix for cube 1 from cube 1's position vector
    DirectX::XMMATRIX translationMat = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat4(&m_cube1Position));

    DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);

    // create cube1's world matrix by first rotating the cube, then positioning the rotated cube
    DirectX::XMMATRIX worldMat = scaleMat * rotMat * translationMat;

    // store cube1's world matrix
    DirectX::XMStoreFloat4x4(&m_cube1WorldMat, worldMat);

}
