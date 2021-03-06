

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
	//return float4(input.lightDir, 1.0f);
	float4 color;
	color.a = 1.0f;

	float3 normalMap = t1.Sample(s1, input.uv).rgb * 2.0f - 1.0f;
	float3 specCol = float3(0.1f, 0.1f, 0.1f);

   /* uncomment to remove normalmap and spec map */
   //bump = vec3(0,0,1);
   //specCol = vec3(0.2);

	float diffuse = dot(input.lightDir, normalMap);
	const float3 baseCol = float3(0.80f, 0.80f, 1.0f);

	color.rgb = baseCol * (0.25f + 0.75f * diffuse) + specCol * pow(diffuse, 24.0f);
	
	return color;
	
	
	
	
	
	
	//float3 normalMap = t1.Sample(s1, input.uv).rgb * 2.0 - 1.0f;
	//float3 specCol;// = t2.Sample(s1, input.uv).rgb;
	//float3 lightDir = float3(0.0f, 0.0f, 1.0f);
	
	//float3 bump = (normalMap.x * input.tang) + (normalMap.y * input.bitang) + (normalMap.z * input.norm);
	//bump = normalize(bump);
	
	///* uncomment to remove normalmap and spec map */
 //   //bump = float3(0.0f,0.0f,1.0f);
	//specCol = float3(0.1f, 0.1f, 0.1f);
	
	//float diffuse = dot(lightDir, bump);
	//const float3 baseCol = float3(0.80f, 0.80f, 1.0f);
	
	//float4 color = saturate(float4(1.0f, 1.0f, 1.0f, 1.0f));
	//color.rgb = baseCol * (0.25 + 0.75 * diffuse) + specCol * pow(diffuse, 24.0);
	////color.rgb = input.norm;
	
	//return color;
	

}
