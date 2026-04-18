#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

//前方宣言
class PostEffect;

//============================================================
//	PostEffectFactory class
//============================================================
class PostEffectFactory {
public:

	// エフェクト生成用関数型
	using Creator = std::function<std::unique_ptr<PostEffect>()>;

	PostEffectFactory();
	~PostEffectFactory() = default;

	/// <summary>
	// エフェクトの登録
	/// </summary>
	void Register(const std::string& name, Creator creator);

	/// <summary>
	// エフェクトの生成
	/// </summary>
	std::unique_ptr<PostEffect> Create(const std::string& name) const;

	/// <summary>
	/// エフェクトが登録されているか確認
	/// </summary>
	/// <param name="name"></param>
	/// <returns></returns>
	bool IsRegistered(const std::string& name) const;

private:
	
	std::unordered_map<std::string, Creator> creators_;
};
