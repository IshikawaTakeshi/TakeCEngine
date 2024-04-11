struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main() {
	PixelShaderOutPut output;
	output.color = float4(1.0, 1.0, 1.0, 1.0);
	return output;
}