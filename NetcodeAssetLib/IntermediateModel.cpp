#include "Model.h"
#include "BoundingBoxHelpers.h"
#include <type_traits>
#include "IntermediateModel.h"

template<typename FloatType>
FloatType & GetElement(typename std::conditional<std::is_const<FloatType>::value, const uint8_t *, uint8_t *>::type ptr,
	const Netcode::Intermediate::InputElement & inputElement) {
	return *reinterpret_cast<FloatType *>(ptr + inputElement.byteOffset);
}
namespace Netcode::Intermediate {

	DirectX::BoundingBox Mesh::CalculateBoundingBox(const Mesh & mesh)
	{
		if(mesh.lods.empty()) {
			return DirectX::BoundingBox{};
		}

		const Intermediate::LOD & lod = mesh.lods.front();

		const uint8_t * ptr = lod.vertexData.get();

		auto it = std::find_if(std::cbegin(mesh.inputLayout), std::cend(mesh.inputLayout), [](const InputElement & b) ->bool {
			return b.semanticName == "POSITION";
		});

		if(it == std::cend(mesh.inputLayout)) {
			return DirectX::BoundingBox{};
		}

		BoundingBoxGenerator bg;

		const InputElement & positionElement = *it;

		for(uint32_t i = 0; i < lod.vertexCount; ++i) {
			const Netcode::Float3 & p = GetElement<const Netcode::Float3>(ptr, positionElement);
			bg.UpdateForPoint(p);
			ptr += mesh.vertexStride;
		}

		return bg.GetBoundingBox();
	}

	Mesh Mesh::Clone() const {
		Mesh m;
		m.bones = bones;
		m.boundingBox = boundingBox;
		m.inputLayout = inputLayout;
		m.materialIdx = materialIdx;
		m.name = name;
		m.vertexStride = vertexStride;
		m.lods.reserve(lods.size());

		for(const LOD & lod : lods) {
			LOD l;
			l.indexCount = lod.indexCount;
			l.indexDataSizeInBytes = lod.indexDataSizeInBytes;
			l.vertexCount = lod.vertexCount;
			l.vertexDataSizeInBytes = lod.vertexDataSizeInBytes;

			l.indexData = std::make_unique<uint8_t[]>(l.indexDataSizeInBytes);
			l.vertexData = std::make_unique<uint8_t[]>(l.vertexDataSizeInBytes);

			memcpy(l.indexData.get(), lod.indexData.get(), l.indexDataSizeInBytes);
			memcpy(l.vertexData.get(), lod.vertexData.get(), l.vertexDataSizeInBytes);

			m.lods.emplace_back(std::move(l));
		}

		return m;
	}

	void Mesh::ApplyTransformation(const Netcode::Float4x4 & mat)
	{
		Netcode::Matrix transform = mat;

		std::vector<InputElement> transformableElements;
		transformableElements.reserve(4);

		Intermediate::InputElement positionElement;

		for(const Intermediate::InputElement & e : inputLayout) {
			if(e.semanticName == "POSITION" ||
				e.semanticName == "NORMAL" ||
				e.semanticName == "BINORMAL" ||
				e.semanticName == "TANGENT") {
				transformableElements.emplace_back(e);
			}

			if(e.semanticName == "POSITION") {
				positionElement = e;
			}
		}

		BoundingBoxGenerator bg;

		for(LOD & lod : lods) {
			uint8_t * ptr = lod.vertexData.get();
			for(uint32_t i = 0; i < lod.vertexCount; ++i) {
				for(const Intermediate::InputElement & e : transformableElements) {
					Netcode::Float3 & value = GetElement<Netcode::Float3>(ptr, e);

					Netcode::Vector3 v = value;

					value = v.Transform(transform);
				}

				const Netcode::Float3 & p = GetElement<const Netcode::Float3>(ptr, positionElement);

				bg.UpdateForPoint(p);

				ptr += vertexStride;
			}
		}

		boundingBox = bg.GetBoundingBox();
	}

}
