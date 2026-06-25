#pragma once
#include <packages/Microsoft.AI.DirectML.1.15.4/include/DirectML.h>
#include <wrl.h>
#include "engine/AI/OnnxModel.h"
#include "engine/base/DirectXCommon.h"

namespace TakeC {


	//====================================================================
	// 	OnnxRuntimeSystem class
	//====================================================================
	class OnnxRuntimeSystem {
	public:
		
		//================================================================
		// functions
		//================================================================

		/// <summary>
		/// インスタンスの取得
		/// </summary>
		/// <returns></returns>
		static OnnxRuntimeSystem& GetInstance();

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="dxCommon"></param>
		void Initialize(DirectXCommon* dxCommon);

		/// <summary>
		/// 終了・開放処理
		/// </summary>
		void Finalize();

		//OnnxModel* LoadModel(const std::string& name, const std::wstring& modelPath);
		//OnnxModel* FindModel(const std::string& name);

		bool IsInitialized() const { return initialized_; }

	private:
		OnnxRuntimeSystem() = default;

		ComPtr<IDMLDevice> dmlDevice_ = nullptr;
		DirectXCommon* dxCommon_ = nullptr;

		std::unique_ptr<Ort::Env> env_;
		std::unique_ptr<Ort::MemoryInfo> cpuMemoryInfo_;

		std::unordered_map<std::string, std::unique_ptr<OnnxModel>> models_;

		bool initialized_ = false;
	};

}