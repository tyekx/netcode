#include "System.h"
#include <Windows.h>

namespace Netcode {

	using NTSTATUS = long;
	
	static struct NativeWinFunctions {
		NTSTATUS(__stdcall * NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval);
		NTSTATUS(__stdcall * ZwSetTimerResolution)(IN ULONG RequestedResolution, IN BOOLEAN Set, OUT PULONG ActualResolution);
		NTSTATUS(__stdcall * ZwQueryTimerResolution)(OUT PULONG CoarsestResolution, OUT PULONG FinestResolution, OUT PULONG ActualResolution);

		NativeWinFunctions() : NtDelayExecution{ nullptr }, ZwSetTimerResolution{ nullptr }, ZwQueryTimerResolution{ nullptr } {
			HMODULE ntdllModule = GetModuleHandleW(L"ntdll.dll");

			if(ntdllModule == nullptr) {
				return;
			}

			NtDelayExecution = reinterpret_cast<decltype(NtDelayExecution)>(GetProcAddress(ntdllModule, "NtDelayExecution"));
			ZwSetTimerResolution = reinterpret_cast<decltype(ZwSetTimerResolution)>(GetProcAddress(ntdllModule, "ZwSetTimerResolution"));
			ZwQueryTimerResolution = reinterpret_cast<decltype(ZwQueryTimerResolution)>(GetProcAddress(ntdllModule, "ZwQueryTimerResolution"));
		}
	} NT;
	
	static void NtSleepFor(const Duration & t) {
		LARGE_INTEGER interval;

		/*
		 * sign bit: (0) => absolute (1) => relative time
		 * interval is in [100ns] metric
		 */
		interval.QuadPart = -1 * std::chrono::duration_cast<std::chrono::microseconds>(t).count() * 10;

		NT.NtDelayExecution(false, &interval);
	}
	
	static void SetTimerResolution() {
		ULONG coarsest;
		ULONG finest;
		ULONG actual;
		NT.ZwQueryTimerResolution(&coarsest, &finest, &actual);

		ULONG actualSet;
		NT.ZwSetTimerResolution(finest, true, &actualSet);
	}
	
	void Initialize() {
		SetTimerResolution();
	}

	/*
	* precise sleep function
	*/
	void SleepFor(const Duration & duration) {
		const Timestamp t = SystemClock::LocalNow();
		Duration tmp = std::chrono::seconds(0);
		while(tmp < duration) {
			auto delta = duration - tmp;
			if(delta > std::chrono::milliseconds(1)) {
				NtSleepFor(delta);
			}
			tmp = SystemClock::LocalNow() - t;
		}
	}
	
}
