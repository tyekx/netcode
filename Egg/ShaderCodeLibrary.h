#pragma once

#include "Common.h"
#include "Utility.h"
#include "PreprocessorDefinitions.h"
#include "ShaderCodeCollection.h"
#include "Path.h"
#include <fstream>

namespace Egg::Graphics::Internal {

	class ShaderCodeLibrary {
		struct StorageItem {
			PreprocessorDefinitions definitions;
			com_ptr<ID3DBlob> VS;
			com_ptr<ID3DBlob> PS;
		};

		struct FileBasedItem {
			ShaderPath vsPath;
			ShaderPath psPath;
			com_ptr<ID3DBlob> VS;
			com_ptr<ID3DBlob> PS;

			FileBasedItem(const ShaderPath & vsP, const ShaderPath & psP, com_ptr<ID3DBlob> vs, com_ptr<ID3DBlob> ps) :
				vsPath{ vsP }, psPath{ psP }, VS{ vs }, PS{ ps } {

			}

		};

		ID3D12Device * device;

		std::vector<StorageItem> items;
		std::vector<FileBasedItem> filebasedStorage;

		bool Exists(const PreprocessorDefinitions & pd) {
			for(const auto & i : items) {
				if(i.definitions == pd) {
					return true;
				}
			}
			return false;
		}

		bool Exist(const ShaderPath & vs, const ShaderPath & ps) {
			for(const auto & i : filebasedStorage) {
				if(i.vsPath == vs && i.psPath == ps) {
					return true;
				}
			}
			return  false;
		}

		com_ptr<ID3DBlob> LoadCSO(const ShaderPath & shaderPath) {
			std::wstring filename = shaderPath.GetAbsolutePath();

			std::ifstream file{ filename.c_str(), std::ios::binary | std::ios::in | std::ios::ate };

			ASSERT(file.is_open(), "Failed to open blob file: %S", filename.c_str());

			std::streamsize size = file.tellg();

			file.seekg(0, std::ios::beg);

			com_ptr<ID3DBlob> shaderByteCode{ nullptr };

			DX_API("Failed to allocate memory for blob")
				D3DCreateBlob((size_t)size, shaderByteCode.GetAddressOf());

			if(file.read(reinterpret_cast<char *>(shaderByteCode->GetBufferPointer()), size)) {
				return shaderByteCode;
			} else {
				ASSERT(false, "Failed to load CSO file: %S", filename.c_str());
				return nullptr;
			}
		}

		void LoadWith(const ShaderPath & vs, const ShaderPath & ps) {

			com_ptr<ID3DBlob> vertexShader = LoadCSO(vs);
			com_ptr<ID3DBlob> pixelShader = LoadCSO(ps);

			FileBasedItem fbi{ vs, ps, vertexShader, pixelShader };
			filebasedStorage.push_back(fbi);
		}

		ShaderCodeCollection GetCollection(const ShaderPath & vs, const ShaderPath & ps) {
			ShaderCodeCollection coll;
			for(const auto & i : filebasedStorage) {
				if(i.vsPath == vs && i.psPath == ps) {
					coll.vertexShader = i.VS.Get();
					coll.pixelShader = i.PS.Get();
					return coll;
				}
			}
			return coll;
		}

		void CompileWith(const PreprocessorDefinitions & pd) {
			com_ptr<ID3DBlob> vsByteCode;
			com_ptr<ID3DBlob> psByteCode;

			std::unique_ptr<D3D_SHADER_MACRO[]> preprocDefinitions{ nullptr };

			if(pd.defs.size() > 0) {
				preprocDefinitions = std::make_unique<D3D_SHADER_MACRO[]>(pd.defs.size() + 1);

				int preprocIt = 0;
				for(const auto & kv : pd.defs) {
					preprocDefinitions[preprocIt].Name = kv.first.c_str();
					preprocDefinitions[preprocIt].Definition = (kv.second.size() > 0) ? kv.second.c_str() : nullptr;
					++preprocIt;
				}
				// "NULL" termination, API requires this, dont delete
				preprocDefinitions[preprocIt].Name = nullptr;
				preprocDefinitions[preprocIt].Definition = nullptr;
			}

			const char * vertexShaderEntry = "Vertex_Main";
			const char * pixelShaderEntry = "Pixel_Main";

			std::string file;
			ShaderPath path{ L"EggShaderLib.hlsli" };
			Egg::Utility::SlurpFile(file, L"C:/work/directx12/Egg/EggShaderLib.hlsli");
			com_ptr<ID3DBlob> errorMsg;

			/*
			com_ptr<ID3DBlob> preprocessed;
			DX_API("Failed to preprocess shader")
			D3DPreprocess(file.c_str(), file.size(), nullptr, preprocDefinitions.get(), nullptr, preprocessed.GetAddressOf(), errorMsg.GetAddressOf());

			if(errorMsg != nullptr) {
				Egg::Utility::Debugf("Error while preprocessing shader (-Wall): \r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
				return;
			}

			Egg::Utility::Debugf("Successful preproc:\r\n");
			Egg::Utility::DebugPrintBlob(preprocessed); */


			D3DCompile(file.c_str(), file.size(), nullptr, preprocDefinitions.get(), nullptr, vertexShaderEntry, "vs_5_0", 0, 0, vsByteCode.GetAddressOf(), errorMsg.GetAddressOf());

			if(errorMsg != nullptr) {
				Egg::Utility::Debugf("Error while compiling vertex shader (-Wall): \r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
				return;
			}

			D3DCompile(file.c_str(), file.size(), nullptr, preprocDefinitions.get(), nullptr, pixelShaderEntry, "ps_5_0", 0, 0, psByteCode.GetAddressOf(), errorMsg.GetAddressOf());

			if(errorMsg != nullptr) {
				Egg::Utility::Debugf("Error while compiling pixel shader (-Wall): \r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
				return;
			}

			StorageItem item;
			item.VS = std::move(vsByteCode);
			item.PS = std::move(psByteCode);
			item.definitions = pd;

			items.push_back(item);
		}

		ShaderCodeCollection GetCollection(const PreprocessorDefinitions & pd) {
			ShaderCodeCollection coll;
			for(auto & i : items) {
				if(i.definitions == pd) {
					coll.vertexShader = i.VS.Get();
					coll.pixelShader = i.PS.Get();
					break;
				}
			}
			return coll;
		}

	public:

		void CreateResources(ID3D12Device * dev) {
			device = dev;
		}

		ShaderCodeLibrary() :device{ nullptr }, items{}, filebasedStorage{} { }

		ShaderCodeCollection GetShaderCodeCollection(const ShaderPath & vs, const ShaderPath & ps) {
			if(!Exist(vs, ps)) {
				LoadWith(vs, ps);
			}
			return GetCollection(vs, ps);
		}

		ShaderCodeCollection GetShaderCodeCollection(const PreprocessorDefinitions & pd) {
			if(!Exists(pd)) {
				CompileWith(pd);
			}

			return GetCollection(pd);
		}
	};

}


