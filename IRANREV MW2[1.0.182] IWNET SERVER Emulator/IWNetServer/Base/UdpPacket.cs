﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace IWNetServer
{
    public class UdpPacket : IDisposable
    {
        private MemoryStream _inStream;
        private BinaryReader _inReader;

        private IPEndPoint _ipEndpoint;
        private Socket _socket;

        private string _server;

        private bool _secure;

        public UdpPacket(byte[] input, int length, IPEndPoint ipEndpoint, Socket socket, string server, bool encrypted)
        {
            _inStream = new MemoryStream();
            _inStream.Write(input, 0, length);
            _inStream.Position = 0;

            _inReader = new BinaryReader(_inStream);

            _ipEndpoint = ipEndpoint;
            _socket = socket;

            _server = server;

            _secure = encrypted;
        }

        ~UdpPacket()
        {
            Dispose();
        }

        public void Dispose()
        {
            _inReader.Close();
        }

        public BinaryReader GetReader()
        {
            return _inReader;
        }

        public IPEndPoint GetSource()
        {
            return _ipEndpoint;
        }

        public UdpResponse MakeResponse()
        {
            return new UdpResponse(_ipEndpoint, _socket, _server);
        }

        public bool Secure
        {
            get
            {
                return _secure;
            }
        }

    }

    public class UdpPacketReceivedEventArgs : EventArgs
    {
        public UdpPacketReceivedEventArgs(UdpPacket packet)
        {
            Packet = packet;
        }

        public UdpPacket Packet
        {
            get;
            private set;
        }
    }
}
