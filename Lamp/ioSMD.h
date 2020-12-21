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
#include <stdio.h>
#ifndef IOSMD_H
#define IOSMD_H

class Mesh;
class Animation;
class Pose;
class Skeleton;

/* a static class to import SMD files (meshes and animations) */

class ioSMD
{
public:

  // a SMD file can contain both. The one which is not contained will be "isEmpty"
	static bool import(FILE* file, Mesh& m, Animation& a);

  static const char* lastErrorString();

private:

  static bool importPose(FILE* f, Pose &pose);
  static bool importTriangles(FILE*f, Mesh &m );
  static bool importNodes(FILE*f,Skeleton &s );

};

#endif // IOSMD_H
