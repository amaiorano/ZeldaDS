using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Windows.Forms;

namespace Zelous
{
    class Command
    {
        public Command(){}

        public virtual void Do() {}
        public virtual void Undo() {}
        public virtual void Redo() { }
    };

    //Command to set a tile on the world map
    class SetTileCommand : Command
    {
        //Some of these data members might be common to all Commands.
        //I'll leave them here until we know the other commands we'll have
        private int mTileLayerIndex;
        private TileLayer mTileLayer;
        private Point mTileMapPos;
        private int mNewTileIndex;
        private int mOldTileIndex; //used in Undo()
        private TabControl mTabControl;

        public SetTileCommand( int tileLayerIndex, TileLayer tileLayer, Point tileMapPos, 
            int newTileIndex, int oldTileIndex, TabControl tabControl )
        {
            mTileLayerIndex = tileLayerIndex;
            mTileLayer = tileLayer;
            mTileMapPos = tileMapPos;
            mNewTileIndex = newTileIndex;
            mOldTileIndex = oldTileIndex;
            mTabControl = tabControl;
        }

        public override void Do()
        {
            mTabControl.SelectedIndex = mTileLayerIndex;
            mTileLayer.TileMap[mTileMapPos.X, mTileMapPos.Y] = mNewTileIndex;
        }

        public override void Undo()
        {
            mTabControl.SelectedIndex = mTileLayerIndex;
            mTileLayer.TileMap[mTileMapPos.X, mTileMapPos.Y] = mOldTileIndex;
        }

        public override void Redo()
        {
            mTabControl.SelectedIndex = mTileLayerIndex;
            Do();
        }
    };
}
