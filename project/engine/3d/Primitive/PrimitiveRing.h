#pragma once
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "base/PipelineStateObject.h"
#include "3d/Material.h"
#include "3d/VertexData.h"
#include "3d/Primitive/PrimitiveBase.h"

#include <memory>
#include <cstdint>
#include <unordered_map>
#include <string>

namespace TakeC {

//============================================================================
// PrimitiveRing class
//============================================================================
class PrimitiveRing {
public:

	//========================================================================
	// Ring専用データ構造体
	//========================================================================
	struct RingData {
		PrimitiveMesh mesh;                  // 頂点バッファ情報
		VertexData* vertexData = nullptr;    // 頂点データポインタ
		std::unique_ptr<Material> material;  // マテリアル（unique_ptrで管理）
		float outerRadius = 1.0f;            // 外側の半径
		float innerRadius = 0.01f;           // 内側の半径
		uint32_t subDivision = 32;           // 分割数
		uint32_t vertexCount = 0;            // この Ring 固有の頂点数
	};

public:

	PrimitiveRing() = default;
	~PrimitiveRing() = default;

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// リングデータの生成
	/// </summary>
	/// <returns>生成したハンドル</returns>
	uint32_t Generate(float outerRadius, float innerRadius, const std::string& textureFilePath);

	/// <summary>
	/// 描画処理（パーティクル用）
	/// </summary>
	void DrawParticle(PSO* pso, UINT instanceCount, uint32_t handle);

	/// <summary>
	/// 描画処理（オブジェクト用）
	/// </summary>
	void DrawObject(PSO* pso, uint32_t handle);

	//========================================================================
	// accessors
	//========================================================================

	/// データ取得
	RingData* GetData(uint32_t handle);

	/// マテリアル色設定
	void SetMaterialColor(uint32_t handle, const Vector4& color);

private:

	//========================================================================
	// private functions
	//========================================================================

	/// 頂点データ作成
	void CreateVertexData(RingData* ringData);

	/// マテリアル作成
	void CreateMaterial(const std::string& textureFilePath, RingData* ringData);

private:

	//========================================================================
	// member variables
	//========================================================================

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	// プリミティブデータ管理用マップ
	std::unordered_map<uint32_t, std::unique_ptr<RingData>> datas_;
	uint32_t nextHandle_ = 0;

	// 定数
	static constexpr uint32_t kVerticesPerSegment = 6;  // 1セグメントあたりの頂点数
};

} // namespace TakeC
