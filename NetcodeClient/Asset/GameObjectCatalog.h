#pragma once

#include <Netcode/URI/Model.h>
#include <vector>

class GameObject;
class GameScene;

struct CatalogEntry {
	GameObject * gameObject;
	Netcode::URI::Model uri;
	int used;
};

class CatalogProxy {
	GameScene * scene;
	CatalogEntry * catalogEntry;
public:
	CatalogProxy() : scene{ nullptr }, catalogEntry{ nullptr } { }
	CatalogProxy(GameScene * scene, CatalogEntry * entry) : scene{ scene }, catalogEntry{ entry } { }

	[[nodiscard]] GameObject * Clone() const;
	bool operator!=(std::nullptr_t) const;
};

class GameObjectCatalog {
	GameScene * scene;
	std::vector<CatalogEntry> entries;
public:
	GameObjectCatalog() : scene{ nullptr }, entries{} { }

	[[nodiscard]] CatalogProxy Find(const Netcode::URI::Model & uri);

	void Reset();

	void Clear();

	void Insert(GameObject * obj, Netcode::URI::Model uri);

	void SetScene(GameScene * s);
};

