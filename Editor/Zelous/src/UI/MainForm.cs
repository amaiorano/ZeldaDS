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
        // Provide global access to the main form
        public static MainForm Instance = null;

        enum LayerType : int
        {
            Background,
            Foreground,
            Collision
        }

        private WorldMap mWorldMap;
        private int[] mActiveTileIndex = new int[WorldMap.NumLayers];
       
        public MainForm()
        {
            Instance = this;

            InitializeComponent();

            Size tileSize = new Size(16, 16);

            TileSet bgTileSet = new TileSet("overworld_bg.bmp", tileSize.Width, tileSize.Height);
            TileSet fgTileSet = new TileSet("overworld_fg.bmp", tileSize.Width, tileSize.Height);
            TileSet colTileSet = new TileSet("collision_tileset.bmp", tileSize.Width, tileSize.Height);

            // Init world map view
            {
                mWorldMap = new WorldMap();
                mWorldMap.Init(20, 10, new TileSet[] { bgTileSet, fgTileSet, colTileSet });
                mWorldMapView.Init("World Map", mWorldMap.TileLayers);
                mWorldMapView.SetLayerRenderable((int)LayerType.Collision, false);

                mWorldMapView.TileSelected += new TileMapView.TileSelectEventHandler(this.OnTileSelected);

                // Serialization test
                mWorldMapView.RenderScale = Zelous.Properties.Settings.Default.WorldMapViewScale;
            }

            // Init tile set views
            InitTileSetView(mTileSetView1, "Background Tiles", bgTileSet);
            InitTileSetView(mTileSetView2, "Foreground Tiles", fgTileSet);

            // Init collision view
            {
                // For now just show a TileMapView, but eventually we'll show something more clear
                InitTileSetView(mCollisionView, "Collision Tiles", colTileSet);
            }
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

        private void OnTileSelected(TileMapView sender, TileMapView.TileSelectEventArgs e)
        {
            if (sender == mWorldMapView) // Paste tile
            {
                e.TileIndex = mActiveTileIndex[ActiveLayer];
            }
            else // Copy tile
            {
                Debug.Assert(sender.Parent is TabPage);
                mActiveTileIndex[ActiveLayer] = e.TileIndex;
                
                sender.LastTileSelectedPosition = e.TileMapPos; 
            }
            sender.Refresh();
        }

        private void mTabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            ActiveLayer = mTabControl.SelectedIndex;

            bool showCollisionTiles = mTabControl.SelectedIndex == (int)LayerType.Collision;
            Debug.Assert(showCollisionTiles ? mTabControl.SelectedTab.Contains(mCollisionView) : true);            
            mWorldMapView.SetLayerRenderable((int)LayerType.Collision, showCollisionTiles);
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            // Serialization test
            Properties.Settings.Default.WorldMapViewScale = mWorldMapView.RenderScale;
            Properties.Settings.Default.Save();
        }

        private void saveMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // For now, just do the same as Save As...
            //@TODO: Store the current map path and save to it
            saveMapAsToolStripMenuItem_Click(sender, e);
        }

        private void saveMapAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (SaveFileDialog fileDlg = new SaveFileDialog())
            {
                fileDlg.Filter = "map files (*.map)|*.map";
                //fileDlg.InitialDirectory = Environment.CurrentDirectory;
                DialogResult dlgRes = fileDlg.ShowDialog(this);

                if (dlgRes == DialogResult.OK)
                {
                    mWorldMap.Serialize(SerializationType.Saving, fileDlg.FileName);
                }
            }
        }

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