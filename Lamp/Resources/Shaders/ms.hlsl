
#define ROOT_SIG "CBV(b0), \
                  SRV(t0), \
                  SRV(t1), \
                  SRV(t2), \
                  SRV(t3)"
//\
//                  SRV(t4), \
//                  StaticSampler(s0)"


//Texture2D t1 : register(t0);
//SamplerState s1 : register(s0);

cbuffer ConstantBufferTest : register(b0)
{
	float4x4 wvpMat;
	float4x4 normalMatrix;
	float4x4 boneMatrix[32];
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 norm : NORMAL0;
	float3 lightDir : LIGHTDIR;
	//float4 color : COLOR0;
};

struct Vertex
{
	float3 pos;
	float2 uv;
	float3 norm;
	float3 tang;
	float3 bitang;
	
	int4 boneIndex;
	float4 boneWeight;
	
	float3 deformFactorsTang;
	float3 deformFactorsBitang;
	float isTextureFlipped;
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
StructuredBuffer<uint>		PrimitiveIndices	: register(t2);
ByteAddressBuffer			UniqueVertexIndices : register(t3);

uint3 UnpackPrimitive(uint primitive)
{
    // Unpacks a 10 bits per index triangle from a 32-bit uint.
	//return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
	return uint3((primitive >> 20) & 0x3FF, (primitive >> 10) & 0x3FF, primitive & 0x3FF); // vände på trianglarna?
}

uint3 GetPrimitive(Meshlet m, uint index)
{
	return UnpackPrimitive(PrimitiveIndices[m.PrimOffset + index]);
}

float4 TransformPos(float4 v)
{
	return mul(v, wvpMat);
}

float4 Skin(uint vertexID)
{
	float4x4 T = 0;
	for (int i = 0; i < 4; i++)
		T += boneMatrix[Vertices[vertexID].boneIndex[i]] * Vertices[vertexID].boneWeight[i];
        
	//float3 tmpPos = mul(float4(Vertices[vertexID].pos, 1.0f), (float4x3) T);
	float3 tmpPos = mul(T, float4(Vertices[vertexID].pos, 1.0f));
	return float4(tmpPos, 1.0f);
}

[RootSignature(ROOT_SIG)]
[OutputTopology("triangle")]
[NumThreads(128, 1, 1)]
void MSmain(in uint threadID : SV_GroupThreadID, in uint groupID : SV_GroupID,
    out vertices VertexOut outVerts[128],
    out indices uint3 outIndices[128])
{
	
	Meshlet m = Meshlets[groupID];

	SetMeshOutputCounts(m.VertCount, m.PrimCount);
	
	if (threadID < m.PrimCount)
	{
		outIndices[threadID] = GetPrimitive(m, threadID);
	}
	
	if (threadID < m.VertCount)
	{
		uint vertexIndex = UniqueVertexIndices.Load((threadID + m.VertOffset) * 4);
		
		float4 pos = Skin(vertexIndex);
		//pos = float4(Vertices[vertexIndex].pos,1.0f);
		
		pos.x += 10.0f; //bridovivel
        //pos.x += 3.0f;
		
		outVerts[threadID].pos = TransformPos(pos);
		outVerts[threadID].uv = Vertices[vertexIndex].uv;
		outVerts[threadID].norm = (float4(Vertices[vertexIndex].norm, 0.0f)).xyz;
		outVerts[threadID].lightDir = float3(0.0f,0.0f,1.0f);
		//outVerts[threadID].color = (int)groupID * float4(1.0f,1.0f,1.0f,1.0f);

	}
}