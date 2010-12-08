using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Diagnostics;
using System.Windows.Forms;
using System.IO;

namespace Zelous
{
    // Constants copied from game code
    public class GameConstants
    {
        public const int HwScreenSizeX = 256;
        public const int HwScreenSizeY = 192;

        public const int GameMetaTileSizeX = 16; // Doesn't need to be fixed for all - can be just background-specific...
        public const int GameMetaTileSizeY = 16;
        public const int GameNumScreenMetaTilesX = (HwScreenSizeX / GameMetaTileSizeX);
        public const int GameNumScreenMetaTilesY = (HwScreenSizeY / GameMetaTileSizeY);

        public static int EventLayerIndex = 4; //@TODO: Should we have an enum with all of these?
    }

    //@TODO: Move to BitOps.cs
    public class BitOps
    {
        // Returns an int with numBits 1s set
        // ex: GetNumBits(5) returns 11111b == 0x1F == 31
        public static int GenNumBits(int numBits)
        {
            return (1 << numBits) - 1;
        }
    }

    // A grouped set of tiles, 1 per layer
    public class TileSet
    {
        private string mFilePath;
        private Size mTileSize;
        private Point mNumTiles;
        private Image mImage;

        public TileSet(string filePath, int tileWidth, int tileHeight)
        {
            mFilePath = filePath;
            mTileSize = new Size(tileWidth, tileHeight);

            LoadImage(filePath);

            Debug.Assert(mImage.Size.Width % mTileSize.Width == 0);
            Debug.Assert(mImage.Size.Height % mTileSize.Height == 0);

            mNumTiles.X = mImage.Size.Width / mTileSize.Width;
            mNumTiles.Y = mImage.Size.Height / mTileSize.Height;
        }

        public void GetTileRect(int tileIndex, ref Rectangle rect)
        {
            Debug.Assert(rect != null);
            int numTilesX = Image.Width / TileSize.Width;
            int tileY = tileIndex / numTilesX;
            int tileX = tileIndex % numTilesX;
            
            rect.X = tileX * TileSize.Width;
            rect.Y = tileY * TileSize.Height;
            rect.Width = TileSize.Width;
            rect.Height = TileSize.Height;
        }

        public string FilePath { get { return mFilePath; } }
        public Image Image { get { return mImage; } }
        public Point NumTiles { get { return mNumTiles; } }
        public Size TileSize { get { return mTileSize; } }
        public Size ImageSize { get { return mImage.Size; } }
        public float AspectRatio { get { return ImageSize.Width / ImageSize.Height; } }

        private void LoadImage(string filePath)
        {
            // For optimal speed, we want to make sure we load the bitmap into an Image that
            // matches the pixel format the default Graphics instance used to render everything.

            Bitmap origBitmap = (Bitmap)Image.FromFile(mFilePath);

            mImage = new Bitmap(origBitmap.Width, origBitmap.Height, MainForm.Instance.CreateGraphics());

            // Now render the original image into mImage at target pixel format
            Graphics gfx = Graphics.FromImage(mImage);
            GraphicsHelpers.PrepareGraphics(gfx);

            // Also set the transparent color
            ImageAttributes imgAttr = new ImageAttributes();
            Color magenta = Color.FromArgb(255, 0, 255);
            imgAttr.SetColorKey(magenta, magenta);

            Rectangle r = new Rectangle(0, 0, origBitmap.Width, origBitmap.Height);
            gfx.DrawImage(origBitmap, r, r.X, r.Y, r.Width, r.Height, GraphicsUnit.Pixel, imgAttr);
       }
    }

    public enum SerializationType
    {
        Saving,
        Loading
    }

    // Represents a single Tile
    // It's a struct because we allocate large 2d arrays of these - consider making this a class,
    // as it's a pain that we can't pass references to Tiles around.
    public struct Tile : IEquatable<Tile>
    {
        // Index of this tile in the current tile set
        public int Index { get; set; }

