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
        if (cur_dir == "") {
            MSGBOX.Content = "管理するフォルダを選択してください.";
        }
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

    /// <summary>
    /// git reset
    /// </summary>
    private void Button_Click_Reset(object sender, RoutedEventArgs e)
    {
        if (CommitList.SelectedItem != null) {
            MSGBOX.Content = CommitList.SelectedItem.ToString();
        }
    }

    /// <summary>
    /// git add & commit
    /// </summary>
    private void Button_Click_Commit(object sender, RoutedEventArgs e)
    {
        // string author = AuthorText.Text;
        // string email = EmailText.Text;
        string commit_message = CommitMsgText.Text;
        commit_message = "\"" + commit_message + "\"";

        string[] args1 = {cur_dir, "MiniGit", "add", "-A"};
        string[] args2 = {cur_dir, "MiniGit", "commit", "-m", commit_message};
        MSGBOX.Content = commit_message;
        AwaitProcess("..\\Core\\Wrapper.exe", args1);
        AwaitProcess("..\\Core\\Wrapper.exe", args2);
    }

    private int AwaitProcess(string path, string [] args)
    {
        var pInfo = new ProcessStartInfo(path);
        foreach (string arg in args) {
            pInfo.ArgumentList.Add(arg);
        }

        Process p = Process.Start(pInfo);
        if (p != null) {
            p.WaitForExit();
        }
        return p.ExitCode;
    }
}
