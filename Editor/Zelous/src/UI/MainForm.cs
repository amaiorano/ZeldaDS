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
            public int mActiveTileIndex; // Index of actively selected tile on this layer
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
        private ShellCommandArgs mRunGameShellCommandArgs = new ShellCommandArgs(@"%NDSGAMEROOT%\Tools\desmume\desmume_dev.exe", @"%NDSGAMEROOT%\Game\ZeldaDS\ZeldaDS_d.nds");
        
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

            if (serializer.IsLoading)
            {
                if (settings.mMapFile.Length > 0)
                {
                    LoadMap(settings.mMapFile);
                }
                else
                {
                    NewMap();
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

            // Use to hook into windows messages the application level before they are processed
            //Application.AddMessageFilter(new AppMessageFilter());

            mCommandManager.OnCommand += new CommandManager.CommandEventHandler(OnCommandManagerCommand);

            // Always start with a "new map", which initializes all controls
            NewMap();
        }

        // Used to init a TileMapView used for viewing/selecting from a TileSet
        private void InitTileSetView(TileMapView tileMapView, string title, TileSet tileSet)
        {
            // Init tile set view - we create a layer that simply displays all the tiles
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

            if (tileMapView.Parent is System.Windows.Forms.TabPage)
            {
                tileMapView.Parent.Text = title;
                title = "";
            }

            tileMapView.Reset(title, new TileLayer[] { tileSetLayer });
            tileMapView.ShowScreenGridOption = false;
            tileMapView.OnBrushCreated += new TileMapView.BrushCreatedEventHandler(this.OnBrushCreated);
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

            // Map each TileSetView instance into an array so we can index them
            mGuiTileLayers[0].mTileMapView = mTileSetView1;
            mGuiTileLayers[1].mTileMapView = mTileSetView2;
            mGuiTileLayers[2].mTileMapView = mCollisionView;
            mGuiTileLayers[3].mTileMapView = mCharacterView;

            Size tileSize = new Size(16, 16);

            string[] tileSetFiles = new string[] {"overworld_bg.bmp", "overworld_fg.bmp", "collision_tileset.bmp", "editor_characters.bmp"};
            string[] tileSetViewTitles = new string[] { "Background", "Foreground", "Collision", "Characters" };

            Debug.Assert(tileSetFiles.Length == mGuiTileLayers.Length);
            Debug.Assert(tileSetFiles.Length == tileSetViewTitles.Length);

            // Create TileSets and init TileSetViews with them
            TileSet[] tileSets = new TileSet[mGuiTileLayers.Length];
            for (int i = 0; i < tileSets.Length; ++i)
            {
                tileSets[i] = new TileSet(tileSetFiles[i], tileSize.Width, tileSize.Height);
                InitTileSetView(mGuiTileLayers[i].mTileMapView, tileSetViewTitles[i], tileSets[i]);
            }

            // Init world map view
            {
                mWorldMap = new WorldMap();
                mWorldMap.Init(20, 10, tileSets);

                mWorldMapView.Reset("World Map", mWorldMap.TileLayers);
                mWorldMapView.SetLayerRenderable((int)LayerType.Collision, false);
                mWorldMapView.OnBrushCreated += new TileMapView.BrushCreatedEventHandler(this.OnBrushCreated);
                mWorldMapView.OnBrushPasteRequested += new TileMapView.BrushPasteRequestedEventHandler(this.OnBrushPasteRequested);
            }

            SetCurrMap("");
            mCommandManager.OnNewMap();

            // Clean up any references left behind
            GC.Collect();
        }

        private void LoadMap(string fileName)
        {
            //@TODO: Check if file exists, if it doesn't we should return false. Calling code should handle this.

            mWorldMap.Serialize(SerializationType.Loading, fileName);
            mWorldMapView.RedrawTileMap();

            SetCurrMap(fileName);

            mCommandManager.OnLoadMap();
            UpdateUndoRedoToolStripItems();
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
            if (mCommandManager.NumUndoCommands == 0)
                undoToolStripMenuItem.Enabled = false;
            else
                undoToolStripMenuItem.Enabled = true;

            if (mCommandManager.NumRedoCommands == 0)
                redoToolStripMenuItem.Enabled = false;
            else
                redoToolStripMenuItem.Enabled = true;
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

        private void mTabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            bool showCollisionTiles = mTabControl.SelectedIndex == (int)LayerType.Collision;
            Debug.Assert(showCollisionTiles ? mTabControl.SelectedTab.Contains(mCollisionView) : true);
            mWorldMapView.SetLayerRenderable((int)LayerType.Collision, showCollisionTiles);
        }

        private void MainForm_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Control && e.KeyCode == Keys.Tab)
            {
                ControlHelpers.TabControl_SelectAdjacentTab(mTabControl, !e.Shift);
                e.Handled = true;
            }
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