        // Optional metadata tied to this Tile (ex: GameEvent).
        public object Metadata { get; set; }

        public override int GetHashCode()
        {
            Debug.Fail("not implemented");
            return base.GetHashCode();
        }

        public bool Equals(Tile rhs)
        {
            return Index == rhs.Index
                && ((Metadata != null && rhs.Metadata != null && Metadata.Equals(rhs.Metadata) || Metadata == rhs.Metadata));
        }    
    }

    // A single layer of tiles, which includes a TileSet and a 2d array of Tiles
    public class TileLayer
    {
        private Tile[,] mTileMap;
        private TileSet mTileSet;

        public void Init(int numTilesX, int numTilesY, TileSet tileSet)
        {
            mTileSet = tileSet;
            mTileMap = new Tile[numTilesX, numTilesY];
        }

        public TileSet TileSet { get { return mTileSet; } }
        public Tile[,] TileMap { get { return mTileMap; } }

        public Size SizePixels
        {
            get { return new Size(TileMap.GetLength(0) * TileSet.TileSize.Width, TileMap.GetLength(1) * TileSet.TileSize.Height); }
        }
    };

    // The world map, which is a collection of TileLayers
    public class WorldMap
    {
        public const int NumLayers = 5;
        private TileLayer[] mTileLayers = new TileLayer[NumLayers];
        int mNumScreensX, mNumScreensY;
        int mNumTilesX, mNumTilesY;

        public void Init(int numScreensX, int numScreensY, TileSet[] tileSets)
        {
            mNumScreensX = numScreensX;
            mNumScreensY = numScreensY;
            mNumTilesX = mNumScreensX * GameConstants.GameNumScreenMetaTilesX;
            mNumTilesY = mNumScreensY * GameConstants.GameNumScreenMetaTilesY;

            for (int i = 0; i < NumLayers; ++i)
            {
                mTileLayers[i] = new TileLayer();
                mTileLayers[i].Init(mNumTilesX, mNumTilesY, tileSets[i]);
            }
        }

        public TileLayer[] TileLayers { get { return mTileLayers; } }

