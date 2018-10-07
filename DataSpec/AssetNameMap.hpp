#pragma once

#include <unordered_map>
#include <string>
#include "DNACommon/DNACommon.hpp"

namespace DataSpec::AssetNameMap
{
void InitAssetNameMap();
const std::string* TranslateIdToName(const UniqueID32&);
const std::string* TranslateIdToName(const UniqueID64&);
}
