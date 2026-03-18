import globals, sys, subprocess, os, argparse

if __name__ == "__main__":
    globals.SetHome()

    parser = argparse.ArgumentParser(
        description="Based CLI action for creating new projects from a template",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument("-p", "--project", default="New Project",
                        help="Name of the project to create")
    parser.add_argument("-t", "--template", default="Default",
                        help="Template to create project from (Templates can be found in BasedEngine/Templates)")
    parser.add_argument("-l", "--location", default=os.getcwd(),
                        help="Directory to create project in")

    args = parser.parse_args()

    TEMPLATE_DIR = "{}/Templates/{}".format(os.getcwd(), args.template);
    ret = 0

    if os.path.exists(TEMPLATE_DIR):
        dest = "{}/{}".format(args.location, args.project)
        os.mkdir(dest)
        ret = globals.SafeCopyDir(TEMPLATE_DIR, dest)
        globals.CopyBuildFiles(dest, args.project)
        cmd = ["python3", "{}/based.py".format(globals.ENGINE_DIR), "gensln", "-p", args.project, "-l", dest]
        if globals.IsWindows(): 
            cmd.insert(0, "cmd.exe")
            cmd.insert(1, "/c")
        ret = subprocess.call(cmd)
    else:
        print("Could not find specified template!")
        ret = 1

    sys.exit(ret)