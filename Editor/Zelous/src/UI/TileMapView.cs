using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Xml.Serialization;

namespace Zelous
{
    public partial class TileMapView : UserControl, ISerializationClient
    {
        private static Point InvalidPoint = new Point(-1, -1);

        // Consider grouping layer-specific data together (array of struct)
        private TileLayer[] mTileLayers;
        private bool[] mLayersToRender;
        private bool[] mLayersToSelect; //@NOTE: Technically we don't need this, can just make SelectableLayers return mLayersToRender

        private Point mCameraPos = new Point(0, 0);
        private Size mTotalSizePixels;
        
        private EditMode mEditMode = EditMode.SelectBrush;
        private bool mAltEditModeActive = false;
        private Dictionary<EditMode, EditMode> mEditModeAltMap; // Maps EditMode to it's Alt EditMode (mode that is selected while Alt is being held)
        
        private BrushManager mBrushManager;

        // This inner class is implemented in a separate file
        private partial class BrushManager
        {
        }

        public enum EditMode
        {
            PasteBrush,
            SelectBrush
        }

        public EditMode ActiveEditMode
        {
            private set
            {
                mEditMode = value;
                OnEditModeChanged();
            }
            get
            {
                return mEditMode;
            }
        }

        public int NumLayers { get; private set; }

        public TileMapView()
        {
            //@TODO: Replace with "is design time" check, happens only from designer.
            // This stupid check is required for the designer to display this control
            if (MainForm.Instance != null)
            {
                MainForm.Instance.AppSettingsMgr.RegisterSerializable(this, typeof(Settings));
            }
            mBrushManager = new BrushManager(this);

            mEditModeAltMap = new Dictionary<EditMode, EditMode>();
            mEditModeAltMap[EditMode.PasteBrush] = EditMode.SelectBrush;

            InitializeComponent();
        }

        //@TODO: Move to TileMapView.Settings.cs (along with ISerializationClient)
        [XmlRootAttribute(Namespace = "TileMapViewSettings")]
        public class Settings
        {
            public decimal mScale;
            public int mScrollBarX; //@TODO: If form is saved maximized, these get crushed due to HACK_ResetScrollAndCameraPositions()
            public int mScrollBarY;
            public bool mCheckBoxScreenGrid;
            public bool mCheckBoxTileGrid;
        }

        void ISerializationClient.OnSerialize(Serializer serializer, ref object saveData)
        {
            Settings settings = (Settings)saveData;
            serializer.AssignProperty(ref settings.mScale, "Value", mScaleCtrl);
            serializer.AssignProperty(ref settings.mScrollBarX, "Value", mScrollBarX);
            serializer.AssignProperty(ref settings.mScrollBarY, "Value", mScrollBarY);
            serializer.AssignProperty(ref settings.mCheckBoxScreenGrid, "Checked", mCheckBoxScreenGrid);
            serializer.AssignProperty(ref settings.mCheckBoxTileGrid, "Checked", mCheckBoxTileGrid);
        }

        //////////////////////////
        // Public methods
        //////////////////////////

        // Called only once at start up
        public void Init(string title, int numLayers)
        {
            Title = title;

            Debug.Assert(numLayers > 0);
            NumLayers = numLayers;

            mLayersToRender = new bool[NumLayers];
            mLayersToSelect = new bool[NumLayers];
            for (int i = 0; i < mLayersToRender.Length; ++i)
            {
                mLayersToRender[i] = true;
                mLayersToSelect[i] = mLayersToRender[i];
            }

            //@HACK: Temporary until I figure out a better way
            ActiveEditMode = NumLayers == 1 ? EditMode.SelectBrush : EditMode.PasteBrush;
            if (NumLayers == 1)
            {
                mRadioButton_PasteBrush.Enabled = false;
            }
        }

        // Called multiple times (new map, load map) - avoid resetting GUI vars
        // to preserve user settings (i.e. state of checkboxes, etc)
        public void Reset(TileLayer[] tileLayers)
        {
            Debug.Assert(tileLayers != null);
            Debug.Assert(tileLayers.Length == NumLayers);
            mTileLayers = tileLayers; // Can now use NumLayers property

            mTotalSizePixels = mTileLayers[0].SizePixels; //@TODO: assert that all layers return this same value
            UpdateScrollBarValues();

            // Clear events
            OnBrushCreated = null;
            OnBrushPasteRequested = null;

            ResetSelection(true);
        }

        public string Title
        {
            set { mTitle.Text = value; }
            get { return mTitle.Text; }
        }

        public int RenderScale
        {
            get { return (int)mScaleCtrl.Value; }
        }

        public bool ShowScreenGridOption
        {
            set { mCheckBoxScreenGrid.Visible = value; }
        }

