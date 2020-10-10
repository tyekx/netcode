#include "ServerConverters.h"
#include <NetcodeAssetLib/Model.h>
#include <Netcode/Utility.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include "../GameObject.h"
#include "../Services.h"
#include "../Scripts/DebugScript.h"
#include "ComponentProxy.hpp"

Ref<Netcode::Material> ServerAssetConverter::ConvertMaterial(const Netcode::JsonValue & values, Ref<Netcode::Material> sourceMaterial) {
	return nullptr;
}

GameObject * ServerAssetConverter::ConvertModelComponent(GameObject * gameObject, const Netcode::JsonValue & values) {
	std::wstring assetPath;
	Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"asset"), assetPath);
	const Netcode::URI::Model modelUri{ std::move(assetPath) };
	const auto catalogProxy = catalog->Find(modelUri);

	if(catalogProxy != nullptr) {
		gameObject = catalogProxy.Clone();
	} else {
		gameObject = scene->Create();
		AssetManager * assetManager = Service::Get<AssetManager>();
		Netcode::Asset::Model * ncAsset = assetManager->Import(modelUri);
		gameObject = ConvertComponents(gameObject, ncAsset);
	}

	return gameObject;
}

GameObject * ServerAssetConverter::ConvertLightComponent(GameObject * gameObject, const Netcode::JsonValue & values) {
	return gameObject;
}

void ServerAssetConverter::ConvertScriptComponent(GameObject * gameObject, const Netcode::JsonValue & values) {
	ComponentProxy<Script> proxy{ gameObject };

	std::wstring typeString;
	Netcode::JsonValueConverter<>::ConvertFromJson(Netcode::GetMember(values, L"type"), typeString);

	if(typeString == L"TransformDebugScript") {
		proxy->AddScript(std::make_unique<TransformDebugScript>());
		return;
	}

	Log::Error("Script type: {0} is not registered", Netcode::Utility::ToNarrowString(typeString));
}

GameObject * ServerAssetConverter::ConvertGameObject(const Netcode::JsonValue & values) {
	if(!values.IsObject()) {
		throw Netcode::UndefinedBehaviourException{ "Trying to convert a non object" };
	}

	GameObject * gameObj = nullptr;
	if(const auto it = values.FindMember(L"model"); it != values.MemberEnd()) {
		gameObj = ConvertModelComponent(gameObj, it->value);
	} else {
		gameObj = scene->Create();
	}

	if(const auto it = values.FindMember(L"transform"); it != values.MemberEnd()) {
		if(!it->value.IsObject()) {
			throw Netcode::UndefinedBehaviourException{ "transform must be a json object" };
		}
		gameObj = ConvertTransformComponent(gameObj, it->value);
	}

	if(const auto it = values.FindMember(L"script"); it != values.MemberEnd()) {
		if(!it->value.IsArray()) {
			throw Netcode::UndefinedBehaviourException{ "script must be a json array" };
		}

		for(const auto & scriptJson : it->value.GetArray()) {
			try {
				ConvertScriptComponent(gameObj, scriptJson);
			} catch(Netcode::ExceptionBase & e) {
				Log::Error("While loading script: {0}", e.ToString());
			}
		}
	}

	if(const auto it = values.FindMember(L"children"); it != values.MemberEnd()) {
		if(!it->value.IsArray()) {
			throw Netcode::UndefinedBehaviourException{ "children node must be a json array" };
		}

		for(const auto & child : it->value.GetArray()) {
			GameObject * childObject;
			try {
				childObject = ConvertGameObject(child);
			} catch(Netcode::ExceptionBase & e) {
				Log::Error("While loading game object: {0}", e.ToString());
				continue;
			}
			gameObj->AddChild(childObject);
		}
	}

	if(gameObj->HasComponent<Model>()) {
		throw Netcode::UndefinedBehaviourException{ "Server object must not have a Model component" };
	}

	scene->Spawn(gameObj);

	return gameObj;
}

Ref<Netcode::Material> ServerAssetConverter::ConvertMaterial(const Netcode::Asset::Material * mat) {
	return nullptr;
}

Ref<Mesh> ServerAssetConverter::ConvertMesh(const Netcode::Asset::Mesh * mesh) {
	return nullptr;
}

GameObject * ServerAssetConverter::ConvertModelComponent(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) {
	return gameObject;
}

GameObject * ServerAssetConverter::ConvertComponents(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) {
	if(ncAsset->colliders.Size() > 0) {
		gameObject = ConvertColliderComponent(gameObject, ncAsset);
	}

	return gameObject;
}
