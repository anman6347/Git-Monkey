using System.Configuration;
using System.Data;
using System.Windows;
using System.Windows.Threading;
namespace Git_Monkey;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    public void App_DispatcherUnhandledException(object sender, DispatcherUnhandledExceptionEventArgs e)
    {
        MessageBox.Show(e.Exception.ToString());
    }
}

