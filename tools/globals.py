import subprocess, os

ENGINE_NAME = "based"
PROJECT_NAME = "basededitor"
TOOLS_DIR = "tools"
ENGINE_DIR = os.environ.get("BASED_ENGINE_HOME")

V_MAJOR = 2
V_MINOR = 0

import sys, platform
PLATFORM = sys.platform

for x in platform.uname():
    if "microsoft" in x.lower():
        PLATFORM = "windows"
        break

def IsWindows():
    return PLATFORM == "windows" or PLATFORM == "win32"

def IsLinux():
    return PLATFORM == "linux"

def IsMac():
    return PLATFORM == "darwin"

def GetOSDir():
    if IsWindows(): return "Windows"
    if IsLinux(): return "Linux"
    if IsMac(): return "Mac"
    else: return ""

def GetOSExtension():
    if IsWindows(): return ".exe"
    else: return ""

def ProcessArguments(argv):
    ret = {} # return a key:value dict
    for arg in argv:
        try:
            k = arg[0:arg.index("=")]
            v = arg[arg.index("=")+1:]
        except:
            k = arg
            v = 0
        ret[k] = v
    return ret

def GetArgumentValue(args, name, default):
    return args[name] if name in args else default

def SetHome():
    if IsWindows() and ENGINE_DIR is None:
        subprocess.call(["setx", "BASED_ENGINE_HOME", os.getcwd()])
        ENGINE_DIR = os.environ.get("BASED_ENGINE_HOME")
    
    if (IsLinux() or IsMac()) and ENGINE_DIR is None:
        subprocess.call(["export", "BASED_ENGINE_HOME={}".format(os.getcwd())])
        ENGINE_DIR = os.environ.get("BASED_ENGINE_HOME")

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
    except:
        print("Either premake file already exists, or could not find template to create!")
        ret = 1
    return

def CheckForPremakeScript(loc, project):
    if not os.path.exists("{}/premake5.lua".format(loc)):
            print("WARNING: A premake5.lua script was not found in: {}".format(loc))

            try:
                choice = input("\nWould you like to clone a basic premake template to this directory? [Y/n]: ").strip().lower()
            except EOFError:
                choice = "y"

            if choice in ("", "y", "yes"):
                CopyBuildFiles(loc, project)