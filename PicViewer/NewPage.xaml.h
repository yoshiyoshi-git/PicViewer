#pragma once

#include "NewPage.g.h"

namespace winrt::PicViewer::implementation
{
    struct NewPage : NewPageT<NewPage>
    {
        NewPage()
        {
        }

        //‚±‚ê‚Å‰æ‘œ‚ð•\Ž¦‚³‚¹‚é
        void SetImageSource(winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage const& source);
        void Page_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
    };
}

namespace winrt::PicViewer::factory_implementation
{
    struct NewPage : NewPageT<NewPage, implementation::NewPage>
    {
    };
}
