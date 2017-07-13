#ifndef _DATASPEC_ASSETNAMEMAP_HPP_
#define _DATASPEC_ASSETNAMEMAP_HPP_

#include <unordered_map>
#include <string>
#include "DNACommon/DNACommon.hpp"

namespace DataSpec
{
namespace AssetNameMap
{
void InitAssetNameMap();
const std::string* TranslateIdToName(const UniqueID32&);
const std::string* TranslateIdToName(const UniqueID64&);
}
}
#endif // _DATASPEC_ASSETNAMEMAP_HPP_
