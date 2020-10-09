#include "Json.h"

namespace Netcode {

	class JsonErrorCategory : public std::error_category {
	public:
		[[nodiscard]]
		const char * name() const noexcept override {
			return "Netcode.Json";
		}

		[[nodiscard]]
		std::string message(int ev) const override {
			switch(static_cast<rapidjson::ParseErrorCode>(ev)) {
				case rapidjson::kParseErrorNone: return "Success";
				case rapidjson::kParseErrorDocumentEmpty: return "Document is empty";
				case rapidjson::kParseErrorDocumentRootNotSingular: return "Document root not singular";
				case rapidjson::kParseErrorValueInvalid: return "Invalid value";
				case rapidjson::kParseErrorObjectMissName: return "Missing name for object member";
				case rapidjson::kParseErrorObjectMissColon: return "Missing colon after name";
				case rapidjson::kParseErrorObjectMissCommaOrCurlyBracket: return "Missing a comma or '}' after an object member";
				case rapidjson::kParseErrorArrayMissCommaOrSquareBracket: return "Missing a comma or ']' after an array element";
				case rapidjson::kParseErrorStringUnicodeEscapeInvalidHex: return "Incorrect hex digit after \\u escape";
				case rapidjson::kParseErrorStringUnicodeSurrogateInvalid: return "The surrogate pair in string is invalid";
				case rapidjson::kParseErrorStringEscapeInvalid: return "Invalid escape character in string";
				case rapidjson::kParseErrorStringMissQuotationMark: return "Missing quotation mark in string";
				case rapidjson::kParseErrorStringInvalidEncoding: return "Invalid encoding in string";
				case rapidjson::kParseErrorNumberTooBig: return "Number too big";
				case rapidjson::kParseErrorNumberMissFraction: return "Missing fraction part in number";
				case rapidjson::kParseErrorNumberMissExponent: return "Missing exponent in number";
				case rapidjson::kParseErrorTermination: return "Parsing was terminated";
				default: return "Unknown error";
			}
		}
	};
	
	ErrorCode make_error_code(JsonErrc ec) {
		static JsonErrorCategory cat{};
		return ErrorCode{ static_cast<int>(ec), cat };
	}
	
}
