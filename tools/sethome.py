import globals, subprocess, os

def SetHome():
    if globals.IsWindows() and os.environ.get("BASED_ENGINE_HOME") is None:
        subprocess.call(["setx", "BASED_ENGINE_HOME", os.getcwd()])
    
    if (globals.IsLinux() or globals.IsMac()) and os.environ.get("BASED_ENGINE_HOME") is None:
        subprocess.call(["export", "BASED_ENGINE_HOME={}".format(os.getcwd())])

SetHome();