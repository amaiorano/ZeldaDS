using System;
using System.Windows.Forms;
using System.Drawing;
using System.Diagnostics;

namespace Zelous
{
    // Used to dynamically build a dialog and retrieve its values
    class DynamicDialog
    {
        private Form mForm;
        private Form mParentForm;
        private int mNextTabIndex = 0;

        private static readonly Point Padding = new Point(5, 5);

        private Point mCursor = new Point(Padding.X, Padding.Y);
        private int mRowHeight = 0;

        public DynamicDialog(Form parent)
        {
            mForm = new Form();
            mParentForm = parent;
            mForm.MaximizeBox = false;
            mForm.MinimizeBox = false;
            mForm.StartPosition = FormStartPosition.CenterParent;

            if (mParentForm != null)
            {
                mForm.AutoScaleDimensions = mParentForm.AutoScaleDimensions;
            }
            mForm.AutoScaleMode = AutoScaleMode.Inherit;
            mForm.Load += new EventHandler(OnLoadForm); //@TODO: Remove this event so that the form can be garbage collected?
        }

        public void Begin(string title, int width)
        {
            mForm.Text = title;
            mForm.Size = new Size(width, 0);
        }

        public void AddStringInput(string name, string labelText, string value)
        {
            CreateLabel(labelText);
            CreateTextBox(name, 0).Text = value;
            MoveCursorDown();
        }

        public void AddIntegerInput(string name, string labelText, int value, int min, int max)
        {
            CreateLabel(labelText);
            CreateNumericUpDown(name, value, min, max, 0);
            MoveCursorDown();
        }

        public void End()
        {
            Button okButton = CreateButton("OK");
            okButton.DialogResult = DialogResult.OK;
            mForm.AcceptButton = okButton;

            Button cancelButton = CreateButton("Cancel");
            cancelButton.DialogResult = DialogResult.Cancel;
            mForm.CancelButton = cancelButton;
        }

        public DialogResult ShowDialog()
        {
            return mForm.ShowDialog(mParentForm);
        }

        public object GetValue(string name)
        {
            Control[] controlsFound = mForm.Controls.Find(name, false);
            Debug.Assert(controlsFound.Length == 1);

            using (TextBox control = controlsFound[0] as TextBox)
            {
                if (control != null) return control.Text;
            }

            using (NumericUpDown control = controlsFound[0] as NumericUpDown)
            {
                if (control != null) return (int)control.Value;
            }

            Debug.Fail("Unhandled control type");
            return null;
        }

        ///////////////////////////////////////////////////////////////////////
        // Private methods
        ///////////////////////////////////////////////////////////////////////

        private Label CreateLabel(string labelText)
        {
            Label control = new Label();
            control.Size = new Size(0, 0);
            control.AutoSize = true;
            control.Text = labelText;

            control.Location = mCursor;
            MoveCursorRight(control);
            mForm.Controls.Add(control);
            return control;
        }

        private TextBox CreateTextBox(string name, int width)
        {
            TextBox control = new TextBox();
            control.Name = name;
            control.TabIndex = mNextTabIndex++;
            control.Anchor = AnchorStyles.Top | AnchorStyles.Left;

            control.Location = mCursor;
            SetControlWidth(control, width);

            MoveCursorRight(control);
            mForm.Controls.Add(control);
            return control;
        }

        private NumericUpDown CreateNumericUpDown(string name, int curr, int min, int max, int width)
        {
            NumericUpDown control = new NumericUpDown();
            control.Name = name;
            control.TabIndex = mNextTabIndex++;
            control.Minimum = min;
            control.Maximum = max;
            control.Value = curr;

            control.Location = mCursor;
            SetControlWidth(control, width);

            MoveCursorRight(control);
            mForm.Controls.Add(control);
            return control;
        }

        private Button CreateButton(string buttonText)
        {
            Button control = new Button();
            control.Text = buttonText;

            control.Location = mCursor;
            MoveCursorRight(control);
            mForm.Controls.Add(control);
            return control;
        }

        private void MoveCursorRight(Control control)
        {
            mRowHeight = System.Math.Max(mRowHeight, GetControlHeight(control));
            mCursor.Offset(GetControlWidth(control), 0);
        }

        private int GetControlWidth(Control control)
        {
            return Math.Max(control.Width, control.PreferredSize.Width);
        }

        private int GetControlHeight(Control control)
        {
            return Math.Max(control.Height, control.PreferredSize.Height);
        }

        private void SetControlWidth(Control control, int width)
        {
            if (width == 0)
            {
                control.Size = new Size(mForm.ClientSize.Width - control.Location.X - Padding.X, control.Size.Height);
                control.Anchor |= AnchorStyles.Right;
            }
            else
            {
                control.Size = new Size(width, control.Size.Height);
            }
        }

        private void MoveCursorDown()
        {
            // Move back to left edge, and down to next line
            mCursor.X = Padding.X;
            mCursor.Y += (mRowHeight + Padding.Y);
            mRowHeight = 0;
        }

        private void OnLoadForm(object sender, EventArgs e)
        {
            // Resize height of form to show all controls
            int formHeight = mForm.Controls[mForm.Controls.Count - 1].Bounds.Bottom + Padding.Y;
            mForm.ClientSize = new Size(mForm.ClientSize.Width, formHeight);
        }
    }
}
