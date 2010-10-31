using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using System.Xml;
using System.Xml.Serialization;

namespace Zelous
{
    // A single element of a GameEvent
    public struct GameEventElement
    {
        public GameEventElement(string name, string friendlyName, Object value)
            : this()
        {
            Name = name;
            Value = value;
            FriendlyName = friendlyName;
        }

        public GameEventElement Clone()
        {
            object valueCopy = null;
            if (Value.GetType().IsPrimitive) // Primitives support shallow copy
            {
                valueCopy = Value;
            }
            else // Must be an ICloneable
            {
                Debug.Assert(Value is ICloneable);
                valueCopy = ((ICloneable)Value).Clone();
            }

            return new GameEventElement(Name, FriendlyName, valueCopy);
        }

        // Key used to lookup value from UI control
        [XmlAttribute]
        public string Name { get; set; }

        // What gets shown on the UI
        [XmlAttribute]
        public string FriendlyName { get; set; }

        // The value of this element
        public object Value { get; set; }
    }

    // GameEvents are instantiated and tied to WorldMap tiles
    public class GameEvent
    {
        public GameEvent() // Default constructor required for xml serialization
        {
            TypeId = -1;
            FriendlyName = "UNSET";
        }

        public GameEvent(int typeId, string friendlyName)
        {
            TypeId = typeId;
            FriendlyName = friendlyName;
            Elements = new List<GameEventElement>();
        }

        public GameEvent Clone()
        {
            GameEvent gameEvent = new GameEvent(TypeId, FriendlyName);
            foreach (var elem in Elements)
            {
                gameEvent.Elements.Add(elem.Clone());
            }
            return gameEvent;
        }

        public void AddElement(string name, string friendlyName, Object value)
        {
            Elements.Add(new GameEventElement(name, friendlyName, value));
        }

        // Unique type identifier (used to map to data-driven event description (file) and to tile index)
        [XmlAttribute]
        public int TypeId { get; set; }

        [XmlAttribute]
        public string FriendlyName { get; set; }

        [XmlElementAttribute(ElementName = "Elem")]
        public List<GameEventElement> Elements { get; set; }
    }

    // GameEventFactory is an optional class that can be used to store GameEvent prototypes by integer id
    // from which it creates clones.
    public class GameEventFactory
    {
        private Dictionary<int, GameEvent> mEventByTypeId = new Dictionary<int, GameEvent>();

        public void RegisterGameEventPrototype(GameEvent prototype)
        {
            Debug.Assert(!mEventByTypeId.ContainsKey(prototype.TypeId));
            mEventByTypeId[prototype.TypeId] = prototype;
        }

        public GameEvent CreateNewGameEventFromPrototype(int typeId)
        {
            GameEvent prototype = mEventByTypeId[typeId];
            return prototype.Clone();
        }

        public List<GameEvent> GameEventListForSerialization
        {
            get
            {
                var list = new List<GameEvent>();
                foreach (KeyValuePair<int, GameEvent> pair in mEventByTypeId)
                {
                    list.Add(pair.Value);
                }
                return list;
            }
            set
            {
                mEventByTypeId.Clear();
                foreach (GameEvent gameEvent in value)
                {
                    RegisterGameEventPrototype(gameEvent);
                }
            }
        }
    }

    class GameEventHelpers
    {
        public static void SaveGameEventFactoryToXml(string filename, GameEventFactory gameEventFactory)
        {
            XmlWriterSettings xmlWriterSettings = new XmlWriterSettings();
            xmlWriterSettings.Indent = true;
            xmlWriterSettings.IndentChars = "    ";
            XmlWriter xmlWriter = XmlWriter.Create(filename, xmlWriterSettings);

            var gameEventlist = gameEventFactory.GameEventListForSerialization;
            XmlSerializer xmlSerializer = new XmlSerializer(gameEventlist.GetType());
            xmlSerializer.Serialize(xmlWriter, gameEventlist);

            xmlWriter.Close();
        }

        public static void LoadGameEventFactoryFromXml(string filename, ref GameEventFactory gameEventFactory)
        {
            XmlReader xmlReader = XmlReader.Create(filename);

            XmlSerializer xmlSerializer = new XmlSerializer(typeof(List<GameEvent>));
            var gameEventList = (List<GameEvent>)xmlSerializer.Deserialize(xmlReader);
            gameEventFactory.GameEventListForSerialization = gameEventList;

            xmlReader.Close();
        }

        // Creates and displays a dialog for the input GameEvent, and modifies it with values
        // input by the user. Returns true if user pressed OK on the dialog.
        public static bool DisplayGameEventDialog(ref GameEvent gameEvent)
        {
            var dlg = new DynamicDialog(null);
            dlg.Begin(gameEvent.FriendlyName, 300);

            foreach (GameEventElement elem in gameEvent.Elements)
            {
                if (elem.Value.GetType() == typeof(string))
                {
                    dlg.AddStringInput(elem.Name, elem.FriendlyName + ": ", (string)elem.Value);
                }
                else if (elem.Value.GetType() == typeof(int))
                {
                    dlg.AddIntegerInput(elem.Name, elem.FriendlyName + ": ", (int)elem.Value, 0, System.Int32.MaxValue);
                }
            }

            dlg.End();
            DialogResult result = dlg.ShowDialog();

            if (result != DialogResult.OK)
            {
                return false;
            }

            // Update event from dialog
            for (int i = 0; i < gameEvent.Elements.Count; ++i)
            {
                GameEventElement elem = gameEvent.Elements[i];
                elem.Value = dlg.GetValue(elem.Name);

                gameEvent.Elements[i] = elem; // Copy back value type
            }

            return true;
        }
    }
}
