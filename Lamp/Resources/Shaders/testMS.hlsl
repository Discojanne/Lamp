
#define ROOT_SIG "CBV(b0)"


cbuffer ConstantBufferTest : register(b0)
{
    float4x4 wvpMat[2];
};

struct MSvertex
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 TransformPos(float4 v) 
{
    return mul(v, wvpMat[1]);
}

static float4 cubeVertices[] =
{
    float4(1.0f, 1.0f, 1.0f, 1.0f),
    float4(1.0f, 1.0f, -1.0f, 1.0f),
    float4(1.0f, -1.0f, 1.0f, 1.0f),
    float4(1.0f, -1.0f, -1.0f, 1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(-1.0f, 1.0f, -1.0f, 1.0f),
    float4(-1.0f, -1.0f, 1.0f, 1.0f),
    float4(-1.0f, -1.0f, -1.0f, 1.0f),
};

static float4 cubeColors[] =
{
    float4(0.0f,0.0f,0.0f,1.0f),
    float4(0.0f,0.0f,1.0f,1.0f),
    float4(0.0f,1.0f,0.0f,1.0f),
    float4(0.0f,1.0f,1.0f,1.0f),
    float4(1.0f,0.0f,0.0f,1.0f),
    float4(1.0f,0.0f,1.0f,1.0f),
    float4(1.0f,1.0f,0.0f,1.0f),
    float4(1.0f,1.0f,1.0f,1.0f),
};

static uint3 cubeIndices[] =
{
    uint3(0,2,1),
    uint3(1,2,3),
    uint3(4,5,6),
    uint3(5,7,6),
    uint3(0,1,5),
    uint3(0,5,4),
    uint3(2,6,7),
    uint3(2,7,3),
    uint3(0,4,6),
    uint3(0,6,2),
    uint3(1,3,7),
    uint3(1,7,5),
};

[RootSignature(ROOT_SIG)]
[OutputTopology("triangle")]
[NumThreads(12, 1, 1)]
void MSmain(
    in uint groupThreadId : SV_GroupThreadID,
    out vertices MSvertex outVerts[8],
    out indices uint3 outIndices[12])
{
    const uint numVertices = 8;
    const uint numPrimitives = 12;

    SetMeshOutputCounts(numVertices, numPrimitives);

    if (groupThreadId < numVertices)
    {
        float4 pos = cubeVertices[groupThreadId];
        outVerts[groupThreadId].pos = TransformPos(pos);
        outVerts[groupThreadId].color = cubeColors[groupThreadId];
    }

    outIndices[groupThreadId] = cubeIndices[groupThreadId];
}