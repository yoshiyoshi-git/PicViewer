#pragma once

#include "MainWindow.g.h"

namespace winrt::PicViewer::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow()
        {
            //�^�C�g�����̕ύX
            this->Title(L"PicViewer");
            
            //_thumbViewModelVec�̏�����
            _thumbViewModelVec = winrt::single_threaded_observable_vector<IInspectable>();

            //mainCurrent��*this����
            mainCurrent = *this;

            //MainWindow����������NewPage�����i�C�x���g�̐ݒ�j
            Closed([&](auto&& sender, auto&& e) 
                {
                    if (_newPage)
                    {
                        _newPage.Close();
                        _newPage = nullptr;
                    }
                });


        }

        //mainCurrent��n��
        static PicViewer::MainWindow GetMainCurrent()
        {
            return mainCurrent;
        }

        //_thumbViewModelVec��index��n����NewPage�̕\���摜��ύX����֐�
        winrt::Windows::Foundation::IAsyncAction changeNewPageImage(unsigned int _thumbViewModelVec_index);

        //idl�Œ�`�������̉摜��\������NextItem
        winrt::Windows::Foundation::IAsyncAction NextItem();
        
        //idl�Œ�`�����O�̉摜��\������PrevItem
        winrt::Windows::Foundation::IAsyncAction PrevItem();

        //idl�Œ�`�����I������Ă���摜���폜����DeleteItem�B�摜�͑O�̉摜�ɖ߂��
        winrt::Windows::Foundation::IAsyncAction DeleteItem();

        //_thumbViewModelVec��index��n���Ƃ��̉摜���폜����֐�
        winrt::Windows::Foundation::IAsyncAction  DeleteIndexPicture(unsigned int _thumbViewModelVec_index);

        //HWND�擾�֐�
        HWND getHwnd();

        //idl�Ő錾�����Q�b�^�[
        Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> viewModelVec() const
        {
            return _thumbViewModelVec;
        }

        //Open���N���b�N���ꂽ��I-�����͔񓯊��֕ύX
        winrt::Windows::Foundation::IAsyncAction Open_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
       
        //Exit���N���b�N���ꂽ��I-�����͔񓯊��֕ύX
        winrt::Windows::Foundation::IAsyncAction Exit_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    
    private:
        //�������g�ւ̃n���h��
        static PicViewer::MainWindow MainWindow::mainCurrent;

        //thumbViewModel��ۑ����郁���o
        Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> _thumbViewModelVec{ nullptr };

        //NewPage����p�����o
        winrt::Microsoft::UI::Xaml::Window _newPage{ nullptr };

    public:
        //GridView�̃A�C�e�����N���b�N���ꂽ
        winrt::Windows::Foundation::IAsyncAction thumbGrid_ItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e);
        
        //�L�[�{�[�h�������ꂽ
        winrt::Windows::Foundation::IAsyncAction thumbGrid_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
    };
}

namespace winrt::PicViewer::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
