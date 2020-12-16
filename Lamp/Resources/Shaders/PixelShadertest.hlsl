
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
    float3 normal: Normal;
};

float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
    // lightdata
    float3 lightPos = float3(500.0f, 500.0f, 500.0f);
    float4 ambientColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 diffuseColor = t1.Sample(s1, input.texCoord);
    float4 color = ambientColor;
    
    float3 L = normalize(lightPos - input.pos.xyz); // vector from the point we want to shade to the light 

	// Calculate the amount of light.
    float lightIntensity = saturate(dot(input.normal.xyz, L)); // % how much light is applied
    
    color = saturate(diffuseColor * lightIntensity);

     return diffuseColor;
}