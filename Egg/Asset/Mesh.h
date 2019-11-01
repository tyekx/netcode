#pragma once

namespace Egg::Asset {

	struct Mesh {
		unsigned int materialId;
		unsigned int vertexType;
		unsigned int vertexSize;

		unsigned int verticesLength;
		void * vertices;

		unsigned int indicesLength;
		unsigned int * indices;
	};

}
