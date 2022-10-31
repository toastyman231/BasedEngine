import globals, os, subprocess, sys

args = globals.ProcessArguments(sys.argv)
CONFIG = globals.GetArgumentValue(args, "config", "debug")

print("Building config: " + CONFIG)
print("\n")
ret = 0

if globals.IsWindows():
    MS_BUILD_PATH = "C:\\\\" + os.environ["MS_BUILD_PATH"][8:-1].replace("/", "\\\\")

    ret = subprocess.call(["cmd.exe", "/c", MS_BUILD_PATH, "{}.sln".format(globals.ENGINE_NAME), "/property:Configuration={}".format(CONFIG)])

if globals.IsLinux():
    ret = subprocess.call(["make", "config={}".format(CONFIG)])

if globals.IsMac():
    ret = subprocess.call(["make", "config={}".format(CONFIG)])

sys.exit(ret)