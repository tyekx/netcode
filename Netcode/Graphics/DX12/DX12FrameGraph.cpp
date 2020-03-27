#include "DX12FrameGraph.h"

namespace Netcode::Graphics::DX12 {

	FrameGraph::FrameGraph(std::vector<DX12RenderPassRef> setupRenderPasses) : resources{ }, renderPasses{ } {

		resources[0].numReads = 1 << 30;

		for(auto & rp : setupRenderPasses) {
			auto reads = rp->GetReadResources();
			auto writes = rp->GetWrittenResources();

			for(size_t i = 0; i < reads.Size(); ++i) {
				resources[reads[i]].numReads += 1;
			}

			for(size_t i = 0; i < writes.Size(); ++i) {
				resources[writes[i]].numWrites += 1;
			}
		}

		renderPasses.assign(setupRenderPasses.begin(), setupRenderPasses.end());
	}

	std::vector<RenderPassRef> FrameGraph::QueryDanglingRenderPasses() {
		std::vector<RenderPassRef> cullable;

		for(auto & rp : renderPasses) {
			auto writes = rp->GetWrittenResources();

			bool isCullable = true;

			for(size_t i = 0; i < writes.Size(); ++i) {
				if(resources[writes[i]].numReads > 0) {
					isCullable = false;
					break;
				}
			}

			if(isCullable) {
				cullable.push_back(rp);
			}
		}


		return cullable;
	}

	void FrameGraph::EraseRenderPasses(std::vector<RenderPassRef> rps) {
		for(auto & rp : rps) {
			for(auto it = std::begin(renderPasses); it != std::end(renderPasses); ++it) {

				if(rp == *it) {
					auto reads = (*it)->GetReadResources();
					auto writes = (*it)->GetWrittenResources();

					for(size_t i = 0; i < reads.Size(); ++i) {
						resources[reads[i]].numReads -= 1;
					}

					for(size_t i = 0; i < writes.Size(); ++i) {
						resources[writes[i]].numWrites -= 1;
					}

					renderPasses.erase(it);

					break;
				}

			}
		}
	}

	std::vector<RenderPassRef> FrameGraph::QueryCompleteRenderPasses() {
		std::vector<RenderPassRef> runnable;

		for(auto & rp : renderPasses) {
			auto reads = rp->GetReadResources();

			bool isRunnable = true;

			for(size_t i = 0; i < reads.Size(); ++i) {
				if(resources[reads[i]].numWrites != 0) {
					isRunnable = false;
					break;
				}
			}

			if(isRunnable) {
				runnable.push_back(rp);
			}
		}

		return runnable;
	}
}

