#include "DX12FrameGraph.h"
#include <NetcodeFoundation/ArrayView.hpp>
#include "DX12RenderPass.h"
#include <vector>

namespace Netcode::Graphics::DX12 {

	FrameGraphImpl::FrameGraphImpl(Vector<Ref<RenderPassImpl>> setupRenderPasses) : resources{ }, renderPasses{ } {

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

	Vector<Ref<RenderPass>> FrameGraphImpl::QueryDanglingRenderPasses() {
		Vector<Ref<RenderPass>> cullable;

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

	void FrameGraphImpl::EraseRenderPasses(Vector<Ref<RenderPass>> rps) {
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

	Vector<Ref<RenderPass>> FrameGraphImpl::QueryCompleteRenderPasses() {
		Vector<Ref<RenderPass>> runnable;

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

	bool FrameGraphImpl::UsingBackbuffer() const {
		decltype(resources)::const_iterator it = resources.find(0);

		if(it == std::end(resources)) {
			return false;
		}

		return it->second.numWrites > 0;
	}
}

