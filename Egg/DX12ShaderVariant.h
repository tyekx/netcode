#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <map>

namespace Egg::Graphics::DX12 {

	enum class EShaderType : unsigned {
		VERTEX = 0,
		PIXEL = 1,
		UNDEFINED = 0xFFFFFFFF
	};

	struct ShaderPreprocDefs {

		std::map<std::string, std::string> defs;

		bool operator==(const ShaderPreprocDefs & o) const {
			if(defs.size() != o.defs.size()) {
				return false;
			}

			for(const auto & i : defs) {
				const auto & oi = o.defs.find(i.first);

				if(oi == o.defs.end()) {
					return false;
				}

				if(oi->second != i.second) {
					return false;
				}
			}
			return true;
		}

		inline bool operator!=(const ShaderPreprocDefs & o) const {
			return !operator==(o);
		}

		void Define(const std::string & key, float value) {
			Define(key, std::to_string(value));
		}

		void Define(const std::string & key, int value) {
			Define(key, std::to_string(value));
		}

		void Define(const std::string & key, const std::string & value) {
			defs[key] = value;
		}

		void Define(const std::string & key) {
			defs[key];
		}
	};

	class ShaderVariant {
		ShaderPreprocDefs defs;
		std::wstring sourceFile;
		std::string source;
		std::string entryFunctionName;
		EShaderType shaderType;

	public:
		ShaderVariant() = default;

		void SetShaderType(EShaderType type) {
			shaderType = type;
		}

		void SetSourceReference(const std::wstring & shader) {
			sourceFile = shader;
		}

		void SetShaderSource(const std::string & shaderSource) {
			source = shaderSource;
		}

		void SetMacros(const ShaderPreprocDefs & preprocDefs) {
			defs = preprocDefs;
		}

		void SetMacros(const std::map<std::string, std::string> & macros) {
			defs = ShaderPreprocDefs{ macros };
		}

		void SetEntryFunction(const std::string & fname) {
			entryFunctionName = fname;
		}
		
		EShaderType GetShaderType() const {
			return shaderType;
		}

		const std::wstring & GetFileReference() const {
			return sourceFile;
		}

		const std::string & GetEntryFunction() const {
			return entryFunctionName;
		}

		const std::string & GetSource() const {
			return source;
		}

		const ShaderPreprocDefs & GetPreprocDefs() const {
			return defs;
		}

	};

}