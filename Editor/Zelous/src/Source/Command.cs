using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace Zelous
{
    class Command
    {
        public Command(){}

        public virtual void Do() {}
        public virtual void Undo() {}
    };

    //Command to set a tile on the world map
    class SetTileCommand : Command
    {
        //Some of these data members might be common to all Commands.
        //I'll leave them here until we know the other commands we'll have
        private TileLayer mTileLayer;
        private Point mTileMapPos;
        private int mNewTileIndex;
        private int mOldTileIndex; //used in Undo()

        public SetTileCommand( TileLayer tileLayer, Point tileMapPos, int newTileIndex, int oldTileIndex )
        {
            mTileLayer = tileLayer;
            mTileMapPos = tileMapPos;
            mNewTileIndex = newTileIndex;
            mOldTileIndex = oldTileIndex;
        }

        public override void Do()
        {
            mTileLayer.TileMap[mTileMapPos.X, mTileMapPos.Y] = mNewTileIndex;
        }

        public override void Undo()
        {
            mTileLayer.TileMap[mTileMapPos.X, mTileMapPos.Y] = mOldTileIndex;
        }
    };
}
