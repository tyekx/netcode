#include "pch.h"
#include "Adapters.h"

void EggEditor::CxCppGameObjectObserverAdapter::GameObjectNameChanged(Egg::GameObject * obj)
{
	CxAdapterInstance->InvokeGameObjectNameChanged(reinterpret_cast<UINT_PTR>(obj));
}

void EggEditor::CxCppGameObjectObserverAdapter::GameObjectSignatureChanged(Egg::GameObject * obj)
{
	CxAdapterInstance->InvokeGameObjectSignatureChanged(reinterpret_cast<UINT_PTR>(obj));
}

void EggEditor::CxCppGameObjectObserverAdapter::GameObjectOwnerChanged(Egg::GameObject * obj)
{
	CxAdapterInstance->InvokeGameObjectOwnerChanged(reinterpret_cast<UINT_PTR>(obj));
}

EggEditor::CxCppGameObjectObserverAdapterCx ^ EggEditor::CxCppGameObjectObserverAdapter::GetCx() {
	return CxAdapterInstance;
}

void EggEditor::CxCppGameObjectObserverAdapter::GameObjectDestroyed(Egg::GameObject * obj)
{
	CxAdapterInstance->InvokeGameObjectDestroyed(reinterpret_cast<UINT_PTR>(obj));
}

void EggEditor::CxCppGameObjectObserverAdapterCx::InvokeGameObjectSignatureChanged(UINT_PTR obj) {
	GameObjectSignatureChanged(obj);
}

void EggEditor::CxCppGameObjectObserverAdapterCx::InvokeGameObjectDestroyed(UINT_PTR obj) {
	GameObjectDestroyed(obj);
}

void EggEditor::CxCppGameObjectObserverAdapterCx::InvokeGameObjectOwnerChanged(UINT_PTR obj) {
	GameObjectOwnerChanged(obj);
}

void EggEditor::CxCppGameObjectObserverAdapterCx::InvokeGameObjectNameChanged(UINT_PTR obj) {
	GameObjectNameChanged(obj);
}

void EggEditor::CxCppSceneObserverAdapterCx::InvokeSceneGameObjectAdded(UINT_PTR obj) {
	SceneGameObjectAdded(obj);
}

void EggEditor::CxCppSceneObserverAdapter::SceneGameObjectAdded(Egg::GameObject * obj) {
	CxAdapterInstance->InvokeSceneGameObjectAdded(reinterpret_cast<UINT_PTR>(obj));
}

EggEditor::CxCppSceneObserverAdapterCx ^ EggEditor::CxCppSceneObserverAdapter::GetCx() {
	return CxAdapterInstance;
}
