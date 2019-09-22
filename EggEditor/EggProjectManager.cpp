#include "pch.h"
#include "EggProjectManager.h"

Windows::Foundation::IAsyncOperation<bool> ^ EggEditor::EggProjectManager::OpenProject() {

	return concurrency::create_async([]() -> bool {
		return true;
	});
}

Windows::Foundation::IAsyncOperation<bool> ^ EggEditor::EggProjectManager::SaveProjectAs() {

	return concurrency::create_async([]() -> bool {
		return true;
	});
}

Windows::Foundation::IAsyncOperation<bool> ^ EggEditor::EggProjectManager::SaveProject() {

	return concurrency::create_async([]() -> bool {
		return true;
	});
}

Windows::Foundation::IAsyncOperation<bool> ^ EggEditor::EggProjectManager::CloseProject() {

	return concurrency::create_async([]() -> bool {
		return true;
	});
}

Windows::Foundation::IAsyncOperation<bool> ^ EggEditor::EggProjectManager::NewProject() {

	return concurrency::create_async([]() -> bool {
		return true;
	});
}
