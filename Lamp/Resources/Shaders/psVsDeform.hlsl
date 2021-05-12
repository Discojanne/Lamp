

Texture2D t1 : register(t0); // normal
//Texture2D t2 : register(t0);
SamplerState s1 : register(s0);


struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	//float3 norm : NORMAL0;
	//float3 tang : TANGENT0;
	//float3 bitang : BITANGENT0;
	float3 lightDir : LIGHTDIR;
};


float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	
	float4 color;
	color.a = 1.0f;
	float3 normalMap = t1.Sample(s1, input.uv).rgb * 2.0f - 1.0f;
	float3 specCol = float3(0.1f, 0.1f, 0.1f);

	float diffuse = dot(input.lightDir, normalMap);
	const float3 baseCol = float3(0.80f, 0.80f, 1.0f);

	color.rgb = baseCol * (0.25f + 0.75f * diffuse) + specCol * pow(diffuse, 24.0f);
	
	return color;
	
}
