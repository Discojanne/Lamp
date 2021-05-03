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
	//float3 tang : TANGENT0;
	//float3 bitang : BITANGENT0;
	float3 lightDir : LIGHTDIR;
	float3 v_norm_orig : ORIG;
};

float3 colorRamp(float err)
{
	err = (err + 1.0f) * 0.5f;
	err *= err * err;
	return 0.8f * float3(1.0f, err, err);
}

[RootSignature(ROOT_SIG)]
float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	float4 color;
	color.a = 1.0f;

	color.rgb = colorRamp(dot(normalize(input.norm), normalize(input.v_norm_orig)));
	
	return color;

}
