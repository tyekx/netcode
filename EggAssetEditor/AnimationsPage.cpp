#include "pch.h"
#include "AnimationsPage.h"
#if __has_include("AnimationsPage.g.cpp")
#include "AnimationsPage.g.cpp"
#endif
#include "XamlGlobal.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggAssetEditor::implementation
{
    AnimationsPage::AnimationsPage()
    {
        InitializeComponent();

        stopwatch.Reset();

        playbackTime = 0.0f;
        selectedAnimation = 0;

        timer.Interval(std::chrono::milliseconds{ 16 });
        timer.Tick(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>(this, &AnimationsPage::OnTick));

        animations = winrt::single_threaded_observable_vector<EggAssetEditor::DC_Animation>();
    }

    Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Animation> AnimationsPage::Animations()
    {
        return animations;
    }

    void AnimationsPage::Animations(Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Animation> const & value)
    {
        animations = value;
    }

    static void UpdateAnimation(float t, const OptimizedAnimation & anim, const Skeleton & skeleton, BoneData* destBuffer) {
        DirectX::XMVECTOR stPos;
        DirectX::XMVECTOR endPos;

        DirectX::XMVECTOR stQuat;
        DirectX::XMVECTOR endQuat;

        DirectX::XMVECTOR stScale;
        DirectX::XMVECTOR endScale;

        DirectX::XMMATRIX bindTrans;
        DirectX::XMMATRIX toRoot[128];
        int parentId;

        unsigned int idx;
        for(idx = 1; idx < anim.keyTimes.size(); ++idx) {
            if(anim.keyTimes[idx - 1] <= t && anim.keyTimes[idx] >= t) {
                t = (t - anim.keyTimes[idx - 1]) / (anim.keyTimes[idx] - anim.keyTimes[idx - 1]);
                break;
            }
        }

        auto * startKey = anim.keys.at(idx - 1).boneData.data();
        auto * endKey = anim.keys.at(idx).boneData.data();

        size_t boneCount = skeleton.bones.size();

         for(unsigned int i = 0; i < boneCount; ++i) {
            stPos = DirectX::XMLoadFloat3(&startKey[i].position);
            stQuat = DirectX::XMLoadFloat4(&startKey[i].rotation);
            stScale = DirectX::XMLoadFloat3(&startKey[i].scale);

            endPos = DirectX::XMLoadFloat3(&endKey[i].position);
            endQuat = DirectX::XMLoadFloat4(&endKey[i].rotation);
            endScale = DirectX::XMLoadFloat3(&endKey[i].scale);

            stPos = DirectX::XMVectorLerp(stPos, endPos, t);
            stQuat = DirectX::XMQuaternionSlerp(stQuat, endQuat, t);
            stScale = DirectX::XMVectorLerp(stScale, endScale, t);

            toRoot[i] = DirectX::XMMatrixAffineTransformation(stScale, DirectX::XMQuaternionIdentity(), stQuat, stPos);
         }

         for(unsigned int i = 0; i < boneCount; ++i) {
             int parentId = skeleton.bones[i].parentIndex;
             if(parentId > -1) {
                 toRoot[i] = DirectX::XMMatrixMultiply(toRoot[i], toRoot[parentId]);
             }

             bindTrans = DirectX::XMLoadFloat4x4(&skeleton.bones[i].transform);
             bindTrans = DirectX::XMMatrixMultiply(bindTrans, toRoot[i]);

             DirectX::XMStoreFloat4x4A(&destBuffer->ToRootTransform[i], DirectX::XMMatrixTranspose(toRoot[i]));
             DirectX::XMStoreFloat4x4A(&destBuffer->BindTransform[i], DirectX::XMMatrixTranspose(bindTrans));
         }
    }
    
    void AnimationsPage::OnTick(Windows::Foundation::IInspectable const & sender, Windows::Foundation::IInspectable const & e)
    {
        float dt = stopwatch.Restart();

        playbackTime += dt * Global::Model->animations[selectedAnimation].framesPerSecond;

        if(playbackTime > duration) {
            playbackTime = 0.0f;
        }

        UpdateAnimation(playbackTime, Global::Model->animations[selectedAnimation], Global::Model->skeleton, Global::EditorApp->GetBoneData());

        Global::EditorApp->Run();
    }

    void AnimationsPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e) {
        animations.Clear();

        stopwatch.Start();

        if(Global::Model == nullptr) {
            return;
        }

        for(const OptimizedAnimation & optAnim : Global::Model->animations) {
            auto dcAnim = winrt::make<DC_Animation>();
            
            dcAnim.Timescale(1.0f);
            dcAnim.Name(to_hstring(optAnim.name));

            animations.Append(dcAnim);
        }

        __super::OnNavigatedTo(e);
    }

    void AnimationsPage::OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e) {
        timer.Stop();

        __super::OnNavigatedFrom(e);
    }

    void AnimationsPage::ListView_SelectionChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e)
    {
        if(animationsView().SelectedItems().Size() == 1) {
            selectedAnimation = animationsView().SelectedIndex();

            playbackTime = 0.0f;

            if(!timer.IsEnabled()) {
                timer.Start();
            }

            duration = Global::Model->animations[selectedAnimation].duration;
        }
    }

    float AnimationsPage::PlaybackTime()
    {
        return playbackTime;
    }

    void AnimationsPage::PlaybackTime(float value)
    {
        playbackTime = value;
    }
}


