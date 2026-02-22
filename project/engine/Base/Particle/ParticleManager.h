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

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
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

		/// <summary>
		/// プリミティブタイプの更新
		/// </summary>
		/// <typeparam name="TPrimitive"></typeparam>
		/// <typeparam name="...Args"></typeparam>
		/// <param name="groupName"></param>
		/// <param name="...args"></param>
		template<typename TPrimitive, typename...  Args>
		void UpdatePrimitiveType(const std::string& groupName, Args&&... args);

		/// テクスチャアニメーションの更新
		template<typename TAnimation, typename ...  Args>
		void UpdateTextureAnimationType(const std::string& groupName, Args&&... args);

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
		// エミッターIDを指定したパーティクル射出（新規）
		void EmitWithEmitter(uint32_t emitterID, const std::string& name, const Vector3& emitPosition, const Vector3& direction, uint32_t count);

		//パーティクルグループの開放
		void ClearParticleGroups();
		//エミッターの開放
		void ClearEmitters();

		//出現しているパーティクルのクリア
		void ClearParticles();
		//全プリセットの読み込み
		void LoadAllPresets();

		//エミッター用のハンドルの割り当て
		uint32_t EmitterAllocate(ParticleEmitter* emitter);
		// エミッター用のハンドルの解放
		void EmitterRelease(uint32_t emitterID);

		//=====================================================================================
		// accsusser
		//=====================================================================================

		// エミッターIDから位置を取得
		std::optional<Vector3> GetEmitterPosition(uint32_t emitterID) const;
		// エミッターIDから回転を取得
		std::optional<Vector3> GetEmitDirection(uint32_t emitterID) const;

		//パーティクルグループの取得
		BaseParticleGroup* GetParticleGroup(const std::string& name);
		//groupnameからプリミティブハンドルの取得
		uint32_t GetPrimitiveHandle(const std::string& groupName);

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
			// 新しいプリミティブパーティクルを作成して初期化
			auto newPrimitiveParticle = std::make_unique<PrimitiveParticle>(type);
			//テクスチャファイルパスの取得
			std::string textureFilePath = it->second->GetPreset().textureFilePath;
			newPrimitiveParticle->Initialize(particleCommon_, textureFilePath); 
			newPrimitiveParticle->SetPrimitiveHandle(handle); // Set handle immediately
			particleGroups_[groupName] = std::move(newPrimitiveParticle);
		} else {
			// 既存のプリミティブパーティクルのプリミティブハンドルを更新
			//古いハンドルを解放
			primitiveDrawer_->Release(particleGroups_[groupName]->GetPrimitiveHandle());
			particleGroups_[groupName]->SetPrimitiveHandle(handle);
		}
	}

	//---------------------------------------------------------------------------------
	// テクスチャアニメーションの更新
	//---------------------------------------------------------------------------------
	template<typename TAnimation, typename ...Args>
	inline void ParticleManager::UpdateTextureAnimationType(const std::string& groupName, Args && ...args) {
	
		auto it = particleGroups_.find(groupName);
		if (it != particleGroups_.end()) {
			// プリミティブハンドルからベースデータを取得
			PrimitiveBaseData* baseData = primitiveDrawer_->GetBaseData(it->second->GetPrimitiveHandle());

			if (baseData && baseData->material) {
				// Materialのアニメーションオブジェクトを取得
				TakeC::UVTextureAnimation* animation = baseData->material->Animation();

				if (animation) {
					// テンプレート型に応じて適切なアニメーションを設定
					if constexpr (std::is_same_v<TAnimation, UVScrollSettings>) {
						// UVScrollの場合、設定オブジェクトを直接受け取って設定
						animation->SetUVScrollAnimation(std::forward<Args>(args)...);
					}
					else if constexpr (std::is_same_v<TAnimation, SpriteSheetSettings>) {
						// SpriteSheetの場合、設定オブジェクトを直接受け取って設定
						animation->SetSpriteSheetAnimation(std:: forward<Args>(args)...);
					}
				}
			}
		}
	}
}