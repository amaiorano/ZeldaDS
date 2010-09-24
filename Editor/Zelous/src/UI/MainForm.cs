using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using System.Xml.Serialization;

namespace Zelous
{
    public partial class MainForm : Form, ISerializationClient
    {
        public static MainForm Instance = null; // Provides global access to the main form
        private string mCurrMapFile = "";
        private WorldMap mWorldMap;
        
        enum LayerType : int // Do we really need this?
        {
            Background,
            Foreground,
            Collision,
            Character
        }

        public struct GuiTileLayer
        {
            public TileMapView mTileMapView; // This layer's associated TileMapView (tile set control)
        }
        private GuiTileLayer[] mGuiTileLayers = new GuiTileLayer[WorldMap.NumLayers];

        private TileMapView.Brush mActiveBrush;

        CommandManager mCommandManager = new CommandManager();

        public struct ShellCommandArgs
        {
            public ShellCommandArgs(string cmd, string args)
            {
                mCmd = cmd;
                mArgs = args;
            }
            public string mCmd;
            public string mArgs;
        }

        // This build script will rebuild the NDS file with the updates files in nitrofiles directory (which includes the map files, for example)
        private ShellCommandArgs mBuildGameShellCommandArgs = new ShellCommandArgs(@"%NDSGAMEROOT%\Game\ZeldaDS\build.bat", @"build DEBUG");
        // This commmand runs the emulator with the game
        private ShellCommandArgs mRunGameShellCommandArgs = new ShellCommandArgs(@"%NDSGAMEROOT%\Tools\desmume\desmume_dev.exe", @"--gbaslot-rom=%NDSGAMEROOT%\Game\ZeldaDS\ZeldaDS_d.nds %NDSGAMEROOT%\Game\ZeldaDS\ZeldaDS_d.nds");
        
        private string mAppSettingsFilePath = Directory.GetCurrentDirectory() + @"\ZelousSettings.xml";
        private SerializationMgr mAppSettingsMgr = new SerializationMgr();


        ///////////////////////////////////////////////////////////////////////
        // Settings
        ///////////////////////////////////////////////////////////////////////
        //@TODO: Move to MainForm.Settings.cs (along with ISerializationClient)

        public SerializationMgr AppSettingsMgr { get { return mAppSettingsMgr; } }

        [XmlRootAttribute(Namespace = "MainFormSettings")]
        public class Settings
        {
            public string mMapFile = "";
            public ShellCommandArgs mBuildGameShellCommandArgs;
            public ShellCommandArgs mRunGameShellCommandArgs;
            public FormWindowState mWindowState;
            public Point mLocation;
            public Size mSize;
            public int mSplitterDistance;
            public int[] mTabPageIconIndices;
        }

