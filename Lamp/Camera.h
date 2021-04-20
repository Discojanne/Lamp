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
	DirectX::XMMATRIX GenerateWVP(DirectX::XMMATRIX worldMatrixOfObject = DirectX::XMMatrixIdentity());
	DirectX::XMMATRIX GenerateNormalMatrix(DirectX::XMMATRIX worldMatrixOfObject = DirectX::XMMatrixIdentity());
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
	float m_rotX = 0;
	float m_rotY = 0;

	DirectX::XMFLOAT3 m_cameraPosition; // this is our cameras position vector
	DirectX::XMFLOAT3 m_lookDirection; // a vector describing the point in space our camera is looking at
	DirectX::XMFLOAT3 m_cameraUp; // the worlds up vector
	float m_moveSpeed;            // Speed at which the camera moves, in units per second.
	
};



#endif
