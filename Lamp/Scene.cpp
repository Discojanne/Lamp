#include "Scene.h"
#include "ioSMD.h"
#include "d3dx12.h"

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
    //if (!ioSMD::import(openFile(f), currentMesh, tmpAni)) {
    if (!ioSMD::import(pFile, currentMesh, tmpAni)) {
        //qDebug("Loading mesh: %s", ioSMD::lastErrorString());
        return false;
    }
    //fclose(pFile);

    if (!tmpAni.isEmpty()) {
        // if the mesh file also embeds an animation???
        int a = 5;
    }


    currentMesh.orderBoneSlots();
    currentMesh.computeIsTextureFlipped();
    currentMesh.unifyVertices(); // comment this line for flat shading!
    currentMesh.computeTangentDirs();
    currentMesh.computeDeformFactors();

    //meshBuffersReady = false;
    //texturesReady = false;

    std::string basename = filename;

    currentBumpmapFilename = texturepath + basename + "_normalmap.dds";
    currentSpecmapFilename = texturepath + basename + "_specular.dds";

    //update();
    return true;
}

bool Scene::LoadAnimation(std::string filename)
{
    //qDebug("Loading animation %s", filename.toLocal8Bit().data());

    FILE* pFile;
    char buffer[100];

    currentNameAni = animationpath + filename + ".SMD";
    errno_t err;
    err = fopen_s(&pFile, currentNameAni.c_str(), "rt");

    Mesh tmpMesh;
    if (!ioSMD::import(pFile, tmpMesh, currentAni))
        return false;

    // small hack: in our looped animations, last frame == 1st frame, so we remove it
    currentAni.pose.pop_back();

    currentAniDqs.buildFromAnimation(currentAni);
    currentFrame = 0;

    //update();

    return true;
}

bool Scene::CreateVertexBuffers(ID3D12Device6* device, ID3D12GraphicsCommandList6* commandList)
{
    HRESULT hr;

    int nrOfVertices = currentMesh.vert.size();
    int vBufferSize = sizeof(Vert) * nrOfVertices;

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

    /*XMFLOAT3 pos[2576];
    for (size_t i = 0; i < 2576; i++)
    {
        pos[i] = currentMesh.vert[i].pos;
    }*/

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = reinterpret_cast<BYTE*>(currentMesh.vert.data()); // pointer to our vertex array
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
    //subset.m_vertexBufferView.BufferLocation = subset.m_vertexBuffer->GetGPUVirtualAddress();
    currentMesh.vertexBufferView.BufferLocation = currentMesh.vertexBuffer->GetGPUVirtualAddress(); // temporärt upload heap för memcopy medans cpu animering
    currentMesh.vertexBufferView.StrideInBytes = sizeof(Vert);
    currentMesh.vertexBufferView.SizeInBytes = vBufferSize;

    // create a vertex buffer view for the triangle.We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    currentMesh.indexBufferView.BufferLocation = currentMesh.indexBuffer->GetGPUVirtualAddress();
    currentMesh.indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
    currentMesh.indexBufferView.SizeInBytes = iBufferSize;

    

    return true;

}

void Scene::testAnimationFunc(int animFrame)
{
    Pose p = currentAni.pose[animFrame];

    //for each vertex
    for (size_t i = 0; i < currentMesh.vert.size(); i++)
    {
        Vert v = currentMesh.vert[i];

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


        currentMesh.vert[i] = v;
    }

}

void Scene::ReleaseUploadHeaps()
{
    currentMesh.vBufferUploadHeap->Release();
    currentMesh.iBufferUploadHeap->Release();
}
