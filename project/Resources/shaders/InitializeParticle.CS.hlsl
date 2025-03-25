#include "GPUParticle.hlsli"

static const uint kMaxParticles = 1024;

RWStructuredBuffer<ParticleForCS> gParticles : register(u0);

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	uint particleIndex = DTid.x;
	if (particleIndex > kMaxParticles) {
		return;
	}
	//カウント分Particleを射出
	gParticles[particleIndex].translate = float3(-3.0f, 0.0f, 0.0f);
	gParticles[particleIndex].scale = float3(1.0f, 1.0f, 1.0f);
	gParticles[particleIndex].color = float4(1.0f, 1.0f, 1.0f, 1.0f);
}