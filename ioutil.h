#pragma once

#include <optional>
#include <string>
#include <vector>

struct AAssetManager;

namespace Util
{
#ifdef __ANDROID__
void setAssetManager(AAssetManager *manager);
#endif
std::optional<std::vector<unsigned char>> readFile(const std::string &path);
}
