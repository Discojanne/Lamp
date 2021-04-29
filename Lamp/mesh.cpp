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


#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "mesh.h"
#include "animation/animation.h"



const int MAX_TOTAL_BONES = 35;


Mesh::Mesh(){}

Mesh::~Mesh()
{
    Cleanup();
}


void Mesh::ReleaseUploadHeaps()
{
    //currentMesh.vBufferUploadHeap->Release();
    //currentMesh.iBufferUploadHeap->Release();

    if (vBufferUploadHeap)
    {
        vBufferUploadHeap->Release();
        vBufferUploadHeap = nullptr;
    }
    if (iBufferUploadHeap)
    {
        iBufferUploadHeap->Release();
        iBufferUploadHeap = nullptr;
    }

    // Mesh shader resources
    if (vertexUploads)
    {
        vertexUploads->Release();
        vertexUploads = nullptr;
    }
    if (indexUpload)
    {
        indexUpload->Release();
        indexUpload = nullptr;
    }
    if (meshletUpload)
    {
        meshletUpload->Release();
        meshletUpload = nullptr;
    }
    if (uniqueUpload)
    {
        uniqueUpload->Release();
        uniqueUpload = nullptr;
    }
}

void Mesh::Cleanup()
{

    if (vertexBuffer)
    {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }
    if (indexBuffer)
    {
        indexBuffer->Release();
        indexBuffer = nullptr;
    }

    if (vBufferUploadHeap)
    {
        vBufferUploadHeap->Release();
        vBufferUploadHeap = nullptr;
    }
    if (iBufferUploadHeap)
    {
        iBufferUploadHeap->Release();
        iBufferUploadHeap = nullptr;
    }

    // mesh shader resources
    if (VertResSB)
    {
        VertResSB->Release();
        VertResSB = nullptr;
    }
    if (IndexResSB)
    {
        IndexResSB->Release();
        IndexResSB = nullptr;
    }
    if (MeshletResSB)
    {
        MeshletResSB->Release();
        MeshletResSB = nullptr;
    }
    if (UniqueResSB)
    {
        UniqueResSB->Release();
        UniqueResSB = nullptr;
    }

    if (vertexUploads)
    {
        vertexUploads->Release();
        vertexUploads = nullptr;
    }
    if (indexUpload)
    {
        indexUpload->Release();
        indexUpload = nullptr;
    }
    if (meshletUpload)
    {
        meshletUpload->Release();
        meshletUpload = nullptr;
    }
    if (uniqueUpload)
    {
        uniqueUpload->Release();
        uniqueUpload = nullptr;
    }
}

