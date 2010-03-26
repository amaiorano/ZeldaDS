using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;

namespace Zelous
{
    class MathEx
    {
        public static Point Add(Point lhs, Point rhs)
        {
            return new Point(lhs.X + rhs.X, lhs.Y + lhs.Y);
        }

        public static Point Div(Point lhs, Point rhs)
        {
            return new Point(lhs.X / rhs.X, lhs.Y / lhs.Y);
        }

        public static Point Div(Point lhs, Size rhs)
        {
            return new Point(lhs.X / rhs.Width, lhs.Y / rhs.Height);
        }

        public static Point Mod(Point lhs, Point rhs)
        {
            return new Point(lhs.X % rhs.X, lhs.Y % rhs.Y);
        }

        public static Point Mod(Point lhs, Size rhs)
        {
            return new Point(lhs.X % rhs.Width, lhs.Y % rhs.Height);
        }
    }
}
