using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using aIW;

namespace TestPlugin
{
    public class TestPlugin : AdminPluginBase
    {
        public override EventEat OnSay(AdminClient client, string message)
        {
            if (message.StartsWith("!guid"))
            {
                SayAll("GUIDServ", "your GUID is " + client.GUID.ToString("x16"));
                Log.Debug("your GUID is " + client.GUID.ToString("x16"));
                return EventEat.EatGame;
            }

            if (message.StartsWith("!pm"))
            {
                SayTo(client, "PMServ", "hello there " + client.Name);
                return EventEat.EatNone;
            }

            return EventEat.EatNone;
        }
    }
}
