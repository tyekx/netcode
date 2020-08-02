#include "pch.h"
#include "GCommandTransform.h"
#include "GCommandTransform.g.cpp"

#include "DC_GeometryPage.h"
#include <NetcodeFoundation/Math.h>

namespace winrt::NetcodeAssetEditor::implementation
{
    winrt::event_token GCommandTransform::CanExecuteChanged(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable> const& handler)
    {
        return canExecuteChanged.add(handler);
    }

    void GCommandTransform::CanExecuteChanged(winrt::event_token const& token) noexcept
    {
        canExecuteChanged.remove(token);
    }

    bool GCommandTransform::CanExecute(Windows::Foundation::IInspectable const& parameter)
    {
        auto dataContext = parameter.try_as<DC_GeometryPage>();

        return dataContext != nullptr;
    }

    void GCommandTransform::Execute(Windows::Foundation::IInspectable const& parameter)
    {
        namespace wmath = Windows::Foundation::Numerics;

        auto dataContext = parameter.try_as<DC_GeometryPage>();

        float uniformScale = dataContext->UniformScale();
        wmath::float3 rotInDeg = dataContext->RotationInDegrees();

        if(uniformScale == 0.0f) {
            return;
        }

        wmath::float4x4 rotationMatrix = wmath::make_float4x4_from_yaw_pitch_roll(
            Netcode::DegreesToRadians(rotInDeg.y),
            Netcode::DegreesToRadians(rotInDeg.x),
            Netcode::DegreesToRadians(rotInDeg.z));

        wmath::float4x4 scaleMatrix = wmath::make_float4x4_scale(uniformScale);

        wmath::float4x4 transform = scaleMatrix * rotationMatrix;

        dataContext->TransformBuffer().Append(transform);

        dataContext->UniformScale(1.0f);
        dataContext->RotationInDegrees(wmath::float3::zero());
    }
}
