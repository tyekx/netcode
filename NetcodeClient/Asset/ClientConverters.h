#pragma once

#include "Converters.h"

class GameObjectCatalog;

class ClientAssetConverter : public AssetConverterBase {
protected:
	GameObjectCatalog * catalog;

	/*
	 * Converters from JSON to Netcode
	 */
	virtual Ref<Netcode::Material> ConvertMaterial(const Netcode::JsonValue & values, Ptr<const Netcode::Material> sourceMaterial) override;
	virtual GameObject * ConvertColliderComponent(GameObject * gameObject, const Netcode::JsonValue & values) override;
	virtual GameObject * ConvertModelComponent(GameObject * gameObject, const Netcode::JsonValue & values) override;
	virtual GameObject * ConvertLightComponent(GameObject * gameObject, const Netcode::JsonValue & values) override;
	virtual GameObject * ConvertTransformComponent(GameObject * gameObject, const Netcode::JsonValue & values);
	virtual GameObject * ConvertGameObject(const Netcode::JsonValue & values) override;

	/*
	 * Converters from Netcode Asset to Netcode
	 */
	virtual Ref<Netcode::Material> ConvertMaterial(const Netcode::Asset::Material * mat) override;
	virtual Ref<Mesh> ConvertMesh(const Netcode::Asset::Mesh * mesh) override;
	virtual GameObject * ConvertColliderComponent(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) override;
	virtual GameObject * ConvertModelComponent(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) override;
	virtual GameObject * ConvertComponents(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) override;
public:

	ClientAssetConverter(GameObjectCatalog * catalog, Netcode::JsonValue * json, GameScene * scene);
	
	// Converts the scene json to the scene
	virtual void ConvertScene() override;
};
