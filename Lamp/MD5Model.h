#pragma once

#include "directxmath.h"
#include <string>
#include <vector>
#include "d3dx12.h"

using namespace DirectX;



class MD5Model
{
public:

    struct Vertex    //Overloaded Vertex Structure
    {
        Vertex() {}
        Vertex(float x, float y, float z,
            float u, float v,
            float nx, float ny, float nz,
            float tx, float ty, float tz)
            : pos(x, y, z), texCoord(u, v), normal(nx, ny, nz),
            tangent(tx, ty, tz) {}

        XMFLOAT3 pos;
        XMFLOAT2 texCoord;
        XMFLOAT3 normal;
        XMFLOAT3 tangent;
        XMFLOAT3 biTangent;

        // Will not be sent to shader
        int StartWeight;
        int WeightCount;
    };

    struct Joint
    {
        std::wstring name;
        int parentID;

        XMFLOAT3 pos;
        XMFLOAT4 orientation;
    };

    struct Weight
    {
        int jointID;
        float bias;
        XMFLOAT3 pos;
    };

    struct ModelSubset
    {
        int texArrayIndex;
        int numTriangles;

        std::vector<Vertex> vertices;
        std::vector<DWORD> indices;
        std::vector<Weight> weights;

        std::vector<XMFLOAT3> positions;

        ID3D12Resource* m_vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
                                                   // the total size of the buffer, and the size of each element (vertex)

        ID3D12Resource* m_indexBuffer; // a default buffer in GPU memory that we will load index data for our triangle into
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView; // a structure holding information about the index buffer

    };

    struct Model3D
    {
        int numSubsets;
        int numJoints;

        std::vector<Joint> joints;
        std::vector<ModelSubset> subsets;
    };

    MD5Model();
    ~MD5Model();

    bool LoadMD5Model(std::wstring filename,
        Model3D& MD5Model,
        std::vector<ID3D12Resource*>& shaderResourceViewArray,
        std::vector<std::wstring> texFileNameArray);
    
private:

    void CleanUp();

    XMMATRIX smilesWorld;
    Model3D NewMD5Model;
};