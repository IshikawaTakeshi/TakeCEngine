#include "GPUParticle.hlsli"
#include "ParticleEmitter.hlsli"
#include "RandomGenerator.hlsli"

static const uint kMaxParticleEmitters = 2;

StructuredBuffer<EmitterSphere> gEmitterSphere : register(t0);
RWStructuredBuffer<ParticleForCS> gParticles : register(u0);
RWStructuredBuffer<int> gFreeCounter : register(u1);
ConstantBuffer<PerFrame> gPerFrame : register(b0);

//MEMO:複数のemitterを扱う場合は、適宜スレッド数を増やす
[numthreads(2, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	//乱数ジェネレーターを初期化
	RamdomGenerator randomGenerator;
	randomGenerator.seed = (DTid + gPerFrame.time) * gPerFrame.time;
	
	//EmitterのIndexを取得
	uint emitterIndex = DTid.x;

	//Emitterが有効かどうかをチェック
	if ( gEmitterSphere[emitterIndex].isEmit != 0 ) {
		
		//EmitterのParticle数分ループ
		//MEMO:Generate3dを呼ぶたびにseedが変わるので、結果的にすべての乱数が変わる
		for ( uint countIndex = 0; countIndex < gEmitterSphere[emitterIndex].particleCount; ++countIndex ) {
			
			int particleIndex;
			//gFreeCounter[0]に1を加算して、足す前の値をParticleIndexに格納
			InterlockedAdd(gFreeCounter[0], 1, particleIndex);
			
			//最大数よりもparticleが少ない場合射出可能
			if( particleIndex < kMaxParticles ) {
				gParticles[particleIndex].translate = randomGenerator.Generate3d();
				gParticles[particleIndex].scale = float3(1.0f, 1.0f, 1.0f);
				gParticles[particleIndex].color.rgb = randomGenerator.Generate3d();
				gParticles[particleIndex].color.a = 1.0f;
				gParticles[particleIndex].velocity = randomGenerator.Generate3d();
				gParticles[particleIndex].lifetime = 10.0f;
			}
			
		}
	}
}