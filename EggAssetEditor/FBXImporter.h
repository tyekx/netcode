#pragma once

#include "Model.h"

class FBXImporter {
	~FBXImporter() = delete;
	FBXImporter() = delete;

public:
	static Model FromFile(const std::string & file);
	static Model FromMemory(const uint8_t * source, uint32_t sizeInBytes);
	static std::vector<Animation> ImportAnimationsFromMemory(const uint8_t * source, uint32_t sizeInBytes, const Skeleton & skeleton);
	static std::vector<Animation> ImportAnimationsFromFile(const std::string & file, const Skeleton & skeleton);
	static OptimizedAnimation OptimizeAnimation(const Animation & anim, const Skeleton & skeleton);
};
