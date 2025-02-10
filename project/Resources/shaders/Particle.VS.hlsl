#include "Particle.hlsli"

struct PerView {
	float4x4 viewProjection;
	float4x4 billboardMatrix;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);
ConstantBuffer<PerView> gPerView : register(b0);

struct VertexShaderInput {
	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input,uint instanceId : SV_InstanceID) {
	VertexShaderOutput output;
	ParticleForGPU particle = gParticle[instanceId];
	float4x4 worldMatrix = gPerView.billboardMatrix;
	worldMatrix[0] *= particle.scale.x;
	worldMatrix[1] *= particle.scale.y;
	worldMatrix[2] *= particle.scale.z;
	worldMatrix[3] *= particle.translate;
	output.position = mul(input.position, mul(worldMatrix, gPerView.viewProjection));
	output.texcoord = input.texcoord;
	output.color = gParticle[instanceId].color;
	return output;
}