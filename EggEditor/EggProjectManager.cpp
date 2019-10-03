#include "pch.h"
#include "EggProjectManager.h"
#include <winrt/Windows.UI.Core.h>

concurrency::task<bool> EggEditor::EggProjectManager::OpenProject() {
	if(HasProject) {
		auto closeTask = CloseProject();

		bool closeTaskResult = closeTask.get();

		if(!closeTaskResult) {
			return concurrency::create_task([]() -> bool {
				return false;
			});
		}
	}

	auto filePicker = ref new Windows::Storage::Pickers::FileOpenPicker();

	filePicker->FileTypeFilter->Append(L".eggproj");

	auto asyncOpenFiles = filePicker->PickSingleFileAsync();

	concurrency::task<Windows::Storage::StorageFile ^> openTask = concurrency::create_task(asyncOpenFiles);

	return concurrency::create_task([openTask]() -> bool {
		Windows::Storage::StorageFile ^ file = openTask.get();
		
		if(file != nullptr) {
			return true;
		} else {
			return false;
		}
	});
}

concurrency::task<bool> EggEditor::EggProjectManager::SaveProjectAs() {
	return concurrency::create_task([]() -> bool {
		return true;
	});
}

concurrency::task<bool> EggEditor::EggProjectManager::SaveProject() {
	return concurrency::create_task([]() -> bool {
		
		return true;
	});
}

concurrency::task<bool> EggEditor::EggProjectManager::CloseProject() {
	if(!HasProject) {
		return concurrency::create_task([]() -> bool {
			return true;
		});
	}

	if(Project->Root->IsContentDirty) {
		auto saveTask = SaveProject();

		bool saveTaskResult = saveTask.get();

		if(!saveTaskResult) {
			return concurrency::create_task([]() -> bool {
				return false;
			});
		}
	}

	OnClosed(Project);

	projectFile = nullptr;

	return concurrency::create_task([]()-> bool {
		return true;
	});
}

concurrency::task<bool> EggEditor::EggProjectManager::NewProject() {
	if(HasProject) {
		auto closeTask = CloseProject();

		bool closeTaskResult = closeTask.get();

		if(!closeTaskResult) {
			return concurrency::create_task([]() -> bool {
				return false;
			});
		}
	}

	auto fileSavePicker = ref new Windows::Storage::Pickers::FileSavePicker();
	auto fileTypePostfixes = ref new Platform::Collections::Vector<Platform::String ^>();
	fileTypePostfixes->Append(L".eggproj");
	fileSavePicker->FileTypeChoices->Insert(L"Egg Project", fileTypePostfixes);
	fileSavePicker->SuggestedFileName = L"newproj";

	auto saveAsync = fileSavePicker->PickSaveFileAsync();
	
	auto saveTask = concurrency::create_task(saveAsync);

	return saveTask.then([](Windows::Storage::StorageFile^ file) -> bool {
		return file != nullptr;
	});
}