        public bool ShowTileGrid
        {
            get { return mCheckBoxTileGrid.Checked; }
        }

        public bool ShowScreenGrid
        {
            get { return mCheckBoxScreenGrid.Checked; }
        }

        public bool[] RenderableLayers
        {
            get { return mLayersToRender; }
        }

        public bool[] SelectableLayers
        {
            get { return mLayersToSelect; }
        }

        public void RedrawTileMap()
        {
            mViewPanel.Invalidate();
        }

        public void PasteBrush(Point targetTilePos, TileMapView.Brush brush, ref TileMapView.Brush undoBrush)
        {
            mBrushManager.PasteBrush(targetTilePos, brush, ref undoBrush);
        }

        public void OnGlobalKeyDown(KeyEventArgs e)
        {
            // Handle changing to alternate edit mode
            Point mousePos = PointToClient(Control.MousePosition);
            bool isMouseOverPanel = mViewPanel.Bounds.Contains(mousePos);

            if (e.Modifiers == Keys.Alt && isMouseOverPanel && mEditModeAltMap.ContainsKey(ActiveEditMode))
            {
                ActiveEditMode = mEditModeAltMap[ActiveEditMode];
                mAltEditModeActive = true;
                OnEditModeChanged();
                e.SuppressKeyPress = true;
            }
        }

        public void OnGlobalKeyUp(KeyEventArgs e)
        {
            // Handle going back from alternate edit mode
            if (!e.Alt && mAltEditModeActive)
            {
                // Find current alt edit mode in values, and go back to its key
                foreach (KeyValuePair<EditMode, EditMode> entry in mEditModeAltMap)
                {
                    if (entry.Value == ActiveEditMode)
                    {
                        ActiveEditMode = entry.Key;
                        mAltEditModeActive = false;
                        OnEditModeChanged();
                        e.SuppressKeyPress = true;
                        return;
                    }                    
                }
            }
        }

        //////////////////////////
        // Public events
        //////////////////////////

        // BrushCreated event
        public class BrushCreatedEventArgs : EventArgs
        {
            public Brush Brush { get; set; }
        }
        public delegate void BrushCreatedEventHandler(TileMapView sender, BrushCreatedEventArgs e);
        public event BrushCreatedEventHandler OnBrushCreated;

