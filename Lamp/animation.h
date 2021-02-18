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

#ifndef ANIMATION_H
#define ANIMATION_H


#include "ExtraMath.h"
/*
 * classes for animations (Linear Blend Skinning).
 */



/* class Pose: a set of per-bone transformations */

struct Pose{
    std::vector< XMMATRIX > matr;

    void invert();
    void operator *=( const Pose& p );

    void setRotation( int bonei, XMMATRIX r );
    void setTranslation( int bonei, XMFLOAT3 r );

};


/* class Animation: a sequence of Poses */

struct Animation
{
    std::vector< Pose > pose;
    void clear();
    bool isEmpty() const;
};



/* classes Bone and Skeleton: needed only for loading and construction */

struct Bone{
    int attach; // index of bone this bone is attached to
    std::vector<int> next; // index of bone(s) attached to this bone
};

struct Skeleton{
    std::vector< int > root; // many roots?
    std::vector< Bone > bone;

    void cumulate( Pose &p );
    void buildTree();
    void clear();
private:
    void cumulateRecusrive( Pose &p , int boneIndex );
};


#endif // ANIMATION_H
