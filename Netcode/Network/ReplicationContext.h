#pragma once

#include <NetcodeFoundation/Exceptions.h>
#include <NetcodeFoundation/Json.h>
#include <rapidjson/document.h>
#include <vector>
#include <cstdint>

namespace Netcode::Network {

	class ReplicationContext {
	public:
		using DocType = rapidjson::Document;
		using AllocType = DocType::AllocatorType;
		using Value = DocType::ValueType;
		using StringKeyType = DocType::StringRefType;
	protected:
		AllocType & allocator;
		Value * startValue;
		Value * scopedValue;
		std::vector<Value *> scopeStack;

	public:
		ReplicationContext(AllocType & alloc, Value * startV) : allocator{ alloc }, startValue{ startV }, scopedValue{ startV }, scopeStack{} {
			scopeStack.reserve(4);
		}

		void Reset() {
		  scopeStack.clear();
		  scopedValue = startValue;
		}

		constexpr ReplicationContext * operator->() {
			return this;
		}

		void Pop() {
			if(scopeStack.empty()) {
				scopedValue = startValue;
			} else {
				scopedValue = scopeStack.back();
				scopeStack.pop_back();
			}
		}

		Value * GetScopedValue() {
			return scopedValue;
		}

		// push a Value into an array and scope it
		ReplicationContext & Push(rapidjson::Type type) {
			if(scopedValue->IsArray()) {
				Value obj{ type };
				scopedValue->PushBack(obj.Move(), allocator);
				const auto & arr = scopedValue->GetArray();
				scopeStack.push_back(scopedValue);
				scopedValue = arr.Begin() + (arr.Size() - 1);
			} else {
				throw UndefinedBehaviourException{ "PushArray(): invoked on a non-array" };
			}

		  return *this;
		}

		// push an existing Value from an array
		ReplicationContext & Push(size_t idx) {
			if(scopedValue->IsArray()) {
				if(idx < scopedValue->GetArray().Size()) {
					auto it = scopedValue->Begin() + idx;
					scopeStack.push_back(scopedValue);
					scopedValue = it;
				} else throw OutOfRangeException{ "Push(idx): idx is out of range" };
			} else throw UndefinedBehaviourException{ "Push(idx): invoked on a non array" };
			return *this;
		}

		// push a Value from an object (creates the entry if it does not exist)
		bool Push(StringKeyType key, rapidjson::Type type = rapidjson::Type::kObjectType) {
			if(scopedValue->IsObject()) {
				auto it = scopedValue->FindMember(key);

				if(it != scopedValue->MemberEnd()) {
					if(it->value.GetType() == type) {
						scopeStack.push_back(scopedValue);
						scopedValue = &it->value;
					} else return false;
				} else {
					Value obj{ type };
					scopedValue->AddMember(key, obj.Move(), allocator);
					scopeStack.push_back(scopedValue);
					scopedValue = &scopedValue->FindMember(key)->value;
				}
			} else return false;

			return false;
		}

		template<typename T>
		void Add(T && value) {
			if(scopedValue->IsArray()) {
				Value v;
				JsonValueConverter<DocType>::ConvertToJson(value, v, allocator);

				scopedValue->PushBack(v.Move(), allocator);
			} else throw UndefinedBehaviourException{ "Add(T): invoked on a non array" };
		}
		
		template<typename T>
		void Set(T && value) {
			if(!scopedValue->IsObject() && !scopedValue->IsArray()) {
				Value v;
				JsonValueConverter<DocType>::ConvertToJson(value, v, allocator);
				*scopedValue = v.Move();
			} else throw UndefinedBehaviourException{ "Set(T): invoked on an object or an array" };
		}

		template<typename T>
		void Set(StringKeyType key, T && value) {
			if(scopedValue->IsObject()) {
				Value v;
				JsonValueConverter<DocType>::ConvertToJson(value, v, allocator);

				auto it = scopedValue->FindMember(key);

				if(it != scopedValue->MemberEnd()) {
					it->value = v.Move();
				} else {
					scopedValue->AddMember(key, v.Move(), allocator);
				}
			} else throw UndefinedBehaviourException{ "Set(string, T): invoked on a non object" };
		}

		template<typename T>
		void Get(uint64_t idx, T & value) {
			if(scopedValue->IsArray()) {
				const auto & arr = scopedValue->GetArray();
				if(idx < arr.Size()) {
					JsonValueConverter<DocType>::ConvertFromJson(*(arr.Begin() + idx), value);
				} else throw OutOfRangeException{ "Get(idx): idx is out of range" };
			} else throw UndefinedBehaviourException{ "Get(idx): invoked on a non array" };
		}

		template<typename T>
		void Get(const wchar_t * key, T & value) {
			if(scopedValue->IsObject()) {
				auto it = scopedValue->FindMember(key);
				if(it != scopedValue->MemberEnd()) {
					JsonValueConverter<DocType>::ConvertFromJson(it->value, value);
				} else throw OutOfRangeException{ "Get(string): key was not found" };
			} else throw UndefinedBehaviourException{ "Get(string): invoked on a non object" };
		}

		template<typename T>
		void GetOptional(uint64_t idx, T & value) {
			if(scopedValue->IsArray()) {
				const auto & arr = scopedValue->GetArray();
				if(idx < arr.Size()) {
					JsonValueConverter<DocType>::ConvertFromJson(*(arr.Begin() + idx), value);
				}
			} else throw UndefinedBehaviourException{ "GetOptional(idx): invoked on a non array" };
		}

		template<typename T>
		void GetOptional(const wchar_t * key, T & value) {
			if(scopedValue->IsObject()) {
				auto it = scopedValue->FindMember(key);
				if(it != scopedValue->MemberEnd()) {
					JsonValueConverter<DocType>::ConvertFromJson(it->value, value);
				}
			} else throw UndefinedBehaviourException{ "GetOptional(string): invoked on a non object" };
		}
	};
	
}
