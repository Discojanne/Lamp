
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
	//float3 tang : TANGENT0;
	//float3 bitang : BITANGENT0;
	float3 lightDir : LIGHTDIR;
	float3 v_norm_orig : ORIG;
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

// reduces the length of a vector to X=0.9, if it is bigger
float3 capped(float3 p)
{
	float div = rsqrt(dot(p, p));
	return p * min(1.0f, div * 0.9f);
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
		
		//float4 pos = Skin(vertexIndex);
		////pos = float4(Vertices[vertexIndex].pos,1.0f);
		
		//pos.x += 10.0f; //bridovivel
  //      //pos.x += 3.0f;
		
		//outVerts[threadID].pos = TransformPos(pos);
		//outVerts[threadID].uv = Vertices[vertexIndex].uv;
		//outVerts[threadID].norm = (float4(Vertices[vertexIndex].norm, 0.0f)).xyz;
		//outVerts[threadID].lightDir = float3(0.0f,0.0f,1.0f);
		
		float4 pos4 = float4(Vertices[vertexIndex].pos, 1.0f);
		
		
		float3 q0 = mul(boneMatrix[Vertices[vertexIndex].boneIndex[0]], pos4).xyz;
		float3 q1 = mul(boneMatrix[Vertices[vertexIndex].boneIndex[1]], pos4).xyz - q0;
		float3 q2 = mul(boneMatrix[Vertices[vertexIndex].boneIndex[2]], pos4).xyz - q0;
		float3 q3 = mul(boneMatrix[Vertices[vertexIndex].boneIndex[3]], pos4).xyz - q0;
	
	// eq 14
		float3 _pos = q0
			+ q1 * Vertices[vertexIndex].boneWeight[1]
			+ q2 * Vertices[vertexIndex].boneWeight[2]
			+ q3 * Vertices[vertexIndex].boneWeight[3];
	
		// move it from the vs one
		_pos.x += 10.0f;
		outVerts[threadID].pos = mul(wvpMat, float4(_pos.xyz, 1.0f));
	
	// Vertex Skinning
		float4x4 T = 0;
		for (int i = 0; i < 4; i++)
			T += boneMatrix[Vertices[vertexIndex].boneIndex[i]] * Vertices[vertexIndex].boneWeight[i];
	
	//float3 tmpPos = mul(float4(input.pos, 1.0f), (float4x3) T);
	//float3 tmpPos = float4(input.pos, 1.0f);
	
		float3 _tang = mul((float3x3) T, Vertices[vertexIndex].tang);
		float3 _bitang = mul((float3x3) T, Vertices[vertexIndex].bitang);
	
		 // only for illustration purposes:
		outVerts[threadID].v_norm_orig = normalize(cross(_tang, _bitang)) * Vertices[vertexIndex].isTextureFlipped;
		
	
		_tang += capped(
          Vertices[vertexIndex].deformFactorsTang.x * q1 +
          Vertices[vertexIndex].deformFactorsTang.y * q2 +
          Vertices[vertexIndex].deformFactorsTang.z * q3
    );

		_bitang += capped(
          Vertices[vertexIndex].deformFactorsBitang.x * q1 +
          Vertices[vertexIndex].deformFactorsBitang.y * q2 +
          Vertices[vertexIndex].deformFactorsBitang.z * q3
    );
	
	// vectors are "capped" is because of Sec 4.5

		_tang = normalize(_tang);
		_bitang = normalize(_bitang);
		float3 _norm = normalize(cross(_tang, _bitang)) * Vertices[vertexIndex].isTextureFlipped;
	
		float3 lightDir = float3(0.0f, 0.0f, -1.0f);
	
	// take lightDir to OBJECT space
		lightDir = mul(lightDir, (float3x3) normalMatrix);

    // take lightDir to TANGENT space
		lightDir = float3(
			dot(lightDir, _tang),
			dot(lightDir, _bitang),
			dot(lightDir, _norm)
		);

		outVerts[threadID].lightDir = normalize(lightDir);
		outVerts[threadID].uv = Vertices[vertexIndex].uv;
		
		// only for illustration purposes:
		outVerts[threadID].norm = _norm;

	}
}