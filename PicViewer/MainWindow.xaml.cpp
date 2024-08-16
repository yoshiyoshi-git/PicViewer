#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

//::IWindowNativeと::IInitializeWithWindowを使うのにmicrosoft.ui.xaml.window.h、Shobjidl.hが必要
//MessageDialogを使うのにWindows.UI.Popups.hが必要
#include <microsoft.ui.xaml.window.h>
#include <winrt/Windows.UI.Popups.h>
#include <Shobjidl.h>

//FolderPickerを使用するのにwinrt/Windows.Storage.Pickers.hが必要
#include <winrt/Windows.Storage.Pickers.h>

//Windows::Storage::Search～を使用するのに必要
#include <winrt/Windows.Storage.Search.h>

//thumbViewModel.hも忘れずに
#include"thumbViewModel.h"

//NewPage.xaml.hも新しいウインドウ表示に必要
#include "NewPage.xaml.h"

//staticメンバなので初期化が必要
winrt::PicViewer::MainWindow winrt::PicViewer::implementation::MainWindow::mainCurrent{ nullptr };

namespace winrt::PicViewer::implementation
{	
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::changeNewPageImage(unsigned int _thumbViewModelVec_index)
{
	if (_thumbViewModelVec.Size() == 0)
		return;

	if (_thumbViewModelVec_index >= _thumbViewModelVec.Size())
		return;

	//現在選択されているViewModelを取得
	auto&& imageViewModel = _thumbViewModelVec.GetAt(_thumbViewModelVec_index).as<PicViewer::thumbViewModel>();

	//StorageFileと画像名と画像を取得するためStorage::Streamを取得
	auto&& imageFile = imageViewModel.GetImageFile();
	auto&& imageName = imageFile.Name();
	auto&& imageStream = co_await imageFile.OpenAsync(Windows::Storage::FileAccessMode::Read);

	//BitmapImageを取得
	Microsoft::UI::Xaml::Media::Imaging::BitmapImage bitmapImage{};
	bitmapImage.SetSource(imageStream);

	//無ければ作る
	if (!_newPage)
	{
		_newPage = winrt::Microsoft::UI::Xaml::Window();
		_newPage.Content(winrt::PicViewer::NewPage());
	}

	//それぞれを変更
	_newPage.Content().as<PicViewer::NewPage>().SetImageSource(bitmapImage);
	_newPage.Title(imageName);

	//ウインドウが閉じた時にNewWindowをnullptrにする（イベントの設定）
	_newPage.Closed([&](auto&& sender, auto&& e){_newPage = nullptr;});
	_newPage.Activate();
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::NextItem()
{
	//GridViewの選ばれているindexを取得
	auto selectindex = thumbGrid().SelectedIndex();

	//最後のindexだったら0へ、そうで無ければ次のindexをnextindexへ
	unsigned int nextindex = (selectindex + 1) % _thumbViewModelVec.Size();

	//GrigViewの選択されているサムネイルをnextindexへ変更
	thumbGrid().SelectedIndex(nextindex);

	//NewPageの画像をnextindexへ変更
	co_await changeNewPageImage(nextindex);
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::PrevItem()
{
	//GridViewの選ばれているindexを取得
	auto selectindex = thumbGrid().SelectedIndex();

	//最初のindexだったら最後へ、そうで無ければ前のindexをprevindexへ
	unsigned int previndex = (selectindex == 0) ? _thumbViewModelVec.Size() - 1 : selectindex - 1;

	//GrigViewの選択されているサムネイルをprevindexへ変更
	thumbGrid().SelectedIndex(previndex);

	//NewPageの画像をprevindexへ変更
	co_await changeNewPageImage(previndex);
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::DeleteItem()
{
	if (_thumbViewModelVec.Size() == 0)
		co_return;

	//削除する現在の画像のindexを取得
	auto selectindex = thumbGrid().SelectedIndex();

	if (_thumbViewModelVec.Size() == 1)
	{
		//画像が1つだけの場合は_newPageを閉じる
		_newPage.Close();
		_newPage = nullptr;
	}
	else
	{
		//画像が2つ以上あれば、前の画像へ戻る
		PrevItem();
	}

	//削除
	co_await DeleteIndexPicture(selectindex);
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::DeleteIndexPicture(unsigned int _thumbViewModelVec_index)
{
	//indexからStorageFileを取得
	auto imagefile = _thumbViewModelVec.GetAt(_thumbViewModelVec_index).as<PicViewer::thumbViewModel>().GetImageFile();

	//画像を削除
	co_await imagefile.DeleteAsync();

	//_thumbViewModelVecからも削除
	_thumbViewModelVec.RemoveAt(_thumbViewModelVec_index);
}

HWND winrt::PicViewer::implementation::MainWindow::getHwnd()
{
	auto windowNative{ this->m_inner.as<::IWindowNative>() };
	HWND hWnd{ 0 };
	windowNative->get_WindowHandle(&hWnd);

	return hWnd;
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::Open_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	//各種ピッカーもwinrt::Windows～
	auto folderpicker = Windows::Storage::Pickers::FolderPicker();
	folderpicker.as<::IInitializeWithWindow>()->Initialize(getHwnd());

	//初期に開くフォルダはピクチャフォルダ
	folderpicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::PicturesLibrary);

	//ピッカーを表示
	auto&& serchfolder = co_await folderpicker.PickSingleFolderAsync();

	//キャンセルだったらco_return
	if (serchfolder == nullptr)
		co_return;

	//ここから選択されたフォルダの画像ファイルを変換・保存
	_thumbViewModelVec.Clear();

	//jpgとpngとbmpとgif画像を探す。さがすフォルダはそのフォルダのみ
	Windows::Storage::Search::QueryOptions serchoptions{};
	serchoptions.FileTypeFilter().Append(L".jpg");
	serchoptions.FileTypeFilter().Append(L".png");
	serchoptions.FileTypeFilter().Append(L".bmp");
	serchoptions.FileTypeFilter().Append(L".gif");
	serchoptions.FolderDepth(Windows::Storage::Search::FolderDepth::Shallow);

	//フォルダ内をサーチして、画像ファイルのStorageFileをallimageへ保存
	auto serchresult = serchfolder.as<Windows::Storage::Search::IStorageFolderQueryOperations>().CreateFileQueryWithOptions(serchoptions);
	auto&& allimage = co_await serchresult.GetFilesAsync();

	//allimageをすべて走査して3つすべて取得できれば、_thumbViewModelVecへ保存
	for (auto&& imagefile : allimage)
	{
		auto&& thumbnail{ co_await imagefile.GetThumbnailAsync(Windows::Storage::FileProperties::ThumbnailMode::PicturesView, 190, Windows::Storage::FileProperties::ThumbnailOptions::ResizeThumbnail) };

		Microsoft::UI::Xaml::Media::Imaging::BitmapImage thumbImage{};
		if (thumbnail)
			thumbImage.SetSource(thumbnail);
		else
			continue;

		auto&& thumbViewModel = make<PicViewer::implementation::thumbViewModel>(imagefile, thumbImage, imagefile.Name());
		_thumbViewModelVec.Append(thumbViewModel);
	}
}


winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::Exit_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	//ContentDialogはwinrt/Microsoft...となる。必要なのはXamlRoot
	Microsoft::UI::Xaml::Controls::ContentDialog isenddialog{};
	isenddialog.XamlRoot(this->Content().XamlRoot());
	isenddialog.Title(box_value(L"Do you want to close?"));
	isenddialog.PrimaryButtonText(L"Yes");
	isenddialog.SecondaryButtonText(L"No");
	isenddialog.CloseButtonText(L"Cancel");

	auto result = co_await isenddialog.ShowAsync();

	if (result == Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary)
	{
		//アプリ終了
		Application::Current().Exit();
	}
	else if (result == Microsoft::UI::Xaml::Controls::ContentDialogResult::Secondary)
	{
		//MessageDialogはwinrt::Windows...となる。必要なのはHWND
		auto showdialog{ Windows::UI::Popups::MessageDialog(L"Continue without closing this application") };
		showdialog.as<::IInitializeWithWindow>()->Initialize(getHwnd());

		co_await showdialog.ShowAsync();
	}
}


winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::thumbGrid_ItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e)
{
	if (_thumbViewModelVec.Size() == 0)
		return;

	//クリックしたthumbViewModelを取得
	auto&& selectitem = e.ClickedItem().as<PicViewer::thumbViewModel>();

	//thumbViewModelからファイル名（NewPageのタイトルに使用）を取得
	auto&& imagename = selectitem.GetImageName();

	//thumbViewModelからBitmapImageを取得
	auto&& bitmapimage = winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage();
	auto&& imagestream = co_await selectitem.GetImageFile().OpenAsync(Windows::Storage::FileAccessMode::Read);
	bitmapimage.SetSource(imagestream);

	//初期化されていなければ、初期化
	if (!_newPage)
	{
		_newPage = winrt::Microsoft::UI::Xaml::Window();
		_newPage.Content(winrt::PicViewer::NewPage());
	}

	//NewPageのSetImageSourceを呼び出して、画像をセット
	_newPage.Content().as<PicViewer::NewPage>().SetImageSource(bitmapimage);
	
	//タイトルにファイル名を表示
	_newPage.Title(imagename);

	//ウインドウが閉じた時にNewWindowをnullptrにする（イベントの設定）
	_newPage.Closed([&](auto&& sender, auto&& e)
		{
			_newPage = nullptr;
		});

	//アクティブ化
	if (_newPage)
		_newPage.Activate();
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::thumbGrid_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
{
	//GridViewのindexを取得
	auto selectindex = thumbGrid().SelectedIndex();

	if (selectindex < 0)
		co_return;

	//Deleteキーが押された時の処理。画像を削除し、サムネイルも削除
	if (e.Key() == winrt::Windows::System::VirtualKey::Delete)
	{
		co_await DeleteIndexPicture(selectindex);
		_thumbViewModelVec.RemoveAt(selectindex);
	}
}
