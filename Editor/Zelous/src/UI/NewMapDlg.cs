using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Zelous
{
    public partial class NewMapDlg : Form
    {
        public NewMapDlg()
        {
            InitializeComponent();

            this.AcceptButton = mOkButton;
            this.CancelButton = mCancelButton;

            mNumScreensX.Minimum = 1;
            mNumScreensX.Maximum = 20;
            mNumScreensX.Increment = 1;
            mNumScreensX.Value = 10;

            mNumScreensY.Minimum = 1;
            mNumScreensY.Maximum = 20;
            mNumScreensY.Increment = 1;
            mNumScreensY.Value = 10;

            mTileSetGroup.DropDownStyle = ComboBoxStyle.DropDownList;
            
            for (int i = 0; i < MainForm.Instance.GameTileSetGroupMgr.NumGroups; ++i)
            {
                mTileSetGroup.Items.Add(MainForm.Instance.GameTileSetGroupMgr.GetGroup(i).Name);
            }
            mTileSetGroup.SelectedIndex = 0;
        }

        public int NumScreensX { get { return (int)mNumScreensX.Value; } }
        public int NumScreensY { get { return (int)mNumScreensY.Value; } }
        public int TileSetGroupIndex { get { return mTileSetGroup.SelectedIndex; } }
    }
}
