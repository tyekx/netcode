#include "MaterialConverter.h"
#include <Netcode/Graphics/Material.h>
#include <Netcode/Utility.h>
#include "JsonUtility.h"

namespace Netcode {

    static void LoadParameters(Ptr<Netcode::Material> mat, const json11::Json::object & params) {
        switch(mat->GetType()) {
            case Netcode::MaterialType::BRDF: {
                mat->SetParameter(Netcode::MaterialParamId::DIFFUSE_ALBEDO, Netcode::Asset::LoadFloat4(params.find("diffuse_albedo")->second));
                mat->SetParameter(Netcode::MaterialParamId::SPECULAR_ALBEDO, Netcode::Asset::LoadFloat3(params.find("specular_albedo")->second));
                mat->SetParameter(Netcode::MaterialParamId::ROUGHNESS, static_cast<float>(params.find("roughness")->second.number_value()));
                mat->SetParameter(Netcode::MaterialParamId::REFLECTANCE, static_cast<float>(params.find("reflectance")->second.number_value()));
                mat->SetParameter(Netcode::MaterialParamId::METAL_MASK, params.find("metal_mask")->second.bool_value());
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_TILES, Netcode::Asset::LoadFloat2(params.find("texture_tiles")->second));
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_TILES_OFFSET, Netcode::Asset::LoadFloat2(params.find("texture_tiles_offset")->second));
                mat->SetParameter(Netcode::MaterialParamId::DISPLACEMENT_SCALE, static_cast<float>(params.find("displacement_scale")->second.number_value()));
                mat->SetParameter(Netcode::MaterialParamId::DISPLACEMENT_BIAS, static_cast<float>(params.find("displacement_bias")->second.number_value()));
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_DIFFUSE_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("diffuse_path")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_NORMAL_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("normal_path")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_AMBIENT_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("ambient_path")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_SPECULAR_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("specular_path")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_ROUGHNESS_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("roughness_path")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_DISPLACEMENT_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("displacement_path")->second.string_value()), Netcode::FullPathToken{} });
            } break;

            default: break;
        }
    }

    static json11::Json::object StoreParameters(const Ref<Netcode::Material> & mat) {
        json11::Json::object convertedParams;

        switch(mat->GetType()) {
            case Netcode::MaterialType::BRDF: {
                convertedParams["diffuse_albedo"] = Netcode::Asset::StoreFloat4(mat->GetRequiredParameter<Netcode::Float4>(Netcode::MaterialParamId::DIFFUSE_ALBEDO));
                convertedParams["specular_albedo"] = Netcode::Asset::StoreFloat3(mat->GetRequiredParameter<Netcode::Float3>(Netcode::MaterialParamId::SPECULAR_ALBEDO));
                convertedParams["roughness"] = mat->GetRequiredParameter<float>(Netcode::MaterialParamId::ROUGHNESS);
                convertedParams["reflectance"] = mat->GetRequiredParameter<float>(Netcode::MaterialParamId::REFLECTANCE);
                convertedParams["metal_mask"] = mat->GetRequiredParameter<bool>(Netcode::MaterialParamId::METAL_MASK);
                convertedParams["texture_tiles"] = Netcode::Asset::StoreFloat2(mat->GetRequiredParameter<Netcode::Float2>(Netcode::MaterialParamId::TEXTURE_TILES));
                convertedParams["texture_tiles_offset"] = Netcode::Asset::StoreFloat2(mat->GetRequiredParameter<Netcode::Float2>(Netcode::MaterialParamId::TEXTURE_TILES_OFFSET));
                convertedParams["displacement_scale"] = mat->GetRequiredParameter<float>(Netcode::MaterialParamId::DISPLACEMENT_SCALE);
                convertedParams["displacement_bias"] = mat->GetRequiredParameter<float>(Netcode::MaterialParamId::DISPLACEMENT_BIAS);
                convertedParams["diffuse_path"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_DIFFUSE_PATH).GetFullPath());
                convertedParams["normal_path"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_NORMAL_PATH).GetFullPath());
                convertedParams["ambient_path"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_AMBIENT_PATH).GetFullPath());
                convertedParams["specular_path"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_SPECULAR_PATH).GetFullPath());
                convertedParams["roughness_path"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_ROUGHNESS_PATH).GetFullPath());
                convertedParams["displacement_path"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_DISPLACEMENT_PATH).GetFullPath());
            } break;
            default: break;
        }

        return convertedParams;
    }

    std::vector<Netcode::Asset::Manifest::Material> ConvertToManifestMaterials(const std::vector<Ref<Netcode::Material>> & mats) {
        std::vector<Netcode::Asset::Manifest::Material> convertedMaterials;

        for(const auto & mat : mats) {
            Netcode::Asset::Manifest::Material m;
            m.name = mat->GetName();
            m.type = static_cast<int32_t>(mat->GetType());
            m.parameters = StoreParameters(mat);
            convertedMaterials.emplace_back(std::move(m));
        }

        return convertedMaterials;
    }

    std::vector<Ref<Netcode::Material>> ConvertToNetcodeMaterials(const std::vector<Netcode::Asset::Manifest::Material> & mats) {
        std::vector<Ref<Netcode::Material>> convertedMaterial;

        for(const auto & mat : mats) {
            Ref<Netcode::Material> m;

            switch(static_cast<Netcode::MaterialType>(mat.type)) {
                case Netcode::MaterialType::BRDF:
                    m = std::make_shared<Netcode::BrdfMaterial>(Netcode::MaterialType::BRDF, mat.name);
                    break;
                default: break;
            }

            LoadParameters(m.get(), mat.parameters);

            convertedMaterial.push_back(m);
        }

        return convertedMaterial;
    }

}
