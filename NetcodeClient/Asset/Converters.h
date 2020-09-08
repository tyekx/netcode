#pragma once

#include <Netcode/HandleDecl.h>
#include <NetcodeFoundation/Json.h>

namespace Netcode::Asset {
	class Model;
	struct Mesh;
	struct Material;
}

class Mesh;
class GameObject;
class GameScene;
class Transform;
class Model;

class AssetConverterBase {
protected:
	Netcode::JsonValue * sourceJson;
	GameScene * scene;

	/*
	 * Converters from JSON to Netcode 
	 */
	virtual Ref<Netcode::Material> ConvertMaterial(const Netcode::JsonValue & values, Ptr<const Netcode::Material> sourceMaterial = nullptr) = 0;
	virtual GameObject * ConvertColliderComponent(GameObject * gameObject, const Netcode::JsonValue & values) = 0;
	virtual GameObject * ConvertModelComponent(GameObject * gameObject, const Netcode::JsonValue & values) = 0;
	virtual GameObject * ConvertLightComponent(GameObject * gameObject, const Netcode::JsonValue & values) = 0;
	virtual GameObject * ConvertGameObject(const Netcode::JsonValue & values) = 0;

	/*
	 * Converters from Netcode Asset to Netcode
	 */
	virtual Ref<Mesh> ConvertMesh(const Netcode::Asset::Mesh * mesh) = 0;
	virtual Ref<Netcode::Material> ConvertMaterial(const Netcode::Asset::Material * mat) = 0;
	virtual GameObject * ConvertColliderComponent(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) = 0;
	virtual GameObject * ConvertModelComponent(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) = 0;
	virtual GameObject * ConvertComponents(GameObject * gameObject, const Netcode::Asset::Model * ncAsset) = 0;
public:
	AssetConverterBase(GameScene * scene, Netcode::JsonValue * sourceJson) : sourceJson{ sourceJson }, scene { scene } { }
	virtual ~AssetConverterBase() = default;
	// Converts the scene json to the scene
	virtual void ConvertScene() = 0;
};
