#include "Scene.h"
#include "ioSMD.h"
#include "d3dx12.h"
#include "InputSystem.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

bool Scene::LoadMesh(std::string filename)
{
    FILE* pFile;
    char buffer[100];

    //qDebug("Loading mesh %s", filename.toLocal8Bit().data());
    currentNameMesh = meshpath + filename + ".SMD";
    //fopen_s(&pFile, currentNameMesh.c_str(), "rt");

    //std::string path = "bride_dress.SMD";
    errno_t err;
    err = fopen_s(&pFile, currentNameMesh.c_str(), "rt");
    //QFile f(meshPath + filename + ".SMD");
    if (err == 0)
    {
        printf("The file 'crt_fopen_s.c' was opened\n");
    }
   else
   {
   printf("The file 'crt_fopen_s.c' was not opened\n");
   }


    Animation tmpAni;
    if (!ioSMD::import(pFile, currentMesh, tmpAni)) {
        return false;
    }
    //fclose(pFile);

    if (!tmpAni.isEmpty()) {
        // if the mesh file also embeds an animation???
        int q = 5;
    }


    currentMesh.orderBoneSlots();
    currentMesh.computeIsTextureFlipped();
    currentMesh.unifyVertices(); // comment this line for flat shading!
    currentMesh.computeTangentDirs();
    currentMesh.computeDeformFactors();

    std::string basename = filename;

    currentBumpmapFilename = texturepath + basename + "_normalmap.dds";
    currentSpecmapFilename = texturepath + basename + "_specular.dds";

    return true;
}

bool Scene::LoadAnimation(std::string filename)
{
    FILE* pFile;
    char buffer[100];

    currentNameAni = animationpath + filename + ".SMD";
    errno_t err;
    err = fopen_s(&pFile, currentNameAni.c_str(), "rt");

    Mesh tmpMesh;
    if (!ioSMD::import(pFile, tmpMesh, currentAni))
        return false;

    // in our looped animations, last frame == 1st frame, so we remove it
    currentAni.pose.pop_back();

    return true;
}

bool Scene::CreateVertexBuffers(ID3D12Device6* device, ID3D12GraphicsCommandList6* commandList)
{
    HRESULT hr;

    ///
   
    // Temporary data type used for only transfering pos and bone info.
    //std::vector<VertLite> vertLiteVector;

    for (size_t i = 0; i < currentMesh.vert.size(); i++)
    {
        VertLite tmpVL;
        tmpVL.pos = currentMesh.vert[i].pos;
        tmpVL.uv = currentMesh.vert[i].uv;
        tmpVL.norm = currentMesh.vert[i].norm;
        tmpVL.tang = currentMesh.vert[i].tang;
        tmpVL.bitang = currentMesh.vert[i].bitang;
        for (size_t j = 0; j < 4; j++)
        {
            tmpVL.boneIndex[j] = currentMesh.vert[i].boneIndex[j];
            tmpVL.boneWeight[j] = currentMesh.vert[i].boneWeight[j];
        }
        currentMesh.vertLiteVector.push_back(tmpVL);
    }

    //currentMesh.meshletVector.push_back(Meshlet2(78,0,32,0));

    ///

    int nrOfVertices = currentMesh.vertLiteVector.size();
    int vBufferSize = sizeof(VertLite) * nrOfVertices;

    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                        // from the upload heap to this heap
        nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        IID_PPV_ARGS(&currentMesh.vertexBuffer));

    currentMesh.vertexBuffer->SetName(L"Vertex Default Heap");


    currentMesh.vBufferUploadHeap = nullptr;
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&currentMesh.vBufferUploadHeap));
    currentMesh.vBufferUploadHeap->SetName(L"Vertex Upload Heap");

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = reinterpret_cast<BYTE*>(currentMesh.vertLiteVector.data()); // pointer to our vertex array
    vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
    vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    if (!currentMesh.vBufferUploadHeap)
    {
        return false;
    }
    else
    {
        UpdateSubresources(commandList, currentMesh.vertexBuffer, currentMesh.vBufferUploadHeap, 0, 0, 1, &vertexData);
    }

    // transition the vertex buffer data from copy destination state to vertex buffer state
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(currentMesh.vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));



    
    int nrOfIndices = currentMesh.face.size()*3;
    int iBufferSize = sizeof(int) * nrOfIndices;

    // create default heap to hold index buffer
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
        nullptr, // optimized clear value must be null for this type of resource
        IID_PPV_ARGS(&currentMesh.indexBuffer));

    // create upload heap to upload index buffer
    currentMesh.iBufferUploadHeap = nullptr;
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&currentMesh.iBufferUploadHeap));
    currentMesh.iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap2");

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA indexData = {};
    indexData.pData = reinterpret_cast<BYTE*>(currentMesh.face.data()); // pointer to our index array
    indexData.RowPitch = iBufferSize;   // size of all our index buffer
    indexData.SlicePitch = iBufferSize; // also the size of our index buffer

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    if (!currentMesh.iBufferUploadHeap)
    {
        return false;
    }
    else
    {
        UpdateSubresources(commandList, currentMesh.indexBuffer, currentMesh.iBufferUploadHeap, 0, 0, 1, &indexData);
    }

    // transition the vertex buffer data from copy destination state to vertex buffer state
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(currentMesh.indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    currentMesh.vertexBufferView.BufferLocation = currentMesh.vertexBuffer->GetGPUVirtualAddress(); // temporärt upload heap för memcopy medans cpu animering
    currentMesh.vertexBufferView.StrideInBytes = sizeof(VertLite);
    currentMesh.vertexBufferView.SizeInBytes = vBufferSize;

    // create a vertex buffer view for the triangle.We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    currentMesh.indexBufferView.BufferLocation = currentMesh.indexBuffer->GetGPUVirtualAddress();
    currentMesh.indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
    currentMesh.indexBufferView.SizeInBytes = iBufferSize;

    currentMesh.UploadGpuResources(device, commandList);

    return true;

}

