#pragma once
#include "Kyoto/SObjectTag.hpp"
#include "rstl/string.hpp"
#include <string_view>
namespace metaforce::ResourceNameDatabase {
const rstl::string* GetNameForResource(CAssetId uid);
bool HaveNameForResource(CAssetId uid);
bool Initialize(std::string_view databasePath);
} // namespace metaforce::ResourceNameDatabase