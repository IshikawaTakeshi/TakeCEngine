#pragma once
#include <string>
#include <memory>

#include "BaseScene.h"

//============================================================================
// AbstractSceneFactory class
//============================================================================
/// <summary>
/// 要求された種類に応じてAbstractSceneを生成するクラスです。
/// </summary>
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