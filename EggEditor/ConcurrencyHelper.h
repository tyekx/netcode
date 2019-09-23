#pragma once

#include <ppltasks.h>
#include <functional>

namespace EggEditor {

	ref class ConcurrencyHelper {

		template<typename T>
		using async_t = concurrency::task<T>;


	internal:

		/*
		* Helper function to chain async calls without manually embedding tasks
		* The continuation function gets the first one's return value 
		*/
		template<typename ResultType>
		static async_t<ResultType> ChainConcurrentCalls(async_t<ResultType> promise,
														std::function<ResultType(ResultType)> continueWith) {
			return concurrency::create_task([=]() -> ResultType {
				return continueWith(promise.get());
			});
		}

		/*
		* ResultType required to have operator==
		* conditionValue must be copy assignable
		*/
		template<typename ResultType>
		static async_t<ResultType> ChainConditionalConcurrentCalls(async_t<ResultType> promise,
																   std::function<ResultType()> continueWith,
																   const ResultType & conditionValue) {

			return concurrency::create_task([=]() -> ResultType {
				ResultType t = promise.get();

				if(conditionValue == t) {
					async_t<ResultType> secondTask = concurrency::create_task(continueWith);
					return secondTask.get();
				}

				return t;
			});

		}
	};

}
