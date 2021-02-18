

Texture2D t1 : register(t0); // normal
//Texture2D t2 : register(t0);
SamplerState s1 : register(s0);


struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};


float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	
	float4 color = float4(0.5f, 0.5f, 0.5f, 1.0f);
	return color;
}
