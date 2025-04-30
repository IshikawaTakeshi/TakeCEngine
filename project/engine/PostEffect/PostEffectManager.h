#pragma once
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "base/RtvManager.h"
#include "base/PipelineStateObject.h"
#include <string>

class PostEffectManager {
public:
	PostEffectManager() = default;
	~PostEffectManager() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	///	描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

private:

	DirectXCommon* dxCommon_ = nullptr; //DirectXCommonのポインタ
	SrvManager* srvManager_ = nullptr; //SrvManagerのポインタ
	RtvManager* rtvManager_ = nullptr; //RtvManagerのポインタ

	//clearValue
	const Vector4 kRenderTargetClearColor_ = { 0.3f, 0.3f, 0.3f, 1.0f };
	const float clearValue_[4] = { kRenderTargetClearColor_.x, kRenderTargetClearColor_.y, kRenderTargetClearColor_.z, kRenderTargetClearColor_.w };
	//RTV
	ComPtr<ID3D12Resource> renderTextureResource_;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	//RTVManagerで使用するDiscriptorHandleのインデックス
	uint32_t rtvIndex_ = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
	//SRVManagerで使用するDiscriptorHandleのインデックス
	uint32_t srvIndex_ = 0;
	//DSVハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_{};

	//RenderTexture描画パイプライン
	std::unique_ptr<PSO> renderTexturePSO_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
};