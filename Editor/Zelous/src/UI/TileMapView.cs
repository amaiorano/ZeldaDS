using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace Zelous
{
    public partial class TileMapView : UserControl
    {
        private TileLayer[] mTileLayers;
        private bool[] mLayersToRender;

        private Point mCameraPos = new Point(0, 0);
        private Size mTotalSizePixels = new Size(0, 0);
        private int mActiveLayer = 0;
        private bool mShowTileGrid = false;
        private bool mShowScreenGrid = false;

        private Point mLastTileSelectedPos = new Point(-1, -1);

        public TileMapView()
        {
            InitializeComponent();
        }

        //////////////////////////
        // Public methods
        //////////////////////////

        public void Init(string title, TileLayer[] tileLayers)
        {
            Title = title;

            Debug.Assert(tileLayers != null);
            Debug.Assert(tileLayers.Length > 0);
            mTileLayers = tileLayers;

            mLayersToRender = new bool[mTileLayers.Length];
            for (int i = 0; i < mLayersToRender.Length; ++i)
                mLayersToRender[i] = true;

            mTotalSizePixels = mTileLayers[0].SizePixels; //@TODO: assert that all layers return this same value

            mCheckBoxScreenGrid.Checked = mShowScreenGrid;
            mCheckBoxTileGrid.Checked = mShowTileGrid;

            UpdateScrollBarValues();
        }

        public string Title
        {
            set { mTitle.Text = value; }
            get { return mTitle.Text; }
        }

        public int ActiveLayer
        {
            set { mActiveLayer = value; }
            get { return mActiveLayer; }
        }

        public int RenderScale
        {
            set { mScaleCtrl.Value = value; }
            get { return (int)mScaleCtrl.Value; }
        }

        public bool ShowScreenGridOption
        {
            set { mCheckBoxScreenGrid.Visible = value; }
        }

        public Point LastTileSelectedPosition
        {
            set { mLastTileSelectedPos = value; }
            get { return mLastTileSelectedPos; }
        }

        public void SetLayerRenderable(int layer, bool render)
        {
            mLayersToRender[layer] = render;
            RedrawTileMap();
        }

        public void RedrawTileMap()
        {
            mViewPanel.Invalidate();
        }

        //////////////////////////
        // Public events
        //////////////////////////

        public class TileSelectEventArgs : EventArgs
        {
            public Point TileMapPos; // 2d index into tile map
            public TileLayer TileLayer; // which layer this tile is on

            // Helper property to get/set the selected tile index
            public int TileIndex
            {
                get { return TileLayer.TileMap[TileMapPos.X, TileMapPos.Y]; }
            }
        }

        public delegate void TileSelectEventHandler(TileMapView sender, TileSelectEventArgs e);
        public event TileSelectEventHandler TileSelected;
        
        //////////////////////////
        // Private methods
        //////////////////////////

        private Size RenderScaleAsSize
        {
            get { return new Size(RenderScale, RenderScale); }
        }

        private Size TotalSizePixels
        {
            get { return mTotalSizePixels; }
        }

        private Size ViewableSizePixels
        {
            get { return new Size(mViewPanel.ClientSize.Width / RenderScale, mViewPanel.ClientSize.Height / RenderScale); }
        }

        private void UpdateScrollBarValues()
        {
            if (mTileLayers == null)
                return;

            Size viewableSizePixels = ViewableSizePixels;
            Size totalSizePixels = TotalSizePixels;

            mScrollBarX.Visible = false;
            mScrollBarY.Visible = false;

            if (viewableSizePixels.Width < totalSizePixels.Width)
            {
                mScrollBarX.Visible = true;
                mScrollBarX.SmallChange = 1;
                mScrollBarX.LargeChange = viewableSizePixels.Width;
                mScrollBarX.Minimum = 0;
                mScrollBarX.Maximum = totalSizePixels.Width - 1;
            }

            if (viewableSizePixels.Height < totalSizePixels.Height)
            {
                mScrollBarY.Visible = true;
                mScrollBarY.SmallChange = 1;
                mScrollBarY.LargeChange = viewableSizePixels.Height;
                mScrollBarY.Minimum = 0;
                mScrollBarY.Maximum = totalSizePixels.Height - 1;
            }
        }

        private void HACK_ResetScrollAndCameraPositions()
        {
            mScrollBarX.Value = 0;
            mScrollBarY.Value = 0;
            mCameraPos.X = 0;
            mCameraPos.Y = 0;
            RedrawTileMap();
        }

        // Returns false if mouse position is out of map bounds
        private bool MousePosToTileMapPos(Point pt, TileLayer layer, out Point tileMapPos)
        {
            // Compute map-space (world) position (remove scale and offset by camera pos)
            Point finalPos = MathEx.Div(pt, RenderScaleAsSize) + (Size)mCameraPos;

            if (finalPos.X < 0 || finalPos.Y < 0 || finalPos.X >= layer.SizePixels.Width || finalPos.Y >= layer.SizePixels.Height)
            {
                tileMapPos = new Point(-1,  -1);
                return false;
            }

            tileMapPos = MathEx.Div(finalPos, layer.TileSet.TileSize);
            return true;
        }

        //////////////////////////
        // Event handlers
        //////////////////////////

        private void mViewPanel_Paint(object sender, PaintEventArgs e)
        {
            if (mTileLayers == null)
                return;

            Rectangle dstRect = new Rectangle();
            Rectangle srcRect = new Rectangle();
            Rectangle selectedRect = new Rectangle();

            Graphics tgtGfx = e.Graphics;
            GraphicsHelpers.PrepareGraphics(tgtGfx);

            tgtGfx.Clear(Color.Gray);
            tgtGfx.ScaleTransform((float)RenderScale, (float)RenderScale);

            // We assume (and assert) that all layers have the same tile size. This doesn't have to be
            // true, but it simplifies a lot of stuff, like drawing the grid (below)
            Size tileSize = mTileLayers[0].TileSet.TileSize;

            Point firstTile = MathEx.Div(mCameraPos, tileSize);
            Point offset = MathEx.Mod(mCameraPos, tileSize);
            Point startPos = new Point(-offset.X, -offset.Y);
            dstRect.Size = tileSize;

            // Draw only enough tiles to fit the client view. Note that since the client area is not
            // exactly a multiple of a tile's size, we must round up (ciel) the result of the division.
            Point numTilesToDraw = new Point();
            numTilesToDraw.X = (int)Math.Ceiling((double)mViewPanel.ClientSize.Width / RenderScale / tileSize.Width) + 1;
            numTilesToDraw.Y = (int)Math.Ceiling((double)mViewPanel.ClientSize.Height / RenderScale / tileSize.Height) + 1;

            for (int layerIndex = 0; layerIndex < mTileLayers.Length; ++layerIndex)
            {
                if (!mLayersToRender[layerIndex])
                    continue;

                TileLayer layer = mTileLayers[layerIndex];
                TileSet tileSet = layer.TileSet;
                Debug.Assert(tileSet.TileSize == tileSize);

                // Draw tiles
                for (int x = 0; x < numTilesToDraw.X; ++x)
                {
                    for (int y = 0; y < numTilesToDraw.Y; ++y)
                    {
                        // If client area is larger than entire map, this condition will be true. Note that we could
                        // also just clamp numTilesToDraw above (but this is easier)
                        if (firstTile.X + x >= layer.TileMap.GetLength(0) || firstTile.Y + y >= layer.TileMap.GetLength(1))
                            break;

                        int tileIndex = layer.TileMap[firstTile.X + x, firstTile.Y + y];
                        tileSet.GetTileRect(tileIndex, ref srcRect);

                        dstRect.X = startPos.X + (x * dstRect.Size.Width);
                        dstRect.Y = startPos.Y + (y * dstRect.Size.Height);

                        tgtGfx.DrawImage(tileSet.Image, dstRect, srcRect, GraphicsUnit.Pixel);

                        //If we're currently in the active layer, and drawing the last selected tile
                        if (layerIndex == mActiveLayer && 
                            (mLastTileSelectedPos.X == (firstTile.X + x) && mLastTileSelectedPos.Y == (firstTile.Y + y)) )
                        {
                            selectedRect = dstRect;
                        }
                    }
                }
            } // for each layer

            if (mShowTileGrid || mShowScreenGrid)
            {
                Pen tileGridPen = new Pen(Color.DarkSlateGray, 1);
                Pen screenGridPen = new Pen(Color.Black, 1);

                TileLayer layer = mTileLayers[0]; // Only use the first tile layer

                for (int x = 0; x < numTilesToDraw.X; ++x)
                {
                    for (int y = 0; y < numTilesToDraw.Y; ++y)
                    {
                        int currTileX = firstTile.X + x;
                        int currTileY = firstTile.Y + y;

                        if (currTileX >= layer.TileMap.GetLength(0) || currTileY >= layer.TileMap.GetLength(1))
                            break;

                        dstRect.X = startPos.X + (x * dstRect.Size.Width);
                        dstRect.Y = startPos.Y + (y * dstRect.Size.Height);

                        if (mShowTileGrid)
                        {
                            if (x == 0)
                            {
                                tgtGfx.DrawLine(tileGridPen, dstRect.Left + 0.1f, dstRect.Top, dstRect.Left + 0.1f, dstRect.Bottom);
                            }

                            if (y == 0)
                            {
                                tgtGfx.DrawLine(tileGridPen, dstRect.Left, dstRect.Top + 0.1f, dstRect.Right, dstRect.Top + 0.1f);
                            }

                            tgtGfx.DrawLine(tileGridPen, dstRect.Left, dstRect.Bottom, dstRect.Right, dstRect.Bottom);
                            tgtGfx.DrawLine(tileGridPen, dstRect.Right, dstRect.Top, dstRect.Right, dstRect.Bottom);
                        }

                        if (mShowScreenGrid)
                        {
                            if (currTileX % GameConstants.GameNumScreenMetaTilesX == 0)
                            {
                                tgtGfx.DrawLine(screenGridPen, dstRect.Left + 0.1f, dstRect.Top, dstRect.Left + 0.1f, dstRect.Bottom);
                            }

                            if (currTileY % GameConstants.GameNumScreenMetaTilesY == 0)
                            {
                                tgtGfx.DrawLine(screenGridPen, dstRect.Left, dstRect.Top + 0.1f, dstRect.Right, dstRect.Top + 0.1f);
                            }
                        }
                    }
                }

                tileGridPen.Dispose();
                screenGridPen.Dispose();
            } // end draw grids

            //Draw box around currently selected tile
            if (mLastTileSelectedPos.X > -1 && mLastTileSelectedPos.Y > -1)
            {
                Pen tileGridPen = new Pen(Color.Red, 1);
                tgtGfx.DrawRectangle(tileGridPen, selectedRect);
                tileGridPen.Dispose();
            }
            
        }

        private void mScaleCtrl_ValueChanged(object sender, EventArgs e)
        {
            RedrawTileMap();
            UpdateScrollBarValues();
        }

        private void mViewPanel_Resize(object sender, EventArgs e)
        {
            // Temp hack until I implement logic for the camera to follow
            // the bottom/right edges of the view during resize
            HACK_ResetScrollAndCameraPositions();

            UpdateScrollBarValues();
        }

        private void mScrollBarXY_ValueChanged(object sender, EventArgs e)
        {
            // Update camera from scrollbar values
            mCameraPos.X = mScrollBarX.Value;
            mCameraPos.Y = mScrollBarY.Value;

            RedrawTileMap();
        }

        private int mLastTileSelectedIndex = -1;
        
        private void mViewPanel_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                if (TileSelected == null)
                    return;

                TileLayer layer = mTileLayers[mActiveLayer];
                Point tileMapPos;
                if (!MousePosToTileMapPos(e.Location, layer, out tileMapPos))
                    return; // Happens if mouse location is out of map bounds

                TileSelectEventArgs args = new TileSelectEventArgs();
                args.TileLayer = layer;
                args.TileMapPos = tileMapPos;

                if (args.TileIndex != mLastTileSelectedIndex)
                {
                    TileSelected(this, args);
                    //Console.WriteLine("MouseDown: " + e.Location + ", TileMapPos: " + tileMapPos + ", TileIndex: " + args.TileIndex);
                    mLastTileSelectedIndex = args.TileIndex;
                }
            }
        }

        private void mViewPanel_MouseMove(object sender, MouseEventArgs e)
        {
            if (mLastTileSelectedIndex != -1)
            {
                mViewPanel_MouseDown(sender, e);
            }
        }

        private void mViewPanel_MouseUp(object sender, MouseEventArgs e)
        {
            mLastTileSelectedIndex = -1;
        }

        private void mCheckBoxScreenGrid_CheckedChanged(object sender, EventArgs e)
        {
            mShowScreenGrid = mCheckBoxScreenGrid.Checked;
            RedrawTileMap();
        }

        private void mCheckBoxTileGrid_CheckedChanged(object sender, EventArgs e)
        {
            mShowTileGrid = mCheckBoxTileGrid.Checked;
            RedrawTileMap();
        }
    }

    // Use this Panel when you want to handle the painting of the panel
    // without any flicker (uses backbuffering and no background erasing)
    public class NoFlickerPanel : System.Windows.Forms.Panel
    {
        public NoFlickerPanel()
        {
            SetStyle(ControlStyles.DoubleBuffer, true);
            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            SetStyle(ControlStyles.UserPaint, true); // Probably unnecessary
        }
    }
}
