namespace Zelous
{
    partial class TileMapView
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TileMapView));
            this.mScaleCtrl = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.mTitle = new System.Windows.Forms.Label();
            this.mScrollBarX = new System.Windows.Forms.HScrollBar();
            this.mScrollBarY = new System.Windows.Forms.VScrollBar();
            this.mCheckBoxScreenGrid = new System.Windows.Forms.CheckBox();
            this.mCheckBoxTileGrid = new System.Windows.Forms.CheckBox();
            this.TileMapViewToolTips = new System.Windows.Forms.ToolTip(this.components);
            this.mRadioButton_SelectBrush = new System.Windows.Forms.RadioButton();
            this.mIcons = new System.Windows.Forms.ImageList(this.components);
            this.mRadioButton_PasteBrush = new System.Windows.Forms.RadioButton();
            this.mRadioButton_DragNavigation = new System.Windows.Forms.RadioButton();
            this.mViewPanel = new Zelous.NoFlickerPanel();
            ((System.ComponentModel.ISupportInitialize)(this.mScaleCtrl)).BeginInit();
            this.SuspendLayout();
            // 
            // mScaleCtrl
            // 
            this.mScaleCtrl.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.mScaleCtrl.Location = new System.Drawing.Point(610, 0);
            this.mScaleCtrl.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.mScaleCtrl.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.mScaleCtrl.Name = "mScaleCtrl";
            this.mScaleCtrl.Size = new System.Drawing.Size(39, 20);
            this.mScaleCtrl.TabIndex = 0;
            this.TileMapViewToolTips.SetToolTip(this.mScaleCtrl, "Set the zoom level");
            this.mScaleCtrl.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.mScaleCtrl.ValueChanged += new System.EventHandler(this.mScaleCtrl_ValueChanged);
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(570, 2);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(34, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Zoom";
            // 
            // mTitle
            // 
            this.mTitle.AutoSize = true;
            this.mTitle.Location = new System.Drawing.Point(-3, 2);
            this.mTitle.Name = "mTitle";
            this.mTitle.Size = new System.Drawing.Size(91, 13);
            this.mTitle.TabIndex = 3;
            this.mTitle.Text = "TileMapView Title";
            // 
            // mScrollBarX
            // 
            this.mScrollBarX.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.mScrollBarX.Location = new System.Drawing.Point(28, 319);
            this.mScrollBarX.Name = "mScrollBarX";
            this.mScrollBarX.Size = new System.Drawing.Size(609, 17);
            this.mScrollBarX.TabIndex = 4;
            this.mScrollBarX.ValueChanged += new System.EventHandler(this.mScrollBarXY_ValueChanged);
            // 
            // mScrollBarY
            // 
            this.mScrollBarY.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.mScrollBarY.Location = new System.Drawing.Point(634, 26);
            this.mScrollBarY.Name = "mScrollBarY";
            this.mScrollBarY.Size = new System.Drawing.Size(17, 290);
            this.mScrollBarY.TabIndex = 5;
            this.mScrollBarY.ValueChanged += new System.EventHandler(this.mScrollBarXY_ValueChanged);
            // 
            // mCheckBoxScreenGrid
            // 
            this.mCheckBoxScreenGrid.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.mCheckBoxScreenGrid.AutoSize = true;
            this.mCheckBoxScreenGrid.Location = new System.Drawing.Point(411, 1);
            this.mCheckBoxScreenGrid.Name = "mCheckBoxScreenGrid";
            this.mCheckBoxScreenGrid.Size = new System.Drawing.Size(82, 17);
            this.mCheckBoxScreenGrid.TabIndex = 6;
            this.mCheckBoxScreenGrid.Text = "Screen Grid";
            this.TileMapViewToolTips.SetToolTip(this.mCheckBoxScreenGrid, "Display Screen Grid");
            this.mCheckBoxScreenGrid.UseVisualStyleBackColor = true;
            this.mCheckBoxScreenGrid.CheckedChanged += new System.EventHandler(this.mCheckBoxScreenGrid_CheckedChanged);
            // 
            // mCheckBoxTileGrid
            // 
            this.mCheckBoxTileGrid.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.mCheckBoxTileGrid.AutoSize = true;
            this.mCheckBoxTileGrid.Location = new System.Drawing.Point(499, 1);
            this.mCheckBoxTileGrid.Name = "mCheckBoxTileGrid";
            this.mCheckBoxTileGrid.Size = new System.Drawing.Size(65, 17);
            this.mCheckBoxTileGrid.TabIndex = 6;
            this.mCheckBoxTileGrid.Text = "Tile Grid";
            this.TileMapViewToolTips.SetToolTip(this.mCheckBoxTileGrid, "Display Tile Grid");
            this.mCheckBoxTileGrid.UseVisualStyleBackColor = true;
            this.mCheckBoxTileGrid.CheckedChanged += new System.EventHandler(this.mCheckBoxTileGrid_CheckedChanged);
            // 
            // mRadioButton_SelectBrush
            // 
            this.mRadioButton_SelectBrush.Appearance = System.Windows.Forms.Appearance.Button;
            this.mRadioButton_SelectBrush.AutoSize = true;
            this.mRadioButton_SelectBrush.FlatAppearance.CheckedBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(192)))), ((int)(((byte)(255)))));
            this.mRadioButton_SelectBrush.ImageKey = "RectangleSelection.bmp";
            this.mRadioButton_SelectBrush.ImageList = this.mIcons;
            this.mRadioButton_SelectBrush.Location = new System.Drawing.Point(0, 50);
            this.mRadioButton_SelectBrush.Name = "mRadioButton_SelectBrush";
            this.mRadioButton_SelectBrush.Size = new System.Drawing.Size(22, 22);
            this.mRadioButton_SelectBrush.TabIndex = 8;
            this.TileMapViewToolTips.SetToolTip(this.mRadioButton_SelectBrush, "Create Active Brush");
            this.mRadioButton_SelectBrush.UseVisualStyleBackColor = true;
            this.mRadioButton_SelectBrush.CheckedChanged += new System.EventHandler(this.EditModeRadioButton_CheckedChanged);
            // 
            // mIcons
            // 
            this.mIcons.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("mIcons.ImageStream")));
            this.mIcons.TransparentColor = System.Drawing.Color.White;
            this.mIcons.Images.SetKeyName(0, "PaintBrush.bmp");
            this.mIcons.Images.SetKeyName(1, "RectangleSelection.bmp");
            this.mIcons.Images.SetKeyName(2, "HandOpen.bmp");
            // 
            // mRadioButton_PasteBrush
            // 
            this.mRadioButton_PasteBrush.Appearance = System.Windows.Forms.Appearance.Button;
            this.mRadioButton_PasteBrush.AutoSize = true;
            this.mRadioButton_PasteBrush.FlatAppearance.CheckedBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(192)))), ((int)(((byte)(255)))));
            this.mRadioButton_PasteBrush.ImageKey = "PaintBrush.bmp";
            this.mRadioButton_PasteBrush.ImageList = this.mIcons;
            this.mRadioButton_PasteBrush.Location = new System.Drawing.Point(0, 26);
            this.mRadioButton_PasteBrush.Name = "mRadioButton_PasteBrush";
            this.mRadioButton_PasteBrush.Size = new System.Drawing.Size(22, 22);
            this.mRadioButton_PasteBrush.TabIndex = 7;
            this.TileMapViewToolTips.SetToolTip(this.mRadioButton_PasteBrush, "Paint Active Brush");
            this.mRadioButton_PasteBrush.UseVisualStyleBackColor = true;
            this.mRadioButton_PasteBrush.CheckedChanged += new System.EventHandler(this.EditModeRadioButton_CheckedChanged);
            // 
            // mRadioButton_DragNavigation
            // 
            this.mRadioButton_DragNavigation.Appearance = System.Windows.Forms.Appearance.Button;
            this.mRadioButton_DragNavigation.AutoSize = true;
            this.mRadioButton_DragNavigation.FlatAppearance.CheckedBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(192)))), ((int)(((byte)(255)))));
            this.mRadioButton_DragNavigation.ImageKey = "HandOpen.bmp";
            this.mRadioButton_DragNavigation.ImageList = this.mIcons;
            this.mRadioButton_DragNavigation.Location = new System.Drawing.Point(0, 74);
            this.mRadioButton_DragNavigation.Name = "mRadioButton_DragNavigation";
            this.mRadioButton_DragNavigation.Size = new System.Drawing.Size(22, 22);
            this.mRadioButton_DragNavigation.TabIndex = 9;
            this.TileMapViewToolTips.SetToolTip(this.mRadioButton_DragNavigation, "Navigate with Click & Drag");
            this.mRadioButton_DragNavigation.UseVisualStyleBackColor = true;
            this.mRadioButton_DragNavigation.CheckedChanged += new System.EventHandler(this.EditModeRadioButton_CheckedChanged);
            // 
            // mViewPanel
            // 
            this.mViewPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.mViewPanel.Location = new System.Drawing.Point(28, 26);
            this.mViewPanel.Name = "mViewPanel";
            this.mViewPanel.Size = new System.Drawing.Size(603, 290);
            this.mViewPanel.TabIndex = 0;
            this.mViewPanel.Paint += new System.Windows.Forms.PaintEventHandler(this.mViewPanel_Paint);
            this.mViewPanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.mViewPanel_MouseMove);
            this.mViewPanel.MouseDown += new System.Windows.Forms.MouseEventHandler(this.mViewPanel_MouseDown);
            this.mViewPanel.Resize += new System.EventHandler(this.mViewPanel_Resize);
            this.mViewPanel.MouseUp += new System.Windows.Forms.MouseEventHandler(this.mViewPanel_MouseUp);
            // 
            // TileMapView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.mRadioButton_DragNavigation);
            this.Controls.Add(this.mRadioButton_SelectBrush);
            this.Controls.Add(this.mCheckBoxTileGrid);
            this.Controls.Add(this.mRadioButton_PasteBrush);
            this.Controls.Add(this.mCheckBoxScreenGrid);
            this.Controls.Add(this.mScrollBarY);
            this.Controls.Add(this.mScrollBarX);
            this.Controls.Add(this.mTitle);
            this.Controls.Add(this.mViewPanel);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.mScaleCtrl);
            this.Name = "TileMapView";
            this.Size = new System.Drawing.Size(652, 336);
            ((System.ComponentModel.ISupportInitialize)(this.mScaleCtrl)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.NumericUpDown mScaleCtrl;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label mTitle;
        private NoFlickerPanel mViewPanel;
        private System.Windows.Forms.HScrollBar mScrollBarX;
        private System.Windows.Forms.VScrollBar mScrollBarY;
        private System.Windows.Forms.CheckBox mCheckBoxScreenGrid;
        private System.Windows.Forms.CheckBox mCheckBoxTileGrid;
        private System.Windows.Forms.RadioButton mRadioButton_PasteBrush;
        private System.Windows.Forms.RadioButton mRadioButton_SelectBrush;
        private System.Windows.Forms.ToolTip TileMapViewToolTips;
        private System.Windows.Forms.ImageList mIcons;
        private System.Windows.Forms.RadioButton mRadioButton_DragNavigation;
    }
}
