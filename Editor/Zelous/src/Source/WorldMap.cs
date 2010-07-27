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
    }

    //@TODO: Move to BitOps.cs
    public class BitOps
    {
        // Returns an int with numBits 1s set
        public static int GenNumBits(int numBits)
        {
            return (1 << numBits) - 1;
        }
    }

    // These classes match the same classes in the game code

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

    public class TileLayer
    {
        private int[,] mTileMap; // 2d array (C# is awesome)
        private TileSet mTileSet;

        public void Init(int numTilesX, int numTilesY, TileSet tileSet)
        {
            mTileSet = tileSet;

            //Random rand = new Random(0); //@TEMP TEMP TEMP

            mTileMap = new int[numTilesX, numTilesY];
            for (int x = 0; x < mTileMap.GetLength(0); ++x)
            {
                for (int y = 0; y < mTileMap.GetLength(1); ++y)
                {
                    mTileMap[x, y] = 0;
                }
            }
        }

        public TileSet TileSet { get { return mTileSet; } }
        public int[,] TileMap { get { return mTileMap; } }

        public Size SizePixels
        {
            get { return new Size(TileMap.GetLength(0) * TileSet.TileSize.Width, TileMap.GetLength(1) * TileSet.TileSize.Height); }
        }
    };

    public class WorldMap
    {
        public const int NumLayers = 4;
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

            const string MapTag = "WMAP";
            const UInt32 MapVer = 1;
            const int NumGameLayers = 3; // Doesn't match number of layers in editor

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
                                int tileIndex = tileLayer.TileMap[x, y];
                                w.Write((UInt16)tileIndex);
                            }
                            else if (layerIdx == 2)
                            {
                                TileLayer collLayer = TileLayers[layerIdx];
                                TileLayer charLayer = TileLayers[layerIdx + 1];

                                // Write out data layer (collisions + spawners)
                                UInt16 collValue = (UInt16)collLayer.TileMap[x, y];
                                UInt16 charValue = (UInt16)charLayer.TileMap[x, y];

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

                    w.Write((UInt16)0xFFFF); // Data validation marker
                }

                w.Close();
                fs.Close();
            }
            else // Loading
            {
                FileStream fs = new FileStream(fileName, FileMode.Open);
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
                                tileLayer.TileMap[x, y] = tileIndex;
                            }
                            else if (layerIdx == 2)
                            {
                                // Parse out collision + spawner value
                                UInt16 dataLayerEntry = r.ReadUInt16();
                                int collValue = (dataLayerEntry >> 0) & BitOps.GenNumBits(2);
                                int charValue = (dataLayerEntry >> 2) & BitOps.GenNumBits(6);

                                TileLayer collLayer = TileLayers[layerIdx];
                                TileLayer charLayer = TileLayers[layerIdx + 1];
                                collLayer.TileMap[x, y] = collValue;
                                charLayer.TileMap[x, y] = charValue;
                            }
                            else
                            {
                                Debug.Fail("Invalid layer index");
                            }                            
                        }
                    }

                    UInt16 marker = r.ReadUInt16();
                    Debug.Assert(marker == 0xFFFF);
                }

                r.Close();
                fs.Close();
            }
        }
    };
}