bool Mesh::UploadGpuResources(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    // VS data still uploads in Scene::CreateVertexBuffers


    HRESULT hr;

    

    /// default heap 
    auto defaultHeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    // Vertex buffer
    int nrOfVertices = vertLiteVector.size();
    int vBufferSize = sizeof(VertLite) * nrOfVertices;
    auto vertexDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
    hr = device->CreateCommittedResource(&defaultHeapDesc, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&VertResSB));
    if (FAILED(hr))
        return false;

    // Index buffer TODO: -> PrimitiveIndices
    int iBufferSize = sizeof(MeshletTriangle) * primitiveIndices.size();
    auto indexDesc = CD3DX12_RESOURCE_DESC::Buffer(iBufferSize);
    hr = device->CreateCommittedResource(&defaultHeapDesc, D3D12_HEAP_FLAG_NONE, &indexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&IndexResSB));
    if (FAILED(hr))
        return false;

    // Meshlet buffer
    auto meshletDesc = CD3DX12_RESOURCE_DESC::Buffer(meshletVector.size() * sizeof(Meshlet));
    hr = device->CreateCommittedResource(&defaultHeapDesc, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&MeshletResSB));
    if (FAILED(hr))
        return false;

    // UniqueVertex buffer
    auto uniqueIndexDesc = CD3DX12_RESOURCE_DESC::Buffer(uniqueVertexIndices.size());
    hr = device->CreateCommittedResource(&defaultHeapDesc, D3D12_HEAP_FLAG_NONE, &uniqueIndexDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&UniqueResSB));
    if (FAILED(hr))
        return false;

    /// upload heap 
    
    auto uploadHeapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    // Vertex Buffer
    hr = device->CreateCommittedResource(&uploadHeapDesc, D3D12_HEAP_FLAG_NONE, &vertexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexUploads));
    if (FAILED(hr))
        return false;

    uint8_t* memory = nullptr;
    vertexUploads->Map(0, nullptr, reinterpret_cast<void**>(&memory));
    std::memcpy(memory, vertLiteVector.data(), vBufferSize);
    vertexUploads->Unmap(0, nullptr);


    // Index buffer
    hr = device->CreateCommittedResource(&uploadHeapDesc, D3D12_HEAP_FLAG_NONE, &indexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexUpload));
    if (FAILED(hr))
        return false;

    memory = nullptr;
    indexUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
    std::memcpy(memory, primitiveIndices.data(), iBufferSize);
    indexUpload->Unmap(0, nullptr);


    // Meshlet buffer
    hr = device->CreateCommittedResource(&uploadHeapDesc, D3D12_HEAP_FLAG_NONE, &meshletDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&meshletUpload));
    if (FAILED(hr))
        return false;

    memory = nullptr;
    meshletUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
    std::memcpy(memory, meshletVector.data(), meshletVector.size() * sizeof(Meshlet));
    meshletUpload->Unmap(0, nullptr);


    // UniqueVertex buffer
    hr = device->CreateCommittedResource(&uploadHeapDesc, D3D12_HEAP_FLAG_NONE, &uniqueIndexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uniqueUpload));
    if (FAILED(hr))
        return false;
    
    memory = nullptr;
    uniqueUpload->Map(0, nullptr, reinterpret_cast<void**>(&memory));
    std::memcpy(memory, uniqueVertexIndices.data(), uniqueVertexIndices.size());
    uniqueUpload->Unmap(0, nullptr);


    D3D12_RESOURCE_BARRIER postCopyBarriers[4];

    commandList->CopyResource(VertResSB, vertexUploads);
    postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(VertResSB, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    
    commandList->CopyResource(IndexResSB, indexUpload);
    postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(IndexResSB, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    commandList->CopyResource(MeshletResSB, meshletUpload);
    postCopyBarriers[2] = CD3DX12_RESOURCE_BARRIER::Transition(MeshletResSB, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    commandList->CopyResource(UniqueResSB, uniqueUpload);
    postCopyBarriers[3] = CD3DX12_RESOURCE_BARRIER::Transition(UniqueResSB, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);


    return true;
}

void Mesh::GenerateMeshlets()
{
    HRESULT hr;

    //uint32_t maxVerts = 64;
    //uint32_t maxPrims = 128;
    
    // .smd no indices.. 128:42 leaves no holes in the algor
    uint32_t maxVerts = 128;
    uint32_t maxPrims = 128;

    int nrOfIndices = face.size() * 3;
    uint32_t* tmpIndices = new uint32_t[nrOfIndices];
    for (size_t i = 0; i < face.size(); i++)
    {
        tmpIndices[i * 3 + 0] = face[i].index[0];
        tmpIndices[i * 3 + 1] = face[i].index[1];
        tmpIndices[i * 3 + 2] = face[i].index[2];

        /*tmpIndices[i * 3 + 0] = face[i].index[2];
        tmpIndices[i * 3 + 1] = face[i].index[1];
        tmpIndices[i * 3 + 2] = face[i].index[0];*/
    }
    
    uint32_t nrOfVertices = vert.size();
    DirectX::XMFLOAT3* tmpPositions = new DirectX::XMFLOAT3[nrOfVertices];
    for (size_t i = 0; i < nrOfVertices; i++)
    {
        tmpPositions[i] = vert[i].pos;
    }
    
    hr = ComputeMeshlets(
        tmpIndices,                     // indices
        face.size(),                  // nFaces
        tmpPositions,                   // positions
        nrOfVertices,                   // nVerts
        nullptr,                        // subset
        meshletVector,                  // meshlets
        uniqueVertexIndices,            // uniqueVertexIndices
        primitiveIndices,                // primitiveIndices
        maxVerts,                       // max verts
        maxPrims                       // max prims
    );

    //hr = ComputeMeshlets(
    //    maxVerts,                       // max verts
    //    maxPrims,                       // max prims
    //    tmpIndices,                     // indices
    //    face.size()*3,                  // nFaces
    //    tmpPositions,                   // positions
    //    nrOfVertices,                   // nVerts
    //    subsets,                        // subset
    //    meshletVector,                  // meshlets
    //    uniqueVertexIndices,            // uniqueVertexIndices
    //    primitiveIndices                // primitiveIndices
    //);

    /*std::vector<PackedTriangle> test;
    bool flahg = true;
    for (uint32_t i = 0; i < primitiveIndices.size(); i++)
    {
        for (uint32_t j = 0; j < test.size(); j++)
        {
            if (test[j].indices.i0 == primitiveIndices[i].indices.i0 && test[j].indices.i1 == primitiveIndices[i].indices.i1 && test[j].indices.i2 == primitiveIndices[i].indices.i2)
            {
                primitiveIndices.erase(primitiveIndices.begin() + i);
                j = test.size();
            }
        }
        test.push_back(primitiveIndices[i]);
    }*/


    if (FAILED(hr))
    {
        int by = 0;
    }
    delete[] tmpIndices;
    delete[] tmpPositions;
}


void Mesh::computeDeformFactors(){

    std::vector< XMVECTOR > summator(vert.size(), XMVectorZero());

    for (unsigned int vi=0; vi<vert.size(); vi++){
        for (unsigned int k=0; k<4; k++) {
            vert[vi].deformFactorsTang[k] = 0;
            vert[vi].deformFactorsBtan[k] = 0;

            //vert[vi].weightGradient[k].SetZero();
            //vert[vi].weightGradient[k] = XMFLOAT3(0, 0, 0);
        }
    }

    int nBones = MAX_TOTAL_BONES;

    int overflowCoun = 0;

    for (unsigned int ff=0; ff<face.size(); ff++){
        int pi[3];
        pi[0] = face[ff].index[0];
        pi[1] = face[ff].index[1];
        pi[2] = face[ff].index[2];
        
        
        XMMATRIX m;
        XMFLOAT3 e0 = SubtractFloat3(vert[pi[1]].pos, vert[pi[0]].pos);
        XMFLOAT3 e1 = SubtractFloat3(vert[pi[2]].pos, vert[pi[0]].pos);

        //XMFLOAT3 n = e1^e0; // cross
        XMFLOAT3 n = CrossFloat3(e1,e0);

        //float faceArea = n.Norm();
        float faceArea = LengthFloat3(n);
        SetRowMatrix(m, 0, e0);
        SetRowMatrix(m, 1, e1);
        SetRowMatrix(m, 2, n);
        //m.SetRow(0, e0 );
        //m.SetRow(1, e1 );
        //m.SetRow(2, n );

        m = XMMatrixTranspose(m);

        m = XMMatrixInverse(nullptr, m);
        //m = inverse(m);
        
        std::vector< bool > weightsDone(nBones , false );

        for (int w=0; w<3; w++) {

            for (int r=0; r<4; r++) {

                int bi = vert[ pi[w] ].boneIndex[r];
                if ((bi<0) || (vert[ pi[w] ].boneWeight[r]==0)) continue;
                if ( weightsDone[bi] ) continue;

                weightsDone[bi] = true;

                float dw0 = vert[ pi[1] ].weightOfBone( bi ) - vert[ pi[0] ].weightOfBone( bi );
                float dw1 = vert[ pi[2] ].weightOfBone( bi ) - vert[ pi[0] ].weightOfBone( bi );

                if ((dw0==0)&&(dw1==0)) continue;
                XMFLOAT4 tmpvec = { dw0, dw1, 0, 0 };
                //XMFLOAT3 faceWeightGradient = ( m * tmpvec ) ;

                XMFLOAT4 tmp1 = MulVec4Matrix4x4(tmpvec, m);

                XMFLOAT3 faceWeightGradient = XMFLOAT3(tmp1.x, tmp1.y, tmp1.z);
                //XMStoreFloat3(&faceWeightGradient, XMVector3Transform(tmpvec, m));
                


                // add it to all verteces
                for (int z=0; z<3; z++) {
                    int k = vert[ pi[z] ].slotOfBone( bi );
                    if (k<0) {
                        overflowCoun++;
                        //vert[ pi[z] ].col = 0xFFFF0000;
                        //vert[ pi[z] ].isTextureFlipped *= 1.0;
                    } else {
                        XMFLOAT3 e1 = SubtractFloat3(vert[pi[(z + 1) % 3]].pos, vert[pi[z]].pos);// (vert[pi[(z + 1) % 3]].pos - vert[pi[z]].pos);
                        XMFLOAT3 e2 = SubtractFloat3(vert[ pi[(z+2)%3] ].pos, vert[ pi[z] ].pos);
                        //float wedgeAngle = angle(e1,e2) * faceArea;
                        float wedgeAngle = Angle(e1, e2) * faceArea;
                        //vert[ pi[z] ].deformFactorsTang[k] += (vert[ pi[z] ].tang   * faceWeightGradient) * wedgeAngle;
                        vert[pi[z]].deformFactorsTang[k] += DotFloat3(vert[pi[z]].tang, faceWeightGradient) * wedgeAngle;
                        //vert[ pi[z] ].deformFactorsBtan[k] += (vert[ pi[z] ].bitang * faceWeightGradient) * wedgeAngle;
                        vert[pi[z]].deformFactorsBtan[k] += DotFloat3(vert[pi[z]].bitang, faceWeightGradient) * wedgeAngle;
                        
                        //vert[ pi[z] ].weightGradient[k] += faceWeightGradient * wedgeAngle;
                        //vert[pi[z]].weightGradient[k] += MultiplyFloat3Float(faceWeightGradient, wedgeAngle);
                        //AddToFloat3(vert[pi[z]].weightGradient[k], MultiplyFloat3Float(faceWeightGradient, wedgeAngle));
                        summator[ pi[z] ].m128_f32[k] += wedgeAngle;
                        
                    }
                }
            }
        }

    }

    for (unsigned int vi=0; vi<vert.size(); vi++){
        for (unsigned int k=0; k<4; k++) {
            if (summator[ vi ].m128_f32[k]) {
                vert[vi].deformFactorsTang[k] /= summator[ vi ].m128_f32[k];
                vert[vi].deformFactorsBtan[k] /= summator[ vi ].m128_f32[k];
                //vert[vi].weightGradient[k] /= summator[ vi ].m128_f32[k];
                //divFloat3float(vert[vi].weightGradient[k], summator[vi].m128_f32[k]);
            }
        }
    }


    // shift 1st three
    for (unsigned int vi=0; vi<vert.size(); vi++){
        //vert[vi].orderBoneSlotsWithWeights();
        for (unsigned int k=0; k<3; k++) {
           vert[vi].deformFactorsTang[k] = vert[vi].deformFactorsTang[k+1];
           vert[vi].deformFactorsBtan[k] = vert[vi].deformFactorsBtan[k+1];
        }
    }


    //if (overflowCoun) qDebug("%d overflows!",overflowCoun);
}


/* helper function */
int Vert::slotOfBone(int bi){
    for (int i=0; i<4; i++) {
        if (boneIndex[i]==bi) return i;
    }
    for (int i=0; i<4; i++) {
        if (boneIndex[i]==-1) {
            boneIndex[i] = bi;
            boneWeight[i] = 0;
            return i;
        }
    }
    return -1;
}

/* helper function */
float Vert::weightOfBone(int i) const{
  for ( int k=0;  k<4; k++) if (boneIndex[k]==i) return boneWeight[k];
  return 0;
}

void Mesh::computeTangentDirs(){

    // pass 1: reset tang and bitang
    for (unsigned int vi=0; vi<vert.size(); vi++){
        vert[vi].tang = XMFLOAT3(0,0,0);
        vert[vi].bitang = XMFLOAT3(0,0,0);
    }

    // pass 2: cycle over faces, cumulate tang and bitang
    /* note: we rely on vertex seams, that is, existing vertex duplications
     * in order to store discontinuities of tangent directions.
     */
    for (unsigned int ff=0; ff<face.size(); ff++){
        int vi[3];
        vi[0] = face[ff].index[0];
        vi[1] = face[ff].index[1];
        vi[2] = face[ff].index[2];

        XMFLOAT2 s0=vert[ vi[0] ].uv;
        XMFLOAT2 s1=vert[ vi[1] ].uv;
        XMFLOAT2 s2=vert[ vi[2] ].uv;
        s1 = SubtractFloat2(s1,s0);
        s2 = SubtractFloat2(s2, s0);
        float det = detFloat2(s1, s2);// s1^ s2;
        if (!det) continue;
        float aT,bT,aB,bB;
        aT = -s2.x/det;  bT =  s1.x/det;
        aB =  s2.y/det;  bB = -s1.y/det;

        XMFLOAT3 p0=vert[ vi[0] ].pos;
        XMFLOAT3 p1=vert[ vi[1] ].pos;
        XMFLOAT3 p2=vert[ vi[2] ].pos;
        //p1 -= p0; 
        p1 = SubtractFloat3(p1, p0);
        p2 = SubtractFloat3(p2, p0);
        //float faceArea = (p1^p2).Norm();
        float faceArea = LengthFloat3(CrossFloat3(p1, p2));

        XMFLOAT3 faceTangent = Normalize(AdditionFloat3(MultiplyFloat3Float(p1, aT), MultiplyFloat3Float(p2, bT)));
        XMFLOAT3 faceBitangent = Normalize(AdditionFloat3(MultiplyFloat3Float(p1, aB), MultiplyFloat3Float(p2, bB))); 
        //(p1*aB + p2*bB).normalized();

        for (int z=0; z<3; z++) {
            XMFLOAT3 e1 = SubtractFloat3(vert[ vi[(z+1)%3] ].pos, vert[ vi[z] ].pos);
            XMFLOAT3 e2 = SubtractFloat3(vert[ vi[(z+2)%3] ].pos, vert[ vi[z] ].pos);
          
            float wedgeAngle = Angle(e1, e2) * faceArea;
            //float wedgeAngle = AngelFloat3(e1, e2) * faceArea;

            vert[ vi[z] ].tang = AdditionFloat3(MultiplyFloat3Float(faceTangent, wedgeAngle), vert[vi[z]].tang);
            vert[vi[z]].bitang = AdditionFloat3(MultiplyFloat3Float(faceBitangent, wedgeAngle), vert[vi[z]].bitang);
        }

    }

    // pass 3: for each vertex, make sure tang and bitang are normalized
    //         and orthogonal to normal
    /*for (unsigned int vi=0; vi<vert.size(); vi++){
        std::swap(vert[vi].tang,vert[vi].bitang);
        vert[vi].bitang =    (  vert[vi].norm ^ vert[vi].bitang ^ vert[vi].norm  ) .Normalize();
        vert[vi].tang   =    (  vert[vi].norm ^ vert[vi].tang   ^ vert[vi].norm  ) .Normalize();
    }*/
    for (unsigned int vi = 0; vi < vert.size(); vi++) {
        std::swap(vert[vi].tang, vert[vi].bitang);
        vert[vi].bitang = Normalize(CrossFloat3(CrossFloat3(vert[vi].norm, vert[vi].bitang), vert[vi].norm));
        vert[vi].tang = Normalize(CrossFloat3(CrossFloat3(vert[vi].norm, vert[vi].tang), vert[vi].norm));
        /*vert[vi].bitang = Normalize(CrossFloat3(vert[vi].norm, CrossFloat3(vert[vi].bitang, vert[vi].norm)));
        vert[vi].tang = Normalize(CrossFloat3(vert[vi].norm, CrossFloat3(vert[vi].tang, vert[vi].norm)));*/
    }
}



void Mesh::computeNormals(){

    // a map from vert positions to normals
    std::map< unsigned int, XMFLOAT3 > p2n;

    // pass 1: reset tang and bitang
    for (unsigned int vi=0; vi<vert.size(); vi++){
        p2n[ vi ] = XMFLOAT3(0,0,0);
    }

    // pass 2: cycle over faces, cumulate norms
    for (unsigned int ff=0; ff<face.size(); ff++){
        int vi[3];
        vi[0] = face[ff].index[0];
        vi[1] = face[ff].index[1];
        vi[2] = face[ff].index[2];

        XMFLOAT3 p0=vert[ vi[0] ].pos;
        XMFLOAT3 p1=vert[ vi[1] ].pos;
        XMFLOAT3 p2=vert[ vi[2] ].pos;
        p1 = SubtractFloat3(p1, p0); 
        p2 = SubtractFloat3(p1, p0);

        XMFLOAT3 faceNorm= CrossFloat3(p2, p1); // this includes area weighting

        for (int z=0; z<3; z++) {
            XMFLOAT3 e1 = SubtractFloat3(vert[ vi[(z+1)%3] ].pos, vert[ vi[z] ].pos);
            XMFLOAT3 e2 = SubtractFloat3(vert[ vi[(z+2)%3] ].pos, vert[ vi[z] ].pos);
            float wedgeAngle = AngelFloat3(e1,e2);

            AddToFloat3(p2n[vi[z]], MultiplyFloat3Float(faceNorm, wedgeAngle));
        }

    }

    // pass 3: normalize
    for (unsigned int vi=0; vi<vert.size(); vi++){
        vert[vi].norm = Normalize(p2n[vi]);
    }
}


void Mesh::freezeAt(const Pose &p){

    for (unsigned int vi=0; vi<vert.size(); vi++) {
        Vert &v(vert[vi]);

        XMFLOAT3 restPos = v.pos;

        v.pos = XMFLOAT3(0,0,0);
        for (int k=0; k<MAX_BONES; k++){
            float wieght = v.boneWeight[k];
            int       bi = v.boneIndex [k];
            if (bi>=0 && bi<(int)p.matr.size()) {
                //v.pos += (p.matr[bi] * restPos )*wieght;
                
                v.pos.x += (XMVector3Transform(XMLoadFloat3(&restPos), p.matr[bi]) * wieght).m128_f32[0];
                v.pos.y += (XMVector3Transform(XMLoadFloat3(&restPos), p.matr[bi]) * wieght).m128_f32[1];
                v.pos.z += (XMVector3Transform(XMLoadFloat3(&restPos), p.matr[bi]) * wieght).m128_f32[2];
            }
        }

    }
}

void Mesh::removeUnreferencedVertices(){

    // flag used vertices
    std::vector<bool> used( vert.size(), false );
    for (unsigned int i=0; i<face.size(); i++) {
        for (int w=0; w<3; w++) {
            used[ face[i].index[w] ] = true;
        }
    }

    // compress vertex vector
    std::vector<int> remap( vert.size() );
    int j = 0;
    for (unsigned int i=0; i<vert.size(); i++) {
        if (used[i]) {
            vert[j] = vert[i];
            remap[i] = j;
            j++;
        }
    }
    //qDebug("From %u to %d vertices",vert.size(),j);
    vert.resize(j);

    // update face-vertex links
    for (unsigned int i=0; i<face.size(); i++) {
        for (int w=0; w<3; w++) {
            face[i].index[w] = remap[ face[i].index[w] ];
        }
    }

}

/* fuse togheter coinciding vertices
 * (SMD format is not indexed!)
 *
 * Leave needed vertex seams, i.e. won't fuse
 * togheter vertices with different
 */
void Mesh::unifyVertices(){

    std::map< Vert, int > v2i;
    for (unsigned int i = 0; i < vert.size(); i++) {
        v2i[ vert[i] ] = i;
    }
    for (unsigned int i=0; i<face.size(); i++) {
        for (int w=0; w<3; w++) {
            face[i].index[w] = v2i[ vert[ face[i].index[w] ] ];
        }
    }

    removeUnreferencedVertices();

}


/* vertex ordering, to be used by Mesh::unifyVertices
 * (albitrary lexicographic ordering)
 *
 * Two vertices va and vb will be considered joinable
 * if (!(va<vb) && !(va>vb))
 * that is when va==vb
 */
bool Vert::operator < (const Vert &b) const{

    // if any position or attribute is different, vertex are different
    // (a seam will be left)
    if (LessThanFloat3(pos,b.pos)) return true;
    if (GreaterThanFloat3(pos, b.pos)) return false;
    if (LessThanFloat2(uv, b.uv)) return true;
    if (GreaterThanFloat2(uv, b.uv)) return false;
    if (LessThanFloat3(norm, b.norm)) return true;
    if (GreaterThanFloat3(norm, b.norm)) return false;

    // this is needed to preserve tangent directions:
    // if texture orientation is different, a seam must be left
    if (isTextureFlipped<b.isTextureFlipped) return true;

    // no need: if (isTextureFlipped>b.isTextureFlipped) return false;

    return false;
}

static float sign( float x ){
    return (x<0)?-1:+1;
}

void Mesh::computeIsTextureFlipped(){
    for (unsigned int i=0; i<face.size(); i++) {
        XMFLOAT2 e0 = SubtractFloat2(vert[ face[i].index[2] ].uv, vert[ face[i].index[0] ].uv);
        XMFLOAT2 e1 = SubtractFloat2(vert[ face[i].index[1] ].uv, vert[ face[i].index[0] ].uv);

        float flipped = sign(detFloat2(e0, e1));

        for (int w=0; w<3; w++) vert[ face[i].index[w] ].isTextureFlipped = flipped;
    }
}

void Vert::maybeSwapBoneSlots(int i, int j){
    if (boneWeight[i]<boneWeight[j]) {
        std::swap(boneWeight[i],boneWeight[j]);
        std::swap(boneIndex[i],boneIndex[j]);
    }
}


void Vert::orderBoneSlots(){

    maybeSwapBoneSlots( 0, 1 );
    maybeSwapBoneSlots( 2, 3 );
    maybeSwapBoneSlots( 0, 2 );
    maybeSwapBoneSlots( 1, 3 );
    maybeSwapBoneSlots( 1, 2 );

}

void Mesh::orderBoneSlots(){
    for (unsigned int i=0; i<vert.size(); i++)
        vert[i].orderBoneSlots();
}

void Mesh::clear(){
    face.clear();
    vert.clear();
}

bool Mesh::isEmpty() const{
    return (face.size()==0);
}
