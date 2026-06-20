import globals, sys, os, re, configparser
from pathlib import Path

INVALID_VERSION = -1
VERSION_HEADER_PATH = Path("based/include/based/VERSION.h")

# Matches lines like: #define ENGINE_VERSION_MAJOR 2
_VERSION_DEFINE_RE = re.compile(
    r'#define\s+ENGINE_VERSION_(MAJOR|MINOR|PATCH)\s+(\d+)'
)


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
    else:
        return "INVALID OS!"


def GetEngineVersion(header_path: Path):
    """
    Parses ENGINE_VERSION_MAJOR/MINOR/PATCH out of VERSION.h and returns
    them as a "MAJOR.MINOR.PATCH" string, or INVALID_VERSION if the file
    is missing any of the three components or can't be read.
    """
    if not header_path.exists():
        return INVALID_VERSION

    try:
        contents = header_path.read_text(encoding="utf-8")
    except OSError:
        return INVALID_VERSION

    components = {}
    for match in _VERSION_DEFINE_RE.finditer(contents):
        name, value = match.group(1), match.group(2)
        components[name] = value

    if "MAJOR" not in components or "MINOR" not in components or "PATCH" not in components:
        return INVALID_VERSION

    return "{}.{}.{}".format(components["MAJOR"], components["MINOR"], components["PATCH"])


if __name__ == "__main__":
    ret = 0
    version = GetEngineVersion(VERSION_HEADER_PATH)
    if version != INVALID_VERSION:
        UpdateEnginePath(version, os.getcwd())
    else:
        print(
            "Engine version was invalid! There may have been an issue opening "
            "or parsing the engine version header at: {}".format(VERSION_HEADER_PATH)
        )
        ret = 1
    sys.exit(ret)