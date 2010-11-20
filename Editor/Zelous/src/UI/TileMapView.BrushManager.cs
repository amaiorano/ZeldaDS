using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Diagnostics;

namespace Zelous
{
    public partial class TileMapView
    {
        // Populated and returned via event when client has selected tiles (multi-tile and multi-layer).
        public class Brush : IEquatable<Brush>
        {
            public bool Equals(Brush rhs)
            {
                if (rhs == null)
                    return false;

                if (this == rhs) // Refer to the same instance?
                    return true;

                if (NumTiles != rhs.NumTiles)
                    return false;

                if (Layers.Length != rhs.Layers.Length)
                    return false;

                // Deep compare
                //@TODO: More reason to create data types for layers of tile maps (each can have their own equality
                // functions)
                for (int layer = 0; layer < Layers.Length; ++layer)
                {
                    for (int tileX = 0; tileX < NumTiles.Width; ++tileX)
                    {
                        for (int tileY = 0; tileY < NumTiles.Height; ++tileY)
                        {
                            if (!Layers[layer].TileMap[tileX, tileY].Equals(rhs.Layers[layer].TileMap[tileX, tileY]))
                                return false;
                        }
                    }
                }

                return true;
            }

            // This is similar to class TileLayer but with an explicit LayerIndex. Perhaps we can add LayerIndex to TileLayer,
            // and just use it?
            public class LayerData
            {
                // The TileMapView layer index of the tile indices. Note that for the "tile set views", this
                // value will always be 0 as they only have one layer, and does not represent the actual
                // "world map view" layer index.
                public int LayerIndex { get; set; }

                // 2d array of selected tiles
                public Tile[,] TileMap { get; set; }
            }

            public Size NumTiles { get; set; } // Dimensions of this brush
            public LayerData[] Layers { get; set; } // Note: Index is not layer index (which is embedded in LayerData)
        };

        private partial class BrushManager
        {
            TileMapView mTileMapView; // Reference to owner TileMapView

            public BrushManager(TileMapView tileMapView)
            {
                mTileMapView = tileMapView;
            }

            private int GetNumSelectableLayers(bool[] selectableLayers)
            {
                int numSelectableLayers = 0;
                foreach (bool isSeletableLayer in selectableLayers)
                {
                    if (isSeletableLayer)
                        ++numSelectableLayers;
                }
                return numSelectableLayers;
            }

            public Brush CreateBrushFromSelection(Rectangle selectedRect, bool[] selectableLayers)
            {
                Brush brush = new Brush();
                brush.Layers = new Brush.LayerData[GetNumSelectableLayers(selectableLayers)];

                int currBrushLayerIndex = 0;
                for (int layerIndex = 0; layerIndex < mTileMapView.NumLayers; ++layerIndex)
                {
                    if (!selectableLayers[layerIndex])
                        continue;

                    // Create the layer data and add it
                    Brush.LayerData layerData = new Brush.LayerData();
                    brush.Layers[currBrushLayerIndex++] = layerData;

                    layerData.LayerIndex = layerIndex;
                    layerData.TileMap = new Tile[selectedRect.Width, selectedRect.Height];

                    TileLayer srcLayer = mTileMapView.mTileLayers[layerData.LayerIndex];
                    Tile[,] targetTileMap = layerData.TileMap;

                    ArrayHelpers.CopyArray2d<Tile>(srcLayer.TileMap, selectedRect, ref targetTileMap, new Point(0, 0));
                }

                brush.NumTiles = selectedRect.Size;
                return brush;
            }

            private Size GetClippedNumTiles(Point targetTilePos, Size numTiles)
            {
                //@TODO: Add a property to TileMapView to make this easier
                Size tileMapViewNumTiles = new Size(
                    mTileMapView.mTileLayers[0].TileMap.GetLength(0),
                    mTileMapView.mTileLayers[0].TileMap.GetLength(1)
                    );

                Size brushNumTiles = new Size();
                brushNumTiles.Width = Math.Min(numTiles.Width, tileMapViewNumTiles.Width - targetTilePos.X);
                brushNumTiles.Height = Math.Min(numTiles.Height, tileMapViewNumTiles.Height - targetTilePos.Y);
                return brushNumTiles;
            }

            public Brush CreateUndoBrush(Point targetTilePos, Brush baseBrush)
            {
                // Clip brush to right/bottom edge of the TileMapView
                Size brushNumTiles = GetClippedNumTiles(targetTilePos, baseBrush.NumTiles); // We now use this var instead of brush.NumTiles

                // Create brush with layers that match those of the input brush, which don't necessarily
                // match the actively selectable layers of our owner TileMapView
                var layersToSelect = new bool[mTileMapView.SelectableLayers.Length]; // All false by default

                foreach (Brush.LayerData layerData in baseBrush.Layers)
                {
                    layersToSelect[layerData.LayerIndex] = true;
                }

                Rectangle dstRect = new Rectangle(targetTilePos, brushNumTiles);
                return CreateBrushFromSelection(dstRect, layersToSelect);
            }

            public void PasteBrush(Point targetTilePos, Brush brush)
            {
                // Clip brush to right/bottom edge of the TileMapView
                Size brushNumTiles = GetClippedNumTiles(targetTilePos, brush.NumTiles); // We now use this var instead of brush.NumTiles

                // Paste the brush
                foreach (Brush.LayerData layerData in brush.Layers)
                {
                    TileLayer targetLayer = mTileMapView.mTileLayers[layerData.LayerIndex];

                    // Technically, this srcRect should be the same for all layers
                    Rectangle srcRect = new Rectangle(new Point(0, 0), brushNumTiles);
                    Tile[,] targetTileMap = targetLayer.TileMap;

                    ArrayHelpers.CopyArray2d<Tile>(layerData.TileMap, srcRect, ref targetTileMap, targetTilePos);
                }

                mTileMapView.RedrawTileMap();
            }
        }
    }
}
