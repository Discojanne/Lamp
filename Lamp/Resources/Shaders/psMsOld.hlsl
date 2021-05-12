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

Texture2D t1 : register(t4); // normal
//Texture2D t2 : register(t0);
SamplerState s1 : register(s0);


struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 norm : NORMAL0;
	float3 tang : TANGENT0;
	float3 bitang : BITANGENT0;
};

[RootSignature(ROOT_SIG)]
float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	float4 color;
	color.a = 1.0f;

	float3 bumpNormal = t1.Sample(s1, input.uv).rgb * 2.0f - 1.0f;
	bumpNormal = (bumpNormal.x * input.tang) + (bumpNormal.y * input.bitang) + (bumpNormal.z * input.norm);
	bumpNormal = normalize(bumpNormal);
	
	float3 specCol = 0.1f;
	
	float3 lightDir = float3(0.0f, 0.0f, 1.0f);

	float diffuse = dot(lightDir, bumpNormal);
	const float3 baseCol = float3(0.80f, 0.80f, 1.0f);

	color.rgb = baseCol * (0.25f + 0.75f * diffuse) + specCol * pow(diffuse, 24.0f);
	
	return color;

}
