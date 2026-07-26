#include "ResourcePath.h"

#include <utility>

namespace {

	std::filesystem::path& GameRootStorage() {
		static std::filesystem::path root = "Resources";
		return root;
	}

	std::filesystem::path& EngineRootStorage() {
		static std::filesystem::path root = "Resources";
		return root;
	}

	std::filesystem::path NormalizeRoot(const std::filesystem::path& root) {
		return (root.empty() ? std::filesystem::path(".") : root).lexically_normal();
	}

}

void TakeC::ResourcePath::Configure(const ResourceRootConfig& config) {
	SetGameRoot(config.gameRoot);
	SetEngineRoot(config.engineRoot);
}

void TakeC::ResourcePath::SetGameRoot(const std::filesystem::path& root) {
	GameRootStorage() = NormalizeRoot(root);
}

void TakeC::ResourcePath::SetEngineRoot(const std::filesystem::path& root) {
	EngineRootStorage() = NormalizeRoot(root);
}

const std::filesystem::path& TakeC::ResourcePath::GetGameRoot() {
	return GameRootStorage();
}

const std::filesystem::path& TakeC::ResourcePath::GetEngineRoot() {
	return EngineRootStorage();
}

std::filesystem::path TakeC::ResourcePath::Resolve(
	ResourceDomain domain,
	const std::filesystem::path& relativePath) {

	if (relativePath.is_absolute()) {
		return relativePath.lexically_normal();
	}

	const std::filesystem::path& root = domain == ResourceDomain::Engine
		? GetEngineRoot()
		: GetGameRoot();
	return (root / relativePath).lexically_normal();
}

std::filesystem::path TakeC::ResourcePath::Game(
	const std::filesystem::path& relativePath) {
	return Resolve(ResourceDomain::Game, relativePath);
}

std::filesystem::path TakeC::ResourcePath::Engine(
	const std::filesystem::path& relativePath) {
	return Resolve(ResourceDomain::Engine, relativePath);
}
