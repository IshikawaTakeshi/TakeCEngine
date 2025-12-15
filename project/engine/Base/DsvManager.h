#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include "engine/base/ComPtrAliasTemplates.h"

//============================================================
//	DsvManager class
//============================================================

namespace TakeC {

		// 前方宣言
		class DirectXCommon;

	class DsvManager {
	public:


		static const uint32_t kMaxDSVCount; // DSVヒープのディスクリプタ数
		//=========================================================
		// functions
		//=========================================================
		
		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		DsvManager() = default;
		~DsvManager() = default;
		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(TakeC::DirectXCommon* dxCommon);
		
		/// <summary>
		/// SRVインデックスの取得
		/// </summary>
		/// <returns></returns>
		uint32_t Allocate();

		/// <summary>
		/// 描画前処理
		/// </summary>
		void SetDescriptorHeap();

		void CreateDSV(ComPtr<ID3D12Resource> depthStencilResource, uint32_t dsvIndex);

	public:

		//==================================================================
		// accessor
		//==================================================================

		//----- getter ---------------

		/// CPUデスクリプタハンドルの取得
		D3D12_CPU_DESCRIPTOR_HANDLE GetDsvDescriptorHandleCPU(uint32_t index);
		/// GPUデスクリプタハンドルの取得
		D3D12_GPU_DESCRIPTOR_HANDLE GetDsvDescriptorHandleGPU(uint32_t index);
		/// DSV用デスクリプタヒープの取得
		ID3D12DescriptorHeap* GetDsvHeap() { return descriptorHeap_.Get(); }
		/// DSV用デスクリプタヒープの取得
		D3D12_DEPTH_STENCIL_VIEW_DESC GetDsvDesc() { return dsvDesc_; }

	private:

		TakeC::DirectXCommon* dxCommon_ = nullptr; // DirectXCommonのポインタ
		ComPtr<ID3D12DescriptorHeap> descriptorHeap_; // DSV用デスクリプタヒープ
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc_{}; // DSVのデスクリプタ設定構造体
		uint32_t descriptorSize_ = 0; // DSVのデスクリプタサイズ
		uint32_t useIndex_ = 0; // 現在のDSVインデックス
	};
}