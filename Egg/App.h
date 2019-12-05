#pragma once

#include "Common.h"
#include <vector>
#include <chrono>
#include "Stopwatch.h"
#include "Scene.h"

namespace Egg {

	enum class DisplayMode {
		WINDOWED, FULLSCREEN, BORDERLESS
	};

	class App {
	protected:
		// timer objects
		Egg::Stopwatch stopwatch;
		std::unique_ptr<Egg::Scene> scene;
		float elapsedTime;

	public:
		virtual ~App() = default;


		void Run() {
			float deltaTime = stopwatch.Restart();
			elapsedTime += deltaTime;
			Update(deltaTime, elapsedTime);
			Render();
		}

		void SetScene(std::unique_ptr<Egg::Scene> sc) {
			scene = std::move(sc);
		}

		Egg::Scene * GetScene() const {
			return scene.get();
		}
		
		virtual void SetWindow(void * hwnd) = 0;


		virtual void Render() = 0;

		virtual void Update(float dt, float T) = 0;

		virtual void LoadAssets() { }

		virtual void ReleaseAssets() { }

		virtual void CreateResources() {
		
		}

		virtual void ReleaseResources() {
		}

		virtual void Destroy() {
			ReleaseResources();
			ReleaseAssets();
		}

		
		/*
		User input
		*/

		virtual void KeyPressed(uint32_t keyCode) { }
		virtual void KeyReleased(uint32_t keyCode) { }

		virtual void Focused() { }
		virtual void Blur() { }

		virtual void MouseMove(int x, int y) { }

		/*
		Editor
		*/
		virtual void SetSelection(int index) { }
	};

}
