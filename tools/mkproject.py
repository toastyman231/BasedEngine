import globals, sys, subprocess, os

if __name__ == "__main__":
    globals.SetHome()

    args = globals.ProcessArguments(sys.argv)
    prj = globals.GetArgumentValue(args, "prj", "New Project")
    template = globals.GetArgumentValue(args, "template", "Default")
    TEMPLATE_DIR = "{}/Templates/{}".format(os.getcwd(), template);
    location = globals.GetArgumentValue(args, "location", "{}".format(os.getcwd()))
    ret = 0

    if (os.path.exists(TEMPLATE_DIR) and globals.IsWindows()):
        dest = "{}/{}".format(location, prj)
        os.mkdir(dest)
        ret = subprocess.call(["cmd.exe", "/c", "robocopy", TEMPLATE_DIR, dest, "/E"])
        globals.CopyBuildFiles("{}/{}".format(location, prj), prj)
        ret = subprocess.call(["cmd.exe", "/c", "cd", dest, "&&", "{}/premake/premake5".format(os.getcwd()), "vs2019"])
    else:
        print("Could not find specified template or using unsupported OS!")
        ret = 1

    sys.exit(ret)