using System;
using System.IO;
using System.Linq;

namespace Lyra
{
    internal static class ToolPaths
    {
        private static readonly string ToolRoot = AppContext.BaseDirectory;
        private static readonly string ProjectRoot = Path.GetFullPath(Path.Combine(ToolRoot, ".."));

        public static string ResolveFromToolRoot(params string[] segments)
        {
            return Path.GetFullPath(Path.Combine(new[] { ToolRoot }.Concat(segments).ToArray()));
        }

        public static string ResolveFromProjectRoot(params string[] segments)
        {
            return Path.GetFullPath(Path.Combine(new[] { ProjectRoot }.Concat(segments).ToArray()));
        }
    }
}
