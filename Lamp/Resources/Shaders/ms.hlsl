
#define ROOT_SIG "CBV(b0), \
                  SRV(t0), \
                  SRV(t1), \
                  SRV(t2), \
                  SRV(t3)"


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
	float4 color : COLOR0;
	//float2 texCoord : TEXCOORD;
	//float3 normal : NORMAL;
    //   //new stuff
	//float3 color : COLOR;
	//float3 lightDir : LIGHTDIR;
	//float v_norm_err : ERR;
};

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
		outIndices[threadID] = GetPrimitive(m, threadID);
	}
	
	if (threadID < m.VertCount)
	{
		uint vertexIndex = UniqueVertexIndices.Load((threadID + m.VertOffset) * 4);
		
		float4 pos = Skin(vertexIndex);
        
		pos.x += 50.0f; //bridovivel
        //pos.x += 3.0f;
		
		outVerts[threadID].pos = TransformPos(pos);
			outVerts[threadID].color = (int)groupID * float4(1.0f,1.0f,1.0f,1.0f);

	}
}