import subprocess, globals, sys, os, sethome

args = globals.ProcessArguments(sys.argv)
prj = globals.GetArgumentValue(args, "prj", "New Project")
location = globals.GetArgumentValue(args, "location", "{}".format(os.getcwd()))
version = globals.GetArgumentValue(args, "v", "vs2022")
ret = 0;

def CopyBuildFiles(dest, project):
    try:
        # Copy premake5 template
        premakeFile = open("{}/Templates/premakeTemplate.txt".format(os.getcwd()), "r").read();
        #premakeFile = premakeFile.replace("ENGINE_LOCATION", os.getcwd())
        premakeFile = premakeFile.replace("PROJ_NAME", project)
        premakeFile = premakeFile.replace("\\", "\\\\")
        finalFile = open("{}/premake5.lua".format(dest), "x")
        finalFile.write(premakeFile)
        finalFile.close()

        # Copy postbuild script
        postBuiltTemplate = open("{}/Templates/postbuildTemplate.txt".format(os.getcwd()), "r").read();
        #postBuiltTemplate = postBuiltTemplate.replace("ENGINE_LOCATION", os.getcwd())
        postBuiltTemplate = postBuiltTemplate.replace("\\", "\\\\")
        finalFile = open("{}/postbuild.py".format(dest), "x")
        finalFile.write(postBuiltTemplate)
        finalFile.close()
    except:
        print("Could not find premake build file!")
        ret = 1
    return

sethome.SetHome()

if (globals.IsWindows()):
        if (not os.path.exists("{}/premake5.lua".format(location)) or not os.path.exists("{}/postbuild.py".format(location))):
            CopyBuildFiles(location, prj)
        ret = subprocess.call(["cmd.exe", "/c", "cd", location, "&&", "{}/premake/premake5".format(os.getcwd()), version])

if (globals.IsLinux()):
    ret = subprocess.call(["premake/premake5.linux", "gmake2"])

if (globals.IsMac()):
    ret = subprocess.call(["premake/premake5", "gmake2"])
    if ret == 0:
        subprocess.call(["premake/premake5", "xcode4"])

sys.exit(ret)