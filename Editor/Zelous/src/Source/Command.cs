using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;

namespace Zelous
{
    class Command
    {
        public Command(){}

        public virtual void Do() {}
        public virtual void Undo() {}
        public virtual void Redo() { }
    };

    class PasteBrushCommand : Command
    {
        TileMapView mTargetTileMapView;
        Point mTargetTilePos;
        TileMapView.Brush mDoBrush;
        TileMapView.Brush mUndoBrush;

        public PasteBrushCommand(TileMapView targetTileMapView, Point targetTilePos, TileMapView.Brush brush)
        {
            mTargetTileMapView = targetTileMapView;
            mTargetTilePos = targetTilePos;
            mDoBrush = brush;
        }

        public override void Do()
        {
            mUndoBrush = new TileMapView.Brush();
            mTargetTileMapView.PasteBrush(mTargetTilePos, mDoBrush, ref mUndoBrush);
        }

        public override void Undo()
        {
            TileMapView.Brush brush = null;
            mTargetTileMapView.PasteBrush(mTargetTilePos, mUndoBrush, ref brush);
        }

        public override void Redo()
        {
            TileMapView.Brush brush = null;
            mTargetTileMapView.PasteBrush(mTargetTilePos, mDoBrush, ref brush);
        }
    }
}
