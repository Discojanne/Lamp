float4 VSmain( float3 pos : POSITION ) : SV_POSITION
{
	return float4(pos, 1.0f);
}