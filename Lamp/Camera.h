#ifndef CAMERA_H
#define CAMERA_H

#include "directxmath.h"

class Camera
{
public:
	Camera();
	~Camera();

	void BuildCamMatrices(int width, int height);
	DirectX::XMMATRIX GenerateWVP(DirectX::XMMATRIX worldMatrixOfObject = DirectX::XMMatrixIdentity());
	void UpdateCube(float dt);
	void Update(float dt);

	DirectX::XMFLOAT4 m_cameraPosition; // this is our cameras position vector
private:

	DirectX::XMFLOAT4X4 m_cameraProjMat; // this will store our projection matrix
	DirectX::XMFLOAT4X4 m_cameraViewMat; // this will store our view matrix

	
	DirectX::XMFLOAT4 m_cameraTarget; // a vector describing the point in space our camera is looking at
	DirectX::XMFLOAT4 m_cameraUp; // the worlds up vector

	DirectX::XMFLOAT4X4 m_cube1WorldMat; // our first cubes world matrix (transformation matrix)
	DirectX::XMFLOAT4X4 m_cube1RotMat; // this will keep track of our rotation for the first cube
	DirectX::XMFLOAT4 m_cube1Position; // our first cubes position in space

};



#endif
