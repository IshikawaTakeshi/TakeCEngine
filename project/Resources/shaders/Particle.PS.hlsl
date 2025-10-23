#include "Particle.hlsli"
#include "Light/DirectionalLight.hlsli"

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//並行光源
ConstantBuffer<DirectionalLight> gDirLight : register(b1);
//テクスチャ
Texture2D<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;
	DirectionalLight dirLightInfo = gDirLight;
	Material material = gMaterial;

	float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	float4 baseColor = gMaterial.color * textureColor * input.color;

	// アルファ値が0なら描画しない
	if (baseColor.a == 0.0f) {
		discard;
	}
	
	//===============光蓄積計算===============//
	//DirectionalLightの計算
	float3 toEye = normalize(gCamera.worldPosition - input.position);
	float3 dirLighting = CalcDirectionalLighting(dirLightInfo, material, input.normal, toEye, material.color.rgb * textureColor.rgb);
	
	output.color = float4(baseColor.rgb, baseColor.a);

	output.color.rgb *= baseColor.a;
	return output;
}