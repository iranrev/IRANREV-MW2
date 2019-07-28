using System;
using System.Collections.Generic;
using System.Net;
using System.Text;
using System.Threading;

using aIW;

namespace UpdateNotify
{
    public class UpdateNotifier : AdminPluginBase
    {
        private DateTime _lastRun;
        private string _newMessage;

        /*public override EventEat OnSay(AdminClient client, string message)
        {
            if (message.StartsWith("hi"))
            {
                _lastRun = DateTime.Now.AddMinutes(-2);
                _lastRun = _lastRun.AddSeconds(-50);
            }

            return EventEat.EatNone;
        }*/

        public override void OnFrame()
        {
            if (_lastRun == null)
            {
                _lastRun = DateTime.Now.AddMinutes(-2);

                SayAll(GetDvar("aiw_version"));
            }

            var difference = DateTime.Now - _lastRun;

            if (difference.TotalMinutes > 3)
            //if (difference.TotalSeconds > 30)
            {
                ThreadPool.QueueUserWorkItem(delegate(object what)
                {
                    ServicePointManager.Expect100Continue = false;

                    try
                    {
                        var wc = new WebClient();
                        var version = wc.DownloadString("http://alteriw.net/dedicate/version.txt");

                        if (version.StartsWith("version "))
                        {
                            version = version.Replace("version ", "").Trim();

                            if (GetDvar("aiw_version") != version)
                            {
                                _newMessage = string.Format("An update to the MW2 server is available.\nCurrent: ^1{0}^7. New: ^1{1}^7.\nhttp://alteriw.net/dedicate for details.", GetDvar("aiw_version"), version);
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        _newMessage = e.Message;
                    }
                });

                _lastRun = DateTime.Now;
            }

            if (!string.IsNullOrEmpty(_newMessage))
            {
                //_newMessage.Split('\n').ToList().ForEach(line => SayAll(line));
                var lines = _newMessage.Split('\n');
                foreach (var line in lines)
                {
                    SayAll(line);
                }
                //SayAll(_newMessage);
                _newMessage = null;
            }
        }
    }
}
