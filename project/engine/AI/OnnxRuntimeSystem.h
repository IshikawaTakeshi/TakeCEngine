#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <packages/Microsoft.AI.DirectML.1.15.4/include/DirectML.h>
#include <wrl.h>
#include <memory>
#include <string>
#include <unordered_map>

#include "engine/AI/OnnxModel.h"
#include "engine/base/DirectXCommon.h"

namespace TakeC {


	//====================================================================
	// 	OnnxRuntimeSystem class
	//====================================================================
	/// <summary>
	/// ONNX RuntimeとDirectMLの実行環境を初期化し、推論基盤を提供するクラスです。
	/// </summary>
	class OnnxRuntimeSystem {
	public:
		
		//================================================================
		// functions
		//================================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		OnnxRuntimeSystem() = default;
		~OnnxRuntimeSystem() = default;

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="dxCommon"></param>
		void Initialize(DirectXCommon* dxCommon);

		/// <summary>
		/// 終了・開放処理
		/// </summary>
		void Finalize();

		/// <summary>
		/// onnxモデルのロード
		/// </summary>
		/// <param name="name"></param>
		/// <param name="modelPath"></param>
		/// <returns></returns>
		OnnxModel* LoadModel(const std::string& name, const std::wstring& modelPath);

		/// <summary>
		/// onnxモデルの取得
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		OnnxModel* FindModel(const std::string& name);

		/// <summary>
		/// 初期化済みかどうかの確認
		/// </summary>
		/// <returns></returns>
		bool IsInitialized() const { return initialized_; }

	private:
		
		ComPtr<IDMLDevice> dmlDevice_ = nullptr;
		DirectXCommon* dxCommon_ = nullptr;

		std::unique_ptr<Ort::Env> env_;
		std::unique_ptr<Ort::MemoryInfo> cpuMemoryInfo_;

		std::unordered_map<std::string, std::unique_ptr<OnnxModel>> models_;

		bool initialized_ = false;
	};

}
