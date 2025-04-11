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
	//apply scale
	worldMatrix[0] *= particle.scale.x;
	worldMatrix[1] *= particle.scale.y;
	worldMatrix[2] *= particle.scale.z;
	  // Apply rotation (around Z axis)
	float angle = particle.rotate.z;

	float cosR = cos(angle);
	float sinR = sin(angle);

// Z軸回転行列
	float4x4 rotZ = {
	cosR, sinR, 0.0f, 0.0f,
   -sinR, cosR, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
	};
	worldMatrix = mul(worldMatrix, rotZ);
	
	//apply translation
	worldMatrix[3].xyz = particle.translate;
	output.position = mul(input.position, mul(worldMatrix, gPerView.viewProjection));
	output.texcoord = input.texcoord;
	output.color = gParticle[instanceId].color;
	return output;
}