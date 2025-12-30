import subprocess, globals, sys, os, argparse

if __name__ == "__main__":
    globals.SetHome()

    parser = argparse.ArgumentParser(
        description="Based CLI action for generating project files",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument("-l", "--location", default=os.getcwd(),
                        help="Location of premake build script (should be project root folder)")
    parser.add_argument("-p", "--project", default=os.path.basename(os.getcwd()),
                        help="Override project name in generated project files")
    parser.add_argument("-a", "--action", default="DEFAULT",
                        help="Premake action to run")

    args = parser.parse_args()
    ret = 0

    if args.action == "DEFAULT":
        if globals.IsWindows(): args.action = "vs2022"
        if globals.IsLinux() or globals.IsMac(): args.action = "gmake2"

    if (globals.IsWindows()):
        globals.CheckForPremakeScript(args.location, args.project)
        ret = subprocess.call(["cmd.exe", "/c", "cd", args.location, "&&", "{}/premake/premake5".format(os.getcwd()), args.action])

    if (globals.IsLinux()):
        globals.CheckForPremakeScript(args.location, args.project)
        ret = subprocess.call(["premake/premake5.linux", args.action])

    if (globals.IsMac()):
        globals.CheckForPremakeScript(args.location, args.project)
        ret = subprocess.call(["premake/premake5", args.action])
        if ret == 0:
            subprocess.call(["premake/premake5", "xcode4"])

    sys.exit(ret)