#pragma once

#include "AnimationsPage.g.h"
#include "DC_Animation.h"
#include <Netcode/Stopwatch.h>

namespace winrt::NetcodeAssetEditor::implementation
{
    struct AnimationsPage : AnimationsPageT<AnimationsPage>
    {
        float playbackTime;
        float duration;
        uint32_t selectedAnimation;
        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Animation> animations;
        
        Windows::UI::Xaml::DispatcherTimer timer;
        Netcode::Stopwatch stopwatch;

        AnimationsPage();

        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Animation> Animations();
        void Animations(Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Animation> const & value);

        float PlaybackTime();
        void PlaybackTime(float value);

        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e);
        void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e);
		void ListView_SelectionChanged(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e);
        void OnTick(Windows::Foundation::IInspectable const & sender, Windows::Foundation::IInspectable const & e );
	};
}

namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct AnimationsPage : AnimationsPageT<AnimationsPage, implementation::AnimationsPage>
    {
    };
}
