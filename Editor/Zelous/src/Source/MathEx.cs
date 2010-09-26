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
            return new Point(lhs.X + rhs.X, lhs.Y + rhs.Y);
        }

        public static Point Sub(Point lhs, Point rhs)
        {
            return new Point(lhs.X - rhs.X, lhs.Y - rhs.Y);
        }

        public static Rectangle Mul(Rectangle lhs, Size rhs)
        {
            return new Rectangle(
                lhs.X * rhs.Width,
                lhs.Y * rhs.Height,
                lhs.Width * rhs.Width,
                lhs.Height * rhs.Height
                );                
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

        public static T Clamp<T>(T value, T min, T max) where T : System.IComparable<T>
        {
            if (value.CompareTo(min) < 0)
                return min;

            if (value.CompareTo(max) > 0)
                return max;

            return value;
        }
    }
}
