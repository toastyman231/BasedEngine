import globals, sys, subprocess, os

args = globals.ProcessArguments(sys.argv)
prj = globals.GetArgumentValue(args, "prj", "New Project")
template = globals.GetArgumentValue(args, "template", "Default")
location = globals.GetArgumentValue(args, "location", "{}".format(os.getcwd()))
ret = 0;

TEMPLATE_DIR = "{}/Templates/{}".format(os.getcwd(), template);

def CopyBuildFiles():
    dest = "{}/{}".format(location, prj)
    try:
        # Copy premake5 template
        premakeFile = open("{}/Templates/premakeTemplate.txt".format(os.getcwd()), "r").read();
        premakeFile = premakeFile.replace("ENGINE_LOCATION", os.getcwd())
        premakeFile = premakeFile.replace("PROJ_NAME", prj)
        premakeFile = premakeFile.replace("\\", "\\\\")
        finalFile = open("{}/premake5.lua".format(dest), "x")
        finalFile.write(premakeFile)
        finalFile.close()

        # Copy postbuild script
        postBuiltTemplate = open("{}/Templates/postbuildTemplate.txt".format(os.getcwd()), "r").read();
        postBuiltTemplate = postBuiltTemplate.replace("ENGINE_LOCATION", os.getcwd())
        postBuiltTemplate = postBuiltTemplate.replace("\\", "\\\\")
        finalFile = open("{}/postbuild.py".format(dest), "x")
        finalFile.write(postBuiltTemplate)
        finalFile.close()
    except:
        print("Could not find premake build file!")
        ret = 1

if (os.path.exists(TEMPLATE_DIR) and globals.IsWindows()):
    dest = "{}/{}".format(location, prj)
    os.mkdir(dest)
    ret = subprocess.call(["cmd.exe", "/c", "robocopy", TEMPLATE_DIR, dest, "/E"])
    CopyBuildFiles()
    ret = subprocess.call(["cmd.exe", "/c", "cd", dest, "&&", "{}/premake/premake5".format(os.getcwd()), "vs2019"])
else:
    print("Could not find specified template or using unsupported OS!")
    ret = 1

sys.exit(ret)