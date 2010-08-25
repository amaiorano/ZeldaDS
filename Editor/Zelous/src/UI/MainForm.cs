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

namespace Zelous
{
    public partial class MainForm : Form
    {
        public static MainForm Instance = null; // Provides global access to the main form
        private string mCurrMapFile = "";
        private WorldMap mWorldMap;
        private int[] mActiveTileIndex = new int[WorldMap.NumLayers];
        CommandManager mCommandManager = new CommandManager();

        enum LayerType : int
        {
            Background,
            Foreground,
            Collision
        }

        public MainForm()
        {
            Instance = this;

            //@TODO: Modify MainForm to only contain panels, and create all controls dynamically
            // and dock within panels
            InitializeComponent();

            // Use to hook into windows messages the application level before they are processed
            //Application.AddMessageFilter(new AppMessageFilter());

            mCommandManager.OnCommand += new CommandManager.CommandEventHandler(OnCommandManagerCommand);

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
            tileMapView.LastTileSelectedPosition = new Point(0, 0);
            tileMapView.TileSelected += new TileMapView.TileSelectEventHandler(this.OnTileSelected);
        }

        private int ActiveLayer
        {
            get { return mWorldMapView.ActiveLayer; }
            set { mWorldMapView.ActiveLayer = value; }
        }

        private void OnCommandManagerCommand(CommandManager sender, CommandAction action, Command command)
        {
            OnModifiedStateChanged(mCommandManager.IsModified());
        }

        private void OnTileSelected(TileMapView sender, TileMapView.TileSelectEventArgs e)
        {
            if (sender == mWorldMapView) // Paste tile
            {
                mCommandManager.DoCommand(new SetTileCommand(ActiveLayer, e.TileLayer, e.TileMapPos, 
                                            mActiveTileIndex[ActiveLayer], e.TileIndex, mTabControl));
                UpdateUndoRedoToolStripItems();
            }
            else // Copy tile
            {
                Debug.Assert(sender.Parent is TabPage);
                mActiveTileIndex[ActiveLayer] = e.TileIndex;

                sender.LastTileSelectedPosition = e.TileMapPos;
            }
            sender.Refresh();
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

            Size tileSize = new Size(16, 16);

            TileSet bgTileSet = new TileSet("overworld_bg.bmp", tileSize.Width, tileSize.Height);
            TileSet fgTileSet = new TileSet("overworld_fg.bmp", tileSize.Width, tileSize.Height);
            TileSet colTileSet = new TileSet("collision_tileset.bmp", tileSize.Width, tileSize.Height);
            TileSet charTileSet = new TileSet("editor_characters.bmp", tileSize.Width, tileSize.Height);

            // Init world map view
            {
                mWorldMap = new WorldMap();
                mWorldMap.Init(20, 10, new TileSet[] { bgTileSet, fgTileSet, colTileSet, charTileSet });

                mWorldMapView.Reset("World Map", mWorldMap.TileLayers);
                mWorldMapView.SetLayerRenderable((int)LayerType.Collision, false);
                mWorldMapView.TileSelected += new TileMapView.TileSelectEventHandler(this.OnTileSelected);
            }

            // Init tile set views
            InitTileSetView(mTileSetView1, "Background Tiles", bgTileSet);
            InitTileSetView(mTileSetView2, "Foreground Tiles", fgTileSet);
            InitTileSetView(mCollisionView, "Collision Tiles", colTileSet);
            InitTileSetView(mCharacterView, "Character Spawners", charTileSet);

            SetCurrMap("");
            mCommandManager.OnNewMap();

            // Clean up any references left behind
            GC.Collect();
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

        private void mTabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            ActiveLayer = mTabControl.SelectedIndex;

            bool showCollisionTiles = ActiveLayer == (int)LayerType.Collision;
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
                    mWorldMap.Serialize(SerializationType.Loading, fileDlg.FileName);
                    mWorldMapView.RedrawTileMap();
                    
                    SetCurrMap(fileDlg.FileName);

                    mCommandManager.OnLoadMap();
                    UpdateUndoRedoToolStripItems();
                }
            }
        }

        // Save, Build & Run Map
        private void buildAndTestMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!AttemptSaveCurrMap(false))
                return;

            //@TODO: Make these paths configurable

            // This build script will rebuild the NDS file with the updates files in nitrofiles
            // directory (which includes the map files, for example)
            string buildCmd = @"%NDSGAMEROOT%\Game\ZeldaDS\build.bat";
            string buildArgs = @"build DEBUG";

            // Run the emulator
            string runCmd = @"%NDSGAMEROOT%\Tools\desmume\desmume_dev.exe";
            string runArgs = @"%NDSGAMEROOT%\Game\ZeldaDS\ZeldaDS_d.nds";

            ProcessHelpers.RunCommand(buildCmd, buildArgs);
            ProcessHelpers.RunCommand(runCmd, runArgs);
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
            args = Environment.ExpandEnvironmentVariables(args);

            string path = System.IO.Path.GetDirectoryName(cmd);
            path = Environment.ExpandEnvironmentVariables(path);

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
}