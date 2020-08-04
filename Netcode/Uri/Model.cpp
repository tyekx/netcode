#include "Model.h"
#include <Netcode/IO/Path.h>

namespace Netcode::URI {

    Model::Model(const wchar_t * relativePath) : AssetBase{} {
        std::wstring tmp = relativePath;
        Model v(std::move(tmp));
        *this = std::move(v);
    }

    Model::Model(std::wstring relativePath) : AssetBase{} {
        if(relativePath.empty()) {
            return;
        }

        IO::Path::UnifySlashes(relativePath, L'/');

        if(relativePath.front() == L'/') {
            return;
        }

        ConstructPaths(relativePath, GetGlobalId(), IO::Path::MediaRoot());
    }

    Model::Model(std::wstring fullPath, FullPathToken) : AssetBase{} {
        IO::Path::UnifySlashes(fullPath, L'/');

        constexpr auto globalId = Model::GetGlobalId();

        if(fullPath.find(globalId, 0) != 0) {
            return;
        }

        std::wstring_view localPath = { fullPath.c_str() + globalId.size(), fullPath.size() - globalId.size() };

        ConstructPaths(localPath, globalId, IO::Path::MediaRoot());
    }

    const std::wstring & Model::GetModelPath() const
    {
        return GetAssetPath();
    }

    Model Model::Parse(UriBase uri)
    {
        return Model(std::move(uri.GetMutableFullPath()), FullPathToken{});
    }

    Model Model::AsExtensionOf(const AssetBase & anotherAsset, std::wstring relativeToAssetPath) {
        std::wstring relPath = ExtensionOf(anotherAsset, std::move(relativeToAssetPath), IO::Path::MediaRoot());

        return Model{ std::move(relPath) };
    }

}
