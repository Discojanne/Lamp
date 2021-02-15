
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

	static bool importPose(FILE* f, Pose& pose);
	static bool importTriangles(FILE* f, Mesh& m);
	static bool importNodes(FILE* f, Skeleton& s);

};

#endif // IOSMD_H
