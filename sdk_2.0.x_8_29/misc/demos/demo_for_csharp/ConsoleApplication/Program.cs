using System;
using System.Text;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;

namespace ConsoleApplication
{
    class Program
    {
        public static readonly int SKEGN_MESSAGE_TYPE_JSON = 1;
        public static readonly int SKEGN_MESSAGE_TYPE_BIN = 2;

        public static readonly int SKEGN_OPT_GET_VERSION = 1;
        public static readonly int SKEGN_OPT_GET_MODULES = 2;
        public static readonly int SKEGN_OPT_GET_TRAFFIC = 3;

        [DllImport("skegn.dll")]
        public static extern IntPtr skegn_new(string cfg);

        [DllImport("skegn.dll")]
        public static extern int skegn_delete(IntPtr engine);

        public delegate int skegn_callback(IntPtr usrdata, [MarshalAs(UnmanagedType.LPStr)] string id, int type, [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.I1, SizeParamIndex = 4)] byte[] message, int size);

        [DllImport("skegn.dll")]
        public static extern int skegn_start(IntPtr engine, string param, byte[] id, [MarshalAs(UnmanagedType.FunctionPtr)]skegn_callback callback, IntPtr usrdata);

        [DllImport("skegn.dll")]
        public static extern int skegn_feed(IntPtr engine, byte[] data, int size);

        [DllImport("skegn.dll")]
        public static extern int skegn_stop(IntPtr engine);

        [DllImport("skegn.dll")]
        public static extern int skegn_get_device_id(byte[] device_id);

        [DllImport("skegn.dll")]
        public static extern int skegn_cancel(IntPtr engine);

        [DllImport("skegn.dll")]
        public static extern int skegn_opt(IntPtr engine, int opt, byte[] data, int size);
		
        public static int callback(IntPtr usrdata, string record_id, int type, byte[] message, int size)
        {
            if (type == SKEGN_MESSAGE_TYPE_JSON)
            {
                Console.WriteLine("usrdata: " + (string)GCHandle.FromIntPtr(usrdata).Target);
                Console.WriteLine("record id: " + record_id);
                Console.WriteLine("size: " + size);
                Console.WriteLine("message: " + Encoding.UTF8.GetString(message));
            }
            return 0;
        }

        static void Main(string[] args)
        {
            string cfg = "{\"appKey\": \"1504160843000033\",\"secretKey\": \"72fe4340ef1cae4175d63113a777cd55\",\"provision\": \"skegn.provision\",\"cloud\": {\"server\": \"ws://api.17kouyu.com:8080\",\"serverList\": \"\"} }";
            string param = "{\"coreProvideType\":\"cloud\",\"app\":{\"userId\":\"xxx\"},\"audio\":{\"audioType\":\"wav\",\"sampleRate\":16000,\"channel\":1,\"sampleBytes\":2},\"request\":{\"coreType\":\"sent.eval\",\"refText\":\"I know the place very well.\",\"phoneme_output\":0}}";

            int rv, bytes;
            byte[] record_id = new byte[64];
            byte[] device_id = new byte[64];
            byte[] version = new byte[64];
            byte[] buf = new byte[4096];

            string usrdata = "this-is-usrdata";

            IntPtr engine;

            engine = skegn_new(cfg);
            if (engine == IntPtr.Zero)
            {
                Console.WriteLine("failed to create engine");
                return;
            }

            rv = skegn_start(engine, param, record_id, callback, GCHandle.ToIntPtr(GCHandle.Alloc(usrdata, GCHandleType.Normal)));
            if (rv != 0)
            {
                Console.WriteLine("start skegn failed");
                return;
            }

            Console.WriteLine("record id: " + Encoding.ASCII.GetString(record_id));

            FileStream fs = new FileStream("./I.wav", FileMode.Open, FileAccess.Read);
            fs.Seek(44, SeekOrigin.Begin); /* skip wav header */

            while ((bytes = fs.Read(buf, 0, buf.Length)) > 0)
            {
                rv = skegn_feed(engine, buf, bytes);
                if (rv != 0)
                {
                    Console.WriteLine("feed failed");
                    break;
                }
            }
            fs.Close();

            skegn_stop(engine);

            Thread.Sleep(30000);
            skegn_delete(engine);
        }
    }
}
