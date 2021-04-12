

Texture2D t1 : register(t0); // normal
//Texture2D t2 : register(t0);
SamplerState s1 : register(s0);


struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};


float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	
	float cs = input.color.x * 5519 % 255 / 255;
	float cs2 = input.color.x * 7853 % 255 / 255;
	float cs3 = input.color.x * 1667 % 255 / 255;
	float4 color = saturate(float4(cs * 1.0f, cs2 * 1.0f, cs3 * 1.0f, 1.0f));
	return color; //return (float4(1.0f, 1.0f, 1.0f, 1.0f));
	
	
	
	//float cs = (((input.color.x + 1) * 7) % 10) / 10.0f;
	//float cs2 = ((input.color.x + 1) % 10) / 10.0f;
	//float4 color = saturate(float4(cs * 1.0f, cs2 * 1.0f, 1.0f, 1.0f));
	//return color;
	////return (float4(1.0f, 1.0f, 1.0f, 1.0f));
}
