#pragma once
#include "base/DirectXCommon.h"
#include "base/PipelineStateObject.h"
#include "base/SrvManager.h"
#include "3d/Material.h"
#include "3d/VertexData.h"
#include "engine/3d/Primitive/Ring.h"
#include "engine/3d/Primitive/Plane.h"
#include "engine/3d/Primitive/Sphere.h"
#include "engine/3d/Primitive/Cube.h"
#include "engine/3d/Primitive/Cone.h"
#include "engine/3d/Primitive/Cylinder.h"
#include "math/TransformMatrix.h"
#include "Primitive/PrimitiveType.h"
#include <memory>
#include <cstdint>
#include <unordered_map>

//============================================================================
// PrimitiveDrawer class
//============================================================================
namespace TakeC {

	class PrimitiveDrawer {
	public:

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		PrimitiveDrawer() = default;
		~PrimitiveDrawer() = default;

		//=================================================================================
		// ハンドル構造体（型情報を含む）
		//=================================================================================
		struct PrimitiveHandle {
			uint32_t id;
			PrimitiveType type;

			PrimitiveHandle(uint32_t _id, PrimitiveType _type) 
				: id(_id), type(_type) {}

			// 暗黙の型変換（後方互換性のため）
			operator uint32_t() const { return id; }
		};

	private:

		//=================================================================================
		// プリミティブエントリ（マップに格納するデータ）
		//=================================================================================
		struct PrimitiveEntry {
			std::unique_ptr<PrimitiveBaseData> data;  // 実際のデータ
			PrimitiveType type;                        // プリミティブタイプ
			void* primitiveInstance;                   // 対応するプリミティブクラスのインスタンス

			// 型安全なインスタンス取得用テンプレート
			template<typename TPrimitive>
			TPrimitive* GetPrimitiveAs() {
				return static_cast<TPrimitive*>(primitiveInstance);
			}
		};

		/// <summary>
		/// 型からPrimitiveTypeへの変換
		/// </summary>
		/// <typeparam name="TPrimitive"></typeparam>
		/// <returns></returns>
		template<typename TPrimitive>
		static constexpr PrimitiveType GetPrimitiveTypeFromClass();

	public:

		//=================================================================================
		// functions
		//=================================================================================

		// 初期化
		void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager);

		/// 終了処理
		void Finalize();

		/// <summary>
		/// プリミティブの解放
		/// </summary>
		/// <param name="handle"></param>
		void Release(uint32_t handle);

		/// <summary>
		/// プリミティブを生成（テンプレート版）
		/// 使用例: auto handle = drawer. Generate<Ring>(2.0f, 1.0f, "texture.png");
		/// </summary>
		template<typename TPrimitive, typename... Args>
		PrimitiveHandle Generate(Args&&... args);

		//リングデータの生成
		uint32_t GenerateRing(float outerRadius, float innerRadius,uint32_t subDivision, const std::string& textureFilePath);
		// 平面データの生成
		uint32_t GeneratePlane(float width, float height, const std::string& textureFilePath);
		// 球データの生成
		uint32_t GenerateSphere(float radius,uint32_t subDivision, const std::string& textureFilePath);
		// 円錐データの生成
		uint32_t GenerateCone(float radius, float height, uint32_t subDivision, const std::string& textureFilePath);
		//cubeデータの作成
		uint32_t GenerateCube(const AABB& size, const std::string& textureFilePath);
		//cylinderデータの作成
		uint32_t GenerateCylinder(float radius, float height, uint32_t subDivision, const std::string& textureFilePath);

		// 描画処理(particle用)
		void DrawParticle(PSO* pso, UINT instanceCount, PrimitiveType type, uint32_t handle);
		// 描画処理(オブジェクト用)
		void DrawObject(PSO* pso, PrimitiveType type, uint32_t handle);

		PlaneData* GetPlaneData(uint32_t handle);
		SphereData* GetSphereData(uint32_t handle);
		RingData* GetRingData(uint32_t handle);
		ConeData* GetConeData(uint32_t handle);
		CubeData* GetCubeData(uint32_t handle);
		CylinderData* GetCylinderData(uint32_t handle);

		/// <summary>
		/// 可変長引数からテクスチャファイルパスを抽出
		/// </summary>
		template<typename...  Args>
		std::string ExtractTextureFilePath(Args&&... args);

		/// <summary>
		/// 可変長引数から文字列を抽出（再帰終了用）
		/// </summary>
		/// <param name="out"></param>
		/// <param name="value"></param>
		void ExtractIfString(std::string& out, const std::string& value) {
			out = value;
		}

		//テンプレートオーバーロード（string以外は何もしない）
		template<typename T>
		void ExtractIfString(std::string&, const T&) {
			// 何もしない
		}

		//=================================================================================
		// accessors
		//=================================================================================

