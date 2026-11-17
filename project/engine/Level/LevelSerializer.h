#pragma once

#include "engine/Level/LevelDocument.h"

#include <filesystem>

namespace TakeC::Level {

/// <summary>TakeCLevel形式のJSONを検証して文書へ変換します。</summary>
LevelDocument ParseLevelDocument(const nlohmann::json& source);

/// <summary>文書を検証してTakeCLevel形式のJSONへ変換します。</summary>
nlohmann::json SerializeLevelDocument(const LevelDocument& document);

/// <summary>TakeCLevel形式のファイルを読み込みます。</summary>
LevelDocument LoadLevelDocument(const std::filesystem::path& path);

/// <summary>一時ファイルへ書いてから対象レベルファイルを置き換えます。</summary>
void SaveLevelDocument(const std::filesystem::path& path, const LevelDocument& document);

} // namespace TakeC::Level
