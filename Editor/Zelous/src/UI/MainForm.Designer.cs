namespace Zelous
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.mSplitContainer = new System.Windows.Forms.SplitContainer();
            this.mMenuStrip = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openMapToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutZelousToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mTabControl = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.saveMapToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveMapAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mWorldMapView = new Zelous.TileMapView();
            this.mTileSetView1 = new Zelous.TileMapView();
            this.mTileSetView2 = new Zelous.TileMapView();
            this.mCollisionView = new Zelous.TileMapView();
            this.mSplitContainer.Panel1.SuspendLayout();
            this.mSplitContainer.Panel2.SuspendLayout();
            this.mSplitContainer.SuspendLayout();
            this.mMenuStrip.SuspendLayout();
            this.mTabControl.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.SuspendLayout();
            // 
            // mSplitContainer
            // 
            this.mSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mSplitContainer.Location = new System.Drawing.Point(0, 0);
            this.mSplitContainer.Name = "mSplitContainer";
            this.mSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // mSplitContainer.Panel1
            // 
            this.mSplitContainer.Panel1.Controls.Add(this.mWorldMapView);
            this.mSplitContainer.Panel1.Controls.Add(this.mMenuStrip);
            // 
            // mSplitContainer.Panel2
            // 
            this.mSplitContainer.Panel2.Controls.Add(this.mTabControl);
            this.mSplitContainer.Size = new System.Drawing.Size(681, 504);
            this.mSplitContainer.SplitterDistance = 300;
            this.mSplitContainer.TabIndex = 1;
            // 
            // mMenuStrip
            // 
            this.mMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.mMenuStrip.Location = new System.Drawing.Point(0, 0);
            this.mMenuStrip.Name = "mMenuStrip";
            this.mMenuStrip.Size = new System.Drawing.Size(681, 24);
            this.mMenuStrip.TabIndex = 1;
            this.mMenuStrip.Text = "mMenuStrip";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newToolStripMenuItem,
            this.openMapToolStripMenuItem,
            this.saveMapToolStripMenuItem,
            this.saveMapAsToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.newToolStripMenuItem.Text = "New Map";
            // 
            // openMapToolStripMenuItem
            // 
            this.openMapToolStripMenuItem.Name = "openMapToolStripMenuItem";
            this.openMapToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.openMapToolStripMenuItem.Text = "Open Map";
            this.openMapToolStripMenuItem.Click += new System.EventHandler(this.openMapToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutZelousToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // aboutZelousToolStripMenuItem
            // 
            this.aboutZelousToolStripMenuItem.Name = "aboutZelousToolStripMenuItem";
            this.aboutZelousToolStripMenuItem.Size = new System.Drawing.Size(145, 22);
            this.aboutZelousToolStripMenuItem.Text = "About Zelous";
            // 
            // mTabControl
            // 
            this.mTabControl.Controls.Add(this.tabPage1);
            this.mTabControl.Controls.Add(this.tabPage2);
            this.mTabControl.Controls.Add(this.tabPage3);
            this.mTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mTabControl.Location = new System.Drawing.Point(0, 0);
            this.mTabControl.Name = "mTabControl";
            this.mTabControl.SelectedIndex = 0;
            this.mTabControl.Size = new System.Drawing.Size(681, 200);
            this.mTabControl.TabIndex = 0;
            this.mTabControl.SelectedIndexChanged += new System.EventHandler(this.mTabControl_SelectedIndexChanged);
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.mTileSetView1);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(673, 174);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "TileSet 1";
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.mTileSetView2);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(673, 174);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "TileSet 2";
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.mCollisionView);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(673, 174);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Collision";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // saveMapToolStripMenuItem
            // 
            this.saveMapToolStripMenuItem.Name = "saveMapToolStripMenuItem";
            this.saveMapToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.saveMapToolStripMenuItem.Text = "Save Map";
            this.saveMapToolStripMenuItem.Click += new System.EventHandler(this.saveMapToolStripMenuItem_Click);
            // 
            // saveMapAsToolStripMenuItem
            // 
            this.saveMapAsToolStripMenuItem.Name = "saveMapAsToolStripMenuItem";
            this.saveMapAsToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.saveMapAsToolStripMenuItem.Text = "Save Map As...";
            // 
            // mWorldMapView
            // 
            this.mWorldMapView.ActiveLayer = 0;
            this.mWorldMapView.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.mWorldMapView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mWorldMapView.Location = new System.Drawing.Point(0, 24);
            this.mWorldMapView.Name = "mWorldMapView";
            this.mWorldMapView.RenderScale = 1;
            this.mWorldMapView.Size = new System.Drawing.Size(681, 276);
            this.mWorldMapView.TabIndex = 0;
            this.mWorldMapView.Title = "TileMapView Title";
            // 
            // mTileSetView1
            // 
            this.mTileSetView1.ActiveLayer = 0;
            this.mTileSetView1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.mTileSetView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mTileSetView1.Location = new System.Drawing.Point(3, 3);
            this.mTileSetView1.Name = "mTileSetView1";
            this.mTileSetView1.RenderScale = 1;
            this.mTileSetView1.Size = new System.Drawing.Size(667, 168);
            this.mTileSetView1.TabIndex = 0;
            this.mTileSetView1.Title = "TileMapView Title";
            // 
            // mTileSetView2
            // 
            this.mTileSetView2.ActiveLayer = 0;
            this.mTileSetView2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.mTileSetView2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mTileSetView2.Location = new System.Drawing.Point(3, 3);
            this.mTileSetView2.Name = "mTileSetView2";
            this.mTileSetView2.RenderScale = 1;
            this.mTileSetView2.Size = new System.Drawing.Size(667, 168);
            this.mTileSetView2.TabIndex = 2;
            this.mTileSetView2.Title = "TileMapView Title";
            // 
            // mCollisionView
            // 
            this.mCollisionView.ActiveLayer = 0;
            this.mCollisionView.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.mCollisionView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mCollisionView.Location = new System.Drawing.Point(3, 3);
            this.mCollisionView.Name = "mCollisionView";
            this.mCollisionView.RenderScale = 1;
            this.mCollisionView.Size = new System.Drawing.Size(667, 168);
            this.mCollisionView.TabIndex = 3;
            this.mCollisionView.Title = "TileMapView Title";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(681, 504);
            this.Controls.Add(this.mSplitContainer);
            this.MainMenuStrip = this.mMenuStrip;
            this.Name = "MainForm";
            this.Text = "Zelous 1.0";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.mSplitContainer.Panel1.ResumeLayout(false);
            this.mSplitContainer.Panel1.PerformLayout();
            this.mSplitContainer.Panel2.ResumeLayout(false);
            this.mSplitContainer.ResumeLayout(false);
            this.mMenuStrip.ResumeLayout(false);
            this.mMenuStrip.PerformLayout();
            this.mTabControl.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private TileMapView mWorldMapView;
        private TileMapView mTileSetView1;
        private System.Windows.Forms.SplitContainer mSplitContainer;
        private System.Windows.Forms.MenuStrip mMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openMapToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutZelousToolStripMenuItem;
        private System.Windows.Forms.TabControl mTabControl;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private TileMapView mTileSetView2;
        private System.Windows.Forms.TabPage tabPage3;
        private TileMapView mCollisionView;
        private System.Windows.Forms.ToolStripMenuItem saveMapToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveMapAsToolStripMenuItem;

    }
}

