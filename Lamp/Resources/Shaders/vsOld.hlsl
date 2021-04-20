#define ROOT_SIG2 "CBV(b0), \
                  SRV(t0), \
                  StaticSampler(s0)"


struct VS_INPUT
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD0;
	float3 norm : NORMAL0;
	float3 tang : TANGENT0;
	float3 bitang : BITANGENT0;
	
	int4 boneIndex : BONEINDEX;
	float4 boneWeight : BONEWEIGHT;
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

[RootSignature(ROOT_SIG2)]
VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
	VS_OUTPUT output;
	
	// Vertex Skinning
	float4x4 T = 0;
	for (int i = 0; i < 4; i++)
		T += boneMatrix[input.boneIndex[i]] * input.boneWeight[i];
	
	float3 tmpPos = mul(float4(input.pos, 1.0f), (float4x3) T);
	
	float3 _tang = mul((float3x3) T, input.tang);
	float3 _bitang = mul((float3x3) T, input.bitang);
	float3 _norm = mul((float3x3) T, input.norm);
	
	output.pos = mul(float4(tmpPos.xyz, 1.0f), wvpMat);
	output.uv = input.uv;
	output.norm		= _norm;
	output.tang		= _tang;
	output.bitang	= _bitang;
	
	//output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	//output.pos = mul(float4(input.pos, 1.0f), wvpMat);
	
	return output;
}