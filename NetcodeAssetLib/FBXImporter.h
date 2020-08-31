#pragma once

#include "IntermediateModel.h"

namespace Netcode {

	class FBXImporter {
		~FBXImporter() = delete;
		FBXImporter() = delete;

	public:
		static Intermediate::Model FromMemory(const uint8_t * source, uint32_t sizeInBytes, const char * pHint = ".fbx");
		static std::vector<Intermediate::Animation> ImportAnimationsFromMemory(const uint8_t * source, uint32_t sizeInBytes, const Intermediate::Skeleton & skeleton);
		static std::vector<Intermediate::Animation> ImportAnimationsFromFile(const std::string & file, const Intermediate::Skeleton & skeleton);
		static Intermediate::OptimizedAnimation OptimizeAnimation(const Intermediate::Animation & anim, const Intermediate::Skeleton & skeleton);
	};

}
