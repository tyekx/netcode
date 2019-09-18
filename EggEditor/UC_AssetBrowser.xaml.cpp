//
// UC_AssetBrowser.xaml.cpp
// Implementation of the UC_AssetBrowser class
//

#include "pch.h"
#include "UC_AssetBrowser.xaml.h"
#include "UC_Asset.xaml.h"
#include "UC_BreadCrumb.xaml.h"

using namespace EggEditor;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

UC_AssetBrowser::UC_AssetBrowser()
{
	InitializeComponent();
	rootFolder = ref new UC_ProjectFolder(L"Root");
	rootFolder->ContentChanged += ref new EggEditor::ContentChangedCallback(this, &EggEditor::UC_AssetBrowser::ProjectFolder_OnContentChanged);
	ChangeProjectFolder(rootFolder);
	eventInstance = ref new OpenAssetCallback(this, &UC_AssetBrowser::OpenAssetHandler);
}

void EggEditor::UC_AssetBrowser::OpenAssetHandler(Platform::Object ^ obj) {
	OpenAsset(obj);
}

void EggEditor::UC_AssetBrowser::CtxBtnNewProjectFolder_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {

	auto newFolder = ref new UC_ProjectFolder(L"New folder");
	newFolder->DoubleTapped += ref new
		Windows::UI::Xaml::Input::DoubleTappedEventHandler(this, &EggEditor::UC_AssetBrowser::ProjectFolder_DoubleTap);
	newFolder->ParentFolder = currentFolder;
}

void EggEditor::UC_AssetBrowser::CtxBtnNewMaterial_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	auto newMaterial = ref new UC_Asset(L"New Mat", ASSET_TYPE_MATERIAL);
	newMaterial->OpenAsset += static_cast<OpenAssetCallback ^>(eventInstance);
	newMaterial->ParentFolder = currentFolder;
}

void EggEditor::UC_AssetBrowser::CtxBtnNewShadedMesh_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	auto newShadedMesh = ref new UC_Asset(L"New SM", ASSET_TYPE_SHADED_MESH);
	newShadedMesh->OpenAsset += static_cast<OpenAssetCallback ^>(eventInstance);
	newShadedMesh->ParentFolder = currentFolder;
}

void EggEditor::UC_AssetBrowser::BreadCrumb_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs ^ e) {
	ChangeProjectFolder((UC_ProjectFolder ^)(((UC_BreadCrumb ^)sender)->BreadCrumbRef));
}


void EggEditor::UC_AssetBrowser::ImportAsset(Windows::Storage::StorageFile ^ file) {

	unsigned int assetType = UC_Asset::GetAssetTypeFromExtension(file->FileType);

	if(assetType != UINT_MAX) {
		auto newTexture = ref new UC_Asset(file->Name, assetType);
		newTexture->ParentFolder = currentFolder;
	}
}

void EggEditor::UC_AssetBrowser::ImplDetail_RecursiveAddBreadCrumb(UC_ProjectFolder ^ folder) {
	if(folder == nullptr) {
		return;
	}
	// first we go to the beginning
	ImplDetail_RecursiveAddBreadCrumb(folder->ParentFolder);
	// then add each folder to the breadcrumb bar
	auto bc = ref new UC_BreadCrumb(folder);
	bc->Tapped += ref new Windows::UI::Xaml::Input::TappedEventHandler(this, &EggEditor::UC_AssetBrowser::BreadCrumb_OnClick);
	bc->TypedDataContext->BreadCrumbText = folder->TypedDataContext->FolderName;
	breadCrumbPanel->Children->Append(bc);
}

void EggEditor::UC_AssetBrowser::ChangeProjectFolder(UC_ProjectFolder ^ folder) {
	currentFolder = folder;
	assetBrowserPanel->ItemsSource = currentFolder->Children;
	breadCrumbPanel->Children->Clear();
	ImplDetail_RecursiveAddBreadCrumb(currentFolder);
}

void EggEditor::UC_AssetBrowser::ProjectFolder_DoubleTap(Platform::Object ^ sender, Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs ^ e) {
	ChangeProjectFolder((UC_ProjectFolder ^)sender);
}

void EggEditor::UC_AssetBrowser::ProjectFolder_OnContentChanged(Platform::Object ^ sender) {
	assetBrowserPanel->UpdateLayout();
}