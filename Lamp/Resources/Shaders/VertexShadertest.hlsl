

struct VS_INPUT
{
    float3 pos : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    uint sw : SW;
    uint wc : WC;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float2 texCoord: TEXCOORD;
    float3 normal: NORMAL;
};

cbuffer ConstantBufferTest : register(b0)
{
    float4x4 wvpMat;
};

VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
    VS_OUTPUT output;
    output.pos = mul(float4(input.pos, 1.0f), wvpMat);
    output.texCoord = input.texCoord;
    output.normal = input.normal;// mul(float4(input.normal, 0.0f), wvpMat);
    return output;
}