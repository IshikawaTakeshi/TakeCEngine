#pragma once
#include "engine/AI/OnnxModel.h"

namespace TakeC {

	class DirectXCommon;

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

		void Initialize(DirectXCommon* dxCommon);
		void Finalize();

		OnnxModel* LoadModel(const std::string& name, const std::wstring& modelPath);
		OnnxModel* FindModel(const std::string& name);

		bool IsInitialized() const { return initialized_; }

	private:
		OnnxRuntimeSystem() = default;

		DirectXCommon* dxCommon_ = nullptr;

		std::unique_ptr<Ort::Env> env_;
		std::unique_ptr<Ort::MemoryInfo> cpuMemoryInfo_;

		std::unordered_map<std::string, std::unique_ptr<OnnxModel>> models_;

		bool initialized_ = false;
	};

}