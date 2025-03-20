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

}