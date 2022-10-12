#include "ioutil.h"

#ifdef __ANDROID__
#include <android/asset_manager.h>
#else
#include <fstream>
#endif

namespace Util
{
#ifdef __ANDROID__
static AAssetManager *s_assetManager = nullptr;

void setAssetManager(AAssetManager *assetManager)
{
    s_assetManager = assetManager;
}
#endif

std::optional<std::vector<unsigned char>> readFile(const std::string &path)
{
#ifdef __ANDROID__
    if (!s_assetManager)
        return {};
    AAsset *asset = AAssetManager_open(s_assetManager, path.c_str(), 0);
    if (!asset)
        return {};
    const auto size = AAsset_getLength(asset);
    std::vector<unsigned char> data(size);
    AAsset_read(asset, data.data(), size);
    AAsset_close(asset);
    return data;
#else
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return {};

    auto *buf = file.rdbuf();

    const std::size_t size = buf->pubseekoff(0, file.end, file.in);
    buf->pubseekpos(0, file.in);

    std::vector<unsigned char> data(size);
    buf->sgetn(reinterpret_cast<char *>(data.data()), size);

    file.close();

    return data;
#endif
}

} // namespace Util
