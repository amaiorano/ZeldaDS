using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;

namespace Zelous
{
    public abstract class Command
    {
        // Returns the description of this specific command (displayed in the UI)
        public abstract string GetDescription();

        // Returns whether the command was successful; if not, command is not added to the undo stack
        public abstract bool Do();

        // Undoes the command (must be successful)
        public abstract void Undo();

        // We separate Do from Redo since the former often needs to save Undo information
        public abstract void Redo();
    };

    public class PasteBrushCommand : Command
    {
        TileMapView mTargetTileMapView;
        Point mTargetTilePos;
        TileMapView.Brush mDoBrush;
        TileMapView.Brush mUndoBrush;
        int mNumEventsPasted = 0;

        public PasteBrushCommand(TileMapView targetTileMapView, Point targetTilePos, TileMapView.Brush brush)
        {
            mTargetTileMapView = targetTileMapView;
            mTargetTilePos = targetTilePos;
            mDoBrush = brush;
        }

        public override string GetDescription()
        {
            StringBuilder command = new StringBuilder("Paste Brush", 256);

            command.AppendFormat(", {0} Layer{1}", mDoBrush.Layers.Length, mDoBrush.Layers.Length > 1 ? "s" : "");

            if (mNumEventsPasted > 0)
            {
                command.AppendFormat(", {0} Event{1}", mNumEventsPasted, mNumEventsPasted > 1 ? "s" : "");
            }

            command.Append(" at ").Append(CommandHelpers.TilePosToString(mTargetTilePos));

            return command.ToString();
        }

        public override bool Do()
        {
            mUndoBrush = mTargetTileMapView.CreateUndoBrush(mTargetTilePos, mDoBrush);

            if (mUndoBrush.Equals(mDoBrush)) // Don't paste if target tiles match what we're pasting
            {
                System.Media.SystemSounds.Beep.Play();
                return false;
            }

            // Before pasting, if there are any event tiles without event data (metadata), we create and have the user populate
            // them. We must do this on the DoBrush tiles before we paste so that the event is part of the same command (then
            // everything works - copying tiles, undo, etc.).

            //@TODO: This is the wrong place to do this!
            //@TODO: Would be nice to have some kind of iterator to iterate over the tiles in a TileMap
            
            // Get the brush's event layer index (if any)
            int brushEventLayerIndex = new List<TileMapView.Brush.LayerData>(mDoBrush.Layers).FindIndex(layer => layer.LayerIndex == GameConstants.EventLayerIndex);
            if (brushEventLayerIndex != -1)
            {
                for (int tileX = 0; tileX < mDoBrush.NumTiles.Width; ++tileX)
                {
                    for (int tileY = 0; tileY < mDoBrush.NumTiles.Height; ++tileY)
                    {
                        Tile tile = mDoBrush.Layers[brushEventLayerIndex].TileMap[tileX, tileY]; //@NOTE: Wish I could 'alias' a value like in C++ with references

                        if (tile.Index > 0) // Valid event
                        {
                            if (tile.Metadata == null) // Needs metadata (initial paste)
                            {
                                GameEvent gameEvent = MainForm.Instance.GameEventFactory.CreateNewGameEventFromPrototype(tile.Index);
                                if (!GameEventHelpers.DisplayGameEventDialog(ref gameEvent))
                                {
                                    return false; // Bail as soon as user cancels any of the dialogs (can't paste this brush)
                                }
                                mDoBrush.Layers[brushEventLayerIndex].TileMap[tileX, tileY].Metadata = gameEvent;
                            }

                            ++mNumEventsPasted;
                        }
                    }
                }
            }

            // Finally, paste the brush
            mTargetTileMapView.PasteBrush(mTargetTilePos, mDoBrush);

            return true;
        }

        public override void Undo()
        {
            mTargetTileMapView.PasteBrush(mTargetTilePos, mUndoBrush);
        }

        public override void Redo()
        {
            mTargetTileMapView.PasteBrush(mTargetTilePos, mDoBrush);
        }
    }

    public class EditGameEventCommand : Command
    {
        TileMapView mTargetTileMapView;
        Point mTargetTilePos;

        GameEvent mOldGameEvent;
        GameEvent mNewGameEvent;

        delegate void AssignToTileMetadata(GameEvent rhs);
        AssignToTileMetadata mAssignToTileMetadata;

        public EditGameEventCommand(TileMapView targetTileMapView, Point targetTilePos)
        {
            mTargetTileMapView = targetTileMapView;
            mTargetTilePos = targetTilePos;

            // Bind lambda function to tile's metadata reference (via closure). Tile is a value type, so this is as close as I can get to having a 'ref'
            mAssignToTileMetadata = t => mTargetTileMapView.TileLayers[GameConstants.EventLayerIndex].TileMap[mTargetTilePos.X, mTargetTilePos.Y].Metadata = t;
        }

        public override string GetDescription()
        {
            return string.Format("Edit Event at {0}", CommandHelpers.TilePosToString(mTargetTilePos));
        }

        public override bool Do()
        {
            if (mTargetTileMapView.ContainsLayer(GameConstants.EventLayerIndex))
            {
                return EditEvent(mTargetTileMapView.TileLayers[GameConstants.EventLayerIndex].TileMap[mTargetTilePos.X, mTargetTilePos.Y]);
            }

            return false;
        }

        public override void Undo()
        {
            mAssignToTileMetadata(mOldGameEvent);
        }

        public override void Redo()
        {
            mAssignToTileMetadata(mNewGameEvent);
        }

        private bool EditEvent(Tile tile)
        {
            if (tile.Index > 0) // Is a valid event tile
            {
                Debug.Assert(tile.Metadata != null);
                mNewGameEvent = (GameEvent)tile.Metadata;

                mOldGameEvent = mNewGameEvent; // Keep reference to immutable type

                if (GameEventHelpers.DisplayGameEventDialog(ref mNewGameEvent))
                {
                    // Update tile metadata
                    mAssignToTileMetadata(mNewGameEvent);
                    return true;
                }
            }
            return false;
        }
    }

    public class CommandHelpers
    {
        public static string TilePosToString(Point tilePos)
        {
            return string.Format("[{0},{1}]", tilePos.X, tilePos.Y);
        }
    }
}
