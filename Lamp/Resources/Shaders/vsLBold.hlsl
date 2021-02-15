
struct VS_INPUT
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 norm : NORMAL;
	float3 tang : TANGENT;
	float3 bitang : BITANGENT;
    //unsure of the semantic 
	int4 boneIndex : BONEINDEX;
	float4 boneWeight : BONEWEIGHT;
	
	float3 vec3deformFactorsTang : DFT;
	float3 vec3deformFactorsBitang : DFB;
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
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 lightDir : LIGHTDIR;
	float v_norm_err : ERR;
};

VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
	VS_OUTPUT output;
	
	float4x4 T =
				boneMatrix[input.boneIndex[0]] * input.boneWeight[0] +
				boneMatrix[input.boneIndex[1]] * input.boneWeight[1] +
				boneMatrix[input.boneIndex[2]] * input.boneWeight[2] +
				boneMatrix[input.boneIndex[3]] * input.boneWeight[3];
	
	//float4x4 T = 0;

	//if (input.boneIndex[0] >= 0)
	//	T = T + boneMatrix[input.boneIndex[0]] * input.boneWeight[0];

	//if (input.boneIndex[1] >= 0)
	//	T = T + boneMatrix[input.boneIndex[1]] * input.boneWeight[1];
	
	//if (input.boneIndex[2] >= 0)
	//	T = T + boneMatrix[input.boneIndex[2]] * input.boneWeight[2];
	
	//if (input.boneIndex[3] >= 0)
	//	T = T + boneMatrix[input.boneIndex[3]] * input.boneWeight[3];

	float4 tmpPos = mul(float4(input.pos, 1.0f), T);
	
	output.pos = mul(tmpPos, wvpMat);
	//output.pos = mul(float4(input.pos, 1.0f), wvpMat);

	
	
	
	
	
	
	output.lightDir = float3(0.0f, 0.0f, 1.0f);

	float3 _tang = mul(T, float4(input.tang, 0.0f)).xyz;
	float3 _bitang = mul(T, float4(input.bitang, 0.0f)).xyz;
	float3 _norm = mul(T, float4(input.norm, 0.0f)).xyz;


	float3 lightDir = float3(0.0f, 0.0f, 1.0f); // in VIEW space

    // take lightDir to OBJECT space
	lightDir = mul(float4(output.lightDir, 0.0f), normalMatrix).xyz;

    // take lightDir to TANGENT space
	lightDir = float3(
       dot(lightDir, _tang),
       dot(lightDir, _bitang),
       dot(lightDir, _norm)
    );

	output.lightDir = normalize(lightDir);
	output.normal = mul(float4(input.norm, 0.0f), wvpMat).xyz;
	output.texCoord = input.uv;
	
	return output;
}