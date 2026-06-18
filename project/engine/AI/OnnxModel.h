#pragma once
#include <onnxruntime_cxx_api.h>
#include <dml_provider_factory.h>

namespace TakeC {

	//====================================================================
	// 	OnnxModel class
	//====================================================================
	class OnnxModel {
	public:

		//================================================================
		// functions
		//================================================================
		OnnxModel() = default;
		~OnnxModel() = default;
		void Initialize();
		void Finalize();
		void RunInference();

	};

}