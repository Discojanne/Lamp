
#define ROOT_SIG "CBV(b0), \
                  SRV(t0), \
                  SRV(t1), \
                  SRV(t2), \
                  SRV(t3), \
                  DescriptorTable(SRV(t4), visibility = SHADER_VISIBILITY_PIXEL), \
                  StaticSampler(s0, \
                    addressU = TEXTURE_ADDRESS_WRAP, \
                    addressv = TEXTURE_ADDRESS_WRAP, \
                    addressw = TEXTURE_ADDRESS_WRAP, \
                    filter = FILTER_MIN_MAG_MIP_LINEAR)"


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
	float3 tang : TANGENT0;
	float3 bitang : BITANGENT0;
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
	return mul(wvpMat, v);
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
		
		float4x4 T = 0;
		for (int i = 0; i < 4; i++)
			T += boneMatrix[Vertices[vertexIndex].boneIndex[i]] * Vertices[vertexIndex].boneWeight[i];
        
		float3 tmpPos = mul(T, float4(Vertices[vertexIndex].pos, 1.0f));
		tmpPos.x += 10.0f;
		
		float3 _tang = mul((float3x3) T, Vertices[vertexIndex].tang);
		float3 _bitang = mul((float3x3) T, Vertices[vertexIndex].bitang);
		float3 _norm = mul((float3x3) T, Vertices[vertexIndex].norm);
		
		
		
		outVerts[threadID].pos = TransformPos(float4(tmpPos, 1.0f));
		outVerts[threadID].uv = Vertices[vertexIndex].uv;
		outVerts[threadID].norm = _norm;
		outVerts[threadID].tang = _tang;
		outVerts[threadID].bitang = _bitang;
	
		
	}
}