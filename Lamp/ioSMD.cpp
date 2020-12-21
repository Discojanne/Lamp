#include "mesh.h"
#include "animation.h"

#include "ioSMD.h"


//#include <QDebug>

// used to export/import skeletons, rigged meshes, animations...

XMMATRIX euler2matrix(float* eul){
    XMMATRIX m;
    //m.FromEulerAngles(eul[0], eul[1], eul[2]);
    FromEulerAngles(m, eul[0], eul[1], eul[2]);
    // notation clash (sigh): swap Y-Z axes
    float f[16]={1,0,0,0, 0,0,1,0, 0,1,0,0, 0,0,0,1};
    XMMATRIX axesSwapper(f);
    m = axesSwapper * m * axesSwapper;

    return m;
}

static int lastErr;
const char *expectedErr, *foundErr;
int versionErr;

static bool expect(FILE* f, const char* what){
    static char str[255];
    fscanf(f, "%s", str);
    if (strcmp(str,what)){
        expectedErr = what;
        foundErr = str;
        lastErr = 4;
        return false;
    }
    return true;
}

static bool expectLine(FILE* f, const char* what){
    static char str[255];
    fscanf(f, "%s\n", str);
    if (strcmp(str,what)){
        expectedErr = what;
        foundErr = str;
        lastErr = 4;
        return false;
    }
    return true;
}

static bool fscanln(FILE*f, char *ln){
    int i=0;
    while (1) {
        fread(&ln[i],1,1,f);
        if (ln[i]=='\n') { ln[i]=0; return true;}
        if (ln[i]==13) { fread(&ln[i],1,1,f); ln[i]=0; return true;}
        i++;
    }
}

bool ioSMD::importTriangles(FILE*f, Mesh &m ){

    int pi=0;

    while (1){
        char matName[4096];
        fscanln(f, matName); //
        if (strcmp(matName,"end")==0) break;
        if (strcmp(matName,"end\10")==0) break;
        for (int w=0; w<3; w++) {
            int bi;
            Vert v;
            char line[4096];

            int nr=0;
            fscanln(f, line);
            int tmp[4];
            int nread =
                 sscanf(line,"%d %f %f %f %f %f %f %f %f %d %d %f %d %f %d %f %d %f",
                    &bi,
                    &(v.pos.x),&(v.pos.z),&(v.pos.y),
                    &(v.norm.x),&(v.norm.z),&(v.norm.y),
                    &(v.uv.x),&(v.uv.y),
                    &nr,
                    tmp+0, &(v.boneWeight[0]),
                    tmp+1, &(v.boneWeight[1]),
                    tmp+2, &(v.boneWeight[2]),
                    tmp+3, &(v.boneWeight[3])
            );
            if (nr>4) { nr=4;}
            //if  (!( nread==9 || nread == 9+1+nr*2)) qDebug("[%s] (w:%d f:%d),",line,w,m.face.size());
            assert( nread==9 || nread == 9+1+nr*2);
            for (int k=0; k<nr; k++) {
                v.boneIndex[k]=tmp[k];
            }
            for (int k=nr; k<4; k++) {
                v.boneIndex[k]=-1; v.boneWeight[k]=0;
            }
            float sumW = 0;
            for (int k = 0; k<4; k++) sumW += v.boneWeight[k];
            if (sumW<0.999999) {
                if (nr<4) {
                    v.boneIndex[nr] = bi;
                    v.boneWeight[nr] = 1-sumW;
                }
            }

            /* convention clash: flip vertical texture coordinate (sigh) */
            v.uv.y=1-v.uv.y;

            m.vert.push_back(v);

            pi++;
        }
        m.face.push_back( Face( pi-3, pi-2, pi-1 ) );
    }
    return true;
}


bool ioSMD::importNodes(FILE*f,Skeleton &s ){
    s.clear();

    int v=-1;
    if (!expect(f,"version")) return false;
    fscanf(f, "%d\n", &v);
    if (v!=1) { versionErr = v; lastErr=3; return false;}

    if (!expectLine(f,"nodes")) return false;

    while (1) {
        int a, b;
        char line[4096];
        char st[4096];
        fscanln(f,line);

        int res = sscanf(line,"%d \"%s %d",&a, st, &b);
        if (res<3) {
            line[3]=0; // clear str
            if (strcmp(line,"end")!=0) {
                expectedErr = "end";
                foundErr = line;
                lastErr = 4;
                return false;
            }
            break;
        }
        // remove ending '"'
        assert(st[strlen(st)-1]=='"');
        st[strlen(st)-1]=0;

        if (b==-1) {// here is a root
            //if (rootFound) continue; // ignore extra roots;
            //rootFound=true;
            s.root.push_back( b );
        }
        if (a>=(int)s.bone.size()) s.bone.resize(a+1);
        s.bone[a].attach=b;
        //sprintf(s.bone[a].name,"%s",st);
    }
    return true;
}


bool ioSMD::importPose(FILE* f, Pose &pose){
    if (!expect(f,"time")) return false;

    int time; // to be read but be to be ignored
    fscanf(f,"%d",&time);

    while (1) {
        int i;
        int res = fscanf(f,"%d",&i);
        if (res==0) 
            break; // hopefully it is an "end"
        //assert(i<(int)s.bone.size());
        float r[3];
        XMFLOAT3 t;
        fscanf(f,"%f %f %f %f %f %f", &(t.x),&(t.z),&(t.y), r+0, r+1, r+2);
        //if (i>=(int)s.bone.size()) continue; // ignore rotation for non-existing bones
        if (i>=(int)pose.matr.size()) pose.matr.resize(i+1);
        pose.setRotation( i, euler2matrix(r) );
        pose.setTranslation( i, t );
    }
    return true;
}



bool ioSMD::import(FILE* f, Mesh &m , Animation &a ){
    a.clear();
    m.clear();

    lastErr = 0;

    Skeleton s;

    if (!importNodes(f,s)) return false;
    s.buildTree();

    if (!expect(f,"skeleton")) return false;

    // we assume the first pose is the rest pose
    Pose restPose;
    if (!importPose(f,restPose)) return false; // initial pose
    s.cumulate( restPose );
    restPose.invert();

    a.pose.clear();
    while (1) {
        Pose p;
        if (!importPose(f,p)) break;
        s.cumulate( p );
        p *= restPose;
        a.pose.push_back( p );
    }

    expectedErr="end";
    if (strcmp(foundErr,expectedErr)!=0) {
        fclose(f);
        return false;
    }

    if (!expectLine(f,"triangles")) return true; // all ok

    if (!importTriangles(f,m)) return false;

    fclose(f);
    return true;
}


const char* ioSMD::lastErrorString(){
    static char res[255];
    switch(lastErr) {
    case 1: return "File not found"; break;
    case 2: return "Cannot open file for writing"; break;
    case 3: sprintf(res,"Version %d not supported",versionErr); return res; break;
    case 4: sprintf(res,"Expected '%s' found '%s'",expectedErr, foundErr); return res; break;
    case 0: return "(no error)"; break;
    default: return "undocumented error"; break;
    }
}


