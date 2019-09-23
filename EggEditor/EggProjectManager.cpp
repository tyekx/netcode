#include "pch.h"
#include "EggProjectManager.h"
#include "ConcurrencyHelper.h"

concurrency::task<bool> EggEditor::EggProjectManager::OpenProject(Windows::UI::Core::CoreDispatcher ^ dispatcher) {
	auto openProjectFunction = [dispatcher]() -> bool {

		Windows::Storage::StorageFile ^ file = nullptr;

		auto openOnUithread = dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
							 ref new Windows::UI::Core::DispatchedHandler([&file]() -> void {
			auto filePicker = ref new Windows::Storage::Pickers::FileOpenPicker();
			filePicker->FileTypeFilter->Append(L".eggproj");

			auto asyncOpenFiles = filePicker->PickSingleFileAsync();

			auto openTask = concurrency::create_task(asyncOpenFiles);

			file = openTask.get();
		}));

		auto oTask = concurrency::create_task(openOnUithread);

		oTask.get();

		if(file != nullptr) {
			OutputDebugString(file->Path->Data());
		} else {
			OutputDebugString(L"No file were opened\r\n");
		}

		return true;
	};


	if(HasProject) {
		return ConcurrencyHelper::ChainConditionalConcurrentCalls<bool>(CloseProject(),
																		openProjectFunction,
																		true);
	}

	return concurrency::create_task(openProjectFunction);
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

	return concurrency::create_task([]() -> bool {
		return true;
	});
}

concurrency::task<bool> EggEditor::EggProjectManager::NewProject() {

	return concurrency::create_task([]() -> bool {
		return true;
	});
}
