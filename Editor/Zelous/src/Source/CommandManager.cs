using System;
using System.Collections.Generic;
using System.Text;

namespace Zelous
{
    class CommandManager
    {
        private Stack<Command> mUndoCommands = new Stack<Command>();
        private Stack<Command> mRedoCommands = new Stack<Command>();

        public int NumUndoCommands { get { return mUndoCommands.Count; } }
        public int NumRedoCommands { get { return mRedoCommands.Count; } }

        public CommandManager() {}

        public void DoCommand(Command newCommand)
        {
            mUndoCommands.Push(newCommand);
            newCommand.Do();

            //Clear the redo stack
            if (mRedoCommands.Count > 0)
                mRedoCommands.Clear();
        }

        public void UndoCommand() //on the last command
        {
            if (mUndoCommands.Count > 0)
            {
                Command lastCommand = mUndoCommands.Pop();
                lastCommand.Undo();
                mRedoCommands.Push(lastCommand);
            }
        }

        public void RedoCommand()
        {
            if (mRedoCommands.Count > 0)
            {
                Command lastCommand = mRedoCommands.Pop();
                lastCommand.Redo();
                mUndoCommands.Push(lastCommand);
            }
        }

        public void ClearCommandStacks()
        {
            mUndoCommands.Clear();
            mRedoCommands.Clear();
        }
    }
}
