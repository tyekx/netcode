#pragma once

#include <NetcodeFoundation/Memory.h>
#include <NetcodeFoundation/Math.h>

#include "HandleDecl.h"

#include "Input/Key.h"

namespace Netcode {

	template<typename StdAlloc, typename ... T>
	class ManagedEvent;

	class AxisMapBase;

	class Input {
		Input() = delete;
		~Input() = delete;

		struct detail;

		static std::unique_ptr<detail> instance;
	public:

		using Alloc = Memory::ObjectAllocator;

		using StdAlloc = Memory::StdAllocatorAdapter<void, Alloc>;

		template<typename ... T>
		using EventType = ManagedEvent<StdAlloc, T...>;

		/**
		* Raw input, every key, every mouse button, every state
		*/
		static EventType<Key, KeyModifiers> * OnInput;

		/**
		* Raw mouse input, every mouse key, every state
		*/
		static EventType<Key, KeyModifiers> * OnMouseInput;

		/**
		* Rising edges for mouse keys
		*/
		static EventType<Key, KeyModifiers> * OnMouseKeyPressed;

		/**
		* Falling edges for mouse keys
		*/
		static EventType<Key, KeyModifiers> * OnMouseKeyReleased;

		/**
		* Raw keyboard input, every keyboard key, every state
		*/
		static EventType<Key, KeyModifiers> * OnKeyInput;

		/**
		* Rising edges or pulses for keyboard keys
		*/
		static EventType<Key, KeyModifiers> * OnKeyPressed;

		/**
		* Falling edges for keyboard keys
		*/
		static EventType<Key, KeyModifiers> * OnKeyReleased;

		/**
		* Mouse wheel scroll event
		* 1st argument is a 1D vector, direction and longitude. 
		* x > 0: scroll up
		* x < 0: scroll down
		*/
		static EventType<int, KeyModifiers> * OnScroll;

		/**
		* Mouse move event
		* Int2: mouse delta
		*/
		static EventType<Int2, KeyModifiers> * OnMouseMove;

		static EventType<wchar_t> * OnCharInput;

		static bool InputIsCapital();

		static bool IsAlphabetic(KeyCode keyCode);
		
		static bool IsNumber(KeyCode keyCode);

		static bool IsAlphaNumeric(KeyCode keyCode);

		static bool IsModifier(KeyCode keyCode);
		
		static bool IsToggledType(KeyCode keyCode);

		static Key GetKey(KeyCode keyCode);

		static void ProcessBlurredEvent();

		static void ProcessFocusedEvent();

		static void ProcessKeyEvent(Key keyEvent);

		static void ProcessCharEvent(wchar_t value);

		static void ProcessMouseMoveEvent(const Int2 & mouseWindowPosition);

		static void ProcessPlatformEvent(uintptr_t wParam, intptr_t lParam);

		static void Initialize();

		static void UpdateAxisMap(float dt);

		static float GetAxis(uint32_t axis);

		static void SetAxisMap(Ref<AxisMapBase> axisMap);

		static void CompleteFrame();

		static Int2 GetMouseDelta();

		static Int2 GetMousePosition();

	};

}
