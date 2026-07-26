import subprocess, os

ENGINE_NAME = "based"

V_MAJOR = 2
V_MINOR = 1

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

def SafeCopyDir(src, dest):
    ret = 0
    if IsWindows():
        ret = subprocess.call(["cmd.exe", "/c", "robocopy", src, dest, "/E"])
    elif IsLinux() or IsMac():
        ret = subprocess.call(["cp", "-R", src, dest])
    else: ret = 1
    return ret

def CopyBuildFiles(dest, project):
    try:
        # Copy premake5 template
        premakeFile = open("{}/Templates/premakeTemplate.txt".format(os.getcwd()), "r").read();
        #premakeFile = premakeFile.replace("ENGINE_LOCATION", os.getcwd())
        premakeFile = premakeFile.replace("PROJ_NAME", project)
        #premakeFile = premakeFile.replace("\\", "\\\\")
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

def GetPythonCommand() -> str:
    """
    Tests if 'python3' is available and valid.
    Returns 'python3' if successful, otherwise falls back to 'python'.
    """
    # Windows specific fix to prevent a console window from briefly flashing
    startupinfo = None
    if IsWindows():
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW

    try:
        # Run 'python3 --version' safely in the background
        result = subprocess.run(
            ["python3", "--version"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=3, # Prevent hanging if a broken alias blocks the thread
            startupinfo=startupinfo
        )
        
        # Check if it returned a success code (0) and contains "Python" in the output
        # (This protects against Microsoft Store shortcuts that return empty or error stubs)
        if result.returncode == 0 and "Python" in (result.stdout + result.stderr):
            return "python3"
            
    except (FileNotFoundError, subprocess.SubprocessError):
        # Catching FileNotFoundError handles systems where 'python3' doesn't exist at all
        pass

    return "python"
