import globals, sys, os, configparser
from pathlib import Path

INVALID_VERSION = -1
VERSION_PATH = Path("VERSION.txt")

def UpdateEnginePath(version: str, engine_path: str):
    path = GetInstallLocation()
    path.parent.mkdir(parents=True, exist_ok=True)

    config = configparser.ConfigParser()
    
    # Preserve case sensitivity (optional, default lowers all keys)
    config.optionxform = str 

    if path.exists():
        config.read(path, encoding="utf-8")

    if "Installs" not in config:
        config["Installs"] = {}

    config["Installs"][version] = engine_path
    
    # Atomic Write to prevent file corruption
    temp_path = path.with_suffix(".tmp")
    try:
        with temp_path.open("w", encoding="utf-8") as file:
            config.write(file)
        temp_path.replace(path)
    except Exception as e:
        if temp_path.exists():
            temp_path.unlink()
        raise e

def GetInstallLocation():
    if globals.IsWindows():
        local_appdata = os.getenv('LOCALAPPDATA')
        if not local_appdata:
            # Fallback safety layer if environment variable is missing
            return Path.home() / "AppData" / "Local" / "BasedEngine" / "installs.ini"
        return Path(local_appdata) / "BasedEngine" / "installs.ini"
    elif globals.IsLinux():
        return Path("~/.basedengine/installs.ini").expanduser()
    else: return "INVALID OS!"

if __name__ == "__main__":

    ret = 0

    if os.path.exists(VERSION_PATH):
        version = INVALID_VERSION
        with open(VERSION_PATH, "r", encoding="utf-8") as version_file:
            version = version_file.read()
        if version is not INVALID_VERSION:
            UpdateEnginePath(version, os.getcwd())
        else:
            print("Engine version was invalid! There may have been an issue opening the engine version file at: {}".format(VERSION_PATH))
            ret = 1
    else:
        print("Could not find the engine version file!")
        ret = 1

    sys.exit(ret)