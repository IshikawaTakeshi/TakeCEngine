#pragma once
#include "engine/3d/Particle/Particle3d.h"
#include "engine/3d/Particle/PrimitiveParticle.h"
#include "engine/3d/Particle/ParticleEmitterAllocater.h"
#include "engine/3d/Primitive/PrimitiveDrawer.h"
#include "engine/Base/BlendModeStateEnum.h"

#include <unordered_map>
#include <string>
#include <memory>
#include <utility>

// 前方宣言
class ParticleCommon;

//==================================================================================
// パーティクルマネージャー
//==================================================================================
namespace TakeC {
	class ParticleManager {
	public:
		ParticleManager() = default;
		~ParticleManager() = default;

		// 初期化
		void Initialize(ParticleCommon* particleCommon,PrimitiveDrawer* primitiveDrawer);
		// 更新処理
		void Update();
		// ImGuiの更新処理
		void UpdateImGui();
		// 描画処理
		void Draw();
		// 終了処理
		void Finalize();

		template<typename TPrimitive, typename...  Args>
		void UpdatePrimitiveType(const std::string& groupName, Args&&... args);

		/// <summary>
		/// パーティクルグループの生成
		/// </summary>
		/// <param name="particleCommon">パーティクル共通情報</param>
		/// <param name="name">グループ名(固有名)</param>
		/// <param name="filePath">objファイルパス</param>
		void CreateParticleGroup( const std::string& name,
			const std::string& filePath, PrimitiveType primitiveType = PRIMITIVE_PLANE);

		//パーティクルグループの生成(JSONから)
		void CreateParticleGroup(const std::string& presetJson);
		//パーティクル射出
		void Emit(const std::string& name, const Vector3& emitPosition, const Vector3& direction, uint32_t count);
		//エミッター用のハンドルの割り当て
		uint32_t EmitterAllocate();
		//パーティクルグループの開放
		void ClearParticleGroups();

		//出現しているパーティクルのクリア
		void ClearParticles();

		//パーティクルグループの取得
		BaseParticleGroup* GetParticleGroup(const std::string& name);
		//プリセットの設定
		void SetPreset(const std::string& name, const ParticlePreset& preset);

	private:

		//エミッターアロケータ
		std::unique_ptr<ParticleEmitterAllocator> emitterAllocator_;
		//パーティクルグループ
		std::unordered_map<std::string, std::unique_ptr<PrimitiveParticle>> particleGroups_;
		//パーティクル共通情報
		ParticleCommon* particleCommon_ = nullptr;
		//プリミティブドロワー
		PrimitiveDrawer* primitiveDrawer_ = nullptr;
	};


	//---------------------------------------------------------------------------------
	// プリミティブタイプの更新
	//---------------------------------------------------------------------------------
	template<typename TPrimitive, typename ...Args>
	void ParticleManager::UpdatePrimitiveType(const std::string& groupName, Args && ...args) {

		PrimitiveType type = PRIMITIVE_PLANE; 
		uint32_t handle = 0;
		if constexpr (std::is_same_v<TPrimitive, Ring>) {
			type = PRIMITIVE_RING;
			// Verify implicit conversion or cast if needed, but GenerateRing returns uint32_t
			handle = primitiveDrawer_->GenerateRing(std::forward<Args>(args)...);
		} 
		else if constexpr (std::is_same_v<TPrimitive, Plane>) {
			type = PRIMITIVE_PLANE;
			handle = primitiveDrawer_->GeneratePlane(std::forward<Args>(args)...);
		}
		else if constexpr (std::is_same_v<TPrimitive, Sphere>) {
			type = PRIMITIVE_SPHERE;
			handle = primitiveDrawer_->GenerateSphere(std::forward<Args>(args)...);
		}
		else if constexpr (std::is_same_v<TPrimitive, Cone>) {
			type = PRIMITIVE_CONE;
			handle = primitiveDrawer_->GenerateCone(std::forward<Args>(args)...);
		}
		else if constexpr (std::is_same_v<TPrimitive, Cube>) {
			type = PRIMITIVE_CUBE;
			handle = primitiveDrawer_->GenerateCube(std::forward<Args>(args)...);
		}
		else if constexpr (std::is_same_v<TPrimitive, Cylinder>) {
			type = PRIMITIVE_CYLINDER;
			handle = primitiveDrawer_->GenerateCylinder(std::forward<Args>(args)...);
		}
		else {
			static_assert(sizeof(TPrimitive) == 0, "Unsupported primitive type");
			return;
		}
		//既に存在する場合は更新、存在しない場合は新規作成
		auto it = particleGroups_.find(groupName);
		if (it == particleGroups_.end()) {
			auto newPrimitiveParticle = std::make_unique<PrimitiveParticle>(type);
			newPrimitiveParticle->Initialize(particleCommon_, ""); 
			newPrimitiveParticle->SetPrimitiveHandle(handle); // Set handle immediately
			particleGroups_[groupName] = std::move(newPrimitiveParticle);
		} else {
			// 既存のプリミティブパーティクルのプリミティブハンドルを更新
			particleGroups_[groupName]->SetPrimitiveHandle(handle);
		}
	}
}