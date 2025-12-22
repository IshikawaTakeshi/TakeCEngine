#pragma once
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "base/PipelineStateObject.h"
#include "PostEffect/PostEffectManager.h"
#include "engine/base/WinApp.h"

//============================================================
//	RenderTexture class
//============================================================
class RenderTexture {
public:

	//=========================================================
	// functions
	//=========================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	RenderTexture() = default;
	~RenderTexture();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	/// <param name="postEffectManager"></param>
	void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,
		int32_t depthWidth = TakeC::WinApp::kScreenWidth,
		int32_t depthHeight = TakeC::WinApp::kScreenHeight);

	/// <summary>
	/// レンダーターゲットのクリア
	/// </summary>
	void ClearRenderTarget();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	void TransitionToSRV();

	void TransitionToDepthWrite();

	void SetViewport(int32_t width, int32_t height);
	void SetScissorRect(int32_t width, int32_t height);

public:

	//========================================================
	// accessors
	//========================================================

	//------ getter ----------------

	//レンダーテクスチャリソースの取得
	ComPtr<ID3D12Resource> GetRenderTextureResource() const {
		return renderTextureResource_;
	}
	ComPtr<ID3D12Resource> GetDepthStencilResource() const {
		return depthStencilResource_;
	}
	PSO* GetRenderTexturePSO() const {
		return renderTexturePSO_.get();
	}

	uint32_t GetSrvIndex() const {return srvIndex_;}
	uint32_t GetDepthSrvIndex() const { return depthSrvIndex_; }

	uint32_t GetRtvIndex() const { return rtvIndex_; }
	uint32_t GetDsvIndex() const { return dsvIndex_; }

private:

	TakeC::DirectXCommon* dxCommon_ = nullptr; //DirectXCommonのポインタ
	TakeC::SrvManager* srvManager_ = nullptr; //SrvManagerのポインタ
	TakeC::RtvManager* rtvManager_ = nullptr; //RtvManagerのポインタ
	//clearValue
	//clearValue
	const Vector4 kRenderTargetClearColor_ = { 0.1f, 0.25f, 0.5f, 1.0f };
	const float clearValue_[4] = { kRenderTargetClearColor_.x, kRenderTargetClearColor_.y, kRenderTargetClearColor_.z, kRenderTargetClearColor_.w };

	//RTVのハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;
	uint32_t rtvIndex_ = {};
	//DSVのハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;
	uint32_t dsvIndex_ = {};
	//レンダーテクスチャのSRVインデックス
	uint32_t srvIndex_ = 0;
	//深度テクスチャのSRVインデックス
	uint32_t depthSrvIndex_ = 0;

	ComPtr<ID3D12Resource> renderTextureResource_; 
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_;

	ComPtr<ID3D12Resource> depthStencilResource_; //深度ステンシルバッファリソース

	std::unique_ptr<PSO> renderTexturePSO_; //PSO
	ComPtr<ID3D12RootSignature> rootSignature_; //ルートシグネチャ
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_ = {};

};