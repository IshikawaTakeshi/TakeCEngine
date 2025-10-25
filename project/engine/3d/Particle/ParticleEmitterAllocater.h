#pragma once
#include "ParticleEmitter.h"
#include "ParticleCommon.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

//============================================================================
// ParticleEmitterAllocator class
//============================================================================
class ParticleEmitterAllocator {
public:
	ParticleEmitterAllocator() = default;
	~ParticleEmitterAllocator() = default;

	//======================================================================
	// functions
	//======================================================================

	/// <summary>
	/// エミッターの割り当て
	/// </summary>
	uint32_t Allocate();

	/// <summary>
	/// エミッターのクリア
	/// </summary>
	void Clear();

public:

	static const uint32_t kMaxEmitterCount_ = 512; //最大emitter数

private:

	//現在のエミッター数
	uint32_t emitterCount_ = 0;
};

