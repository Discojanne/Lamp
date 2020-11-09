
struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
    
    /*
    float3 L = normalize(input.lightPos - worldPos.xyz); // vector from the point we want to shade to the light 

	// Calculate the amount of light.
    float lightIntensity = saturate(dot(N.xyz, L)); // % how much light is applied
    */



    return input.color;
}