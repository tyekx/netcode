#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <map>

#include "Shader.h"
#include "DX12Common.h"

namespace Egg::Graphics::DX12 {

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

	class ShaderVariant : public Egg::Graphics::Shader {
		ShaderPreprocDefs defs;
		std::string sourceCode;
		std::string entryFunctionName;
		ShaderType shaderType;
		com_ptr<ID3DBlob> shaderByteCode;

	public:
		ShaderVariant() = default;

		void SetShaderType(ShaderType type);

		void SetSourceReference(const std::wstring & shader);

		void SetShaderSource(const std::string & shaderSource);

		void SetMacros(const ShaderPreprocDefs & preprocDefs);

		void SetMacros(const std::map<std::string, std::string> & macros);

		void SetEntryFunction(const std::string & fname);
		
		ShaderType GetShaderType() const;

		const std::wstring & GetFileReference() const;

		const std::string & GetEntryFunction() const;

		const std::string & GetSource() const;

		const ShaderPreprocDefs & GetPreprocDefs() const;

		// Inherited via Shader
		virtual uint8_t * GetBufferPointer() override;

		virtual size_t GetBufferSize() override;

	};

}