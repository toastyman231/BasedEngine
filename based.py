import os, sys, argparse
import subprocess

TOOLS_DIR = "tools"

def RunCommand(cmds):
    ret = 0
    cmds[0] = "{}/{}/{}.py".format(os.getcwd(), TOOLS_DIR, cmds[0])

    if (os.path.exists(cmds[0])):
        cmds.insert(0, "python3")
        ret = subprocess.call(cmds)
    else:
        print("Invalid Command: ", cmds[0])
        ret = -1
    return ret

# ----- Main Entry Point ----- #
def main():
    argv = sys.argv[1:]
    i = 0
    argc = len(argv)

    while i < argc:
        cmd = argv[i]
        i += 1

        args = []
        while i < argc:
            # If this starts a new command, stop
            if not argv[i].startswith("-"):
                break

            flag = argv[i]
            args.append(flag)
            i += 1

            # If flag expects a value, capture it
            if i < argc and not argv[i].startswith("-"):
                args.append(argv[i])
                i += 1

        print("\n--------------------------")
        print("Executing:", cmd)
        if args:
            print("With arguments:", args)

        if RunCommand([cmd] + args) != 0:
            break

if __name__ == "__main__":
    main()