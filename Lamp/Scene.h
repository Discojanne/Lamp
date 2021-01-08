#pragma once

#include <string>

// new stuff
#include "mesh.h"

#include "animation.h"
#include "animation_dqs.h"

struct ID3D12Device6;
struct ID3D12GraphicsCommandList6;

class Scene
{
public:
	Scene();
	~Scene();

    bool LoadMesh(std::string filename);
    bool LoadAnimation(std::string filename);
    bool CreateVertexBuffers(ID3D12Device6* device, ID3D12GraphicsCommandList6* commandList);

    void ReleaseUploadHeaps();

    AnimationDQS currentAniDqs;
    Mesh currentMesh;
private:

    std::string meshpath = "Resources/models/";
    std::string texturepath = "Resources/textures/";
    std::string animationpath = "Resources/animations/";

    /* data being shown */
    
    //QString currentBumpmapFilename;
    //QString currentSpecmapFilename;
    Animation currentAni;
    
    std::string currentNameMesh, currentNameAni;
    int currentFrame;
    std::string currentBumpmapFilename;
    std::string currentSpecmapFilename;
};
