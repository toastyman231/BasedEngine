import globals
import globals, sys

print("based tools - v{}.{}".format(globals.V_MAJOR, globals.V_MINOR))
print("System: {}".format(globals.PLATFORM))
if (len(sys.argv) >= 2):
    print(", ".join(sys.argv))

sys.exit(0)