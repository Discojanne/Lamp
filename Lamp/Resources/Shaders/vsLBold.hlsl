
struct VS_INPUT
{
	float3 pos : POSITION;
	int4 boneIndex : BONEINDEX;
	float4 boneWeight : BONEWEIGHT;
};

cbuffer ConstantBufferTest : register(b0)
{
	float4x4 wvpMat;
	float4x4 boneMatrix[32];
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};

VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
	VS_OUTPUT output;
	
	float4x4 T = 0; 
	
	for (int i = 0; i < 4; i++)
		T += boneMatrix[input.boneIndex[i]] * input.boneWeight[i];

	float4 tmpPos = mul(float4(input.pos, 1.0f),  T);
	output.pos = mul(tmpPos, wvpMat);
	//output.pos = mul(float4(input.pos, 1.0f), wvpMat);
	
	return output;
}