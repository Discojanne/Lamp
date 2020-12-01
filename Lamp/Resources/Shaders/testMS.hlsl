
//#define ROOT_SIG "CBV(b0)"


cbuffer ConstantBufferTest : register(b0)
{
    float4x4 wvpMat[2];
};

struct MSvertex
{
    float4 pos : SV_POSITION;
    float2 texCoord: TEXCOORD;
};

float4 TransformPos(float4 v) 
{
    return mul(v, wvpMat[1]);
}

static float4 cubeVertices[] =
{
      float4( -0.5f,  0.5f, -0.5f, 1.0f),
      float4(  0.5f, -0.5f, -0.5f, 1.0f),
      float4( -0.5f, -0.5f, -0.5f, 1.0f),
      float4(  0.5f,  0.5f, -0.5f, 1.0f),
      float4(  0.5f, -0.5f, -0.5f, 1.0f),
      float4(  0.5f,  0.5f,  0.5f, 1.0f),
      float4(  0.5f, -0.5f,  0.5f, 1.0f),
      float4(  0.5f,  0.5f, -0.5f, 1.0f),
      float4( -0.5f,  0.5f,  0.5f, 1.0f),
      float4( -0.5f, -0.5f, -0.5f, 1.0f),
      float4( -0.5f, -0.5f,  0.5f, 1.0f),
      float4( -0.5f,  0.5f, -0.5f, 1.0f),
      float4(  0.5f,  0.5f,  0.5f, 1.0f),
      float4( -0.5f, -0.5f,  0.5f, 1.0f),
      float4(  0.5f, -0.5f,  0.5f, 1.0f),
      float4( -0.5f,  0.5f,  0.5f, 1.0f),
      float4( -0.5f,  0.5f, -0.5f, 1.0f),
      float4(  0.5f,  0.5f,  0.5f, 1.0f),
      float4(  0.5f,  0.5f, -0.5f, 1.0f),
      float4( -0.5f,  0.5f,  0.5f, 1.0f),
      float4(  0.5f, -0.5f,  0.5f, 1.0f),
      float4( -0.5f, -0.5f, -0.5f, 1.0f),
      float4(  0.5f, -0.5f, -0.5f, 1.0f),
      float4( -0.5f, -0.5f,  0.5f, 1.0f),
};

static float2 cubeCoords[] =
{
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
};

static uint3 cubeIndices[] =
{
        uint3(0, 1, 2   ),
        uint3(0, 3, 1   ), 
        uint3(4, 5, 6   ),
        uint3(4, 7, 5   ),
        uint3(8, 9, 10  ),
        uint3(8, 11, 9  ),
        uint3(12, 13, 14),
        uint3(12, 15, 13),
        uint3(16, 17, 18),
        uint3(16, 19, 17),
        uint3(20, 21, 22),
        uint3(20, 23, 21),
};

[RootSignature(ROOT_SIG)]
[OutputTopology("triangle")]
[NumThreads(24, 1, 1)]
void MSmain(
    in uint groupThreadId : SV_GroupThreadID,
    out vertices MSvertex outVerts[24],
    out indices uint3 outIndices[12])
{
    const uint numVertices = 24;
    const uint numPrimitives = 12;

    SetMeshOutputCounts(numVertices, numPrimitives);

    if (groupThreadId < numVertices)
    {
        float4 pos = cubeVertices[groupThreadId];
        outVerts[groupThreadId].pos = TransformPos(pos);
        outVerts[groupThreadId].texCoord = cubeCoords[groupThreadId];
    }

    outIndices[groupThreadId] = cubeIndices[groupThreadId];
}