        //@TODO: Rename - "serialization" is now confusing because of XML serialization via the SerializationMgr (Save/LoadGameMap?)
        //@TODO: Move this code out into a WorldMapSerializer class (or a partial class)
        //@TODO: Return success status
        //@TODO: Save to a temp file, then replace target file if successful (to avoid losing old map data if we fail during save)
        public void Serialize(SerializationType serializationType, string fileName)
        {
            // Note: When saving, we always save the latest version, but loading must be backwards
            // compatible with any version so that changes can be made to the map format without
            // deprecating existing maps. The game, however, only supports loading the latest
            // version, so old maps must be loaded and resaved within Zelous before loading in the
            // game.

            //=================================================================
            // Version  | Changes
            //=================================================================
            // 0        | initial (no versioning)
            // 1        | versioning (tag+version), character spawners
            // 2        | events

            const string MapTag = "WMAP";
            const UInt32 MapVer = 2;
            const UInt16 NumGameLayers = 3; // Doesn't match number of layers in editor
            const UInt16 ValidationMarker = 0xFFFF;

            //@TODO: Seralize number of screens X/Y (game should support variable sized maps)

            if (serializationType == SerializationType.Saving)
            {
                FileStream fs = new FileStream(fileName, FileMode.Create);
                BinaryWriter w = new BinaryWriter(fs);

                // Header
                w.Write((char[])MapTag.ToCharArray());
                w.Write((UInt32)MapVer);

                // Map data
                w.Write((UInt16)NumGameLayers); // Number of tile layers

                // Write out tile layers
                for (int layerIdx = 0; layerIdx < NumGameLayers; ++layerIdx)
                {
                    w.Write((UInt16)mNumTilesX);
                    w.Write((UInt16)mNumTilesY);

                    for (int y = 0; y < mNumTilesY; ++y)
                    {
                        for (int x = 0; x < mNumTilesX; ++x)
                        {
                            if (layerIdx < 2)
                            {
                                TileLayer tileLayer = TileLayers[layerIdx];
                                int tileIndex = tileLayer.TileMap[x, y].Index;
                                w.Write((UInt16)tileIndex);
                            }
                            else if (layerIdx == 2)
                            {
                                TileLayer collLayer = TileLayers[layerIdx];
                                TileLayer charLayer = TileLayers[layerIdx + 1];

                                // Write out data layer (collisions + spawners)
                                UInt16 collValue = (UInt16)collLayer.TileMap[x, y].Index;
                                UInt16 charValue = (UInt16)charLayer.TileMap[x, y].Index;

                                collValue <<= 0;
                                charValue <<= 2;
                                UInt16 dataLayerEntry = (UInt16)(collValue | charValue);

                                w.Write((UInt16)dataLayerEntry);
                            }
                            else
                            {
                                Debug.Fail("Invalid layer index");
                            }                            
                        }
                    }

                    w.Write((UInt16)ValidationMarker);
                }

                
                // Event data

                // Collect all events
                Dictionary<Point, GameEvent> gameEvents = new Dictionary<Point, GameEvent>();

                TileLayer eventLayer = TileLayers[GameConstants.EventLayerIndex];
                for (int y = 0; y < mNumTilesY; ++y)
                {
                    for (int x = 0; x < mNumTilesX; ++x)
                    {
                        if (eventLayer.TileMap[x, y].Index > 0)
                        {
                            GameEvent gameEvent = eventLayer.TileMap[x, y].Metadata as GameEvent;
                            Debug.Assert(gameEvent != null);

                            gameEvents[new Point(x, y)] = gameEvent;
                        }
                    }
                }

                // Write event data
                w.Write((UInt16)gameEvents.Count);
                foreach (KeyValuePair<Point, GameEvent> kvp in gameEvents)
                {
                    // Format: x(16) y(16) (event data)
                    w.Write((UInt16)kvp.Key.X);
                    w.Write((UInt16)kvp.Key.Y);
                    SaveGameEvent(w, kvp.Value);
                }
                w.Write((UInt16)ValidationMarker);

                w.Close();
                fs.Close();
            }
            else // Loading
            {
                FileStream fs = new FileStream(fileName, FileMode.Open, FileAccess.Read);
                BinaryReader r = new BinaryReader(fs);

                // Header
                string fileTag = new string(r.ReadChars(4));
                UInt32 fileVer = r.ReadUInt32();

                if (fileTag != MapTag)
                {
                    fileVer = 0; // First version had no map tag
                    fs.Position = 0; // Reset stream position
                }

                // Map data
                UInt16 numLayers = r.ReadUInt16();
                Debug.Assert(numLayers == NumGameLayers);

                for (int layerIdx = 0; layerIdx < NumGameLayers; ++layerIdx)
                {
                    UInt16 numTilesX = r.ReadUInt16();
                    UInt16 numTilesY = r.ReadUInt16();
                    Debug.Assert(numTilesX == mNumTilesX && numTilesY == mNumTilesY);

                    for (int y = 0; y < numTilesY; ++y)
                    {
                        for (int x = 0; x < numTilesX; ++x)
                        {
                            if (layerIdx < 2)
                            {
                                UInt16 tileIndex = r.ReadUInt16();

                                TileLayer tileLayer = TileLayers[layerIdx];
                                tileLayer.TileMap[x, y].Index = tileIndex;
                            }
                            else if (layerIdx == 2)
                            {
                                // Parse out collision + spawner value
                                UInt16 dataLayerEntry = r.ReadUInt16();
                                int collValue = (dataLayerEntry >> 0) & BitOps.GenNumBits(2);
                                int charValue = (dataLayerEntry >> 2) & BitOps.GenNumBits(6);

                                TileLayer collLayer = TileLayers[layerIdx];
                                TileLayer charLayer = TileLayers[layerIdx + 1];
                                collLayer.TileMap[x, y].Index = collValue;
                                charLayer.TileMap[x, y].Index = charValue;
                            }
                            else
                            {
                                Debug.Fail("Invalid layer index");
                            }                            
                        }
                    }

                    UInt16 marker = r.ReadUInt16();
                    Debug.Assert(marker == ValidationMarker);
                }

                // Load events
                TileLayer eventLayer = TileLayers[GameConstants.EventLayerIndex];
                if (fileVer >= 2)
                {
                    UInt16 numEvents = r.ReadUInt16();

                    for (int i = 0; i < numEvents; ++i)
                    {
                        Point tilePos = new Point();
                        tilePos.X = r.ReadUInt16();
                        tilePos.Y = r.ReadUInt16();
                        GameEvent gameEvent = LoadGameEvent(r);

                        eventLayer.TileMap[tilePos.X, tilePos.Y].Index = gameEvent.TypeId;
                        eventLayer.TileMap[tilePos.X, tilePos.Y].Metadata = gameEvent;
                    }

                    UInt16 marker = r.ReadUInt16();
                    Debug.Assert(marker == ValidationMarker);
                }

                r.Close();
                fs.Close();
            }
        }

