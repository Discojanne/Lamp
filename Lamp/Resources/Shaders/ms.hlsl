
#define ROOT_SIG "CBV(b0), \
                  SRV(t0), \
                  SRV(t1), \
                  SRV(t2)"


//Texture2D t1 : register(t0);
//SamplerState s1 : register(s0);

cbuffer ConstantBufferTest : register(b0)
{
    float4x4 wvpMat;
	float4x4 boneMatrix[32];
};

struct VertexOut
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



struct Vertex
{
	float3 pos;
	int4 boneIndex;
	float4 boneWeight;
};

struct Meshlet
{
	uint VertCount;
	uint VertOffset;
	uint PrimCount;
	uint PrimOffset;
};

StructuredBuffer<Meshlet>	Meshlets			: register(t0);
StructuredBuffer<Vertex>	Vertices			: register(t1);
StructuredBuffer<int3>		PrimitiveIndices	: register(t2);

float4 Skin(uint vertexID)
{
	float4x4 T = 0;
	for (int i = 0; i < 4; i++)
		T += boneMatrix[Vertices[vertexID].boneIndex[i]] * Vertices[vertexID].boneWeight[i];
        
	float3 tmpPos = mul(float4(Vertices[vertexID].pos, 1.0f), (float4x3) T);
	return float4(tmpPos, 1.0f);
}

[RootSignature(ROOT_SIG)]
[OutputTopology("triangle")]
[NumThreads(128, 1, 1)]
void MSmain(in uint threadID : SV_GroupThreadID, in uint groupID : SV_GroupID,
    out vertices VertexOut outVerts[128],
    out indices uint3 outIndices[64])
{
	
	Meshlet m = Meshlets[groupID];

	SetMeshOutputCounts(m.VertCount, m.PrimCount);
	
	if (threadID < m.PrimCount)
	{
		outIndices[threadID] = PrimitiveIndices[threadID + m.PrimOffset];
	}
	
	if (threadID < m.VertCount)
	{
		float4 pos = Skin(threadID + m.VertOffset);
        
		pos.x += 50.0f;
        
		outVerts[threadID].pos = TransformPos(pos);
	}
}