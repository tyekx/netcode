#pragma once

#include "HandleTypes.h"
#include "DX12Common.h"
#include "DX12RootSignature.h"

namespace Egg::Graphics::DX12 {

	struct CPipelineStateDesc {
		RootSignatureRef rootSignature;
		ShaderBytecodeRef CS;

		void FillNativeDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC & psd) {
			psd.CachedPSO.CachedBlobSizeInBytes = 0;
			psd.CachedPSO.pCachedBlob = nullptr;
			psd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			psd.NodeMask = 0;
			psd.pRootSignature = static_cast<DX12RootSignature *>(rootSignature.get())->GetNativeRootSignature();
		}

		bool operator==(const CPipelineStateDesc & rhs) const {
			return rootSignature == rhs.rootSignature &&
				CS == rhs.CS;
		}
	};

}
