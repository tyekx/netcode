#pragma once

#include "Converters.h"

class GameObjectCatalog;

class ClientAssetConverter : public AssetConverterBase {
	GameObjectCatalog * catalog;

public:
	/*
	 * Converters from JSON to Netcode
	 */
	virtual Ref<Netcode::Material> ConvertMaterial(const Netcode::JsonValue & values, Ref<Netcode::Material> sourceMaterial) override;
	virtual GameObject * ConvertColliderComponent(GameObject * gameObject, const Netcode::JsonValue & values) override;
	virtual GameObject * ConvertModelComponent(GameObject * gameObject, const Netcode::JsonValue & values) override;
	virtual GameObject * ConvertLightComponent(GameObject * gameObject, const Netcode::JsonValue & values) override;
	virtual GameObject * ConvertTransformComponent(GameObject * gameObject, const Netcode::JsonValue & values) override;
	virtual void ConvertScriptComponent(GameObject * gameObject, const Netcode::JsonValue & values) override;
	virtual GameObject * ConvertGameObject(const Netcode::JsonValue & values) override;

	/*
	 * Converters from Netcode Asset to Netcode
	 */
	virtual Ref<Netcode::Material> ConvertMaterial(const Netcode::Asset::Material * mat) override;
	virtual Ref<Mesh> ConvertMesh(const Netcode::Asset::Mesh * mesh) override;
	virtual GameObject * ConvertColliderComponent(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) override;
	virtual GameObject * ConvertModelComponent(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) override;
	virtual GameObject * ConvertComponents(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) override;

	ClientAssetConverter(GameObjectCatalog * catalog, Netcode::JsonValue * json, GameScene * scene);
	
	// Converts the scene json to the scene
	virtual void ConvertScene() override;
};
