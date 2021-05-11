//#define ROOT_SIG2 "CBV(b0), \
//                  SRV(t0), \
//                  StaticSampler(s0)"


struct VS_INPUT
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD0;
	float3 norm : NORMAL0; // not used
	float3 tang : TANGENT0;
	float3 bitang : BITANGENT0;
	
	int4 boneIndex : BONEINDEX;
	float4 boneWeight : BONEWEIGHT;
	
	float3 deformFactorsTang : DEFORMTANG;
	float3 deformFactorsBitang : DEFORMTBTAN;
	float isTextureFlipped : FLIPPED;
};

cbuffer ConstantBufferTest : register(b0)
{
	float4x4 wvpMat;
	float4x4 normalMatrix;
	float4x4 boneMatrix[32];
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	//float3 norm : NORMAL0;
	//float3 tang : TANGENT0;
	//float3 bitang : BITANGENT0;
	float3 lightDir : LIGHTDIR;
};

// reduces the length of a vector to X=0.9, if it is bigger
float3 capped(float3 p)
{
	float div = rsqrt(dot(p, p));
	return p * min(1.0f, div * 0.9f);
}

//[RootSignature(ROOT_SIG2)]
VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
	VS_OUTPUT output;
	
	float4 pos4 = float4(input.pos, 1.0f);
	
	float3 q0 = mul(boneMatrix[input.boneIndex[0]], pos4).xyz;
	float3 q1 = mul(boneMatrix[input.boneIndex[1]], pos4).xyz - q0;
	float3 q2 = mul(boneMatrix[input.boneIndex[2]], pos4).xyz - q0;
	float3 q3 = mul(boneMatrix[input.boneIndex[3]], pos4).xyz - q0;
	
	// eq 14
	float3 _pos = q0 
	+ q1 * input.boneWeight[1]
	+ q2 * input.boneWeight[2] 
	+ q3 * input.boneWeight[3];
	
	output.pos = mul(wvpMat, float4(_pos.xyz, 1.0f));
	
	// Vertex Skinning
	float4x4 T = 0;
	for (int i = 0; i < 4; i++)
		T += boneMatrix[input.boneIndex[i]] * input.boneWeight[i];
	
	//float3 tmpPos = mul(float4(input.pos, 1.0f), (float4x3) T);
	//float3 tmpPos = float4(input.pos, 1.0f);
	
	float3 _tang = mul(T, float4(input.tang, 0.0f)).xyz;
	float3 _bitang = mul(T, float4(input.bitang, 0.0f)).xyz;
	//float3 _norm = mul((float3x3) T, input.norm);
	//float3 _tang = input.tang;
	//float3 _bitang = input.bitang;
	//float3 _norm = input.norm;
	
	_tang += capped(
          input.deformFactorsTang.x * q1 +
          input.deformFactorsTang.y * q2 +
          input.deformFactorsTang.z * q3
    );

	_bitang += capped(
          input.deformFactorsBitang.x * q1 +
          input.deformFactorsBitang.y * q2 +
          input.deformFactorsBitang.z * q3
    );
	
	// vectors are "capped" is because of Sec 4.5

	_tang = normalize(_tang);
	_bitang = normalize(_bitang);
	float3 _norm = normalize(cross(_tang, _bitang)) * input.isTextureFlipped;
	
	output.lightDir = float3(0.0f, 0.0f, -1.0f);
	
	// take lightDir to OBJECT space
	output.lightDir = mul(float4(output.lightDir, 0.0f), normalMatrix).xyz;

    // take lightDir to TANGENT space
	output.lightDir = float3(
       dot(output.lightDir, _tang),
       dot(output.lightDir, _bitang),
       dot(output.lightDir, _norm)
    );

	output.lightDir = normalize(output.lightDir);
	
	
	
	//output.pos = mul(float4(tmpPos.xyz, 1.0f), wvpMat);
	output.uv = input.uv;
	//output.norm		= _norm;
	//output.tang		= _tang;
	//output.bitang	= _bitang;
	
	//output.pos = mul(float4(input.pos, 1.0f), wvpMat);
	
	return output;
}