		//---- getter ------------------------------------------------------------------

		/// <summary>
		/// プリミティブデータを取得（型指定版）
		/// </summary>
		template<typename TPrimitive>
		typename TPrimitive::DataType* GetData(const PrimitiveHandle& handle);

		/// <summary>
		/// プリミティブデータを取得
		/// </summary>
		template<typename TPrimitive>
		typename TPrimitive::DataType* GetData(uint32_t handleId);

		// プリミティブベースデータを取得
		PrimitiveBaseData* GetBaseData(uint32_t handleId);

		// ハンドルからプリミティブタイプを取得
		PrimitiveType GetPrimitiveType(uint32_t handleId);

		/// <summary>
		/// プリミティブインスタンスを取得
		/// </summary>
		template<typename TPrimitive>
		TPrimitive* GetPrimitiveInstance();

		/// <summary>
		/// Transform取得
		/// </summary>
		EulerTransform* GetTransform(const PrimitiveHandle& handle) {
			return GetTransform(handle.id);
		}

		/// <summary>
		/// Transform取得
		/// </summary>
		/// <param name="handleId"></param>
		/// <returns></returns>
		EulerTransform* GetTransform(uint32_t handleId) {
			auto* baseData = GetBaseData(handleId);
			return baseData ? &baseData->transform : nullptr;
		}

		//---- setter ------------------------------------------------------------------

		/// Transform設定
		void SetTransform(const PrimitiveHandle& handle, const EulerTransform& transform) {
			SetTransform(handle.id, transform);
		}

		/// Transform設定
		void SetTransform(uint32_t handleId, const EulerTransform& transform) {
			if (auto* baseData = GetBaseData(handleId)) {
				baseData->transform = transform;
			}
		}

		/// マテリアル色設定
		void SetMaterialColor(const PrimitiveHandle& handle, const Vector4& color) {
			SetMaterialColor(handle.id, color);
		}

		/// マテリアル色設定
		void SetMaterialColor(uint32_t handleId, const Vector4& color) {
			auto* baseData = GetBaseData(handleId);
			if (baseData && baseData->material) {
				baseData->material->SetMaterialColor(color);
			}
		}
		/// 生成パラメータを設定（Ring用）
		void SetGenerateParameters(RingData* data, float outerRadius, float innerRadius,uint32_t subDivision,const std::string&) {
			data->outerRadius = outerRadius;
			data->innerRadius = innerRadius;
			data->subDivision = subDivision;
		}
		/// 生成パラメータを設定（Plane用）
		void SetGenerateParameters(PlaneData* data, float width, float height,const std::string&) {
			data->width = width;
			data->height = height;
		}
		/// 生成パラメータを設定（Sphere用）
		void SetGenerateParameters(SphereData* data, float radius,uint32_t subDivision,const std::string&) {
			data->radius = radius;
			data->subDivision = subDivision;
		}
		/// 生成パラメータを設定（Cone用）
		void SetGenerateParameters(ConeData* data, float radius, float height, uint32_t subDivision,const std::string&) {
			data->radius = radius;
			data->height = height;
			data->subDivision = subDivision;
		}
		/// 生成パラメータを設定（Cube用）
		void SetGenerateParameters(CubeData* data, const AABB& size,const std::string&) {
			data->size = size;
		}
		/// 生成パラメータを設定（Cylinder用）
		void SetGenerateParameters(CylinderData* data, float radius, float height, uint32_t subDivision,const std::string&) {
			data->radius = radius;
			data->height = height;
			data->subDivision = subDivision;
		}

		
private:

		/// <summary>
		/// 描画共通処理
		/// </summary>
		/// <param name="pso"></param>
		/// <param name="data"></param>
		void DrawCommon(PSO* pso, PrimitiveBaseData* data);

	private:

		TakeC::DirectXCommon* dxCommon_ = nullptr;
		TakeC::SrvManager* srvManager_ = nullptr;

		// 統一されたプリミティブデータマップ
		std::unordered_map<uint32_t, PrimitiveEntry> primitiveMap_;
		uint32_t nextHandle_ = 0;

		// プリミティブデータ管理用マップ
		//ring
		std::unique_ptr<TakeC::Ring> ring_ = nullptr;
		//plane
		std::unique_ptr<TakeC::Plane> plane_ = nullptr;
		//sphere
		std::unique_ptr<TakeC::Sphere> sphere_ = nullptr;
		//cone
		std::unique_ptr<TakeC::Cone> cone_ = nullptr;
		//cube
		std::unique_ptr<TakeC::Cube> cube_ = nullptr;
		//cylinder
		std::unique_ptr<TakeC::Cylinder> cylinder_ = nullptr;

