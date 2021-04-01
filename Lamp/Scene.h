#pragma once

#include <string>

#include "mesh.h"
#include "animation.h"
#include "Camera.h"
#include "InputListener.h"

struct ID3D12Device6;
struct ID3D12GraphicsCommandList6;

class Scene : public InputListener
{
public:
	Scene();
	~Scene();

    bool LoadMesh(std::string filename);
    bool LoadAnimation(std::string filename);
    bool CreateVertexBuffers(ID3D12Device6* device, ID3D12GraphicsCommandList6* commandList);
    bool Init(); // move other init / load functions here later
    void Update(float dt); // same here

    // temporary, used to debug animation error
    void testAnimationFunc(int animFrame);

    // Inherited via InputListener
    virtual void onKeyDown(int key) override;
    virtual void onKeyUp(int key) override;

    Mesh currentMesh;
    Animation currentAni;
    Camera cam;

private:

    float m_dt = 0.0f;

    std::string meshpath = "Resources/models/";
    std::string texturepath = "Resources/textures/";
    std::string animationpath = "Resources/animations/";
    
    
    std::string currentNameMesh, currentNameAni;
    std::string currentBumpmapFilename;
    std::string currentSpecmapFilename;

    
};
