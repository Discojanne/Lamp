#ifndef CAMERA_H
#define CAMERA_H

#include "directxmath.h"
//#include <DirectXMath.h

class Point;

class Camera
{
public:
	Camera();
	~Camera();

	void init(int w, int h);
	void BuildCamMatrices();
	DirectX::XMMATRIX GenerateWVP(DirectX::XMMATRIX worldMatrixOfObject = DirectX::XMMatrixIdentity());
	void UpdateCube(float dt);
	void Update(float dt);

	void OnKeyDown(int key);
	void OnKeyUp(int key);
	void OnMouseMove(const Point& delta_mouse_pos);

	
private:

	struct KeysPressed
	{
		bool w;
		bool a;
		bool s;
		bool d;

		bool left;
		bool right;
		bool up;
		bool down;
	};

	KeysPressed m_keysPressed;

	int m_windowWidth = 0;
	int m_windowHeight = 0;
	//DirectX::XMMATRIX m_cameraProjMat; // this will store our projection matrix
	//DirectX::XMMATRIX m_cameraViewMat; // this will store our view matrix
	float m_rotX = 0;
	float m_rotY = 0;

	DirectX::XMFLOAT3 m_cameraPosition; // this is our cameras position vector
	DirectX::XMFLOAT3 m_lookDirection; // a vector describing the point in space our camera is looking at
	DirectX::XMFLOAT3 m_cameraUp; // the worlds up vector
	float m_moveSpeed;            // Speed at which the camera moves, in units per second.
	









	DirectX::XMFLOAT4X4 m_cube1WorldMat; // our first cubes world matrix (transformation matrix)
	DirectX::XMFLOAT4X4 m_cube1RotMat; // this will keep track of our rotation for the first cube
	DirectX::XMFLOAT4 m_cube1Position; // our first cubes position in space

};



#endif
