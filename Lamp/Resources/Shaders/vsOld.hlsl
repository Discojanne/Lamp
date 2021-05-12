
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
	float3 norm : NORMAL0;
	float3 tang : TANGENT0;
	float3 bitang : BITANGENT0;
};

VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
	VS_OUTPUT output;
	
	// Vertex Skinning
	float4x4 T = 0;
	for (int i = 0; i < 4; i++)
		T += boneMatrix[input.boneIndex[i]] * input.boneWeight[i];
	
	float3 tmpPos = mul(T, float4(input.pos, 1.0f));
	
	float3 _tang = mul((float3x3) T, input.tang);
	float3 _bitang = mul((float3x3) T, input.bitang);
	float3 _norm = mul((float3x3) T, input.norm);
	
	output.pos = mul(wvpMat, float4(tmpPos.xyz, 1.0f));
	output.uv = input.uv;
	output.norm		= _norm;
	output.tang		= _tang;
	output.bitang	= _bitang;
	
	return output;
}