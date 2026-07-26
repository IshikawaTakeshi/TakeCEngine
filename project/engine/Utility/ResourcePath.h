#pragma once

#include <filesystem>

namespace TakeC {

	enum class ResourceDomain {
		Game,
		Engine,
	};

	/// <summary>
	/// ゲーム用とエンジン用のリソースルート設定を保持する構造体です。
	/// </summary>
	struct ResourceRootConfig {
		std::filesystem::path gameRoot = "Resources";
		std::filesystem::path engineRoot = "Resources";
	};

	/// <summary>
	/// ゲーム資産とエンジン資産のルートを一元管理する。
	/// Configureは各種マネージャーを初期化する前に呼び出すこと。
	/// </summary>
	class ResourcePath {
	public:
		static void Configure(const ResourceRootConfig& config);
		static void SetGameRoot(const std::filesystem::path& root);
		static void SetEngineRoot(const std::filesystem::path& root);

		static const std::filesystem::path& GetGameRoot();
		static const std::filesystem::path& GetEngineRoot();

		static std::filesystem::path Resolve(
			ResourceDomain domain,
			const std::filesystem::path& relativePath = {});
		static std::filesystem::path Game(const std::filesystem::path& relativePath = {});
		static std::filesystem::path Engine(const std::filesystem::path& relativePath = {});
	};

}
