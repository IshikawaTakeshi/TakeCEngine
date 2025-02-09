#pragma once
#include <string>
#include "Animation/NodeAnimation.h"

class Player;
class BasePlayerState {
public:

	BasePlayerState(const std::string& name,Player* player) : name_(name),player_(player) {};

	virtual ~BasePlayerState() {};

	//初期化
	virtual void Initialize() = 0;

	//更新
	virtual void Update() = 0;

	//状態名の取得
	const std::string& GetName() const { return name_; }

	//アニメーションの取得
	const Animation& GetAnimation() const { return stateAnimation_; }

protected:

	//状態名
	std::string name_;

	Animation stateAnimation_;

	Player* player_ = nullptr;

};