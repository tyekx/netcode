#pragma once

#include "RootSignatureDesc.h"

namespace Egg::Graphics::Internal {


	class RootSignatureLibrary {

		ID3D12Device * device;

		struct CbufferAssoc {
			int assoc[16];

			bool operator==(const CbufferAssoc & cba) const {
				return memcmp(assoc, cba.assoc, sizeof(assoc));
			}
		};

		struct StorageItem {
			com_ptr<ID3D12RootSignature> rootSig;
			RootSignatureDesc rootSigDesc;
		};

		std::vector<CbufferAssoc> associations;
		std::vector<StorageItem> items;

		bool Exist(const RootSignatureDesc & rd) {
			for(const auto & i : items) {
				if(i.rootSigDesc == rd) {
					return true;
				}
			}
			return false;
		}

		void Compile(const RootSignatureDesc & rd) {
			D3D12_ROOT_SIGNATURE_DESC rootDesc = rd.GetDesc();

			com_ptr<ID3DBlob> rootSigBlob;
			com_ptr<ID3DBlob> errorMsg;

			HRESULT rsResult = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, rootSigBlob.GetAddressOf(), errorMsg.GetAddressOf());

			if(FAILED(rsResult)) {
				Egg::Utility::Debugf("Failed to serialize root signature\r\n");
				Egg::Utility::DebugPrintBlob(errorMsg);
			}

			com_ptr<ID3D12RootSignature> rootSignature;

			DX_API("Failed to create root signature")
				device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf()));

			StorageItem si;
			si.rootSig = std::move(rootSignature);
			si.rootSigDesc = rd;
			items.push_back(si);
		}

		ID3D12RootSignature * GetRootSignatureReference(const RootSignatureDesc & rd) {
			ID3D12RootSignature * rs = nullptr;
			for(auto & i : items) {
				if(i.rootSigDesc == rd) {
					rs = i.rootSig.Get();
					break;
				}
			}
			return rs;
		}

	public:

		void CreateResources(ID3D12Device * dev) {
			device = dev;
		}

		ID3D12RootSignature * GetRootSignature(const RootSignatureDesc & rd) {
			if(!Exist(rd)) {
				Compile(rd);
			}
			return GetRootSignatureReference(rd);
		}
	};

}
