

struct VS_INPUT
{
    float3 pos : POSITION;
    float4 color: COLOR;
};

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};


VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
    VS_OUTPUT output;
    input.pos.x += id * 0.05f;
    input.pos.y += id * 0.05f;
    input.pos.z += id * 0.01f;
    output.pos = float4(input.pos, 1.0f);
    output.color = input.color;
    output.color.r += id * 0.1f;

    if (output.color.g == 1)
    {
        output.pos.y -= 1.0f;
    }

    return output;
}