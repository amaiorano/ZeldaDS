using System;
using System.Collections.Generic;
using System.Text;
using System.Reflection;
using System.Resources;
using System.IO;
using System.Windows.Forms;

namespace Zelous
{
    class EmbeddedResourceMgr
    {
        private static Dictionary<string, Cursor> mCursorCache = new Dictionary<string, Cursor>();

        public static void PrintAllResources()
        {
            Assembly asm = Assembly.GetExecutingAssembly();
            Console.WriteLine( "Manifest resources for {0}", asm.FullName );

            foreach( String resourceName in asm.GetManifestResourceNames() )
            {
               Console.WriteLine( "\t{0}", resourceName );
            }
        }

        public static Cursor LoadCursor(string filename)
        {
            //if (mCursorCache.ContainsKey(filename))
            //{
            //    return mCursorCache[filename];
            //}

            Assembly asm = Assembly.GetExecutingAssembly();
            using( Stream resStream = asm.GetManifestResourceStream(filename) )
            {
                //mCursorCache[filename] = new Cursor(resStream);
                //return mCursorCache[filename];
                return new Cursor(resStream);
            }
        }
    }
}
