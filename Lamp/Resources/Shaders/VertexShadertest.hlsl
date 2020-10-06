

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 color: COLOR;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

cbuffer ConstantBufferTest : register(b0)
{
    float4x4 wvpMat[2];
};

VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
    VS_OUTPUT output;
    output.pos = mul(input.pos, wvpMat[id]);
    output.color = input.color;
    return output;
}