using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.Xml;
using System.Xml.Serialization;

namespace Zelous
{
    public struct GameTileSetGroup
    {
        public string Name { get; set; }

        public string BackgroundTiles { get; set; }
        public string ForegroundTiles { get; set; }
        //public string CharactersTiles { get; set; }
        //public string ItemsTiles { get; set; }
    }

    public class GameTileSetGroupMgr
    {
        private List<GameTileSetGroup> mTileSetGroups = new List<GameTileSetGroup>();

        public int NumGroups { get { return mTileSetGroups.Count; } }
        public GameTileSetGroup GetGroup(int index) { return mTileSetGroups[index]; }

        public void SaveGameTileSetsToXml(string filename)
        {
            XmlWriterSettings xmlWriterSettings = new XmlWriterSettings();
            xmlWriterSettings.Indent = true;
            xmlWriterSettings.IndentChars = "    ";
            XmlWriter xmlWriter = XmlWriter.Create(filename, xmlWriterSettings);

            XmlSerializer xmlSerializer = new XmlSerializer(mTileSetGroups.GetType());
            xmlSerializer.Serialize(xmlWriter, mTileSetGroups);

            xmlWriter.Close();
        }

        public void LoadGameTileSetsFromXml(string filename)
        {
            XmlReader xmlReader = XmlReader.Create(filename);

            XmlSerializer xmlSerializer = new XmlSerializer(mTileSetGroups.GetType());
            mTileSetGroups = (List<GameTileSetGroup>)xmlSerializer.Deserialize(xmlReader);
            xmlReader.Close();
        }
    }
}
