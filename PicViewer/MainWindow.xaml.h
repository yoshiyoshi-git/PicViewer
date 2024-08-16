#pragma once

#include "MainWindow.g.h"

namespace winrt::PicViewer::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow()
        {
            //タイトル名の変更
            this->Title(L"PicViewer");
            
            //_thumbViewModelVecの初期化
            _thumbViewModelVec = winrt::single_threaded_observable_vector<IInspectable>();

            //mainCurrentへ*thisを代入
            mainCurrent = *this;

            //MainWindowが閉じた時にNewPageを閉じる（イベントの設定）
            Closed([&](auto&& sender, auto&& e) 
                {
                    if (_newPage)
                    {
                        _newPage.Close();
                        _newPage = nullptr;
                    }
                });


        }

        //mainCurrentを渡す
        static PicViewer::MainWindow GetMainCurrent()
        {
            return mainCurrent;
        }

        //_thumbViewModelVecのindexを渡すとNewPageの表示画像を変更する関数
        winrt::Windows::Foundation::IAsyncAction changeNewPageImage(unsigned int _thumbViewModelVec_index);

        //idlで定義した次の画像を表示するNextItem
        winrt::Windows::Foundation::IAsyncAction NextItem();
        
        //idlで定義した前の画像を表示するPrevItem
        winrt::Windows::Foundation::IAsyncAction PrevItem();

        //idlで定義した選択されている画像を削除するDeleteItem。画像は前の画像に戻るよ
        winrt::Windows::Foundation::IAsyncAction DeleteItem();

        //_thumbViewModelVecのindexを渡すとその画像を削除する関数
        winrt::Windows::Foundation::IAsyncAction  DeleteIndexPicture(unsigned int _thumbViewModelVec_index);

        //HWND取得関数
        HWND getHwnd();

        //idlで宣言したゲッター
        Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> viewModelVec() const
        {
            return _thumbViewModelVec;
        }

        //Openがクリックされたよ！-ここは非同期へ変更
        winrt::Windows::Foundation::IAsyncAction Open_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
       
        //Exitがクリックされたよ！-ここは非同期へ変更
        winrt::Windows::Foundation::IAsyncAction Exit_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    
    private:
        //自分自身へのハンドル
        static PicViewer::MainWindow MainWindow::mainCurrent;

        //thumbViewModelを保存するメンバ
        Windows::Foundation::Collections::IObservableVector<Windows::Foundation::IInspectable> _thumbViewModelVec{ nullptr };

        //NewPage操作用メンバ
        winrt::Microsoft::UI::Xaml::Window _newPage{ nullptr };

    public:
        //GridViewのアイテムがクリックされた
        winrt::Windows::Foundation::IAsyncAction thumbGrid_ItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e);
        
        //キーボードが押された
        winrt::Windows::Foundation::IAsyncAction thumbGrid_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
    };
}

namespace winrt::PicViewer::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
