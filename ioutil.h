#pragma once

#include <optional>
#include <string>
#include <vector>

struct AAssetManager;

namespace Util
{
std::optional<std::vector<unsigned char>> readFile(const std::string &path);
}
