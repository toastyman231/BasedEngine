import subprocess, globals, sys, os

if __name__ == "__main__":
    globals.SetHome()

    args = globals.ProcessArguments(sys.argv)
    prj = globals.GetArgumentValue(args, "prj", "New Project")
    location = globals.GetArgumentValue(args, "location", "{}".format(os.getcwd()))
    version = globals.GetArgumentValue(args, "v", "vs2022")
    ret = 0

    if (globals.IsWindows()):
        globals.CheckForPremakeScript(location, prj)
        ret = subprocess.call(["cmd.exe", "/c", "cd", location, "&&", "{}/premake/premake5".format(os.getcwd()), version])

    if (globals.IsLinux()):
        globals.CheckForPremakeScript(location, prj)
        ret = subprocess.call(["premake/premake5.linux", "gmake2"])

    if (globals.IsMac()):
        globals.CheckForPremakeScript(location, prj)
        ret = subprocess.call(["premake/premake5", "gmake2"])
        if ret == 0:
            subprocess.call(["premake/premake5", "xcode4"])

    sys.exit(ret)