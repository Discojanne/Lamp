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

#include "animation_dqs.h"
#include "animation.h"

//#include "qdebug.h"

typedef unsigned int uint;


AnimationDQS::AnimationDQS()
{
}

void PoseDQS::buildFromPose( Pose& p){
    quat.resize( p.matr.size() );
    for (uint i=0; i<quat.size(); i++){
        quat[i].fromMatrix( p.matr[i] );
    }
}


void AnimationDQS::buildFromAnimation( Animation& a){
    pose.resize( a.pose.size() );
    for (uint i=0; i<pose.size(); i++){
        pose[i].buildFromPose( a.pose[i] );
    }
}

