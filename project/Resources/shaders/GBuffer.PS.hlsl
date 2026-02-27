#include "GBuffer.hlsli"
#include "3d/Object3d.hlsli"
#include "Material.hlsli"

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

GBufferPSOutput main(VertexShaderOutput input) {
	GBufferPSOutput output;
	output.Albedo = gMaterial.color * gTexture.Sample(gSampler, input.texcoord);
    output.Normal = float4(EncodeNormal(input.normal), 1);
	output.Material = float4(gMaterial.roughness, gMaterial.metallic, gMaterial.ao, 0);
    output.EnvMapTextureIndex = gMaterial.envMapTextureIndex;
	return output;
}