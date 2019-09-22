//
// UC_AssetBrowser.xaml.cpp
// Implementation of the UC_AssetBrowser class
//

#include "pch.h"
#include "UC_AssetBrowser.xaml.h"
#include "UC_Asset.xaml.h"
#include "UC_BreadCrumb.xaml.h"
#include "Static.h"
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
	rootFolder = ref new ProjectFolderDataContext();
	rootFolder->Name = L"/";
	ChangeProjectFolder(rootFolder);
}

void EggEditor::UC_AssetBrowser::CtxBtnNewProjectFolder_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	//Static::Project->CloseProject();
	auto newFolder = ref new ProjectFolderDataContext();
	newFolder->Name = L"New Folder";
	newFolder->ParentFolder = currentFolder;
}

void EggEditor::UC_AssetBrowser::CtxBtnNewMaterial_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	auto newMaterial = ref new AssetDataContext();
	newMaterial->AssetType = ASSET_TYPE_MATERIAL;
	newMaterial->Name = L"New material";
	newMaterial->ParentFolder = currentFolder;
}

void EggEditor::UC_AssetBrowser::CtxBtnNewShadedMesh_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	auto newShadedMesh = ref new AssetDataContext();
	newShadedMesh->Name = L"New SM";
	newShadedMesh->AssetType = ASSET_TYPE_SHADED_MESH;
	newShadedMesh->ParentFolder = currentFolder;
}

void EggEditor::UC_AssetBrowser::BreadCrumb_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs ^ e) {
	ChangeProjectFolder((ProjectFolderDataContext ^)(((UC_BreadCrumb ^)sender)->BreadCrumbRef));
}


void EggEditor::UC_AssetBrowser::ImportAsset(Windows::Storage::StorageFile ^ file) {
	unsigned int assetType = UC_Asset::GetAssetTypeFromExtension(file->FileType);

	if(assetType != UINT_MAX) {
		auto newTexture = ref new AssetDataContext();
		newTexture->Name = file->Name;
		newTexture->AssetType = assetType;
		newTexture->ParentFolder = currentFolder;
	}
}

void EggEditor::UC_AssetBrowser::ImplDetail_RecursiveAddBreadCrumb(ProjectFolderDataContext ^ folder) {
	// first we go to the beginning
	if(folder == nullptr) {
		return;
	}
	
	ImplDetail_RecursiveAddBreadCrumb((ProjectFolderDataContext^)folder->ParentFolder);
	// then add each folder to the breadcrumb bar
	auto bc = ref new UC_BreadCrumb(folder);
	bc->Tapped += ref new Windows::UI::Xaml::Input::TappedEventHandler(this, &EggEditor::UC_AssetBrowser::BreadCrumb_OnClick);
	bc->TypedDataContext->BreadCrumbText = folder->Name;
	breadCrumbPanel->Children->Append(bc);
}

void EggEditor::UC_AssetBrowser::ChangeProjectFolder(ProjectFolderDataContext ^ folder) {
	currentFolder = folder;
	assetBrowserPanel->ItemsSource = folder->Children;

	/*
	* @TODO: Add breadcrumb to the AssetBrowserDataContext
	*/
	breadCrumbPanel->Children->Clear();
	ImplDetail_RecursiveAddBreadCrumb(currentFolder);
}

void EggEditor::UC_AssetBrowser::ProjectFolder_DoubleTap(Platform::Object ^ sender, Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs ^ e) {
	ChangeProjectFolder(((UC_ProjectFolder ^)sender)->TypedDataContext);
}
