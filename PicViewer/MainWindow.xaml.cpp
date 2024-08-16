#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

//::IWindowNative��::IInitializeWithWindow���g���̂�microsoft.ui.xaml.window.h�AShobjidl.h���K�v
//MessageDialog���g���̂�Windows.UI.Popups.h���K�v
#include <microsoft.ui.xaml.window.h>
#include <winrt/Windows.UI.Popups.h>
#include <Shobjidl.h>

//FolderPicker���g�p����̂�winrt/Windows.Storage.Pickers.h���K�v
#include <winrt/Windows.Storage.Pickers.h>

//Windows::Storage::Search�`���g�p����̂ɕK�v
#include <winrt/Windows.Storage.Search.h>

//thumbViewModel.h���Y�ꂸ��
#include"thumbViewModel.h"

//NewPage.xaml.h���V�����E�C���h�E�\���ɕK�v
#include "NewPage.xaml.h"

//static�����o�Ȃ̂ŏ��������K�v
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

	//���ݑI������Ă���ViewModel���擾
	auto&& imageViewModel = _thumbViewModelVec.GetAt(_thumbViewModelVec_index).as<PicViewer::thumbViewModel>();

	//StorageFile�Ɖ摜���Ɖ摜���擾���邽��Storage::Stream���擾
	auto&& imageFile = imageViewModel.GetImageFile();
	auto&& imageName = imageFile.Name();
	auto&& imageStream = co_await imageFile.OpenAsync(Windows::Storage::FileAccessMode::Read);

	//BitmapImage���擾
	Microsoft::UI::Xaml::Media::Imaging::BitmapImage bitmapImage{};
	bitmapImage.SetSource(imageStream);

	//������΍��
	if (!_newPage)
	{
		_newPage = winrt::Microsoft::UI::Xaml::Window();
		_newPage.Content(winrt::PicViewer::NewPage());
	}

	//���ꂼ���ύX
	_newPage.Content().as<PicViewer::NewPage>().SetImageSource(bitmapImage);
	_newPage.Title(imageName);

	//�E�C���h�E����������NewWindow��nullptr�ɂ���i�C�x���g�̐ݒ�j
	_newPage.Closed([&](auto&& sender, auto&& e){_newPage = nullptr;});
	_newPage.Activate();
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::NextItem()
{
	//GridView�̑I�΂�Ă���index���擾
	auto selectindex = thumbGrid().SelectedIndex();

	//�Ō��index��������0�ցA�����Ŗ�����Ύ���index��nextindex��
	unsigned int nextindex = (selectindex + 1) % _thumbViewModelVec.Size();

	//GrigView�̑I������Ă���T���l�C����nextindex�֕ύX
	thumbGrid().SelectedIndex(nextindex);

	//NewPage�̉摜��nextindex�֕ύX
	co_await changeNewPageImage(nextindex);
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::PrevItem()
{
	//GridView�̑I�΂�Ă���index���擾
	auto selectindex = thumbGrid().SelectedIndex();

	//�ŏ���index��������Ō�ցA�����Ŗ�����ΑO��index��previndex��
	unsigned int previndex = (selectindex == 0) ? _thumbViewModelVec.Size() - 1 : selectindex - 1;

	//GrigView�̑I������Ă���T���l�C����previndex�֕ύX
	thumbGrid().SelectedIndex(previndex);

	//NewPage�̉摜��previndex�֕ύX
	co_await changeNewPageImage(previndex);
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::DeleteItem()
{
	if (_thumbViewModelVec.Size() == 0)
		co_return;

	//�폜���錻�݂̉摜��index���擾
	auto selectindex = thumbGrid().SelectedIndex();

	if (_thumbViewModelVec.Size() == 1)
	{
		//�摜��1�����̏ꍇ��_newPage�����
		_newPage.Close();
		_newPage = nullptr;
	}
	else
	{
		//�摜��2�ȏ゠��΁A�O�̉摜�֖߂�
		PrevItem();
	}

	//�폜
	co_await DeleteIndexPicture(selectindex);
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::DeleteIndexPicture(unsigned int _thumbViewModelVec_index)
{
	//index����StorageFile���擾
	auto imagefile = _thumbViewModelVec.GetAt(_thumbViewModelVec_index).as<PicViewer::thumbViewModel>().GetImageFile();

	//�摜���폜
	co_await imagefile.DeleteAsync();

	//_thumbViewModelVec������폜
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
	//�e��s�b�J�[��winrt::Windows�`
	auto folderpicker = Windows::Storage::Pickers::FolderPicker();
	folderpicker.as<::IInitializeWithWindow>()->Initialize(getHwnd());

	//�����ɊJ���t�H���_�̓s�N�`���t�H���_
	folderpicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::PicturesLibrary);

	//�s�b�J�[��\��
	auto&& serchfolder = co_await folderpicker.PickSingleFolderAsync();

	//�L�����Z����������co_return
	if (serchfolder == nullptr)
		co_return;

	//��������I�����ꂽ�t�H���_�̉摜�t�@�C����ϊ��E�ۑ�
	_thumbViewModelVec.Clear();

	//jpg��png��bmp��gif�摜��T���B�������t�H���_�͂��̃t�H���_�̂�
	Windows::Storage::Search::QueryOptions serchoptions{};
	serchoptions.FileTypeFilter().Append(L".jpg");
	serchoptions.FileTypeFilter().Append(L".png");
	serchoptions.FileTypeFilter().Append(L".bmp");
	serchoptions.FileTypeFilter().Append(L".gif");
	serchoptions.FolderDepth(Windows::Storage::Search::FolderDepth::Shallow);

	//�t�H���_�����T�[�`���āA�摜�t�@�C����StorageFile��allimage�֕ۑ�
	auto serchresult = serchfolder.as<Windows::Storage::Search::IStorageFolderQueryOperations>().CreateFileQueryWithOptions(serchoptions);
	auto&& allimage = co_await serchresult.GetFilesAsync();

	//allimage�����ׂđ�������3���ׂĎ擾�ł���΁A_thumbViewModelVec�֕ۑ�
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
	//ContentDialog��winrt/Microsoft...�ƂȂ�B�K�v�Ȃ̂�XamlRoot
	Microsoft::UI::Xaml::Controls::ContentDialog isenddialog{};
	isenddialog.XamlRoot(this->Content().XamlRoot());
	isenddialog.Title(box_value(L"Do you want to close?"));
	isenddialog.PrimaryButtonText(L"Yes");
	isenddialog.SecondaryButtonText(L"No");
	isenddialog.CloseButtonText(L"Cancel");

	auto result = co_await isenddialog.ShowAsync();

	if (result == Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary)
	{
		//�A�v���I��
		Application::Current().Exit();
	}
	else if (result == Microsoft::UI::Xaml::Controls::ContentDialogResult::Secondary)
	{
		//MessageDialog��winrt::Windows...�ƂȂ�B�K�v�Ȃ̂�HWND
		auto showdialog{ Windows::UI::Popups::MessageDialog(L"Continue without closing this application") };
		showdialog.as<::IInitializeWithWindow>()->Initialize(getHwnd());

		co_await showdialog.ShowAsync();
	}
}


winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::thumbGrid_ItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e)
{
	if (_thumbViewModelVec.Size() == 0)
		return;

	//�N���b�N����thumbViewModel���擾
	auto&& selectitem = e.ClickedItem().as<PicViewer::thumbViewModel>();

	//thumbViewModel����t�@�C�����iNewPage�̃^�C�g���Ɏg�p�j���擾
	auto&& imagename = selectitem.GetImageName();

	//thumbViewModel����BitmapImage���擾
	auto&& bitmapimage = winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage();
	auto&& imagestream = co_await selectitem.GetImageFile().OpenAsync(Windows::Storage::FileAccessMode::Read);
	bitmapimage.SetSource(imagestream);

	//����������Ă��Ȃ���΁A������
	if (!_newPage)
	{
		_newPage = winrt::Microsoft::UI::Xaml::Window();
		_newPage.Content(winrt::PicViewer::NewPage());
	}

	//NewPage��SetImageSource���Ăяo���āA�摜���Z�b�g
	_newPage.Content().as<PicViewer::NewPage>().SetImageSource(bitmapimage);
	
	//�^�C�g���Ƀt�@�C������\��
	_newPage.Title(imagename);

	//�E�C���h�E����������NewWindow��nullptr�ɂ���i�C�x���g�̐ݒ�j
	_newPage.Closed([&](auto&& sender, auto&& e)
		{
			_newPage = nullptr;
		});

	//�A�N�e�B�u��
	if (_newPage)
		_newPage.Activate();
}

winrt::Windows::Foundation::IAsyncAction winrt::PicViewer::implementation::MainWindow::thumbGrid_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
{
	//GridView��index���擾
	auto selectindex = thumbGrid().SelectedIndex();

	if (selectindex < 0)
		co_return;

	//Delete�L�[�������ꂽ���̏����B�摜���폜���A�T���l�C�����폜
	if (e.Key() == winrt::Windows::System::VirtualKey::Delete)
	{
		co_await DeleteIndexPicture(selectindex);
		_thumbViewModelVec.RemoveAt(selectindex);
	}
}
