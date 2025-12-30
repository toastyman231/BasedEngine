import globals, sys

if __name__ == "__main__":
    print("based tools - v{}.{}".format(globals.V_MAJOR, globals.V_MINOR))
    print("System: {} {}".format(globals.PLATFORM,  "({})".format(sys.platform) if globals.PLATFORM != sys.platform else ""))
    if (len(sys.argv) >= 2):
        print(", ".join(sys.argv))

    sys.exit(0)