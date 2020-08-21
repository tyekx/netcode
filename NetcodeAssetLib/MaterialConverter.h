#pragma once

#include "Manifest.h"
#include <Netcode/HandleDecl.h>

namespace Netcode {

    std::vector<Netcode::Asset::Manifest::Material> ConvertToManifestMaterials(const std::vector<Ref<Netcode::Material>> & mats);

    std::vector<Ref<Netcode::Material>> ConvertToNetcodeMaterials(const std::vector<Netcode::Asset::Manifest::Material> & mats);

}
