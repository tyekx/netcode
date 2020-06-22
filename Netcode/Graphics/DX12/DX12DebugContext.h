
#include "../../Modules.h"
#include "../../Vertex.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	class DebugContext : public IDebugContext {
		Float3 defaultColor;
		Float4x4 viewProj;
		size_t bufferSize;
		size_t numNoDepthVertices;
		size_t numDepthVertices;
		std::vector<PC_Vertex> vertices;

		GpuResourceRef uploadBuffer;

		PipelineStateRef depthPso;
		PipelineStateRef noDepthPso;
		RootSignatureRef rootSignature;

		inline void PushVertex(const PC_Vertex & vertex, bool depthEnabled) {
			// assert: bufferSize > (numNoDepthVertices + numDepthVertices)
			if(depthEnabled) {
				vertices[numDepthVertices++] = vertex;
			} else {
				vertices[bufferSize - numNoDepthVertices++] = vertex;
			}
		}

		inline PC_Vertex * GetBufferForVertices(size_t numVertices, bool depthEnabled) {
			// assert: bufferSize >= (numNoDepthVertices + numDepthVertices + numVertices)
			PC_Vertex * tmp;
			if(depthEnabled) {
				tmp = vertices.data() + numDepthVertices;
				numDepthVertices += numVertices;
			} else {
				tmp = vertices.data() + (bufferSize - numNoDepthVertices - numVertices);
				numNoDepthVertices += numVertices;
			}
			return tmp;
		}

	public:
		void CreateResources(Module::IGraphicsModule * graphics);

		virtual void UploadResources(IResourceContext * context) override;

		virtual void Draw(IRenderContext * context, const Float4x4 & viewProjMatrix) override;

		virtual void DrawPoint(const Float3 & point, float extent) override;
		virtual void DrawPoint(const Float3 & point, float extent, bool depthEnabled) override;

		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd) override;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, bool depthEnabled) override;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color) override;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color, bool depthEnabled) override;

		virtual void DrawSphere(Vector3 worldPosOrigin, float radius) override;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, bool depthEnabled) override;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color) override;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, bool depthEnabled) override;

		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents) override;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled) override;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color) override;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled) override;

		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents) override;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled) override;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color) override;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled) override;
	
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight) override;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, bool depthEnabled) override;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color) override;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, bool depthEnabled) override;
	};

	using DX12DebugContext = Netcode::Graphics::DX12::DebugContext;
	using DX12DebugContextRef = std::shared_ptr<DX12DebugContext>;

}
