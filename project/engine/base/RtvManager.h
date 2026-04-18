#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

#include "engine/base/ComPtrAliasTemplates.h"



//======================================================================
// RTVManager class
//======================================================================
namespace TakeC {

	// 前方宣言
	class DirectXCommon;

	class RtvManager {
	public:

		static const uint32_t kMaxRTVCount_; // RTVの最大数

	public:

		//==================================================================
		// functions
		//==================================================================

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(TakeC::DirectXCommon* directXCommon);

		/// <summary>
		/// 終了・開放処理
		/// </summary>
		void Finalize();

		/// <summary>
		/// RTVインデックスの取得
		/// </summary>
		/// <returns></returns>
		uint32_t Allocate();

		/// <summary>
		/// RTV生成
		/// </summary>
		void CreateRTV(ID3D12Resource* pResource, uint32_t rtvIndex);

		/// <summary>
		/// レンダーターゲットクリア
		/// </summary>
		void ClearRenderTarget();
	public:

		//==================================================================
		// accessor
		//==================================================================

		//----- getter ---------------

		/// CPUデスクリプタハンドルの取得
		D3D12_CPU_DESCRIPTOR_HANDLE GetRtvDescriptorHandleCPU(uint32_t index);
		/// GPUデスクリプタハンドルの取得
		D3D12_GPU_DESCRIPTOR_HANDLE GetRtvDescriptorHandleGPU(uint32_t index);
		/// RTV用デスクリプタヒープの取得
		ID3D12DescriptorHeap* GetRtvHeap() { return descriptorHeap_.Get(); }
		/// RTV用デスクリプタヒープの取得
		D3D12_RENDER_TARGET_VIEW_DESC GetRtvDesc() { return rtvDesc_; }

	private:

		//dxCommon
		TakeC::DirectXCommon* dxCommon_ = nullptr;

		//RTV用デスクリプタヒープ
		ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;
		//デスクリプタ1個分のサイズ
		uint32_t descriptorSize_ = 0;
		//使用中のインデックス
		uint32_t useIndex_ = 0;
		//RTV用デスクリプタヒープの記述子
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_ = {};
	};
}