#pragma once

#include "thumbViewModel.g.h"

namespace winrt::PicViewer::implementation
{
    struct thumbViewModel : thumbViewModelT<thumbViewModel>
    {
        thumbViewModel() = default;

        thumbViewModel(
            //StorageFileはWindows～
            Windows::Storage::StorageFile const& imagefile,
            //BitmapImageはMicrosoft～
            Microsoft::UI::Xaml::Media::Imaging::BitmapImage const& thumbimage,
            hstring const& imagename
        ) :
            _imageFile(imagefile),
            _thumbImage(thumbimage),
            _imageName(imagename)
        {}

        Windows::Storage::StorageFile GetImageFile() const
        {
            return _imageFile;
        }

        Microsoft::UI::Xaml::Media::Imaging::BitmapImage GetThumbImage() const
        {
            return _thumbImage;
        }

        hstring GetImageName() const
        {
            return _imageName;
        }

    private:
        Windows::Storage::StorageFile _imageFile{ nullptr };
        Microsoft::UI::Xaml::Media::Imaging::BitmapImage _thumbImage;
        hstring _imageName;

    };
}

namespace winrt::PicViewer::factory_implementation
{
    struct thumbViewModel : thumbViewModelT<thumbViewModel, implementation::thumbViewModel>
    {
    };
}

