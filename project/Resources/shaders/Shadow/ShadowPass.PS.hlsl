#include "../3d/Object3d.hlsli"

PixelShaderOutput main(VertexShaderOutput input) {
	PixelShaderOutput output;
	output.color = float4(0.0f, 0.0f, 0.0f, 0.5f);
	return output;
}