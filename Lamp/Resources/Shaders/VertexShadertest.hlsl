

struct VS_INPUT
{
    float4 pos : POSITION;
    float2 texCoord: TEXCOORD;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float2 texCoord: TEXCOORD;
};

cbuffer ConstantBufferTest : register(b0)
{
    float4x4 wvpMat;
};

VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
    VS_OUTPUT output;
    output.pos = mul(input.pos, wvpMat);
    output.texCoord = input.texCoord;
    return output;
}