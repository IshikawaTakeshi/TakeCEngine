#pragma once
#include "engine/3d/Particle/Particle3d.h"
#include "engine/3d/Particle/PrimitiveParticle.h"
#include "engine/3d/Particle/ParticleEmitterAllocater.h"
#include "engine/base/BlendModeStateEnum.h"
#include <list>
#include <wrl.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <random>

// 前方宣言
class ParticleCommon;

//==================================================================================
// パーティクルマネージャー
//==================================================================================
class ParticleManager {
public:
	ParticleManager() = default;
	~ParticleManager() = default;

	// 初期化
	void Initialize(ParticleCommon* particleCommon);
	// 更新処理
	void Update();
	// ImGuiの更新処理
	void UpdateImGui();
	// 描画処理
	void Draw();
	// 終了処理
	void Finalize();

	void UpdatePrimitiveType(const std::string& groupName, PrimitiveType type,const Vector3& param);

	/// <summary>
	/// パーティクルグループの生成
	/// </summary>
	/// <param name="particleCommon">パーティクル共通情報</param>
	/// <param name="name">グループ名(固有名)</param>
	/// <param name="filePath">objファイルパス</param>
	void CreateParticleGroup(ParticleCommon* particleCommon,const std::string& name,
		const std::string& filePath,PrimitiveType primitiveType = PRIMITIVE_PLANE);

	//パーティクルグループの生成(JSONから)
	void CreateParticleGroup(ParticleCommon* particleCommon, const std::string& presetJson);
	//パーティクル射出
	void Emit(const std::string& name, const Vector3& emitPosition,const Vector3& direction, uint32_t count);
	//エミッター用のハンドルの割り当て
	uint32_t EmitterAllocate();
	//パーティクルグループの開放
	void ClearParticleGroups();

	//パーティクルグループの取得
	BaseParticleGroup* GetParticleGroup(const std::string& name);
	//プリセットの設定
	void SetPreset(const std::string& name, const ParticlePreset& preset);

private:

	//エミッターアロケータ
	std::unique_ptr<ParticleEmitterAllocator> emitterAllocater_;
	//パーティクルグループ
	std::unordered_map<std::string, std::unique_ptr<PrimitiveParticle>> particleGroups_;
	//パーティクル共通情報
	ParticleCommon* particleCommon_ = nullptr;
};