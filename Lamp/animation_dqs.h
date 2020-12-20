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

#ifndef ANIMATION_DQS_H
#define ANIMATION_DQS_H

#include "dual_quaternion.h"


/* class AnimationDQS:
 *
 * Animations done with Dual Quaterion Skinning.
 *
 * This one substitutes class Animation (which uses matrices instead, i.e. Linear Blend Skinning).
 * If you use DQS, you need class Animations only as temporary variables during construction
 */

class Animation;
class Pose;

struct PoseDQS{
    std::vector< DualQuaternion > quat;
    void buildFromPose( Pose& p);

};

struct AnimationDQS
{
public:
    std::vector< PoseDQS > pose;
    AnimationDQS();
    void buildFromAnimation( Animation& p);
};

#endif // ANIMATION_DQS_H
