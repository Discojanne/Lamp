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

#include "animation.h"

//#include <QDebug>

typedef unsigned int uint;

void Skeleton::cumulate(Pose &p){
    //p.matr[root].SetIdentity();
    for (uint i=0; i<root.size(); i++)
        cumulateRecusrive(p,root[i]);
}

void Skeleton::cumulateRecusrive(Pose &p, int boneIndex){
    for (uint i=0; i<bone[boneIndex].next.size(); i++) {
        int j = bone[boneIndex].next[i];
        p.matr[ j ] =  p.matr[ boneIndex ] * p.matr[ j ] ;
        cumulateRecusrive( p, j ); // recursive call
    }
}



void Pose::invert(){
    for (uint i=0; i<matr.size(); i++) {
        matr[i] = inverseOfIsometry(matr[i]);
    }
    
}

void Pose::operator *=(const Pose &p){
    assert( p.matr.size() == matr.size() );
    for (uint i=0; i<matr.size(); i++){
        matr[i] = matr[i] * p.matr[i];
    }
}

void Pose::setRotation(int bonei, XMMATRIX r){
   
    /*for (unsigned int i = 0; i < 3; i++)
    {
        SetColMatrix(matr[bonei], i, XMFLOAT3(r.r[i].m128_f32[0], 
            r.r[i].m128_f32[1], r.r[i].m128_f32[2]));
    }*/
    matr[bonei].r[0].m128_f32[0] = r.r[0].m128_f32[0];
    matr[bonei].r[1].m128_f32[0] = r.r[0].m128_f32[1];
    matr[bonei].r[2].m128_f32[0] = r.r[0].m128_f32[2];
    matr[bonei].r[3].m128_f32[0] = r.r[0].m128_f32[3];

    matr[bonei].r[0].m128_f32[1] = r.r[1].m128_f32[0];
    matr[bonei].r[1].m128_f32[1] = r.r[1].m128_f32[1];
    matr[bonei].r[2].m128_f32[1] = r.r[1].m128_f32[2];
    matr[bonei].r[3].m128_f32[1] = r.r[1].m128_f32[3];

    matr[bonei].r[0].m128_f32[2] = r.r[2].m128_f32[0];
    matr[bonei].r[1].m128_f32[2] = r.r[2].m128_f32[1];
    matr[bonei].r[2].m128_f32[2] = r.r[2].m128_f32[2];
    matr[bonei].r[3].m128_f32[2] = r.r[2].m128_f32[3];

    // new
    matr[bonei] = DirectX::XMMatrixTranspose(matr[bonei]);
}

void Pose::setTranslation(int bonei, XMFLOAT3 t){

    //matr[bonei].r[0].m128_f32[3] = t.x;
    //matr[bonei].r[1].m128_f32[3] = t.y;
    //matr[bonei].r[2].m128_f32[3] = t.z;
    //matr[bonei].r[3].m128_f32[3] = 1.0f;

    // new
    matr[bonei].r[3].m128_f32[0] = t.x;
    matr[bonei].r[3].m128_f32[1] = t.y;
    matr[bonei].r[3].m128_f32[2] = t.z;
    matr[bonei].r[3].m128_f32[3] = 1.0f;
}

void Skeleton::buildTree(){

    //root = -1;
    root.clear();
    for (unsigned int i=0; i<bone.size(); i++)
        bone[i].next.clear();
    for (unsigned int i=0; i<bone.size(); i++){
        int a=bone[i].attach;
        if (a==-1) {
            root.push_back(i);
        }
        else {
            bone[a].next.push_back(i);
        }
    }
    assert(root.size());
}


void Skeleton::clear(){
    bone.clear();
    root.clear();
}

void Animation::clear(){
    pose.clear();
}

bool Animation::isEmpty() const{
    return (pose.size()==0);
}
