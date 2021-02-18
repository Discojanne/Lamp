

Texture2D t1 : register(t0); // normal
//Texture2D t2 : register(t0);
SamplerState s1 : register(s0);


struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 lightDir : LIGHTDIR;
	float v_norm_err : ERR;
};


float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f); // ambient color
	//gl_FragColor.a = 1.0;

	//vec3 bump = texture2D( samplerBump, v_uv).rgb * 2.0 - vec3(1.0);
	//float3 bump = t1.Sample(s1, input.texCoord).rgb * 2.0f - float3(1.0f, 1.0f, 1.0f);
	float3 bump = input.normal;
	float3 specCol = float3(0.0f, 0.0f, 0.0f); //t2.Sample(s1, input.texCoord).rgb * 2.0f;

   /* uncomment to remove normalmap and spec map */
   //bump = vec3(0,0,1);
   //specCol = vec3(0.2);

	float diffuse = dot(input.lightDir, bump);
	const float3 baseCol = float3(0.80f, 0.80f, 1.0f);

	color.rgb = baseCol * (0.25f + 0.75f * diffuse) + specCol * pow(diffuse, 24.0f);

	return color;
}
