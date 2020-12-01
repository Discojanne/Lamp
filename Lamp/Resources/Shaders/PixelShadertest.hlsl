
Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

//cbuffer ConstantBufferTest : register(b0)
//{
//    float4x4 wvpMat[2];
//};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float2 texCoord: TEXCOORD;
};

float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
    
    /*
    float3 L = normalize(input.lightPos - worldPos.xyz); // vector from the point we want to shade to the light 

	// Calculate the amount of light.
    float lightIntensity = saturate(dot(N.xyz, L)); // % how much light is applied
    */


     return t1.Sample(s1, input.texCoord);
}