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
	//float3 norm : NORMAL0;
	//float3 tang : TANGENT0;
	//float3 bitang : BITANGENT0;
	float3 lightDir : LIGHTDIR;
};

[RootSignature(ROOT_SIG)]
float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	float4 color;
	color.a = 1.0f;

	float3 normalMap = t1.Sample(s1, input.uv).rgb * 2.0f - 1.0f;
	float3 specCol = float3(0.1f, 0.1f, 0.1f);

   /* uncomment to remove normalmap and spec map */
	//input.lightDir = float3(0, 0, 1);
   //specCol = vec3(0.2);

	float diffuse = dot(input.lightDir, normalMap);
	const float3 baseCol = float3(0.80f, 0.80f, 1.0f);

	color.rgb = baseCol * (0.25f + 0.75f * diffuse) + specCol * pow(diffuse, 24.0f);
	
	//return float4(input.lightDir, 1.0f);
	return color;

}
