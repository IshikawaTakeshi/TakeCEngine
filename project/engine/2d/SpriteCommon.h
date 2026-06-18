#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>
#include <vector>
#include "engine/base/ComPtrAliasTemplates.h"
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector4.h"

// 前方宣言
namespace TakeC {
	class DirectXCommon;
	class PSO;
	class SrvManager;
	class Mesh;
	class Sprite;
}

//============================================================================
// SpriteInstanceData structure
//============================================================================
struct SpriteInstanceData {
	Matrix4x4 WVP;                     // 座標変換行列
	Matrix4x4 World;                   // ワールド行列
	Matrix4x4 WorldInverseTranspose;   // ワールド逆転置行列
	Vector4 color;                     // 色
	Vector2 uvLeftTop;                 // 切り出し左上UV
	Vector2 uvSize;                    // 切り出しサイズUV
	Vector2 anchorPoint;               // アンカーポイント
	Vector2 size;                      // スプライトサイズ
	int isFlipX;                       // 左右反転フラグ
	int isFlipY;                       // 上下反転フラグ
};

//============================================================================
// SpriteCommon class
//============================================================================
namespace TakeC {

class SpriteCommon {
private:

	//コンストラクタ・デストラクタ・コピー禁止
	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(const SpriteCommon&) = delete;
	SpriteCommon& operator=(const SpriteCommon&) = delete;

public:

	//========================================================================
	// functions
	//========================================================================
	static SpriteCommon& GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* directXCommon, SrvManager* srvManager);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 1フレーム分のスプライト描画バッファをリセット
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// 描画リクエストの登録（遅延描画用）
	/// </summary>
	void RegisterDrawRequest(Sprite* sprite);

	/// <summary>
	/// 登録されたスプライトを一括ソートして描画実行
	/// </summary>
	void ExecuteDraws();

public:

	//=========================================================================
	// accessors
	//=========================================================================

	//----- getter ---------------------------

	//DirectXCommonの取得
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }


	//----- setter ---------------------------

	//DirectXCommonの設定
	void SetDirectXCommon(DirectXCommon* dxCommon) { dxCommon_ = dxCommon; }



private:

	/////////////////////////////////////////////////////////////////////////////////////
	///			privateメンバ変数
	/////////////////////////////////////////////////////////////////////////////////////

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;

	//SrvManager
	SrvManager* srvManager_ = nullptr;

	//RootSignature
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	//PipelineStateObject
	std::unique_ptr<PSO> pso_ = nullptr;

	// インスタンス描画用のRootSignatureとPSO
	ComPtr<ID3D12RootSignature> instancedRootSignature_ = nullptr;
	std::unique_ptr<PSO> instancedPSO_ = nullptr;

	// 共有ユニットクアッドMesh
	std::unique_ptr<Mesh> sharedQuadMesh_ = nullptr;

	// インスタンス化StructuredBuffer
	ComPtr<ID3D12Resource> instanceResource_ = nullptr;
	SpriteInstanceData* mappedInstanceData_ = nullptr;
	uint32_t instanceSrvIndex_ = 0;

	// バッチオフセット用ConstantBuffer
	ComPtr<ID3D12Resource> batchConfigResource_ = nullptr;
	uint32_t* mappedBatchConfigData_ = nullptr;
	static const uint32_t kMaxBatches = 512;

	// 最大インスタンス数
	static const uint32_t kMaxSpriteInstances = 8192;

	// 描画リクエストキュー
	std::vector<Sprite*> drawRequests_;

	// 同一フレーム内で複数回ExecuteDrawsしてもGPU参照中のデータを上書きしないためのカーソル
	uint32_t frameInstanceCursor_ = 0;
	uint32_t frameBatchCursor_ = 0;

};

} // namespace TakeC
