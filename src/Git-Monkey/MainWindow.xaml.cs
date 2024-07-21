using System;
using System.Diagnostics;
using System.Windows.Forms;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Git_Monkey;
using MSAPI = Microsoft.WindowsAPICodePack;
/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow : Window
{
    #nullable disable
    /// <summary>
    /// カレントディレクトリ
    /// </summary>
    private string cur_dir = "";

    public MainWindow()
    {
        InitializeComponent();
    }

    /// <summary>
    /// ボタンクリック時の処理
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>
    private void Button_Click_Dir(object sender, RoutedEventArgs e)
    {
        var dlg = new MSAPI::Dialogs.CommonOpenFileDialog();

        // フォルダ選択ダイアログ（falseにするとファイル選択ダイアログ）
        dlg.IsFolderPicker = true;
        // タイトル
        dlg.Title = "フォルダを選択してください";
        // 初期ディレクトリ
        dlg.InitialDirectory = @"D:\Work";

        if (dlg.ShowDialog() == MSAPI::Dialogs.CommonFileDialogResult.Ok)
        {
            // Nothing
            // フォルダ選択しないとフリーズする
        }
        cur_dir = dlg.FileName;
        DirPathLabel.Content = "Dir: " + cur_dir;

        var pInfo = new ProcessStartInfo("..\\Core\\Wrapper.exe")
        {
            ArgumentList =
            {
                cur_dir,
                "MiniGit",
                "init"
            }
        };
        Process p = Process.Start(pInfo);
        if (p != null) {
            p.WaitForExit();
        }
        int code = p.ExitCode;
        if (code == 183) {      // .git が既に存在
            CommitList.Items.Add("msg1 date1");
            CommitList.Items.Add("msg2 date2");
        }
    }

    private void Button_Click_Reset(object sender, RoutedEventArgs e)
    {
        if (CommitList.SelectedItem != null) {
            MSGBOX.Content = CommitList.SelectedItem.ToString();
        }
    }
}
