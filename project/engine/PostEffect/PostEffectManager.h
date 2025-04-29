#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <string>

class PostEffectManager {
public:
	PostEffectManager() = default;
	~PostEffectManager() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	void Finalize();

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

	//clearValue
	const Vector4 kRenderTargetClearColor_ = { 0.3f, 0.3f, 0.3f, 1.0f };
	const float clearValue_[4] = { kRenderTargetClearColor_.x, kRenderTargetClearColor_.y, kRenderTargetClearColor_.z, kRenderTargetClearColor_.w };
	//RTV
	ComPtr<ID3D12Resource> renderTextureResource_;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	uint32_t srvHandle_ = 0;
	//RTVHandleの要素数
	static inline const uint32_t rtvCount_ = 2; 
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_{};

};