#pragma once
#include "ResourceDataStructure.h"
#include "3d/Particle/Particle3d.h"
#include "3d/Particle/PrimitiveParticle.h"
#include "3d/Particle/ParticleCommon.h"
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

	/// <summary>
	/// パーティクルグループの生成
	/// </summary>
	/// <param name="particleCommon">パーティクル共通情報</param>
	/// <param name="name">グループ名(固有名)</param>
	/// <param name="filePath">objファイルパス</param>
	void CreateParticleGroup(ParticleCommon* particleCommon,const std::string& name,ParticleModelType modelType, const std::string& filePath);

	void Update();

	void UpdateImGui();

	void Draw();

	void Finalize();

	void Emit(const std::string& name, const Vector3& emitPosition, uint32_t count);

	BaseParticleGroup* GetParticleGroup(const std::string& name);

	void SetAttributes(const std::string& name, const ParticleAttributes& attributes);


private:

	std::unordered_map<std::string, std::unique_ptr<BaseParticleGroup>> particleGroups_;

};

