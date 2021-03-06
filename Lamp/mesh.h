/**************************************************************
 * This file is part of Deform Factors demo.                  *
 * Project web page:                                          *
 *    http://vcg.isti.cnr.it/deformfactors/                   *
 *                                                            *
 * Copyright (c) 2013 Marco Tarini <marco.tarini@isti.cnr.it> *
 *                                                            *
 * Deform Factors Demo is an implementation of                *
 * the algorithms and data structures described in            *
 * the Scientific Article:                                    *
 *    Accurate and Efficient Lighting for Skinned Models      *
 *    Marco Tarini, Daniele Panozzo, Olga Sorkine-Hornung     *
 *    Computer Graphic Forum, 2014                            *
 *    (presented at EUROGRAPHICS 2014)                        *
 *                                                            *
 * This Source Code is subject to the terms of                *
 * the Mozilla Public License v. 2.0.                         *
 * One copy of the license is available at                    *
 * http://mozilla.org/MPL/2.0/.                               *
 *                                                            *
 * Additionally, this Source Code is CITEWARE:                *
 * any derivative work must cite the                          *
 * above Scientific Article and include the same condition.   *
 *                                                            *
 **************************************************************/

#ifndef MESH_H
#define MESH_H

/* class Mesh: a rigged mesh */

#include <vector>
#include "ExtraMath.h"
//#include <D3D12MeshletGenerator.h>
#include <DirectXMesh.h>
#include "d3dx12.h"

class Pose;

// how many bone-links per vertex:
const int MAX_BONES = 4;

//typedef Eigen::Vector2f Vec2;

class Vert{
public:
    Vert(){}
    XMFLOAT3 pos; // xyz position
    XMFLOAT2 uv; // texture position
    XMFLOAT3 norm; // normal

    XMFLOAT3 tang; // tangent dir
    XMFLOAT3 bitang; // bi-tangent dir

    int boneIndex[MAX_BONES];

    float boneWeight[MAX_BONES];

    // send in as float3 or maxbones - 1
    float deformFactorsTang[MAX_BONES]; /* see paper */
    float deformFactorsBtan[MAX_BONES]; /* see paper */

    float isTextureFlipped; /* -1 if the texture is flipped, 1 otherwise */

    /* helper functions */
    float weightOfBone(int i) const;
    int slotOfBone(int i);
    bool operator < (const Vert &b) const;
    void orderBoneSlots();
    void maybeSwapBoneSlots(int i, int j);

};

class VertLite {
public:
    VertLite() {}
    XMFLOAT3 pos; // xyz position
    XMFLOAT2 uv; // texture position
    XMFLOAT3 norm; // normal

    XMFLOAT3 tang; // tangent dir
    XMFLOAT3 bitang; // bi-tangent dir

    int boneIndex[MAX_BONES];
    float boneWeight[MAX_BONES];

    // send in as float3 or maxbones - 1
    float deformFactorsTang[MAX_BONES - 1]; /* see paper */
    float deformFactorsBtan[MAX_BONES - 1]; /* see paper */

    float isTextureFlipped;
};

class Face{
public:
    unsigned int index[3];

    Face() {}
    Face(int i, int j, int k) {index[0]=i; index[1]=j; index[2]=k; }

};


class Mesh{
public:
    bool isEmpty() const;

    Mesh();
    ~Mesh();
    // Temporary data type used for only transfering pos and bone info.
    std::vector<VertLite> vertLiteVector;

    std::vector<Vert> vert;
    std::vector<Face> face;
    //std::vector<Meshlet2> meshletVector;
    std::vector<std::pair<size_t, size_t>> subsets;
    std::vector<Meshlet> meshletVector; // SOON�
    std::vector<uint8_t> uniqueVertexIndices;
    std::vector<MeshletTriangle> primitiveIndices;

    ID3D12Resource* vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
                                               // the total size of the buffer, and the size of each element (vertex)

    ID3D12Resource* indexBuffer; // a default buffer in GPU memory that we will load index data for our triangle into
    D3D12_INDEX_BUFFER_VIEW indexBufferView; // a structure holding information about the index buffer

    //remove these probably
    ID3D12Resource* vBufferUploadHeap;
    ID3D12Resource* iBufferUploadHeap;

    // structured buffer vertices
    ID3D12Resource* VertResSB;
    ID3D12Resource* IndexResSB;
    ID3D12Resource* MeshletResSB;
    ID3D12Resource* UniqueResSB;

    ID3D12Resource* vertexUploads;
    ID3D12Resource* indexUpload;
    ID3D12Resource* meshletUpload;
    ID3D12Resource* uniqueUpload;

    void ReleaseUploadHeaps();
    void Cleanup();
        
    void setUniformRig(int nbone);

    bool UploadGpuResources(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    void GenerateMeshlets();

    void computeDeformFactors();
    void computeTangentDirs();
    void computeIsTextureFlipped();
    void computeNormals();

    void orderBoneSlots();
    void unifyVertices();
    void removeUnreferencedVertices();

    void freezeAt(const Pose &p);
    void clear();
    void test();

};

#endif