        void ISerializationClient.OnSerialize(Serializer serializer, ref object saveData)
        {
            Settings settings = (Settings)saveData;

            serializer.Assign(ref settings.mMapFile, ref mCurrMapFile);
            serializer.Assign(ref settings.mBuildGameShellCommandArgs, ref mBuildGameShellCommandArgs);
            serializer.Assign(ref settings.mRunGameShellCommandArgs, ref mRunGameShellCommandArgs);
            serializer.AssignProperty(ref settings.mWindowState, "WindowState", this);

            // Don't allow minimized
            if (WindowState == FormWindowState.Minimized)
            {
                WindowState = FormWindowState.Normal;
            }

            // Saving or loading, we only care about location + size if we're normal
            if (WindowState == FormWindowState.Normal)
            {
                serializer.AssignProperty(ref settings.mLocation, "Location", this);
                serializer.AssignProperty(ref settings.mSize, "Size", this);
            }

            serializer.AssignProperty(ref settings.mSplitterDistance, "SplitterDistance", mSplitContainer);
            
            // Serialize the visibility icon on the tab controls. This is a bit more complicated
            // because the tab control doesn't natively support sending out a "image index changed"
            // event.
            if (serializer.IsSaving)
            {
                int[] iconIndices = mTabControl.GetTabPageIconIndices();
                serializer.Assign(ref settings.mTabPageIconIndices, ref iconIndices);
            }
            else
            {
                int[] iconIndices = null;
                serializer.Assign(ref settings.mTabPageIconIndices, ref iconIndices);
                mTabControl.SetTabPageIconIndices(iconIndices);
            }

            if (serializer.IsLoading)
            {
                if (settings.mMapFile.Length > 0)
                {
                    LoadMap(settings.mMapFile);
                }
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // Implementation
        ///////////////////////////////////////////////////////////////////////

        public MainForm()
        {
            Instance = this;

            AppSettingsMgr.RegisterSerializable(this, typeof(Settings));

            //@TODO: Modify MainForm to only contain panels, and create all controls dynamically
            // and dock within panels
            InitializeComponent();

            // Set KeyPreview to true to allow the form to process the key before the control with focus processes it (MainForm_KeyDown)
            this.KeyPreview = true;

            // Use to hook into windows messages the application level before they are processed
            //Application.AddMessageFilter(new AppMessageFilter());

            mCommandManager.OnCommand += new CommandManager.CommandEventHandler(OnCommandManagerCommand);
            mTabControl.OnIconToggled += new IconToggleTabControl.IconToggledEventHandler(mTabControl_OnIconToggled);

            // One-time initialization of controls
            {
                // Map each TileSetView instance into an array so we can index them
                mGuiTileLayers[0].mTileMapView = mTileSetView1;
                mGuiTileLayers[1].mTileMapView = mTileSetView2;
                mGuiTileLayers[2].mTileMapView = mCollisionView;
                mGuiTileLayers[3].mTileMapView = mCharacterView;

                // Initialize the TileSetViews
                string[] tileSetViewTitles = new string[] { "Background", "Foreground", "Collision", "Characters" }; // Move into GuiTileLayers?

                for (int layer = 0; layer < mGuiTileLayers.Length; ++layer)
                {
                    mGuiTileLayers[layer].mTileMapView.Init("", 1); // These contain exactly 1 layer

                    // Initialize the tab page (we must be on one!)
                    TabPage tabPage = (TabPage)mGuiTileLayers[layer].mTileMapView.Parent;
                    tabPage.Text = tileSetViewTitles[layer];
                }
                mWorldMapView.Init("World Map", WorldMap.NumLayers);

                // Initialize the visibility icon on the tab pages from the world map view
                {
                    bool[] renderableLayers = mWorldMapView.RenderableLayers;
                    int[] iconIndices = new int[renderableLayers.Length];
                    for (int i = 0; i < renderableLayers.Length; ++i)
                    {
                        iconIndices[i] = renderableLayers[i] ? 1 : 0;
                    }
                    mTabControl.SetTabPageIconIndices(iconIndices);
                }
            }

            // Always start with a "new map", which initializes all controls
            NewMap();
        }

        // Used to reset a TileMapView used for viewing/selecting from a TileSet
        private void ResetTileSetView(TileMapView tileMapView, TileSet tileSet)
        {
            // Reset tile set view - we create a layer that simply displays all the tiles
            // in the tile set (same dimensions)
            TileLayer tileSetLayer = new TileLayer();
            tileSetLayer.Init(tileSet.NumTiles.X, tileSet.NumTiles.Y, tileSet);

            int value = 0;
            for (int y = 0; y < tileSetLayer.TileMap.GetLength(1); ++y)
            {
                for (int x = 0; x < tileSetLayer.TileMap.GetLength(0); ++x)
                {
                    tileSetLayer.TileMap[x, y] = value++;
                }
            }

            tileMapView.Reset(new TileLayer[] { tileSetLayer }); // Title set on tab page (below)
            tileMapView.ShowScreenGridOption = false;
            tileMapView.OnBrushCreated += new TileMapView.BrushCreatedEventHandler(this.OnBrushCreated);
        }

        // Used to reset all the TileMapViews (for the main map view and the tile sets).
        // Eventually this function will take args telling it what tile sets we need to load (overworld, underworld, etc.)
        private void ResetTileMapViews()
        {
            Size tileSize = new Size(16, 16);
            string[] tileSetFiles = new string[] { "overworld_bg.bmp", "overworld_fg.bmp", "collision_tileset.bmp", "editor_characters.bmp" };

            Debug.Assert(tileSetFiles.Length == mGuiTileLayers.Length);

            // Create TileSets and reset TileSetViews with them
            TileSet[] tileSets = new TileSet[mGuiTileLayers.Length];
            for (int i = 0; i < tileSets.Length; ++i)
            {
                tileSets[i] = new TileSet(tileSetFiles[i], tileSize.Width, tileSize.Height);
                ResetTileSetView(mGuiTileLayers[i].mTileMapView, tileSets[i]);
            }

            // Reset world map view
            {
                mWorldMap = new WorldMap();
                mWorldMap.Init(20, 10, tileSets);

                mWorldMapView.Reset(mWorldMap.TileLayers);
                mWorldMapView.OnBrushCreated += new TileMapView.BrushCreatedEventHandler(this.OnBrushCreated);
                mWorldMapView.OnBrushPasteRequested += new TileMapView.BrushPasteRequestedEventHandler(this.OnBrushPasteRequested);
            }
        }

        private void OnCommandManagerCommand(CommandManager sender, CommandAction action, Command command)
        {
            OnModifiedStateChanged(mCommandManager.IsModified());
        }

        private void OnBrushCreated(TileMapView sender, TileMapView.BrushCreatedEventArgs e)
        {
            if (sender != mWorldMapView)
            {
                // Remap the brush's single layer to the layer it represents on the world map view
                Debug.Assert(e.Brush.Layers.Length == 1);
                e.Brush.Layers[0].LayerIndex = mTabControl.TabPages.IndexOf((TabPage)sender.Parent);
            }

            mActiveBrush = e.Brush;
        }

        private void OnBrushPasteRequested(TileMapView sender, TileMapView.BrushPasteRequestedEventArgs e)
        {
            Debug.Assert(sender == mWorldMapView); // Pasting only allowed on the world map view

            if (mActiveBrush == null) // No active brush selected, bail
                return;

            mCommandManager.DoCommand(new PasteBrushCommand(sender, e.TargetTilePos, mActiveBrush));
            UpdateUndoRedoToolStripItems();
        }

        private void OnModifiedStateChanged(bool isModified)
        {
            UpdateTitle(); // Update title to show asterisk if modified

            // Disable "New Map" menu option if current map is new and not modified
            bool isNewMap = !isModified && mCurrMapFile.Length == 0;
            newToolStripMenuItem.Enabled = !isNewMap;
        }

        private void UpdateTitle()
        {
            string title = Application.ProductName + " (v" + Application.ProductVersion + ")";

            if (mCurrMapFile.Length > 0)
            {
                title += " - " + mCurrMapFile;
            }

            if (mCommandManager.IsModified())
            {
                title += "*";
            }

            this.Text = title;
        }

        private void SetCurrMap(string file)
        {
            mCurrMapFile = file;
        }

        // If the map needs saving, queries the user about it. Returns whether it's okay to continue (user saved, or map
        // was already saved).
        public bool QueryUserIfCurrentMapNeedsSaving(string action)
        {
            if (mCommandManager.IsModified())
            {
                DialogResult result = MessageBox.Show(this,
                    "The current map is not saved, are you sure you want to " + action + " without saving?",
                    "Create New Map", MessageBoxButtons.YesNo, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);

                if (result == DialogResult.No)
                {
                    return false;
                }
            }

            return true;
        }

        public void NewMap()
        {
            //@TODO: Ask user what type of map (overworld, underworld) to determine what tilesets we need,
            // how large the map should be (in screens), etc.

            ResetTileMapViews();
            
            SetCurrMap("");

            mCommandManager.OnNewMap();
            UpdateUndoRedoToolStripItems();
            
            GC.Collect(); // Good time to clean up any references left behind
        }

        private void LoadMap(string fileName)
        {
            //@TODO: Check if file exists, if it doesn't we should return false. Calling code should handle this.

            ResetTileMapViews();

            mWorldMap.Serialize(SerializationType.Loading, fileName);
            mWorldMapView.RedrawTileMap();

            SetCurrMap(fileName);
            
            mCommandManager.OnLoadMap();
            UpdateUndoRedoToolStripItems();

            GC.Collect(); // Good time to clean up any references left behind
        }

        private void SaveCurrMap()
        {
            Debug.Assert(mCurrMapFile.Length > 0);

            // No need to save unless we're modified
            if (mCommandManager.IsModified())
            {
                mWorldMap.Serialize(SerializationType.Saving, mCurrMapFile);
                mCommandManager.OnSaveMap();
            }
            else
            {
                //@TODO: DEBUG ONLY, save the map to a temporary file, and compare it to the actual file,
                // making sure they are exactly the same. If they are not, there's something wrong with
                // how we track the modified state of a file.
            }
        }

        bool AttemptSaveCurrMap(bool forceSaveAs)
        {
            if (!forceSaveAs && mCurrMapFile.Length > 0)
            {
                SaveCurrMap();
                return true;
            }
            else
            {
                using (SaveFileDialog fileDlg = new SaveFileDialog())
                {
                    fileDlg.Filter = "map files (*.map)|*.map";
                    //fileDlg.InitialDirectory = Environment.CurrentDirectory;
                    DialogResult dlgRes = fileDlg.ShowDialog(this);

                    if (dlgRes == DialogResult.OK)
                    {
                        SetCurrMap(fileDlg.FileName);
                        SaveCurrMap();
                        return true;
                    }
                }
            }

            return false;
        }

        private void UpdateUndoRedoToolStripItems()
        {
            undoToolStripMenuItem.Enabled = mCommandManager.NumUndoCommands > 0;
            redoToolStripMenuItem.Enabled = mCommandManager.NumRedoCommands > 0;
        }

        ///////////////////////////////////////////////////////////////////////
        // Control Events
        ///////////////////////////////////////////////////////////////////////

        private void MainForm_Load(object sender, EventArgs e)
        {
            // We load our settings here, right before the MainForm is displayed, so that
            // all controls are created and positioned, and can have their values modified.
            AppSettingsMgr.Load(mAppSettingsFilePath);
        }

        private void MainForm_KeyDown(object sender, KeyEventArgs e)
        {
            // Control + [Shift + ] Tab switches tile set tab
            if (e.Control && e.KeyCode == Keys.Tab)
            {
                ControlHelpers.TabControl_SelectAdjacentTab(mTabControl, !e.Shift);
                e.SuppressKeyPress = true;
            }

            // <Number> selects a tab, Alt + <Number> toggles visibility on that tab
            if (Char.IsNumber((char)e.KeyValue) && !(e.Control || e.Shift))
            {
                int numberPressed = e.KeyValue - (int)Keys.D0;
                
                // For the user, the layer number is 1-based
                if (numberPressed >= 1 && numberPressed <= mGuiTileLayers.Length)
                {
                    int layer = numberPressed - 1;

                    // For the user, the layer number is 1-based
                    if (layer < mGuiTileLayers.Length)
                    {
                        if (e.Alt)
                        {
                            mTabControl.ToggleTabPageIconIndex(layer);                            
                        }
                        else
                        {
                            mTabControl.SelectedIndex = layer;                            
                        }

                        e.SuppressKeyPress = true;
                    }
                }
            }

            // V toggles visibiliy of current tab
            if (e.KeyCode == Keys.V)
            {
                mTabControl.ToggleTabPageIconIndex(mTabControl.SelectedIndex);
                e.SuppressKeyPress = true;
            }

            // Give TileMapViews a chance to handle global key presses without requiring focus
            //@TODO: Iterate all TileMapViews here, and if e.Handled is true after call, break the loop
            mWorldMapView.OnGlobalKeyDown(e);
        }

        private void MainForm_KeyUp(object sender, KeyEventArgs e)
        {
            //@TODO: Iterate all TileMapViews here, and if e.Handled is true after call, break the loop
            mWorldMapView.OnGlobalKeyUp(e);
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (!QueryUserIfCurrentMapNeedsSaving("exit " + Application.ProductName))
            {
                e.Cancel = true;
                return;
            }

            AppSettingsMgr.Save(mAppSettingsFilePath);
        }

        private void mTabControl_OnIconToggled(IconToggleTabControl sender, IconToggleTabControl.IconToggledEventArgs e)
        {
            bool layerVisible = e.IconIndex != 0;
            mWorldMapView.RenderableLayers[e.TabPageIndex] = layerVisible;

            // What you see is what you select
            mWorldMapView.SelectableLayers[e.TabPageIndex] = layerVisible;

            mWorldMapView.RedrawTileMap();
        }


        ///////////////////////////////////////////////////////////////////////
        // Menu Events
        ///////////////////////////////////////////////////////////////////////

        // New
        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!QueryUserIfCurrentMapNeedsSaving("create a new map"))
            {
                return;
            }

            NewMap();
        }

