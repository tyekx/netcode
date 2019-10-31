#pragma once

#include "RootSignatureDesc.h"

namespace Egg::Graphics::Internal {


	class RootSignatureLibrary {

		bool Exist(const RootSignatureDesc & rd) {

		}

		void Compile(const RootSignatureDesc & rd) {

			/*
				com_ptr<ID3DBlob> rootSigBlob;
				com_ptr<ID3DBlob> errorMsg;

				HRESULT rsResult = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, rootSigBlob.GetAddressOf(), errorMsg.GetAddressOf());

				if(FAILED(rsResult)) {
					Egg::Utility::Debugf("Failed to serialize root signature\r\n");
					Egg::Utility::DebugPrintBlob(errorMsg);
				}
				*/
		}

		ID3D12RootSignature * Find(const RootSignatureDesc & rd) {

		}

	public:
		ID3D12RootSignature * GetRootSignature(const RootSignatureDesc & rd) {
			if(!Exist(rd)) {
				Compile(rd);
			}
			return Find(rd);
		}
	};

}
