#include "pch.h"
#include "NewPage.xaml.h"
#if __has_include("NewPage.g.cpp")
#include "NewPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

//忘れずにインクルード
#include "MainWindow.xaml.h"

namespace winrt::PicViewer::implementation
{
	void NewPage::SetImageSource(winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage const& source)
	{
		showImage().Source(source);
		return;
	}
}


void winrt::PicViewer::implementation::NewPage::Page_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
{
	auto main = winrt::PicViewer::implementation::MainWindow::GetMainCurrent();

	if (e.Key() == winrt::Windows::System::VirtualKey::Right)
	{
		main.NextItem();
	}
	else if (e.Key() == winrt::Windows::System::VirtualKey::Left)
	{
		main.PrevItem();
	}
	else if (e.Key() == winrt::Windows::System::VirtualKey::Delete)
	{
		main.DeleteItem();
	}

}
