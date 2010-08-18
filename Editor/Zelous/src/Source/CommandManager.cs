using System;
using System.Collections.Generic;
using System.Text;

namespace Zelous
{
    // These actions reflect any important state changes made to the CommandManager
    public enum CommandAction
    {
        Do,
        Redo,
        Undo,
        SavePointChanged,
    }

    class CommandManager
    {
        private Stack<Command> mUndoCommands = new Stack<Command>();
        private Stack<Command> mRedoCommands = new Stack<Command>();
        private Command mSavePointCommand = null;

        public CommandManager() {}

        public int NumUndoCommands { get { return mUndoCommands.Count; } }
        public int NumRedoCommands { get { return mRedoCommands.Count; } }

        public delegate void CommandEventHandler(CommandManager sender, CommandAction action, Command command);
        public event CommandEventHandler OnCommand;

        public void DoCommand(Command newCommand)
        {
            mUndoCommands.Push(newCommand);
            newCommand.Do();

            // Clear the redo stack
            if (mRedoCommands.Count > 0)
            {
                // Note that if the save point command was on the redo stack, it is
                // now impossible for the user to get back to the save point via
                // undo commands (file will always be "modified" until next save).
                // For that reason, we don't clear mSavePointCommand here so that IsModified()
                // will always return true (we have a save point command, but it will never
                // be on the top of the undo stack).

                mRedoCommands.Clear();
            }

            if (OnCommand != null)
            {
                OnCommand(this, CommandAction.Do, newCommand);
            }
        }

        public void UndoCommand() //on the last command
        {
            if (mUndoCommands.Count > 0)
            {
                Command lastCommand = mUndoCommands.Pop();
                lastCommand.Undo();
                mRedoCommands.Push(lastCommand);

                if (OnCommand != null)
                {
                    OnCommand(this, CommandAction.Undo, lastCommand);
                }
            }
        }

        public void RedoCommand()
        {
            if (mRedoCommands.Count > 0)
            {
                Command lastCommand = mRedoCommands.Pop();
                lastCommand.Redo();
                mUndoCommands.Push(lastCommand);

                if (OnCommand != null)
                {
                    OnCommand(this, CommandAction.Redo, lastCommand);
                }
            }
        }

        public void OnSaveMap()
        {
            // Update save point to command on top of undo stack (or null if there isn't any)
            mSavePointCommand = null;
            if (mUndoCommands.Count > 0)
            {
                mSavePointCommand = mUndoCommands.Peek();
            }

            if (OnCommand != null)
            {
                OnCommand(this, CommandAction.SavePointChanged, null);
            }
        }

        public void OnNewMap()
        {
            ClearCommandStacks();
        }

        public void OnLoadMap()
        {
            ClearCommandStacks();
        }

        public bool IsModified()
        {
            // If we have a save command and it's at the top of the undo stack, we're not modified.
            if (mSavePointCommand != null && mUndoCommands.Count > 0 && mSavePointCommand == mUndoCommands.Peek())
            {
                return false;
            }

            // If we have no save command and the undo stack is empty, we're not modified.
            // Note this only happens on new or loaded maps that have not been saved yet.
            //
            //@NOTE: We could get rid of this check if we put a special "null" command on the stack
            // for new files, and set mSavePointCommand to it. But we'd have to make sure that "null"
            // command is never executed or considered as a command on the stack. Not really worth the
            // effort.
            if (mSavePointCommand == null && mUndoCommands.Count == 0)
            {
                return false;
            }

            // Otherwise, we're modified.
            return true;
        }

        private void ClearCommandStacks()
        {
            mUndoCommands.Clear();
            mRedoCommands.Clear();
            mSavePointCommand = null;

            if (OnCommand != null)
            {
                OnCommand(this, CommandAction.SavePointChanged, null);
            }
        }
    }
}
