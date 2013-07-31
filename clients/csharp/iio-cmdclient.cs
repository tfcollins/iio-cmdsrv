using System;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;

/*
 * C# IIO command client example library
 *
 * Copyright 2012-2013 Analog Devices Inc.
 *
 * Michael Hennerich <michael.hennerich@analog.com>
 * Licensed under the GPL-2.
 */

public class IIOCmdSrv
{
    Socket s = null;
    const int IIO_CMDSRV_MAX_RETVAL = 14;

    public enum ConnectStatus : byte
    {
        connected,
        refused,
        noanswer,
        nodevices,
        badname
    }

    public ConnectStatus Connect(string server)
    {
        string [] srv_port = server.Split(':');
		if (srv_port.Length == 2)
		{
			return Connect(srv_port[0], Convert.ToInt32(srv_port[1]));

		}
		else
		{
			return Connect(srv_port[0], 1234);
		}
	}

    public ConnectStatus Connect(string server, int port)
    {

        IPHostEntry hostEntry = null;
        try
        {
            hostEntry = Dns.GetHostEntry(server);
        }
        catch (SocketException e)
        {
            Console.WriteLine(e.Message);
            return ConnectStatus.badname;
        }
        bool sig = false;
        s = null;

        foreach (IPAddress address in hostEntry.AddressList)
        {
            IPEndPoint ipe = new IPEndPoint(address, port);
            Socket tempSocket =
                new Socket(ipe.AddressFamily, SocketType.Stream, ProtocolType.Tcp);


            IAsyncResult result = tempSocket.BeginConnect(ipe, null, null);

            sig = result.AsyncWaitHandle.WaitOne(1000, true);

            if (tempSocket.Connected)
            {
                s = tempSocket;
                break;
            }
            else
            {
                tempSocket.Close();
                continue;
            }
        }

		if (s != null)
		{
			if(Convert.ToDouble(this.Version()) >= 0.2)
			{
				return ConnectStatus.connected;
			}
			else
			{
				s.Close();
				throw new Exception("Unsupported Server Version");
			}
		}
		else
		{
            if (sig)
                return ConnectStatus.refused;
            return ConnectStatus.noanswer;
		}
	}

    public void Disconnect()
    {
        if (s != null)
        {
            if (s.Connected)
                Write("quit\n");
            s.Close();
            s = null;
        }
    }

    public string Read(string request, bool expect_retval)
    {
        Byte[] bytesSent = Encoding.ASCII.GetBytes(request);
        Byte[] bytesReceived = new Byte[256];
        string data = "";

        if (s == null || !s.Connected)
            return null;

        try
        {
            // Send request to the server.
            s.Send(bytesSent, bytesSent.Length, 0);

            string retval = "";
            int pos, bytes, ret;

            do
            {
                bytes = s.Receive(bytesReceived, bytesReceived.Length, 0);
                data = data + Encoding.ASCII.GetString(bytesReceived, 0, bytes);
                if (expect_retval)
                {
                    pos = data.IndexOf('\n');
                    if (pos > 0)
                    {
                        retval = data.Substring(0, pos);
                        ret = Convert.ToInt32(retval);
                        if (ret < 0)
                            return null;
                        //    throw new Exception("TargetError: " + retval + " (" + request + ")");

                        data = data.Substring(pos + 1);
                        expect_retval = false;
                    }
                }

                pos = data.IndexOf('\n');
                if (pos > 0)
                {
                    data = data.Substring(0, pos);
                    break;
                }

            } while (bytes > 0);
        }
        catch(SocketException e)
        {
            /* Network failed */
            Console.WriteLine(e.Message);
            data = null;
        }

        return data;

    }

    public int Write(string request)
    {
        Byte[] bytesSent = Encoding.ASCII.GetBytes(request);
        Byte[] bytesReceived = new Byte[IIO_CMDSRV_MAX_RETVAL];

        if (s == null || !s.Connected)
            return -1;

        try
        {
            // Send request to the server.
            s.Send(bytesSent, bytesSent.Length, 0);

            string retval = "";
            int bytes, pos, ret;

            do
            {
                bytes = s.Receive(bytesReceived, bytesReceived.Length, 0);
                retval = retval + Encoding.ASCII.GetString(bytesReceived, 0, bytes);
                pos = retval.IndexOf('\n');
                if (pos > 0)
                {
                    retval = retval.Substring(0, pos);
                    ret = Convert.ToInt32(retval);
                    if (ret < 0)
                        throw new Exception("TargetError: " + retval + " (" + request + ")");

                    return ret;
                }
            } while (bytes > 0);

        }
        catch (SocketException e)
        {
            Console.WriteLine(e.Message);
        }

        return -1;
    }

