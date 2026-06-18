#include "OnnxRuntimeSystem.h"

using namespace TakeC;


//====================================================================
//	インスタンスの取得
//====================================================================
OnnxRuntimeSystem& TakeC::OnnxRuntimeSystem::GetInstance() {
	static OnnxRuntimeSystem instance;
	return instance;
}
