using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace Zelous
{
    public partial class TileMapView
    {
        // Populated and returned via event when client has selected tiles (multi-tile and multi-layer).
        public class Brush
        {
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

            private int GetNumSelectableLayers()
            {
                int numSelectableLayers = 0;
                foreach (bool isSeletableLayer in mTileMapView.mLayersToSelect)
                {
                    if (isSeletableLayer)
                        ++numSelectableLayers;
                }
                return numSelectableLayers;
            }

            public Brush CreateBrushFromSelection(Rectangle selectedRect)
            {
                Brush brush = new Brush();
                brush.Layers = new Brush.LayerData[GetNumSelectableLayers()];

                int currBrushLayerIndex = 0;
                for (int layerIndex = 0; layerIndex < mTileMapView.NumLayers; ++layerIndex)
                {
                    if (!mTileMapView.mLayersToSelect[layerIndex])
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

            public void PasteBrush(Point targetTilePos, TileMapView.Brush brush, ref TileMapView.Brush oldBrush)
            {
                if (oldBrush != null)
                {
                    //@TODO: Save/Set/Restore the currently active layers to match the ones from the input brush
                    // so that we create a brush with the right layers
                    Rectangle dstRect = new Rectangle(targetTilePos, brush.NumTiles);
                    oldBrush = CreateBrushFromSelection(dstRect);
                }

                foreach (Brush.LayerData layerData in brush.Layers)
                {
                    TileLayer targetLayer = mTileMapView.mTileLayers[layerData.LayerIndex];

                    // Technically, this srcRect should be the same for all layers
                    Rectangle srcRect = new Rectangle(0, 0, layerData.TileMap.GetLength(0), layerData.TileMap.GetLength(1));
                    int[,] targetTileMap = targetLayer.TileMap;

                    ArrayHelpers.CopyArray2d<int>(layerData.TileMap, srcRect, ref targetTileMap, targetTilePos);
                }

                mTileMapView.RedrawTileMap();
            }
        }
    }
}
