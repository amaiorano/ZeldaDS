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
        TileMapView.Brush mNewBrush;
        TileMapView.Brush mOldBrush;

        public PasteBrushCommand(TileMapView targetTileMapView, Point targetTilePos, TileMapView.Brush brush)
        {
            mTargetTileMapView = targetTileMapView;
            mTargetTilePos = targetTilePos;
            mNewBrush = brush;
        }

        public override void Do()
        {
            mOldBrush = new TileMapView.Brush();
            mTargetTileMapView.PasteBrush(mTargetTilePos, mNewBrush, ref mOldBrush);
        }

        public override void Undo()
        {
            TileMapView.Brush brush = null;
            mTargetTileMapView.PasteBrush(mTargetTilePos, mOldBrush, ref brush);
        }

        public override void Redo()
        {
            TileMapView.Brush brush = null;
            mTargetTileMapView.PasteBrush(mTargetTilePos, mNewBrush, ref brush);
        }
    }
}
