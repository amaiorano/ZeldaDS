namespace Zelous
{
    partial class NewMapDlg
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
            this.mNumScreensX = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.mNumScreensY = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.mTileSetGroup = new System.Windows.Forms.ComboBox();
            this.mOkButton = new System.Windows.Forms.Button();
            this.mCancelButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.mNumScreensX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.mNumScreensY)).BeginInit();
            this.SuspendLayout();
            // 
            // mNumScreensX
            // 
            this.mNumScreensX.Location = new System.Drawing.Point(102, 8);
            this.mNumScreensX.Name = "mNumScreensX";
            this.mNumScreensX.Size = new System.Drawing.Size(68, 20);
            this.mNumScreensX.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 10);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(84, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Num Screens X:";
            // 
            // mNumScreensY
            // 
            this.mNumScreensY.Location = new System.Drawing.Point(102, 34);
            this.mNumScreensY.Name = "mNumScreensY";
            this.mNumScreensY.Size = new System.Drawing.Size(68, 20);
            this.mNumScreensY.TabIndex = 3;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 36);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(84, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Num Screens Y:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 63);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(78, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "Tile Set Group:";
            // 
            // mTileSetGroup
            // 
            this.mTileSetGroup.FormattingEnabled = true;
            this.mTileSetGroup.Location = new System.Drawing.Point(102, 60);
            this.mTileSetGroup.Name = "mTileSetGroup";
            this.mTileSetGroup.Size = new System.Drawing.Size(158, 21);
            this.mTileSetGroup.TabIndex = 6;
            // 
            // mOkButton
            // 
            this.mOkButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.mOkButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.mOkButton.Location = new System.Drawing.Point(185, 93);
            this.mOkButton.Name = "mOkButton";
            this.mOkButton.Size = new System.Drawing.Size(75, 23);
            this.mOkButton.TabIndex = 25;
            this.mOkButton.Text = "&OK";
            // 
            // mCancelButton
            // 
            this.mCancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.mCancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.mCancelButton.Location = new System.Drawing.Point(102, 93);
            this.mCancelButton.Name = "mCancelButton";
            this.mCancelButton.Size = new System.Drawing.Size(75, 23);
            this.mCancelButton.TabIndex = 26;
            this.mCancelButton.Text = "&Cancel";
            // 
            // NewMapDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(274, 128);
            this.Controls.Add(this.mCancelButton);
            this.Controls.Add(this.mOkButton);
            this.Controls.Add(this.mTileSetGroup);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.mNumScreensY);
            this.Controls.Add(this.mNumScreensX);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "NewMapDlg";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Create New Map";
            ((System.ComponentModel.ISupportInitialize)(this.mNumScreensX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.mNumScreensY)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.NumericUpDown mNumScreensX;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown mNumScreensY;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox mTileSetGroup;
        private System.Windows.Forms.Button mOkButton;
        private System.Windows.Forms.Button mCancelButton;

    }
}