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
	/// エミッターIDの割り当て（登録）
	/// </summary>
	/// <param name="emitter">登録するエミッターのポインタ</param>
	/// <returns>割り当てられたエミッターID</returns>
	uint32_t Allocate(ParticleEmitter* emitter);

	/// <summary>
	/// エミッターIDの解放（登録解除）
	/// </summary>
	/// <param name="emitterID">解放するエミッターID</param>
	void Release(uint32_t emitterID);

	/// <summary>
	/// エミッターIDから現在のエミッター位置を取得
	/// </summary>
	/// <param name="emitterID">エミッターID</param>
	/// <returns>エミッター位置（存在しない場合はnullopt）</returns>
	std::optional<Vector3> GetEmitterPosition(uint32_t emitterID) const;

	std::optional<Vector3> GetEmitDirection(uint32_t emitterID )const;

	/// <summary>
	/// エミッターIDからエミッターポインタを取得
	/// </summary>
	/// <param name="emitterID">エミッターID</param>
	/// <returns>エミッターポインタ（存在しない場合はnullptr）</returns>
	ParticleEmitter* GetEmitter(uint32_t emitterID) const;

	/// <summary>
	/// 全エミッターのクリア
	/// </summary>
	void Clear();

	/// <summary>
	/// 登録されているエミッター数を取得
	/// </summary>
	uint32_t GetActiveEmitterCount() const { return static_cast<uint32_t>(activeEmitters_.size()); }

public:

	static const uint32_t kMaxEmitterCount_ = 4096; //最大emitter数

private:

	// エミッターID管理
	uint32_t nextEmitterID_ = 1; // 0は無効なIDとして予約
	
	// アクティブなエミッターの管理(参照用)
	std::unordered_map<uint32_t, ParticleEmitter*> activeEmitters_;
	
	// 解放されたIDの再利用リスト（オプション：メモリ効率化）
	std::vector<uint32_t> freeIDs_;
};

