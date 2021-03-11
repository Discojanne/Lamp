
#define ROOT_SIG "CBV(b0), \
                  SRV(t0), \
                  SRV(t1)"


//Texture2D t1 : register(t0);
//SamplerState s1 : register(s0);

cbuffer ConstantBufferTest : register(b0)
{
    float4x4 wvpMat;
	float4x4 boneMatrix[32];
};

struct MSvertex
{
	float4 pos : SV_POSITION;
	//float2 texCoord : TEXCOORD;
	//float3 normal : NORMAL;
    //   //new stuff
	//float3 color : COLOR;
	//float3 lightDir : LIGHTDIR;
	//float v_norm_err : ERR;
};



float4 TransformPos(float4 v) 
{
    return mul(v, wvpMat);
}

static float4 cubeVertices[] =
{
      float4( -0.5f,  0.5f - 5.0f, -0.5f, 1.0f),
      float4(  0.5f, -0.5f - 5.0f, -0.5f, 1.0f),
      float4( -0.5f, -0.5f - 5.0f, -0.5f, 1.0f),
      float4(  0.5f,  0.5f - 5.0f, -0.5f, 1.0f),
      float4(  0.5f, -0.5f - 5.0f, -0.5f, 1.0f),
      float4(  0.5f,  0.5f - 5.0f,  0.5f, 1.0f),
      float4(  0.5f, -0.5f - 5.0f,  0.5f, 1.0f),
      float4(  0.5f,  0.5f - 5.0f, -0.5f, 1.0f),
      float4( -0.5f,  0.5f - 5.0f,  0.5f, 1.0f),
      float4( -0.5f, -0.5f - 5.0f, -0.5f, 1.0f),
      float4( -0.5f, -0.5f - 5.0f,  0.5f, 1.0f),
      float4( -0.5f,  0.5f - 5.0f, -0.5f, 1.0f),
      float4(  0.5f,  0.5f - 5.0f,  0.5f, 1.0f),
      float4( -0.5f, -0.5f - 5.0f,  0.5f, 1.0f),
      float4(  0.5f, -0.5f - 5.0f,  0.5f, 1.0f),
      float4( -0.5f,  0.5f - 5.0f,  0.5f, 1.0f),
      float4( -0.5f,  0.5f - 5.0f, -0.5f, 1.0f),
      float4(  0.5f,  0.5f - 5.0f,  0.5f, 1.0f),
      float4(  0.5f,  0.5f - 5.0f, -0.5f, 1.0f),
      float4( -0.5f,  0.5f - 5.0f,  0.5f, 1.0f),
      float4(  0.5f, -0.5f - 5.0f,  0.5f, 1.0f),
      float4( -0.5f, -0.5f - 5.0f, -0.5f, 1.0f),
      float4(  0.5f, -0.5f - 5.0f, -0.5f, 1.0f),
      float4( -0.5f, -0.5f - 5.0f,  0.5f, 1.0f),
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

struct Vertex
{
	float3 pos;
	int4 boneIndex;
	float4 boneWeight;
};

//struct Meshlet
//{
//	uint VertCount;
//	uint VertOffset;
//	uint PrimCount;
//	uint PrimOffset;
//};

StructuredBuffer<Vertex> Vertices       : register(t0);
StructuredBuffer<int3> PrimitiveIndices : register(t1);
//StructuredBuffer<Meshlet> Meshlets    : register(t2);

[RootSignature(ROOT_SIG)]
[OutputTopology("triangle")]
[NumThreads(128, 1, 1)]
void MSmain(in uint groupThreadId : SV_GroupThreadID,
    out vertices MSvertex outVerts[128], 
    out indices uint3 outIndices[43])
{
	const uint numVertices = 78;
	const uint numPrimitives = 32;

	SetMeshOutputCounts(numVertices, numPrimitives);

	float4x4 T = 0;
    
	if (groupThreadId < numVertices)
	{
        //float4 pos = cubeVertices[groupThreadId];
		T = 0;
		for (int i = 0; i < 4; i++)
			T += boneMatrix[Vertices[groupThreadId].boneIndex[i]] * Vertices[groupThreadId].boneWeight[i];
        
		float3 tmpPos = mul(float4(Vertices[groupThreadId].pos, 1.0f), (float4x3) T);
        
		tmpPos.x += 5.0f;
        
		outVerts[groupThreadId].pos = TransformPos(float4(tmpPos, 1.0f));
		//outVerts[groupThreadId].texCoord = cubeCoords[groupThreadId];
		//outVerts[groupThreadId].normal = float3(1.0f, 0.0f, 0.0f);
		//outVerts[groupThreadId].lightDir = float3(1.0f, 0.0f, 0.0f);
	}

	if (groupThreadId < numPrimitives)
	{
		outIndices[groupThreadId] = PrimitiveIndices[groupThreadId];
	}
    
		
}