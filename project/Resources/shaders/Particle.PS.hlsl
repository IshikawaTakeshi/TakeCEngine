#include "Particle.hlsli"
#include "Light/DirectionalLight.hlsli"

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//テクスチャ
Texture2D<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;
	Material material = gMaterial;

	float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	float4 baseColor = gMaterial.color * textureColor * input.color;

	// アルファ値が0なら描画しない
	if (baseColor.a == 0.0f) {
		discard;
	}
	
	// 擬似的な法線をカメラ正面に固定（煙など想定）
	float3 n = normalize(float3(0, 0, 1));
	
	// basisベクトル（同一でOK）
	float3 b0 = float3(1, 0, 0);
	float3 b1 = float3(0, 1, 0);
	float3 b2 = float3(0, 0, 1);

	float3 w = saturate(float3(
        dot(n, b0),
        dot(n, b1),
        dot(n, b2)
    ));

	float3 diffuse = input.basisColor1 * w.x + input.basisColor2 * w.y + input.basisColor3 * w.z;
	float3 finalColor = baseColor.rgb * diffuse;
	
	output.color = float4(finalColor, baseColor.a);

	output.color.rgb *= baseColor.a;
	return output;
}