

struct VS_OUTPUT_POSTFX
{
	float4 PositionH : SV_Position;
	float2 TextureUV : Texcoord;
};


VS_OUTPUT_POSTFX QuadVS( in uint id : SV_VertexID )
{
	VS_OUTPUT_POSTFX Output;

	Output.PositionH.x = float( id / 2 ) * 4.0 - 1.0;
	Output.PositionH.y = float( id % 2 ) * 4.0 - 1.0;
	Output.PositionH.z = 0.0;
	Output.PositionH.w = 1.0;

	Output.TextureUV.x = (float)(id / 2) * 2.0;
	Output.TextureUV.y = 1.0 - (float)(id % 2) * 2.0;

	return Output;
}

float4 TestPS( VS_OUTPUT_POSTFX Input ) : SV_Target
{
	return float4(1, 0, 0, 1);
}