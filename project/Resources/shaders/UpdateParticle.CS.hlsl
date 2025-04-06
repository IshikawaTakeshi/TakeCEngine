#include "GPUParticle.hlsli"
#include "RandomGenerator.hlsli"

RWStructuredBuffer<ParticleForCS> gParticles : register(u0);
ConstantBuffer<PerFrame> gPerFrame : register(b0);

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID ){
	uint particleIndex = DTid.x;
	
	//particleIndexが最大数を超えた場合は何もしない
	if (particleIndex > kMaxParticles) {
		return;
	}
	
	//透明度が0のParticleは更新しない
	if ( gParticles[particleIndex].color.a == 0 ) {
		return;
	}

	gParticles[particleIndex].translate += gParticles[particleIndex].velocity;
	gParticles[particleIndex].scale += gPerFrame.deltaTime;
	gParticles[particleIndex].currentTime += gPerFrame.deltaTime;
	
	float alpha = 1.0f - (gParticles[particleIndex].currentTime / gParticles[particleIndex].lifetime);
	gParticles[particleIndex].color.a = saturate(alpha);
}