bool Scene::Init(int width, int height)
{
    InputSystem::get()->addListener(this);

    cam.init(width, height);

    return true;
}

void Scene::Update(float dt)
{
    m_dt = dt;
    InputSystem::get()->update();
    
    cam.Update(dt);
    //cam.BuildCamMatrices();
    //cam.UpdateCube(dt);
}

void Scene::testAnimationFunc(int animFrame)
{
    Pose p = currentAni.pose[animFrame];

    currentMesh.vertLiteVector.clear();

    //for each vertex
    for (size_t i = 0; i < currentMesh.vert.size(); i++)
    {
        const Vert& vO = currentMesh.vert[i];

       
        VertLite v;
        v.pos = vO.pos;
        for (size_t j = 0; j < 4; j++)
        {
            v.boneIndex[j] = vO.boneIndex[j];
            v.boneWeight[j] = vO.boneWeight[j];
        }
        
        DirectX::XMMATRIX m = DirectX::XMMATRIX(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

        // max 4 bones
        if (v.boneIndex[0] >= 0)
            m += p.matr[v.boneIndex[0]] * v.boneWeight[0];
        
        if (v.boneIndex[1] >= 0)
            m += p.matr[v.boneIndex[1]] * v.boneWeight[1];
      
        if (v.boneIndex[2] >= 0)
            m += p.matr[v.boneIndex[2]] * v.boneWeight[2];
        
        if (v.boneIndex[3] >= 0)
            m += p.matr[v.boneIndex[3]] * v.boneWeight[3];
        

        DirectX::XMVECTOR tmpPos = DirectX::XMVector3Transform({v.pos.x, v.pos.y, v.pos.z, 1.0f}, m);

        v.pos.x = tmpPos.m128_f32[0];
        v.pos.y = tmpPos.m128_f32[1];
        v.pos.z = tmpPos.m128_f32[2];

        currentMesh.vertLiteVector.push_back(v);
    }


}

void Scene::onKeyDown(int key)
{
    cam.OnKeyDown(key);

    
}

void Scene::onKeyUp(int key)
{
    cam.OnKeyUp(key);

    switch (key)
    {
    case 'W':
       
        break;
    case 0x26://up
        drawThisMany++;
        break;
    case 0x28://down
        drawThisMany--;
        break;
    }
}

void Scene::onMouseMove(const Point& delta_mouse_pos)
{
    cam.OnMouseMove(delta_mouse_pos);
}

void Scene::onLeftMouseDown(const Point& mouse_pos)
{
}

void Scene::onLeftMouseUp(const Point& mouse_pos)
{
}

void Scene::onRightMouseDown(const Point& mouse_pos)
{
}

void Scene::onRightMouseUp(const Point& mouse_pos)
{
}
