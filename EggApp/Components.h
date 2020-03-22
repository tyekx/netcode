#pragma once

#include <functional>
#include <string>
#include "ConstantBufferTypes.h"
#include <EggAssetLib/Collider.h>
#include <Egg/Event.hpp>
#include <Egg/HandleTypes.h>
#include <Egg/PhysXWrapper.h>
#include "Mesh.h"
#include "Material.h"
#include <Egg/Blackboard.h>

using ColliderShape = Egg::Asset::Collider;

enum class HorizontalAnchor : uint32_t {
	LEFT, CENTER, RIGHT
};

enum class VerticalAnchor : uint32_t {
	TOP, MIDDLE, BOTTOM
};

#define COMPONENT_ALIGN __declspec(align(16))

COMPONENT_ALIGN struct UIElement {
	VerticalAnchor verticalAlignment;
	HorizontalAnchor horizontalAlignment;
	DirectX::XMFLOAT4 margin;
	DirectX::XMFLOAT4 padding;
	float width;
	float height;
	float rotationZ;
	DirectX::XMFLOAT2 origin;
};

class UIObject;

COMPONENT_ALIGN struct SpriteAnimation {
	std::function<void(UIObject *, float)> onUpdate;
};

COMPONENT_ALIGN struct Sprite {
	Egg::ResourceViewsRef texture;
	DirectX::XMUINT2 textureSize;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT4 hoverColor;
};

COMPONENT_ALIGN struct Button {
public:
	physx::PxActor * pxActor;
	std::function<void()> onClick;
	std::function<void()> onMouseEnter;
	std::function<void()> onMouseLeave;
	bool isMouseOver;
	bool isSpawned;
};

COMPONENT_ALIGN struct TextBox {
	static int idGenerator;
	static int selectedId;

	static void ClearSelection() {
		selectedId = -1;
	}

	int id;
	int caretPosition;
	int selectionEndIndex;
	Egg::EventToken keyPressedToken;
	std::function<void(const std::wstring&)> contentChanged;
	std::wstring placeholder;
	DirectX::XMFLOAT4 placeholderColor;
	uint32_t maxCharacters;
	bool isPassword;

	TextBox();
	~TextBox();
};

COMPONENT_ALIGN struct Text {
public:
	DirectX::XMFLOAT4 color;
	Egg::SpriteFontRef font;
	std::wstring text;
	HorizontalAnchor horizontalAlignment;
	VerticalAnchor verticalAlignment;
};

class GameObject;

class IBehavior {
public:
	virtual ~IBehavior() = default;
	virtual void Setup(GameObject * gameObject) = 0;
	virtual void Update(float dt) = 0;
};

COMPONENT_ALIGN class Script {
protected:
	std::unique_ptr<IBehavior> behavior;
public:
	void SetBehavior(std::unique_ptr<IBehavior> behavior);
	void Setup(GameObject * owner);
	void Update(float dt);

	Script() = default;

	Script(Script &&) noexcept = default;
	Script & operator=(Script &&) noexcept = default;
};

COMPONENT_ALIGN class Transform {
public:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 worldPosition;
	DirectX::XMFLOAT4 worldRotation;

	Transform() : position{ 0.0f, 0.0f, 0.0f }, rotation{ 0.0f, 0.0f, 0.0f, 1.0f }, scale{ 1.0f, 1.0f, 1.0f }, worldPosition{ position }, worldRotation{ rotation } {

	}

	Transform(Transform &&) noexcept = default;
	Transform & operator=(const Transform &) = default;
	Transform & operator=(Transform &&) noexcept = default;
	~Transform() noexcept = default;
};

struct ShadedMesh {
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

	ShadedMesh(std::shared_ptr<Mesh> m, std::shared_ptr<Material> mat) : mesh{ std::move(m) }, material{ std::move(mat) } {

	}

	ShadedMesh(ShadedMesh &&) noexcept = default;
	~ShadedMesh() noexcept = default;
	ShadedMesh & operator=(ShadedMesh && sm) noexcept = default;
};

COMPONENT_ALIGN class Model {
public:

	PerObjectData perObjectData;
	std::unique_ptr<BoneData> boneData;
	std::vector<ShadedMesh> meshes;

	Model() : perObjectData{ }, boneData{ nullptr }, meshes{ } { }
	~Model() = default;

	Model(Model &&) noexcept = default;
	Model & operator=(Model && m) noexcept = default;

	ShadedMesh & AddShadedMesh(std::shared_ptr<Mesh> m, std::shared_ptr<Material> mat) {
		return meshes.emplace_back(std::move(m), std::move(mat));
	}
};


COMPONENT_ALIGN class Camera {
public:
	DirectX::XMFLOAT3 ahead;
	DirectX::XMFLOAT3 up;

	float fov;
	float aspect;
	float nearPlane;
	float farPlane;

	Camera() : ahead{}, up{ 0.0f, 1.0f, 0.0f }, fov{ 1.0f }, aspect{ 1.0f }, nearPlane{ 0.0f }, farPlane{ 1.0f } { }

	void SetProj(float fov, float aspect, float nearPlane, float farPlane) {
		this->fov = fov;
		this->aspect = aspect;
		this->nearPlane = nearPlane;
		this->farPlane = farPlane;
	}

	Camera & operator=(const Camera &) = default;
};

COMPONENT_ALIGN class Animation {
public:
	Egg::Animation::Blackboard blackboard;

	Animation() = default;
	Animation(Animation &&) noexcept = default;
	Animation & operator=(Animation &&) noexcept = default;
};

COMPONENT_ALIGN class Collider {
public:
	physx::PxActor * actorRef;
	std::vector<ColliderShape> shapes;

	Collider() = default;
	Collider(Collider &&) noexcept = default;

	Collider & operator=(Collider && c) noexcept = default;
};
