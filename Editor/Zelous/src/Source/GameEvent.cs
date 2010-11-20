using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.Linq;

namespace Zelous
{
    // A single element of a GameEvent - immutable
    //@TODO: Any point in this being a struct?
    public struct GameEventElement : IEquatable<GameEventElement>
    {
        public GameEventElement(string name, string friendlyName, Object value) : this()
        {
            Name = name;
            FriendlyName = friendlyName;
            Value = value;
        }

        // Key used to lookup value from UI control
        [XmlIgnore]
        public string Name { get; private set; }
        
        // What gets shown on the UI
        [XmlIgnore]
        public string FriendlyName { get; private set; }

        // The value of this element
        [XmlIgnore]
        public object Value { get; private set; }

        public bool Equals(GameEventElement rhs)
        {
            return Name == rhs.Name
                && FriendlyName == rhs.FriendlyName
                && Value.Equals(rhs.Value);
        }

        // Returns new GameEventElement input value set
        public GameEventElement SetValue(object value)
        {
            return new GameEventElement(Name, FriendlyName, value);
        }
        
        // Properties for XML serialization
        [XmlAttribute(AttributeName="Name")]
        public string __ForXml_Name { get { return Name; } set { Name = value; } }

        [XmlAttribute(AttributeName = "FriendlyName")]
        public string __ForXml_FriendlyName { get { return FriendlyName; } set { FriendlyName = value; } }

        [XmlElement(ElementName = "Value")]
        public object __ForXml_Value { get { return Value; } set { Value = value; } }
    }

    // GameEvents are instantiated and tied to WorldMap tiles - immutable
    public class GameEvent : IEquatable<GameEvent>
    {
        private List<GameEventElement> mElements = new List<GameEventElement>();

        public GameEvent() // Default constructor required for xml serialization
        {
            TypeId = -1;
            FriendlyName = "UNSET";
        }

        public GameEvent(int typeId, string friendlyName, List<GameEventElement> elements)
        {
            TypeId = typeId;
            FriendlyName = friendlyName;
            mElements = elements;
        }

        public bool Equals(GameEvent rhs)
        {
            return TypeId == rhs.TypeId
                && FriendlyName == rhs.FriendlyName
                && mElements.SequenceEqual(rhs.mElements);
        }

        public override bool Equals(object rhs)
        {
            if (rhs == null || (rhs as GameEvent == null))
                return false;

            return Equals(rhs as GameEvent);
        }

        public override int GetHashCode()
        {
            Debug.Fail("not implemented");
            return base.GetHashCode();
        }

        // Returns a new GameEvent with the input elements
        public GameEvent SetElements(List<GameEventElement> elements)
        {
            return new GameEvent(TypeId, FriendlyName, elements);
        }

        // Unique type identifier (used to map to data-driven event description (file) and to tile index)
        [XmlIgnore]
        public int TypeId { get; private set; }

        [XmlIgnore]
        public string FriendlyName { get; private set; }

        [XmlIgnore]
        public ReadOnlyCollection<GameEventElement> Elements { get { return new ReadOnlyCollection<GameEventElement>(mElements); } }

        // Properties for XML serialization
        [XmlAttribute(AttributeName = "TypeId")]
        public int __ForXml_TypeId { get { return TypeId; } set { TypeId = value; } }

        [XmlAttribute(AttributeName="FriendlyName")]
        public string __ForXml_FriendlyName { get { return FriendlyName; } set { FriendlyName = value; } }

        [XmlElementAttribute(ElementName = "Elem")]
        public List<GameEventElement> __ForXml_Elements { get { return mElements; } set { mElements = value; } }
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
            // Since GameEvents are immutable, we just return the prototype directly (it cannot be modified)
            GameEvent prototype = mEventByTypeId[typeId];
            return prototype;
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

    public class GameEventHelpers
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

        public static void LoadGameEventFactoryFromXml(string filename, GameEventFactory gameEventFactory)
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

            // Update GameEvent from dialog - note that because GameEvent is immutable, we build an external list
            // of modified GameEventElements, then call GameEvent.SetElements() to get an updated copy of the
            // GameEvent.
            //@TODO: We could use a GameEventBuilder here (that is initialized to an existing GameEvent)
            List<GameEventElement> elems = new List<GameEventElement>();

            for (int i = 0; i < gameEvent.Elements.Count; ++i)
            {
                GameEventElement elem = gameEvent.Elements[i]; // Copy existing
                elem = elem.SetValue(dlg.GetValue(elem.Name)); // Modify value
                elems.Add(elem); // Add to local list
            }

            gameEvent = gameEvent.SetElements(elems);

            return true;
        }
    }
}
