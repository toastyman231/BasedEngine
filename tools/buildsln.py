import globals, os, subprocess, sys, argparse

# TODO: Make this work on a per-project basis, and make sure it executes pre/post build scripts

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Based CLI action for building projects",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument("-c", "--config", default="debug",
                        help="Configuration to build (release or debug)")

    args = parser.parse_args()
    CONFIG = args.config

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