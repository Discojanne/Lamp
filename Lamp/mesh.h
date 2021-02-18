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

    int boneIndex[MAX_BONES];
    float boneWeight[MAX_BONES];
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
    std::vector<Vert> vert;
    std::vector<Face> face;

    ID3D12Resource* vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
                                               // the total size of the buffer, and the size of each element (vertex)

    ID3D12Resource* indexBuffer; // a default buffer in GPU memory that we will load index data for our triangle into
    D3D12_INDEX_BUFFER_VIEW indexBufferView; // a structure holding information about the index buffer

    //remove these probably
    ID3D12Resource* vBufferUploadHeap;
    ID3D12Resource* iBufferUploadHeap;

    void setUniformRig(int nbone);

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
