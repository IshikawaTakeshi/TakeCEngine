#pragma once
#include "ResourceDataStructure.h"
#include "3d/Particle/Particle3d.h"
#include "3d/Particle/PrimitiveParticle.h"
#include "3d/Particle/ParticleCommon.h"
#include "3d/Particle/ParticleEmitterAllocater.h"
#include <list>
#include <wrl.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <random>

enum class ParticleModelType {
	Primitive,
	ExternalModel
};

class ParticleManager {
public:
	ParticleManager() = default;
	~ParticleManager() = default;

	// 初期化
	void Initialize();
	// 更新処理
	void Update();
	// ImGuiの更新処理
	void UpdateImGui();
	// 描画処理
	void Draw();
	// 終了処理
	void Finalize();

	/// <summary>
	/// パーティクルグループの生成
	/// </summary>
	/// <param name="particleCommon">パーティクル共通情報</param>
	/// <param name="name">グループ名(固有名)</param>
	/// <param name="filePath">objファイルパス</param>
	void CreateParticleGroup(ParticleCommon* particleCommon,const std::string& name,ParticleModelType modelType,
		const std::string& filePath,PrimitiveType primitiveType = PRIMITIVE_PLANE);

	void Emit(const std::string& name, const Vector3& emitPosition, uint32_t count);

	uint32_t EmitterAllocate();

	BaseParticleGroup* GetParticleGroup(const std::string& name);

	void SetAttributes(const std::string& name, const ParticleAttributes& attributes);


private:

	std::unique_ptr<ParticleEmitterAllocater> emitterAllocater_;

	std::unordered_map<std::string, std::unique_ptr<BaseParticleGroup>> particleGroups_;

};

