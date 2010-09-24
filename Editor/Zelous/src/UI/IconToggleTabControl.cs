using System;
using System.Windows.Forms;
using System.Drawing;
using System.Diagnostics;

namespace Zelous
{
    // TabControl where each tab displays an icon that can be toggled between two images (indices 0 and 1
    // of supplied ImageList).
    public class IconToggleTabControl : TabControl
    {
        private bool mIconToggleHandled = false;

        // Events
        public class IconToggledEventArgs : EventArgs
        {
            public TabPage TabPage { get; set; }
            public int TabPageIndex { get; set; }
            public int IconIndex { get; set; }
        }
        public delegate void IconToggledEventHandler(IconToggleTabControl sender, IconToggledEventArgs e);
        public event IconToggledEventHandler OnIconToggled;

        public int[] GetTabPageIconIndices()
        {
            Debug.Assert(TabPages.Count > 0);
            int[] iconIndices = new int[TabPages.Count];
            for (int i = 0; i < TabPages.Count; ++i)
            {
                iconIndices[i] = TabPages[i].ImageIndex;
            }
            return iconIndices;
        }

        public void SetTabPageIconIndices(int[] iconIndices)
        {
            Debug.Assert(iconIndices.Length == TabPages.Count);
            for (int i = 0; i < TabPages.Count; ++i)
            {
                SetTabPageImageIndex(i, iconIndices[i]);
            }
        }

        public void ToggleTabPageIconIndex(int tabPageIndex)
        {
            TabPage tabPage = TabPages[tabPageIndex];
            SetTabPageImageIndex(tabPageIndex, (tabPage.ImageIndex + 1) % 2);
        }

        // Gets called when current tab is about to be deselected. We handle toggling the icon
        // on the non-selected tab here so that we can cancel the deselect without any flicker
        // problems. Doing this on OnSelecting() causes the selecting tab to be repainted, even
        // if we cancel the selecting.
        protected override void OnDeselecting(TabControlCancelEventArgs e)
        {
            // Determine tab about to be selected with mouse
            if (Control.MouseButtons == MouseButtons.Left)
            {
                int selectingTabPageIndex = GetTabPageIndexAtCurrMousePos();
                TabPage selectingTabPage = TabPages[selectingTabPageIndex];

                if (AttempToggleIconAtCurrMousePos(this, selectingTabPage, selectingTabPageIndex))
                {
                    e.Cancel = true;
                }
                mIconToggleHandled = true;
            }
        }

        // Always gets called when a tab gets clicked, active or not (gets called after OnDeselecting)
        protected override void OnClick(EventArgs e)
        {
            if (!mIconToggleHandled)
            {
                AttempToggleIconAtCurrMousePos(this, SelectedTab, SelectedIndex);
            }

            mIconToggleHandled = false;
        }

        private int GetTabPageIndexAtCurrMousePos()
        {
            Point mousePos = PointToClient(Control.MousePosition);

            for (int i = 0; i < TabPages.Count; ++i)
            {
                Rectangle tabRect = GetTabRect(i);
                if (tabRect.Contains(mousePos))
                {
                    return i;
                }
            }

            return -1;
        }

        private bool AttempToggleIconAtCurrMousePos(TabControl tabControl, TabPage tabPage, int tabPageIndex)
        {
            Debug.Assert(ImageList != null);
            Debug.Assert(ImageList.Images.Count == 2);
            //Debug.Assert(tabPage.ImageIndex >= 0 && tabPage.ImageIndex <= 1);

            Rectangle tabRect = tabControl.GetTabRect(tabPageIndex);
            Point mousePos = tabControl.PointToClient(Control.MousePosition);

            Rectangle displayRect = tabPage.DisplayRectangle;

            Size iconSize = tabControl.ImageList.ImageSize;
            int iconPadding = 4;
            int iconWidth = iconSize.Width + iconPadding;

            if (tabRect.Contains(mousePos) && mousePos.X <= tabRect.X + iconWidth)
            {
                SetTabPageImageIndex(tabPageIndex, (tabPage.ImageIndex + 1) % 2);
                return true;
            }
            
            return false;
        }

        private void SetTabPageImageIndex(int tabPageIndex, int imageIndex)
        {
            TabPage tabPage = TabPages[tabPageIndex];
            tabPage.ImageIndex = imageIndex;

            if (OnIconToggled != null)
            {
                IconToggledEventArgs e = new IconToggledEventArgs();
                e.TabPage = tabPage;
                e.TabPageIndex = tabPageIndex;
                e.IconIndex = tabPage.ImageIndex;
                OnIconToggled(this, e);
            }
        }
    }
}
