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

            mCommandManager.OnCommand += new CommandManager.CommandEventHandler(OnCommandManagerCommand);

            SetCurrMap(""); //@TODO: Reload last map?
            mCommandManager.OnNewMap();

            Size tileSize = new Size(16, 16);

            TileSet bgTileSet = new TileSet("overworld_bg.bmp", tileSize.Width, tileSize.Height);
            TileSet fgTileSet = new TileSet("overworld_fg.bmp", tileSize.Width, tileSize.Height);
            TileSet colTileSet = new TileSet("collision_tileset.bmp", tileSize.Width, tileSize.Height);
            TileSet charTileSet = new TileSet("editor_characters.bmp", tileSize.Width, tileSize.Height);

            // Init world map view
            {
                mWorldMap = new WorldMap();
                mWorldMap.Init(20, 10, new TileSet[] { bgTileSet, fgTileSet, colTileSet, charTileSet });
                mWorldMapView.Init("World Map", mWorldMap.TileLayers);
                mWorldMapView.SetLayerRenderable((int)LayerType.Collision, false);

                mWorldMapView.TileSelected += new TileMapView.TileSelectEventHandler(this.OnTileSelected);

                // Serialization test
                mWorldMapView.RenderScale = Zelous.Properties.Settings.Default.WorldMapViewScale;
            }

            // Init tile set views
            InitTileSetView(mTileSetView1, "Background Tiles", bgTileSet);
            InitTileSetView(mTileSetView2, "Foreground Tiles", fgTileSet);
            InitTileSetView(mCollisionView, "Collision Tiles", colTileSet);
            InitTileSetView(mCharacterView, "Character Spawners", charTileSet);
        }

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

            tileMapView.Init(title, new TileLayer[] { tileSetLayer });
            tileMapView.ShowScreenGridOption = false;
            tileMapView.TileSelected += new TileMapView.TileSelectEventHandler(this.OnTileSelected);
        }

        private int ActiveLayer
        {
            get { return mWorldMapView.ActiveLayer; }
            set { mWorldMapView.ActiveLayer = value; }
        }

        private void OnCommandManagerCommand(CommandManager sender, CommandAction action, Command command)
        {
            // On any command, we need to update the title to show the "modified" state of our file
            UpdateTitle();
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
            //UpdateTitle(); // Show the new map name (@NOTE: This is not needed! CommandManager lets us know when to update the title)
        }

        private void SaveCurrMap()
        {
            Debug.Assert(mCurrMapFile.Length > 0);

            // No need to save unless we're modified
            if (mCommandManager.IsModified())
            {
                mWorldMap.Serialize(SerializationType.Saving, mCurrMapFile);
                mCommandManager.OnSaveMap();
                //UpdateTitle(); // Show modified state
            }
            else
            {
                //@TODO: DEBUG ONLY, save the map to a temporary file, and compare it to the actual file,
                // making sure they are exactly the same. If they are not, there's something wrong with
                // how we track the modified state of a file.
            }
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

        private void mTabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            ActiveLayer = mTabControl.SelectedIndex;

            bool showCollisionTiles = ActiveLayer == (int)LayerType.Collision;
            Debug.Assert(showCollisionTiles ? mTabControl.SelectedTab.Contains(mCollisionView) : true);
            mWorldMapView.SetLayerRenderable((int)LayerType.Collision, showCollisionTiles);
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            //@TODO: Get rid of this junk and save/load our own settings file

            // Serialization test
            Properties.Settings.Default.WorldMapViewScale = mWorldMapView.RenderScale;
            Properties.Settings.Default.Save();
        }

        // Save
        private void saveMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (mCurrMapFile.Length > 0)
            {
                SaveCurrMap();                
            }
            else // Save as...
            {
                saveMapAsToolStripMenuItem_Click(sender, e);
            }
        }

        // Save As
        private void saveMapAsToolStripMenuItem_Click(object sender, EventArgs e)
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
                }
            }
        }

        // Open
        private void openMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
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

        private void buildAndTestMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //@TODO: Make these paths configurable

            // This build script will rebuild the NDS file with the updates files in nitrofiles
            // directory (which includes the map files, for example)
            string buildCmd = @"%NDSGAMEROOT%\Game\ZeldaDS\build.bat";
            string buildArgs = @"build DEBUG";

            // Run the emulator
            string runCmd = @"%NDSGAMEROOT%\Tools\desmume\desmume_dev.exe";
            string runArgs = @"%NDSGAMEROOT%\Game\ZeldaDS\ZeldaDS_d.nds";

            ProcessHelpers.RunCommand(buildCmd, buildArgs, true);
            ProcessHelpers.RunCommand(runCmd, runArgs, false);
        }

        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mCommandManager.UndoCommand();
            UpdateUndoRedoToolStripItems();
            mWorldMapView.Refresh();
        }

        private void redoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mCommandManager.RedoCommand();
            UpdateUndoRedoToolStripItems();
            mWorldMapView.Refresh();
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
        public static void RunCommand(string cmd, string args, bool useShellExecute)
        {
            cmd = Environment.ExpandEnvironmentVariables(cmd);
            args = Environment.ExpandEnvironmentVariables(args);

            ProcessStartInfo psi = new ProcessStartInfo(cmd);
            psi.Arguments = args;
            psi.WindowStyle = ProcessWindowStyle.Hidden;
            psi.UseShellExecute = useShellExecute;
            psi.WorkingDirectory = System.IO.Path.GetDirectoryName(cmd);

            Process listFiles = Process.Start(psi);
            listFiles.WaitForExit();

            //@NOTE: If using shell execute, we can't get the output apparently
            //System.IO.StreamReader myOutput = listFiles.StandardOutput;
            //if (listFiles.HasExited)
            //{
            //    string output = myOutput.ReadToEnd();
            //    this.processResults.Text = output;
            //}
        }
    }
}