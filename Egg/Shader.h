#pragma once

#include <string>
#include <map>

namespace Egg::Graphics {

	enum class ShaderType : unsigned {
		VERTEX_SHADER = 0,
		PIXEL_SHADER = 1,
		GEOMETRY_SHADER = 2,
		HULL_SHADER = 3,
		DOMAIN_SHADER = 4,
		UNDEFINED_SHADER = 0xFFFFFFFF
	};

	class Shader {
	protected:
		std::wstring sourceFile;

	public:
		virtual ~Shader() = default;

		virtual uint8_t * GetBufferPointer() = 0;

		virtual size_t GetBufferSize() = 0;

		virtual void SetEntryFunction(const std::string & functionName) = 0;

		virtual void SetShaderSource(const std::string & sourceCode) = 0;

		virtual void SetDefinitions(const std::map<std::string, std::string> & defs) = 0;
		
		const std::wstring & GetSourceFile() const {
			return sourceFile;
		}
	};

}