		const uint32_t kMaxVertexCount_ = 32000;
	};



	//----------------------------------------------------------------------------
	// 型からPrimitiveTypeへの変換
	//----------------------------------------------------------------------------
	template<typename TPrimitive>
	inline constexpr PrimitiveType PrimitiveDrawer::GetPrimitiveTypeFromClass() {
		if constexpr (std::is_same_v<TPrimitive, Ring>) {
			return PRIMITIVE_RING;
		}
		else if constexpr (std::is_same_v<TPrimitive, Plane>) {
			return PRIMITIVE_PLANE;
		}
		else if constexpr (std::is_same_v<TPrimitive, Sphere>) {
			return PRIMITIVE_SPHERE;
		}
		else if constexpr (std::is_same_v<TPrimitive, Cube>) {
			return PRIMITIVE_CUBE;
		}
		else if constexpr (std::is_same_v<TPrimitive, Cone>) {
			return PRIMITIVE_CONE;
		}
		else if constexpr (std::is_same_v<TPrimitive, Cylinder>) {
			return PRIMITIVE_CYLINDER;
		}
		else {
			static_assert(sizeof(TPrimitive) == 0, "Unsupported primitive type");
			return PRIMITIVE_COUNT;
		}
	}

	//----------------------------------------------------------------------------
	// プリミティブデータを取得（型指定版）
	//----------------------------------------------------------------------------
	template<typename TPrimitive>
	inline typename TPrimitive::DataType* PrimitiveDrawer::GetData(const PrimitiveHandle& handle) {
		return GetData<TPrimitive>(handle. id);
	}

	//----------------------------------------------------------------------------
	// プリミティブデータを取得
	//----------------------------------------------------------------------------
	template<typename TPrimitive>
	inline typename TPrimitive::DataType* PrimitiveDrawer::GetData(uint32_t handleId) {
		auto it = primitiveMap_.find(handleId);
		if (it == primitiveMap_.end()) {
			return nullptr;
		}

		// 型チェック
		PrimitiveType expectedType = GetPrimitiveTypeFromClass<TPrimitive>();
		if (it->second.type != expectedType) {
			return nullptr;
		}

		// ダウンキャストして返す
		return static_cast<typename TPrimitive::DataType*>(it->second.data.get());
	}

	template<typename TPrimitive>
	inline TPrimitive* PrimitiveDrawer::GetPrimitiveInstance() {
		if constexpr (std::is_same_v<TPrimitive, Ring>) {
			return ring_. get();
		}
		else if constexpr (std::is_same_v<TPrimitive, Plane>) {
			return plane_.get();
		}
		else if constexpr (std::is_same_v<TPrimitive, Sphere>) {
			return sphere_.get();
		}
		else if constexpr (std::is_same_v<TPrimitive, Cube>) {
			return cube_.get();
		}
		else if constexpr (std::is_same_v<TPrimitive, Cone>) {
			return cone_.get();
		}
		else if constexpr (std::is_same_v<TPrimitive, Cylinder>) {
			return cylinder_.get();
		}
		else {
			static_assert(sizeof(TPrimitive) == 0, "Unsupported primitive type");
			return nullptr;
		}
	}

	//----------------------------------------------------------------------------
	// プリミティブデータを生成（テンプレート版）
	//----------------------------------------------------------------------------
	template<typename TPrimitive, typename ...Args>
	inline PrimitiveDrawer::PrimitiveHandle PrimitiveDrawer::Generate(Args && ...args) {
		using DataType = typename TPrimitive::DataType;

		// データを作成（基底クラスのユニークポインタとして保持）
		auto data = std::make_unique<DataType>();

		// プリミティブインスタンスを取得
		TPrimitive* primitive = GetPrimitiveInstance<TPrimitive>();

		// パラメータ設定
		SetGenerateParameters(data.get(), std::forward<Args>(args)...);

		// 頂点データ作成
		primitive->CreateVertexData(data.get());

		// テクスチャファイルパスを取得
		std::string textureFilePath = ExtractTextureFilePath(std::forward<Args>(args)...);
		primitive->CreateMaterial(data.get(), textureFilePath);

		// ハンドルを生成
		uint32_t handleId = nextHandle_++;
		PrimitiveType type = GetPrimitiveTypeFromClass<TPrimitive>();

		// エントリを作成してマップに登録
		PrimitiveEntry entry;
		entry.data = std:: move(data);  // 基底クラスポインタとして格納
		entry.type = type;
		entry.primitiveInstance = primitive;

		primitiveMap_[handleId] = std::move(entry);

		return PrimitiveHandle(handleId, type);
	}

	//----------------------------------------------------------------------------
	// 可変長引数からテクスチャファイルパスを抽出
	//----------------------------------------------------------------------------
	template<typename ...Args>
	inline std::string PrimitiveDrawer::ExtractTextureFilePath(Args && ...args) {
		std::string result;
		((ExtractIfString(result, std::forward<Args>(args))), ...);
		return result;
	}
}