        // BrushPasteRequested event
        public class BrushPasteRequestedEventArgs : EventArgs
        {
            public Point TargetTilePos { get; set; } // Where to paste to
        }
        public delegate void BrushPasteRequestedEventHandler(TileMapView sender, BrushPasteRequestedEventArgs e);
        public event BrushPasteRequestedEventHandler OnBrushPasteRequested;
        
        
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
        private bool MousePosToTileMapPos(Point pt, out Point tileMapPos)
        {
            TileLayer layer = mTileLayers[0]; // Assume all layers have same dimensions and tile size

            // Compute map-space (world) position (remove scale and firstTileOffsetCS by camera pos)
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

            Graphics tgtGfx = e.Graphics;
            GraphicsHelpers.PrepareGraphics(tgtGfx);

            tgtGfx.Clear(Color.Gray);
            tgtGfx.ScaleTransform((float)RenderScale, (float)RenderScale);

            // We assume (and assert) that all layers have the same tile size. This doesn't have to be
            // true, but it simplifies a lot of stuff, like drawing the grid (below)
            Size tileSize = mTileLayers[0].TileSet.TileSize;

            // CS = client (or view panel) space, otherwise all values assume tile space
            Point firstTile = MathEx.Div(mCameraPos, tileSize);
            Point firstTileOffsetCS = MathEx.Mod(mCameraPos, tileSize); // Offset of first tile from client window's top-left corner
            Point firstTilePosCS = new Point(-firstTileOffsetCS.X, -firstTileOffsetCS.Y); // Start position of first tile in client window
            dstRect.Size = tileSize;

            // Draw only enough tiles to fit the client view. Note that since the client area is not
            // exactly a multiple of a tile's size, we must round up (ciel) the result of the division.
            Point numTilesToDraw = new Point();
            numTilesToDraw.X = (int)Math.Ceiling((double)mViewPanel.ClientSize.Width / RenderScale / tileSize.Width) + 1;
            numTilesToDraw.Y = (int)Math.Ceiling((double)mViewPanel.ClientSize.Height / RenderScale / tileSize.Height) + 1;

            for (int layerIndex = 0; layerIndex < NumLayers; ++layerIndex)
            {
                if (!mLayersToRender[layerIndex])
                    continue;

                TileLayer layer = mTileLayers[layerIndex];
                TileSet tileSet = layer.TileSet;
                Debug.Assert(tileSet.TileSize == tileSize);

                // Draw tiles
                for (int tileX = 0; tileX < numTilesToDraw.X; ++tileX)
                {
                    for (int tileY = 0; tileY < numTilesToDraw.Y; ++tileY)
                    {
                        // If client area is larger than entire map, this condition will be true. Note that we could
                        // also just clamp numTilesToDraw above (but this is easier)
                        if (firstTile.X + tileX >= layer.TileMap.GetLength(0) || firstTile.Y + tileY >= layer.TileMap.GetLength(1))
                            break;

                        int tileIndex = layer.TileMap[firstTile.X + tileX, firstTile.Y + tileY];
                        tileSet.GetTileRect(tileIndex, ref srcRect);

                        dstRect.X = firstTilePosCS.X + (tileX * dstRect.Size.Width);
                        dstRect.Y = firstTilePosCS.Y + (tileY * dstRect.Size.Height);

                        tgtGfx.DrawImage(tileSet.Image, dstRect, srcRect, GraphicsUnit.Pixel);
                    }
                }
            } // for each layer

            if (ShowTileGrid || ShowScreenGrid)
            {
                Pen tileGridPen = new Pen(Color.DarkSlateGray, 1.0f);
                Pen screenGridPen = new Pen(Color.Black, 1.0f);

                TileLayer layer = mTileLayers[0]; // Only use the first tile layer

                for (int tileX = 0; tileX < numTilesToDraw.X; ++tileX)
                {
                    for (int tileY = 0; tileY < numTilesToDraw.Y; ++tileY)
                    {
                        int currTileX = firstTile.X + tileX;
                        int currTileY = firstTile.Y + tileY;

                        if (currTileX >= layer.TileMap.GetLength(0) || currTileY >= layer.TileMap.GetLength(1))
                            break;

                        dstRect.X = firstTilePosCS.X + (tileX * dstRect.Size.Width);
                        dstRect.Y = firstTilePosCS.Y + (tileY * dstRect.Size.Height);

                        if (ShowTileGrid)
                        {
                            if (tileX == 0)
                            {
                                tgtGfx.DrawLine(tileGridPen, dstRect.Left + 0.1f, dstRect.Top, dstRect.Left + 0.1f, dstRect.Bottom);
                            }

                            if (tileY == 0)
                            {
                                tgtGfx.DrawLine(tileGridPen, dstRect.Left, dstRect.Top + 0.1f, dstRect.Right, dstRect.Top + 0.1f);
                            }

                            tgtGfx.DrawLine(tileGridPen, dstRect.Left, dstRect.Bottom, dstRect.Right, dstRect.Bottom);
                            tgtGfx.DrawLine(tileGridPen, dstRect.Right, dstRect.Top, dstRect.Right, dstRect.Bottom);
                        }

                        if (ShowScreenGrid)
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

            // Draw box around selection region
            if (mMouseSelectionRegion.IsValid())
            {
                // Convert tile-space selection rect to client space
                Rectangle selectionRectCS = mMouseSelectionRegion.ToNormalizeRect();
                selectionRectCS.Location = MathEx.Sub(selectionRectCS.Location, firstTile); // offset to top-left visible tile
                selectionRectCS = MathEx.Mul(selectionRectCS, tileSize); // transform to client-space
                selectionRectCS.Location = MathEx.Sub(selectionRectCS.Location, firstTileOffsetCS); // apply sub-tile offset

                Pen tileGridPen = new Pen(Color.Red, 2.0f);
                tgtGfx.DrawRectangle(tileGridPen, selectionRectCS);
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

        //@TODO: Move all this mouse code to a TileMapView.MouseHandling.cs

        private Point mLastBrushPastePos = InvalidPoint;
        private MouseSelectionRegion mMouseSelectionRegion = new MouseSelectionRegion();

        private void ResetSelection(bool force)
        {
            // Avoid needless refreshing
            if (!force && !mMouseSelectionRegion.IsValid())
                return;

            mLastBrushPastePos = InvalidPoint;
            mMouseSelectionRegion.Reset();
            RedrawTileMap();
        }

        private void SendBrushPasteRequestedEvent(Point tileMapPos)
        {
            Debug.Assert(OnBrushPasteRequested != null);

            BrushPasteRequestedEventArgs args = new BrushPasteRequestedEventArgs();
            args.TargetTilePos = tileMapPos;
            OnBrushPasteRequested(this, args);
        }

        private void mViewPanel_MouseDown(object sender, MouseEventArgs e)
        {
            Point tileMapPos;
            if (!MousePosToTileMapPos(e.Location, out tileMapPos))
                return; // Happens if mouse location is out of map bounds

            if (e.Button == MouseButtons.Left)
            {
                if (ActiveEditMode == EditMode.SelectBrush)
                {
                    Debug.Assert(OnBrushCreated != null);

                    mMouseSelectionRegion.StartPoint = tileMapPos;
                    mMouseSelectionRegion.StopPoint = tileMapPos;
                    RedrawTileMap();
                }
                else if (ActiveEditMode == EditMode.PasteBrush)
                {
                    SendBrushPasteRequestedEvent(tileMapPos);
                    mLastBrushPastePos = tileMapPos;
                }
            }
        }

        private void mViewPanel_MouseMove(object sender, MouseEventArgs e)
        {
            Point tileMapPos;
            if (!MousePosToTileMapPos(e.Location, out tileMapPos))
                return; // Happens if mouse location is out of map bounds

            if (e.Button == MouseButtons.Left)
            {
                if (ActiveEditMode == EditMode.SelectBrush)
                {
                    mMouseSelectionRegion.StopPoint = tileMapPos;
                    RedrawTileMap();
                }
                else if (ActiveEditMode == EditMode.PasteBrush)
                {
                    if (mLastBrushPastePos != InvalidPoint && mLastBrushPastePos != tileMapPos)
                    {
                        SendBrushPasteRequestedEvent(tileMapPos);
                        mLastBrushPastePos = tileMapPos;
                    }
                }
            }
            else if (e.Button == MouseButtons.None)
            {
                // In case MouseUp is never received (happens if user alt+tabs away while click & dragging)
                // we make sure to clear the selection on next MouseMove
                ResetSelection(false);
            }
        }

        private void mViewPanel_MouseUp(object sender, MouseEventArgs e)
        {
            Point tileMapPos;
            if (!MousePosToTileMapPos(e.Location, out tileMapPos))
                return; // Happens if mouse location is out of map bounds

            if (e.Button == MouseButtons.Left)
            {
                if (ActiveEditMode == EditMode.SelectBrush)
                {
                    Debug.Assert(OnBrushCreated != null);

                    mMouseSelectionRegion.StopPoint = tileMapPos;

                    if (mMouseSelectionRegion.IsValid())
                    {
                        // Callback client with selected tiles
                        Brush brush = mBrushManager.CreateBrushFromSelection(mMouseSelectionRegion.ToNormalizeRect(), SelectableLayers);

                        BrushCreatedEventArgs args = new BrushCreatedEventArgs();
                        args.Brush = brush;
                        OnBrushCreated(this, args);
                    }

                    ResetSelection(false);
                }
            }
        }

        private void mCheckBoxScreenGrid_CheckedChanged(object sender, EventArgs e)
        {
            RedrawTileMap();
        }

        private void mCheckBoxTileGrid_CheckedChanged(object sender, EventArgs e)
        {
            RedrawTileMap();
        }

        private void EditModeRadioButton_CheckedChanged(object sender, EventArgs e)
        {
            //@TODO: Binding radio controls to an enum manually is crap. Look into generalizing
            // this (class with an array of radio buttons, indexed by enum?)
            if (mRadioButton_PasteBrush.Checked)
            {
                ActiveEditMode = EditMode.PasteBrush;
            }
            else if (mRadioButton_SelectBrush.Checked)
            {
                ActiveEditMode = EditMode.SelectBrush;
            }
            else
            {
                Debug.Assert(false, "Unhandled!");
            }
        }

        private void OnEditModeChanged()
        {
            switch (ActiveEditMode)
            {
                case EditMode.PasteBrush:
                    mRadioButton_PasteBrush.Checked = true;
                    mViewPanel.Cursor = Cursors.Default;
                    break;

                case EditMode.SelectBrush:
                    mRadioButton_SelectBrush.Checked = true;
                    mViewPanel.Cursor = Cursors.Cross;
                    break;
            }
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

    // Helper for tracking mouse-driven region selections
    public class MouseSelectionRegion
    {
        private static Point InvalidPoint = new Point(-1, -1);

        public Point StartPoint { get; set; }
        public Point StopPoint { get; set; }

        public MouseSelectionRegion()
        {
            Reset();
        }

        public void Reset()
        {
            StartPoint = InvalidPoint;
            StopPoint = InvalidPoint;
        }

        public bool IsValid()
        {
            return StartPoint != InvalidPoint && StopPoint != InvalidPoint;
        }

        public Rectangle ToNormalizeRect()
        {
            Point start = StartPoint;
            Point stop = StopPoint;

            if (start.X > stop.X)
            {
                int temp = start.X;
                start.X = stop.X;
                stop.X = temp;
            }

            if (StartPoint.Y > StopPoint.Y)
            {
                int temp = start.Y;
                start.Y = stop.Y;
                stop.Y = temp;
            }

            Size size = new Size(stop.X - start.X + 1, stop.Y - start.Y + 1);
            return new Rectangle(start, size);
        }
    }
}
