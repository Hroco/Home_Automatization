using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using Tizen.Wearable.CircularUI.Forms;
using System.Net;
using ElmSharp;
using System.IO;
using Tizen;
using Tizen.Applications;
using Tizen.Network.Connection;

namespace TizenWearableXamlGarageOpener
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class MainPage : CirclePage
    {
        public MainPage()
        {
            InitializeComponent();
        }
        async void OnButtonClicked(object sender, EventArgs args)
        {
            try
            {
                ConnectionItem currentConnection = ConnectionManager.CurrentConnection;
                //Log.Info(Utility.LOG_TAG, "connection(" + currentConnection.Type + ", " + currentConnection.State + ")");
                HttpWebRequest request = (HttpWebRequest)WebRequest.Create("http://DDNSAdress:PORT/SUBPAGE/"); //inser web page link

                // When a watch is paired with a mobile device, we can use WebProxy.
                if (currentConnection.Type == ConnectionType.Ethernet)
                {
                    string proxyAddr = ConnectionManager.GetProxy(AddressFamily.IPv4);
                    WebProxy myproxy = new WebProxy(proxyAddr, true);
                    request.Proxy = myproxy;
                }

                request.Method = "GET";
                HttpWebResponse response = (HttpWebResponse)request.GetResponse();
                //Log.Info(Utility.LOG_TAG, "StatusDescription: " + ((HttpWebResponse)response).StatusDescription);

                // Get the stream containing content returned by the server.
                Stream dataStream = response.GetResponseStream();
                // Open the stream using a StreamReader for easy access.
                StreamReader reader = new StreamReader(dataStream);
                // Read the content.  
                string responseFromServer = reader.ReadToEnd();
                // Display the content.  
                //Log.Info(Utility.LOG_TAG, "responseFromServer :" + responseFromServer);
                // Clean up the streams and the response.
                reader.Close();
                response.Close();
            }
            catch (Exception ex)
            {
                //Log.Info(Utility.LOG_TAG, "An error occurs : " + ex.Message);
            }
        }
    }
   
}