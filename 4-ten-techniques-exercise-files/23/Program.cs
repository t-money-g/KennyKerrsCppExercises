using System.Threading.Tasks;
using System.Net;
using System;
using System.Text;
using System.Net.WebSockets;
using System.Threading;

class Program
{
    static async Task Run()
    {
        var server = new HttpListener();
        server.Prefixes.Add("http://localhost:8000/ws/");
        server.Start();

        var http = await server.GetContextAsync();
        Console.WriteLine("Connected");

        if (!http.Request.IsWebSocketRequest)
        {
            http.Response.StatusCode = 400;
            http.Response.Close();
            Console.WriteLine("Not a websocket request");
            return;
        }

        var ws = await http.AcceptWebSocketAsync(null);

        for (var i = 0; i != 10; ++i)
        {
            await Task.Delay(1000);

            var time = DateTime.Now.ToLongTimeString();
            var buffer = Encoding.UTF8.GetBytes(time);
            var segment = new ArraySegment<byte>(buffer);

            Console.WriteLine(time);

            await ws.WebSocket.SendAsync(segment,
                                         WebSocketMessageType.Text,
                                         true,
                                         CancellationToken.None);
        }

        await ws.WebSocket.CloseAsync(WebSocketCloseStatus.NormalClosure,
                                      null,
                                      CancellationToken.None);
    }

    static void Main()
    {
        Run().Wait();
    }
}