

Texture2D t1 : register(t4); // normal
//Texture2D t2 : register(t0);
SamplerState s1 : register(s0);


struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 norm : NORMAL0;
	float3 lightDir : LIGHTDIR;
	//float4 color : COLOR0;
};


float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	
	float3 bump = t1.Sample(s1, input.uv).rgb * 2.0 - float3(1.0f,1.0f,1.0f);
	float3 specCol;// = t2.Sample(s1, input.uv).rgb;
	
	/* uncomment to remove normalmap and spec map */
    bump = float3(0.0f,0.0f,1.0f);
	specCol = float3(0.2f, 0.2f, 0.2f);
	
	float diffuse = dot(input.lightDir, input.norm);
	const float3 baseCol = float3(0.80f, 0.80f, 1.0f);
	
	float4 color = saturate(float4(1.0f, 1.0f, 1.0f, 1.0f));
	color.rgb = baseCol * (0.25 + 0.75 * diffuse) + specCol * pow(diffuse, 24.0);
	//color.rgb = clamp(input.norm,0,1);
	
	return color;
	

}
