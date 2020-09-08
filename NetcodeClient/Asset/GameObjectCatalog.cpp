#include "GameObjectCatalog.h"
#include "../GameScene.h"

[[nodiscard]]
GameObject * CatalogProxy::Clone() const {
	auto * ptr = scene->Clone(catalogEntry->gameObject);
	catalogEntry->used = 1;
	return ptr;
}

bool  CatalogProxy::operator!=(std::nullptr_t) const {
	return catalogEntry != nullptr;
}

[[nodiscard]]
CatalogProxy GameObjectCatalog::Find(const Netcode::URI::Model & uri) {
	auto it = std::find_if(std::begin(entries), std::end(entries), [&uri](const CatalogEntry & e) -> bool {
		return e.uri.GetFullPath() == uri.GetFullPath();
	});

	if(it != std::end(entries)) {
		CatalogProxy{ scene, &(*it) };
	}

	return CatalogProxy{ };
}

void GameObjectCatalog::Reset() {
	for(auto & e : entries) {
		e.used = 0;
	}
}

void GameObjectCatalog::Clear() {
	entries.clear();
}

void GameObjectCatalog::Insert(GameObject * obj, Netcode::URI::Model uri) {
	CatalogEntry e;
	e.used = 1;
	e.gameObject = obj;
	e.uri = std::move(uri);
	entries.emplace_back(std::move(e));
}

void GameObjectCatalog::SetScene(GameScene * s) {
	scene = s;
}
