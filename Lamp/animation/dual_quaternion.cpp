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

#include "dual_quaternion.h"



/* convention clash! (sigh)
 * in vcg, real component of quaternion is the first one
 * anywhere else, it is the last one
 */
void putRealFirst( Quaternion &a ){
    a = Quaternion( a.w, a.x, a.y, a.z );
}

void putRealLast( Quaternion &a ){
    a = Quaternion( a.y, a.z, a.w, a.x );
}

void DualQuaternion::fromMatrix(const XMMATRIX &m)
{
    //a.FromMatrix(m);
    MatrixToQuaternion(m, a);

    //b = m.GetColumn4(3) / 2;
    b = XMFLOAT4(m.r[0].m128_f32[3] / 2, m.r[1].m128_f32[3] / 2, m.r[2].m128_f32[3] / 2, m.r[3].m128_f32[3] / 2);
    b.w  = 0;

    putRealFirst(b);

    //b = b * a;
    // unsure if this is the right function for this
    b = Hamilton(b, a);

    putRealLast(a);
    putRealLast(b);
}


float DualQuaternion::aW() { return  a.w; }
float DualQuaternion::bW() { return  b.w; }
XMFLOAT3 DualQuaternion::aXYZ() { return XMFLOAT3(a.x,a.y,a.z); }
XMFLOAT3 DualQuaternion::bXYZ() { return XMFLOAT3(b.x,b.y,b.z); }

XMFLOAT3 DualQuaternion::applyToPoint( XMFLOAT3 pos ){
    return AdditionFloat3(pos, MultiplyFloat3Float(
        SubtractFloat3(AdditionFloat3(CrossFloat3( aXYZ(), AdditionFloat3(AdditionFloat3(CrossFloat3( aXYZ(), pos), 
            MultiplyFloat3Float(pos, aW())), bXYZ())), MultiplyFloat3Float(bXYZ(), aW())), MultiplyFloat3Float(aXYZ(), bW())), 2.0));
}


void DualQuaternion::normalize(){
    float len = LengthFloat4(a);// a.Norm();
    divFloat4float(a, len); //a /= len; 
    divFloat4float(b, len);
    //b -= a*dot(a,b);
    b = SubtractFloat4(b, MultiplyFloat4Float(a, DotFloat4(a, b)));
}

void DualQuaternion::multiplyAndAdd( DualQuaternion d, float scale ){
    if (DotFloat4(a,d.a)<0) {
        d.a = MultiplyFloat4Float(d.a, -1.0f);
        d.b = MultiplyFloat4Float(d.b, -1.0f);
    }
    a = AdditionFloat4(a, MultiplyFloat4Float(d.a, scale));
    b = AdditionFloat4(b, MultiplyFloat4Float(d.b, scale));
}

void DualQuaternion::mult( float scale ){
    a = MultiplyFloat4Float(a, scale);
    b = MultiplyFloat4Float(b, scale);
}


