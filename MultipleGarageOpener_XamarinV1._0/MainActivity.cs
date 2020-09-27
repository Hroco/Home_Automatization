using Android.App;
using Android.OS;
using Android.Support.V7.App;
using Android.Runtime;
using Android.Widget;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Threading;
using System.Net;

namespace MultipleGarageOpener_XamarinV1._0
{
    [Activity(Label = "@string/app_name", Theme = "@style/AppTheme.NoActionBar", MainLauncher = true)]
    public class MainActivity : AppCompatActivity
    {
        TextView txtNumber;
        delegate void SetTextCallback(string text);
        TcpClient clientSenzor;       
        NetworkStream nss;
        private Button btngaraz, btnzbc, btnzbp, btnpbc, btnpbp;

        protected override void OnCreate(Bundle savedInstanceState)
        {
            base.OnCreate(savedInstanceState);
            Xamarin.Essentials.Platform.Init(this, savedInstanceState);
            SetContentView(Resource.Layout.activity_main);

            clientSenzor = new TcpClient();
            txtNumber = FindViewById<TextView>(Resource.Id.textView1);
            btngaraz = FindViewById<Button>(Resource.Id.button1);
            btnzbc = FindViewById<Button>(Resource.Id.button2);
            btnzbp = FindViewById<Button>(Resource.Id.button3);
            btnpbc = FindViewById<Button>(Resource.Id.button4);
            btnpbp = FindViewById<Button>(Resource.Id.button5);
            txtNumber.Text = "0";
            ThreadPool.QueueUserWorkItem(o => DoWork());

            btngaraz.Click += delegate
            {
                // insert DDNS adress, port, code and true for DDNS adres oo false for local adress
                Arduino.RelayConnect("", 0000, "706", false);
            };

            btnzbc.Click += delegate
            {
                // insert DDNS adress, port, code and true for DDNS adres oo false for local adress
                Arduino.RelayConnect("", 0000, "828", true);
            };

            btnzbp.Click += delegate
            {
                // insert DDNS adress, port, code and true for DDNS adres oo false for local adress
                Arduino.RelayConnect("", 0000, "939", true);
            };

            btnpbc.Click += delegate
            {
                // insert DDNS adress, port, code and true for DDNS adres oo false for local adress
                Arduino.RelayConnect("", 0000, "606", true);
            };

            btnpbp.Click += delegate
            {
                // insert DDNS adress, port, code and true for DDNS adres oo false for local adress
                Arduino.RelayConnect("", 0000, "717", true);
            };

        }
        private void SetText(string text)
        {
            double location;
            location = Convert.ToDouble(text);
            location /= 3;
            location = Math.Ceiling(location);
            RunOnUiThread(() => txtNumber.Text = location.ToString());
        }

        public override void OnRequestPermissionsResult(int requestCode, string[] permissions, [GeneratedEnum] Android.Content.PM.Permission[] grantResults)
        {
            Xamarin.Essentials.Platform.OnRequestPermissionsResult(requestCode, permissions, grantResults);

            base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
        }

        public void DoWork()
        {
            byte[] bytes = new byte[1024];
            // insert DDNS adress and port
            clientSenzor.Connect("hroco.chickenkiller.com", 8889);
            while (true)
            {
                try
                {                  
                    if (clientSenzor.Connected)
                    {
                        nss = clientSenzor.GetStream();
                        int bytesRead = nss.Read(bytes, 0, bytes.Length);
                        string string_pozicia = Encoding.ASCII.GetString(bytes, 0, bytesRead);
                        string str = string_pozicia.Split((char)2, (char)3)[1];
                        //Console.WriteLine(str);
                        this.SetText(str);
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            }
        }    
    }
    public static class Arduino
    {
        public static bool state = true;
        public static bool RelayConnect(string ip, int port, string data, bool adresstype)
        {
            string m_ip = ip;
            int m_port = port;
            string m_data = data;
            bool m_adresstype = adresstype;
            bool complete = false;
            NetworkStream nsm;

            try
            {
                TcpClient clientMotor;
                clientMotor = new TcpClient();
                if (m_adresstype)
                {
                    clientMotor.Connect(m_ip, m_port);      // DDNS adress
                }
                else
                {
                    clientMotor.Connect(IPAddress.Parse(m_ip), m_port);     //Local IP adress
                }

                if (clientMotor.Connected)
                {
                    nsm = clientMotor.GetStream();
                    String start = null;
                    String end = null;
                    String str = null;
                    start = Convert.ToString((char)2);
                    end = Convert.ToString((char)3);
                    str = start + m_data + end;
                    byte[] byteTime = Encoding.ASCII.GetBytes(str);
                    Console.WriteLine("sending packet");
                    nsm.Write(byteTime, 0, byteTime.Length);
                    byte[] bytes = new byte[1024];
                    int bytesRead = nsm.Read(bytes, 0, bytes.Length);
                    string string_ctrlmsg = Encoding.ASCII.GetString(bytes, 0, bytesRead);
                    Console.WriteLine("recieved packet:");
                    Console.WriteLine(string_ctrlmsg);
                    if (string_ctrlmsg == m_data)
                    {                      
                        Console.WriteLine(string_ctrlmsg);
                        clientMotor.Close();
                        complete = true;
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            return complete;
        }

    }
}