    public byte[] Sample(string device, uint count, uint itemsize)
    {
        string request = "sample " + device + String.Format(" {0}", count) + String.Format(" {0}\n", itemsize);
        Byte[] bytesSent = Encoding.ASCII.GetBytes(request);
        Byte[] data = new Byte[count * itemsize];
        Byte[] tmp = new Byte[count * itemsize];

        try
        {
            // Send request to the server.
            s.Send(bytesSent, bytesSent.Length, 0);

            int len = 0, len_tmp, do_ret = 1, ret, pos;
            string retval = "";

            do
            {
                len_tmp = s.Receive(tmp, data.Length - len, 0);
                if (len_tmp == 0)
                    break;

                if (do_ret == 1)
                {
                    retval = retval + Encoding.ASCII.GetString(tmp, 0, len_tmp);
                    pos = retval.IndexOf('\n');
                    if (pos > 0)
                    {
                        retval = retval.Substring(0, pos);
                        ret = Convert.ToInt32(retval);
                        if (ret < 0)
                            throw new Exception("TargetError: " + retval + " (" + request + ")");
                        do_ret = 0;
                        pos = pos + 1;

                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    pos = 0;
                }

                for (var i = pos; i < len_tmp; i++)
                {
                    data[len++] = tmp[i];
                }

            } while (len < data.Length);

        }
        catch (SocketException e)
        {
            /* Network failed */
            Console.WriteLine(e.Message);
            data = null;
        }
        return data;
    }

    public byte[] BufRead(string device, uint count, uint itemsize)
    {
        string request = "bufread " + device + String.Format(" {0}", count) + String.Format(" {0}\n", itemsize);
        Byte[] bytesSent = Encoding.ASCII.GetBytes(request);
        Byte[] tmp = new Byte[count * itemsize];
        Byte[] data = new Byte[count * itemsize];
        // Send request to the server.
        s.Send(bytesSent, bytesSent.Length, 0);

        int len = 0, len_tmp, do_ret = 1, rlen = 0, pos;
        string retval = "";

        do
        {
            len_tmp = s.Receive(tmp, tmp.Length - len, 0);
            if (len_tmp == 0)
                break;

            if (do_ret == 1)
            {
                retval = retval + Encoding.ASCII.GetString(tmp, 0, len_tmp);
                pos = retval.IndexOf('\n');
                if (pos > 0)
                {
                    retval = retval.Substring(0, pos);
                    rlen = Convert.ToInt32(retval);
                    if (rlen < 0)
                        return null;
                        //throw new Exception("TargetError: " + retval + " (" + request + ")");

                    do_ret = 0;
                    pos = pos + 1;

                }
                else
                {
                    continue;
                }
            }
            else
            {
                pos = 0;
            }

            for (var i = pos; i < len_tmp; i++)
            {
                data[len++] = tmp[i];
            }

        } while (len < rlen);

        return data;
    }

    public string Version()
    {
        return Read("version\n", false);
    }

    public IIODevice[] ListDevices()
    {
        IIODevice[] devices = null;
        string[] strDevices;
        string tmp;
        int i = 0;

        tmp = Read("show\n", true);
        if (tmp != null)
        {
            strDevices = tmp.Trim().Split(' ');
            devices = new IIODevice[strDevices.Length];

            foreach (var d in strDevices)
            {
                devices[i] = new IIODevice(this, d);
                i++;
            }
        }
        return devices;
    }

}

public class IIODevice
{
    string name;
    IIOCmdSrv server;
    bool need_update = true;
    uint bytes_per_sample;

    public IIODevice(IIOCmdSrv server, string name)
    {
        this.name = name;
        this.server = server;
    }

    public string Read(string attribute)
    {
        return server.Read("read " + name + " " + attribute + "\n", true);
    }

    public int Write(string attribute, string value)
    {
        if (attribute.StartsWith("scan_elements"))
            need_update = true;

        return server.Write("write " + name + " " + attribute + " " + value + "\n");
    }

    public byte[] Sample(uint count, uint itemsize)
    {
        return server.Sample(name, count, itemsize);
    }

    public byte[] Sample(uint count)
    {
        if (need_update)
        {
            bytes_per_sample = SizeFromScanElements();
            need_update = false;
        }
        return server.Sample(name, count, bytes_per_sample);
    }

    public int RegWrite(uint reg, uint val)
    {
        return server.Write("regwrite " + name + String.Format(" {0}", reg) + String.Format(" {0}\n", val));
    }

    public uint RegRead(uint reg)
    {
        return Convert.ToUInt32(server.Read("regread " + name + String.Format(" {0}\n", reg), true), 16);
    }

    public string ScanElementRead(string attribute)
    {
        return Read("scan_elements/" + attribute);
    }

    public int ScanElementWrite(string attribute, string value)
    {
        return Write("scan_elements/" + attribute, value);
    }

    public uint BufferGetLength()
    {
        return Convert.ToUInt32(Read("buffer/length"));
    }

    public int BufferSetLength(uint length)
    {
        return Write("buffer/length", String.Format("{0}", length));
    }

    public bool BufferIsEnabled()
    {
        return Convert.ToBoolean(Convert.ToUInt32(Read("buffer/enable")));
    }

    public int BufferEnable(bool state)
    {
        return Write("buffer/enable", state ? "1" : "0");
    }

    public string[] ListAttributes()
    {
        return server.Read("show " + name + " .\n", true).Trim().Split(' ');
    }

    public string[] ListScanElements()
    {
        try
        {
            return server.Read("show " + name + " scan_elements\n", true).Trim().Split(' ');
        }
        catch (Exception)
        {
            return new string[0];
        }
    }

    public uint SizeFromScanElements()
    {
        string[] scan_elements = ListScanElements();
        uint bits = 0;

        /* _type example le:u14/16>>0 */

        foreach (string elem in scan_elements)
        {
            if (elem.EndsWith("_en"))
            {
                if (ScanElementRead(elem).Equals("1"))
                {
                    string[] type = ScanElementRead(elem.Replace("_en", "_type")).Split(':', '/', '>', '<');
                    bits = bits + Convert.ToUInt32(type[2]);
                }
            }
        }
        Console.WriteLine("SizeFromScanElements: " + (bits / 8));
        return bits / 8;
    }

    public override string ToString()
    {
        return name;
    }

}