        void SaveGameEvent(BinaryWriter w, GameEvent gameEvent)
        {
            // Format: id(16) version(16) numElems(16) elems*
            w.Write((UInt16)gameEvent.TypeId);
            w.Write((UInt16)gameEvent.Version);
            w.Write((UInt16)gameEvent.Elements.Count);

            foreach (GameEventElement elem in gameEvent.Elements)
            {
                if (elem.Value.GetType() == typeof(string))
                {
                    w.Write((char)'s');

                    string s = (string)elem.Value;
                    w.Write((UInt16)s.Length);
                    w.Write((char[])s.ToCharArray());
                }
                else if (elem.Value.GetType() == typeof(int))
                {
                    w.Write((char)'u');
                    w.Write((UInt16)((int)elem.Value));
                }
                else
                {
                    Debug.Fail("Unexpected value type");
                }
            }
        }

        GameEvent LoadGameEvent(BinaryReader r)
        {
            // For game events elements, we only store the data type and value, but not the name;
            // therefore, we can always load events as long as the order is matched. This means we
            // automatically support loading older events (version is older than current), as long
            // as old elements remain untouched, and new ones are added after the old ones.
            // We may want to provide a hook that allows game event specific code to load older
            // versions.

            UInt16 typeId = r.ReadUInt16();
            UInt16 version = r.ReadUInt16();

            // We always create the latest version of GameEvent, then update its elements
            // from the map file.
            GameEventFactory factory = MainForm.Instance.GameEventFactory;
            GameEvent gameEvent = factory.CreateNewGameEventFromPrototype(typeId);

            //@TODO: if (gameEvent.Version > version) call handler

            UInt16 numElems = r.ReadUInt16();

            List<GameEventElement> elements = gameEvent.GetElementsCopy(); // Copy
            for (int i = 0; i < numElems; ++i)
            {
                char elemType = r.ReadChar();
                
                object elemValue = null;
                switch (elemType)
                {
                    case 's':
                        UInt16 len = r.ReadUInt16();
                        elemValue = new string(r.ReadChars(len));
                        break;

                    case 'u':
                        elemValue = (int)r.ReadUInt16(); // Cast to supported GameEventElement type
                        break;
                }

                //@TODO: Throw an exception, catch outside of Serialize()
                Debug.Assert( elements[i].Value.GetType() == elemValue.GetType() ); // Does this work?

                elements[i] = elements[i].SetValue(elemValue); // Update
            }

            gameEvent = gameEvent.SetElements(elements); // Update
            return gameEvent;
        }
    };
}
