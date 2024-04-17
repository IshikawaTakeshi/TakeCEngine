struct Material {
	float4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutPut
{
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main() {
	PixelShaderOutPut output;
	output.color = gMaterial.color;
	return output;
}