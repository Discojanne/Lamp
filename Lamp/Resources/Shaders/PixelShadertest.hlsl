
struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
    
    return input.color;
}