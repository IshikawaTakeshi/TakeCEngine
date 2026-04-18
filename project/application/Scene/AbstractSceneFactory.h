#pragma once
#include <string>
#include <memory>

#include "BaseScene.h"

//============================================================================
// AbstractSceneFactory class
//============================================================================
class AbstractSceneFactory {
public:

	//========================================================
	// functions
	//========================================================

	//デストラクタ
	virtual ~AbstractSceneFactory() = default;
	//シーン生成
	virtual std::shared_ptr<BaseScene> CreateScene(const std::string& sceneName) = 0;
};