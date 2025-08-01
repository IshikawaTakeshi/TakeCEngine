#include "GPUParticle.hlsli"
#include "ParticleEmitter.hlsli"
#include "RandomGenerator.hlsli"

static const uint kMaxParticleEmitters = 2;

StructuredBuffer<EmitterSphere> gEmitterSphere : register(t0);
RWStructuredBuffer<ParticleForCS> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);
ConstantBuffer<PerFrame> gPerFrame : register(b0);
ConstantBuffer<ParticleAttributes> gAttributes : register(b1);

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
		for ( uint countIndex = 0; countIndex < gEmitterSphere[emitterIndex].particleCount; ++countIndex ) {
			
			int freeListIndex;
			//gFreeListのIndexを1つ前に設定して、現在のIndexを取得
			InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
			
			//最大数よりもparticleが少ない場合射出可能
			if(0 <= freeListIndex && freeListIndex < kMaxParticles ) {
				//射出するParticleのIndexを取得
				int particleIndex = gFreeList[freeListIndex];
				//particle初期化
				gParticles[particleIndex].translate = lerp(
					gAttributes.positionRange.x,
					gAttributes.positionRange.y,
					randomGenerator.Generate3d());
				gParticles[particleIndex].scale = lerp(
					gAttributes.scaleRange.x,
					gAttributes.scaleRange.y,
					randomGenerator.Generate3d());
				if ( gAttributes.editColor ) {
					gParticles[particleIndex].color.xyz = lerp(
						gAttributes.colorRange.x,
						gAttributes.colorRange.y,
						randomGenerator.Generate3d());
					gParticles[particleIndex].color.w = 1.0f;

				} else {
					//色を設定しない場合は、白色にする
					gParticles[particleIndex].color =
					float4(
					gAttributes.color.x,
					gAttributes.color.y,
					gAttributes.color.z,
					1.0f);
					
				}
				gParticles[particleIndex].velocity = lerp(
					gAttributes.velocityRange.x,
					gAttributes.velocityRange.y,
					randomGenerator.Generate3d());
				gParticles[particleIndex].lifetime = lerp(
					gAttributes.lifetimeRange.x,
					gAttributes.lifetimeRange.y,
					randomGenerator.Generate1d());
				
				gParticles[particleIndex].currentTime = 0.0f;
				
			} else {
				//射出できない場合は、gFreeListIndexを減らした分戻す
				InterlockedAdd(gFreeListIndex[0], 1);
				break;
			}
		}
	}
}