        // Save
        private void saveMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AttemptSaveCurrMap(false);
        }

        // Save As
        private void saveMapAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AttemptSaveCurrMap(true);
        }

        // Open
        private void openMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!QueryUserIfCurrentMapNeedsSaving("open another map"))
            {
                return;
            }

            using (OpenFileDialog fileDlg = new OpenFileDialog())
            {
                fileDlg.Filter = "map files (*.map)|*.map";
                //fileDlg.InitialDirectory = Environment.CurrentDirectory;
                DialogResult dlgRes = fileDlg.ShowDialog(this);

                if (dlgRes == DialogResult.OK)
                {
                    LoadMap(fileDlg.FileName);
                }
            }
        }

        // Save, Build & Run Map
        private void buildAndTestMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!AttemptSaveCurrMap(false))
                return;
            ProcessHelpers.RunCommand(mBuildGameShellCommandArgs.mCmd, mBuildGameShellCommandArgs.mArgs);
            ProcessHelpers.RunCommand(mRunGameShellCommandArgs.mCmd, mRunGameShellCommandArgs.mArgs);
        }

        // Exit
        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close(); // Eventually calls MainForm_FormClosing()
        }

        // Undo
        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mCommandManager.UndoCommand();
            UpdateUndoRedoToolStripItems();
            mWorldMapView.Refresh();
        }

        // Redo
        private void redoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mCommandManager.RedoCommand();
            UpdateUndoRedoToolStripItems();
            mWorldMapView.Refresh();
        }

        // About
        private void aboutZelousToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (AboutBox aboutBox = new AboutBox())
            {
                aboutBox.ShowDialog(this);
            }
        }
    }

    public class AppMessageFilter : IMessageFilter
    {
        public bool PreFilterMessage(ref Message m)
        {
            bool blockMessage = false;
            return blockMessage;
        }
    }

    //@TODO: Move into own file
    public class ControlHelpers
    {
        public static void TabControl_SelectAdjacentTab(TabControl tabControl, bool nextTab)
        {
            int tabCount = tabControl.TabCount;
            if (tabCount <= 1)
            {
                return;
            }

            if (nextTab)
            {           
                tabControl.SelectedIndex = ++tabControl.SelectedIndex % tabCount;
            }
            else // Previous tab
            {
                int tabIndex = --tabControl.SelectedIndex;
                tabControl.SelectedIndex = tabIndex >= 0? tabIndex : tabCount - 1;
            }
        }
    }

    //@TODO: Move into own file
    public class GraphicsHelpers
    {
        public static void PrepareGraphics(Graphics gfx)
        {
            gfx.InterpolationMode = InterpolationMode.NearestNeighbor;
            gfx.SmoothingMode = SmoothingMode.None;
            gfx.PixelOffsetMode = PixelOffsetMode.Half; // Required for scaling to work
            gfx.PageUnit = GraphicsUnit.Pixel;
        }
    }

    //@TODO: Move into own file
    public class ProcessHelpers
    {
        public static void RunCommand(string cmd, string args)
        {
            cmd = Environment.ExpandEnvironmentVariables(cmd);
            cmd = Path.GetFullPath(cmd);

            args = Environment.ExpandEnvironmentVariables(args);
            string path = System.IO.Path.GetDirectoryName(cmd);

            ProcessStartInfo psi = new ProcessStartInfo(cmd);
            psi.Arguments = args;
            psi.WindowStyle = ProcessWindowStyle.Hidden;
            psi.UseShellExecute = false;
            psi.WorkingDirectory = path;
            Process proc = Process.Start(psi);

            // While waiting for the process to exit, show the wait cursor
            MainForm.Instance.UseWaitCursor = true;
            while (!proc.WaitForExit(10))
            {
                Application.DoEvents(); // Required for the wait cursor to actually display and animate
            }
            MainForm.Instance.UseWaitCursor = false;
            //@TODO: When user exits process, the cursor will remain in waiting state until
            // it's moved, for some reason. Investigate this.
        }
    }

    //@TODO: Move into own file
    public class ArrayHelpers
    {
        public static void CopyArray2d<T>(T[,] srcTileMap, Rectangle srcRect, ref T[,] dstTileMap, Point dstPos)
        {
            for (int x = 0; x < srcRect.Width; ++x)
            {
                for (int y = 0; y < srcRect.Height; ++y)
                {
                    dstTileMap[dstPos.X + x, dstPos.Y + y] = srcTileMap[srcRect.X + x, srcRect.Y + y];
                }
            }
        }
    }
}