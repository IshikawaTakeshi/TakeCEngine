#include "GPUParticle.hlsli"

static const uint kMaxParticles = 1024;

RWStructuredBuffer<ParticleForCS> gParticles : register(u0);

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	uint particleIndex = DTid.x;
	if (particleIndex > kMaxParticles) {
		return;
	}
	
	gParticles[particleIndex] = (ParticleForCS) 0;
	gParticles[particleIndex].translate = float3(0.0f, 0.0f, 0.0f);
	gParticles[particleIndex].scale = float3(10.0f, 10.0f, 10.0f);
	gParticles[particleIndex].color = float4(1.0f, 1.0f, 0.0f, 1.0f);
}