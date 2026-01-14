#pragma once
#include "base/ComPtrAliasTemplates.h"
#include "base/DirectXCommon.h"
#include "base/PipelineStateObject.h"
#include "base/SrvManager.h"
#include "base/ImGuiManager.h"
#include "base/TextureManager.h"
#include "3d/Material.h"
#include "3d/VertexData.h"
#include "math/TransformMatrix.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <cstdint>

namespace TakeC {

	//============================================================================
	// 共通プリミティブメッシュ構造体
	// 全プリミティブで使用する頂点バッファ・インデックスバッファ情報
	//============================================================================
	struct PrimitiveMesh {
		ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
		ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
		D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};
	};

	struct PrimitiveBaseData {
		PrimitiveMesh mesh;
		VertexData* vertexData = nullptr;
		std::unique_ptr<Material> material = nullptr;
		uint32_t vertexCount = 0;

		EulerTransform transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	};

	//============================================================================
	//プリミティブ基底クラステンプレート
	//============================================================================

	template <typename TData>
	class PrimitiveBase {

	public:
		//========================================================
		//	functions
		//========================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		PrimitiveBase() = default;
		virtual ~PrimitiveBase() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager);

	public:

		/// <summary>
		/// データ取得
		/// </summary>
		TData* GetData(uint32_t handle);

		const EulerTransform& GetTransform(uint32_t handle);

		/// <summary>
		/// マテリアル色設定
		/// </summary>
		void SetMaterialColor(uint32_t handle, const Vector4& color);

		/// <summary>
		/// transform設定
		/// </summary>
		void SetTransform(uint32_t handle, const EulerTransform& transform);

	protected:

	public:

		/// <summary>
		/// 頂点データ作成
		/// </summary>
		/// <param name="handle"></param>
		virtual void CreateVertexData(TData* data) = 0;

		/// <summary>
		/// マテリアル作成
		/// </summary>
		/// <param name="handle"></param>
		/// <param name="textureFilePath"></param>
		virtual void CreateMaterial(TData* data, const std::string& textureFilePath);

		/// <summary>
		/// データ登録
		/// </summary>
		/// <param name="data"></param>
		/// <returns></returns>
		uint32_t RegisterData(std::unique_ptr<TData> data);

		/// <summary>
		/// プリミティブデータ編集
		/// </summary>
		/// <param name="data"></param>
		virtual void EditPrimitiveData(TData* data) = 0;

	protected:

		//========================================================
		// メンバ変数
		//========================================================

		DirectXCommon* dxCommon_ = nullptr;
		SrvManager* srvManager_ = nullptr;
		std::unordered_map<uint32_t, std::unique_ptr<TData>> datas_;
		uint32_t nextHandle_ = 0;
	};


	//----------------------------------------------------------------------------
	// 初期化
	//----------------------------------------------------------------------------
	template<typename TData>
	inline void PrimitiveBase<TData>::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager) {
		dxCommon_ = dxCommon;
		srvManager_ = srvManager;
	}

	//----------------------------------------------------------------------------
	// データ取得
	//----------------------------------------------------------------------------
	template<typename TData>
	inline TData* PrimitiveBase<TData>::GetData(uint32_t handle) {
		auto it = datas_.find(handle);
		if (it == datas_.end()) {
			return nullptr;  // ハンドルが無効
		}
		return it->second.get();
	}

	//----------------------------------------------------------------------------
	// transform取得
	//----------------------------------------------------------------------------

	template<typename TData>
	inline const EulerTransform& PrimitiveBase<TData>::GetTransform(uint32_t handle) {
		auto it = datas_.find(handle);
		if (it == datas_.end()) {
			return EulerTransform();  // ハンドルが無効
		}
		return it->second->transform;
	}

	//----------------------------------------------------------------------------
	// マテリアル色設定
	//----------------------------------------------------------------------------
	template<typename TData>
	inline void PrimitiveBase<TData>::SetMaterialColor(uint32_t handle, const Vector4& color) {
		auto it = datas_.find(handle);
		if (it == datas_.end()) {
			return;  // ハンドルが無効
		}
		it->second->material->SetMaterialColor(color);
	}


	//----------------------------------------------------------------------------
	// transform設定
	//----------------------------------------------------------------------------
	template<typename TData>
	inline void PrimitiveBase<TData>::SetTransform(uint32_t handle, const EulerTransform& transform) {

		datas_[handle]->transform = transform;
	}

	//----------------------------------------------------------------------------
	// データ登録
	//----------------------------------------------------------------------------
	template<typename TData>
	inline uint32_t PrimitiveBase<TData>::RegisterData(std::unique_ptr<TData> data) {
		uint32_t handle = nextHandle_++;
		datas_[handle] = std::move(data);
		return handle;
	}

	//----------------------------------------------------------------------------
	// マテリアル作成
	//----------------------------------------------------------------------------
	template<typename TData>
	inline void PrimitiveBase<TData>::CreateMaterial(TData* data, const std::string& textureFilePath) {
		data->material = std::make_unique<Material>();
		data->material->Initialize(dxCommon_, textureFilePath, "");
		data->material->InitializeMaterialResource(dxCommon_->GetDevice());
	}

} // namespace TakeC