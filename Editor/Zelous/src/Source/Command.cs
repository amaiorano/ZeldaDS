using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;

namespace Zelous
{
    abstract class Command
    {
        public Command(){}

        // Returns whether the command was successful; if not, command is not added to the undo stack
        public abstract bool Do();

        // Undoes the command (must be successful)
        public abstract void Undo();

        // We separate Do from Redo since the former often needs to save Undo information
        public abstract void Redo();
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

        public override bool Do()
        {
            mUndoBrush = new TileMapView.Brush();
            mTargetTileMapView.PasteBrush(mTargetTilePos, mDoBrush, ref mUndoBrush);

            if (mUndoBrush == mDoBrush) // Don't paste if target tiles match what we're pasting
            {
                return false;                
            }

            return true;
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
