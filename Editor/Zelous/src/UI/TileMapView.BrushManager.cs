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
        public class Brush
        {
            // Boiler-plate equality code (annoying)
            public override bool Equals(Object obj) { return obj is Brush && this == (Brush)obj; }
            public override int GetHashCode() { Debug.Assert(false, "Not implemented yet"); return 0;  }
            public static bool operator !=(Brush lhs, Brush rhs) { return !(lhs == rhs); }

            public static bool operator ==(Brush lhs, Brush rhs)
            {
                if (Equals(lhs, null) && Equals(rhs, null))
                    return true;

                if (Equals(lhs, null) || Equals(rhs, null))
                    return false;

                if (lhs.NumTiles != rhs.NumTiles)
                    return false;

                if (lhs.Layers.Length != rhs.Layers.Length)
                    return false;

                //@TODO: More reason to create data types for layers of tile maps (each can have their own equality
                // functions)
                for (int layer = 0; layer < lhs.Layers.Length; ++layer)
                {
                    for (int tileX = 0; tileX < lhs.NumTiles.Width; ++tileX)
                    {
                        for (int tileY = 0; tileY < lhs.NumTiles.Height; ++tileY)
                        {
                            if (lhs.Layers[layer].TileMap[tileX, tileY] != rhs.Layers[layer].TileMap[tileX, tileY])
                                return false;
                        }
                    }
                }
                return true;
            }

            public class LayerData
            {
                // The source layer index of the tile indices. Note that for the "tile set views", this
                // value will always be 1 as they only have one layer, and does not represent the actual
                // "world map view" layer index.
                public int LayerIndex { get; set; }

                // 2d array of selected tile indices
                public int[,] TileMap { get; set; }
            }

            public Size NumTiles { get; set; } // Dimensions of this brush
            public LayerData[] Layers { get; set; } //Note: Index is not layer index
        };

        private partial class BrushManager
        {
            TileMapView mTileMapView;

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
                    layerData.TileMap = new int[selectedRect.Width, selectedRect.Height];

                    TileLayer srcLayer = mTileMapView.mTileLayers[layerData.LayerIndex];
                    int[,] targetTileMap = layerData.TileMap;

                    ArrayHelpers.CopyArray2d<int>(srcLayer.TileMap, selectedRect, ref targetTileMap, new Point(0, 0));
                }

                brush.NumTiles = selectedRect.Size;
                return brush;
            }

            public void PasteBrush(Point targetTilePos, TileMapView.Brush brush, ref TileMapView.Brush undoBrush)
            {
                // Clip brush to right/bottom edge of the TileMapView

                //@TODO: Add a property to TileMapView to make this easier
                Size tileMapViewNumTiles = new Size(
                    mTileMapView.mTileLayers[0].TileMap.GetLength(0),
                    mTileMapView.mTileLayers[0].TileMap.GetLength(1)
                    );

                Size brushNumTiles = new Size(); // We now use this var instead of brush.NumTiles
                brushNumTiles.Width = Math.Min(brush.NumTiles.Width, tileMapViewNumTiles.Width - targetTilePos.X);
                brushNumTiles.Height = Math.Min(brush.NumTiles.Height, tileMapViewNumTiles.Height - targetTilePos.Y);

                // Save the undo brush if requested
                if (undoBrush != null)
                {
                    // Create brush with layers that match those of the input brush, which don't necessarily
                    // match the actively selectable layers of our owner TileMapView
                    var layersToSelect = new bool[mTileMapView.SelectableLayers.Length]; // All false by default

                    foreach (Brush.LayerData layerData in brush.Layers)
                    {
                        layersToSelect[layerData.LayerIndex] = true;
                    }

                    Rectangle dstRect = new Rectangle(targetTilePos, brushNumTiles);
                    undoBrush = CreateBrushFromSelection(dstRect, layersToSelect);
                }

                // Now paste the brush
                foreach (Brush.LayerData layerData in brush.Layers)
                {
                    TileLayer targetLayer = mTileMapView.mTileLayers[layerData.LayerIndex];

                    // Technically, this srcRect should be the same for all layers
                    Rectangle srcRect = new Rectangle(new Point(0, 0), brushNumTiles);
                    int[,] targetTileMap = targetLayer.TileMap;

                    ArrayHelpers.CopyArray2d<int>(layerData.TileMap, srcRect, ref targetTileMap, targetTilePos);
                }

                mTileMapView.RedrawTileMap();
            }
        }
    }
}
