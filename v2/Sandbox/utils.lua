--
-- utils.lua
--
-- Resolves the install directory of the BasedEngine version required by the
-- current project. Meant to be required/dofile'd from within premake5.lua,
-- since it relies on premake's os.* extensions (os.matchfiles, os.host, etc).
--
-- Logic:
--   1. Locate installs.ini the same way GetInstallLocation() does in Python:
--        Windows: %LOCALAPPDATA%/BasedEngine/installs.ini
--                 (falls back to ~/AppData/Local/BasedEngine/installs.ini
--                  if LOCALAPPDATA is unset)
--        Linux:   ~/.basedengine/installs.ini
--   2. Parse the [Installs] section as a flat "version = path" map.
--   3. Find the first *.bproject file next to this script and read its
--      "EngineVersion" field out of the JSON.
--   4. Treat EngineVersion as a version-prefix constraint (semver-style):
--        "2.0"   matches any installed "2.0.*"  -> picks highest patch
--        "2.0.1" matches any installed "2.0.1.*" or exactly "2.0.1"
--      and returns the install path of the highest matching version.
--

local M = {}

-- ---------------------------------------------------------------------
-- Helpers
-- ---------------------------------------------------------------------

-- Split a version string like "2.0.1" into a list of numbers {2, 0, 1}.
local function parseVersion(versionStr)
    local parts = {}
    for part in string.gmatch(versionStr, "[^%.]+") do
        table.insert(parts, tonumber(part) or 0)
    end
    return parts
end

-- Compare two version-number tables. Returns true if a > b.
local function versionGreaterThan(a, b)
    local len = math.max(#a, #b)
    for i = 1, len do
        local av = a[i] or 0
        local bv = b[i] or 0
        if av ~= bv then
            return av > bv
        end
    end
    return false
end

-- Returns true if `candidate` satisfies the `constraint` prefix.
-- e.g. constraint {2,0} matches candidate {2,0,1} and {2,0,0}
--      constraint {2,0,1} matches candidate {2,0,1} and {2,0,1,5} but not {2,0,2}
local function versionMatchesConstraint(candidate, constraint)
    if #candidate < #constraint then
        return false
    end
    for i = 1, #constraint do
        if candidate[i] ~= constraint[i] then
            return false
        end
    end
    return true
end

-- Very small JSON scalar-field extractor. Good enough for flat .bproject
-- files like { "EngineVersion": "2.0.1", ... }. Avoids pulling in a full
-- JSON library for a single field.
local function extractJsonStringField(jsonText, fieldName)
    local pattern = '"' .. fieldName .. '"%s*:%s*"([^"]+)"'
    return string.match(jsonText, pattern)
end

-- ---------------------------------------------------------------------
-- Step 1: locate installs.ini (mirrors GetInstallLocation() in Python)
-- ---------------------------------------------------------------------

local function getInstallsIniPath()
    if os.host() == "windows" then
        local localAppData = os.getenv("LOCALAPPDATA")
        if localAppData and localAppData ~= "" then
            return path.join(localAppData, "BasedEngine", "installs.ini")
        end
        -- Fallback safety layer if environment variable is missing
        local userProfile = os.getenv("USERPROFILE")
        if userProfile and userProfile ~= "" then
            return path.join(userProfile, "AppData", "Local", "BasedEngine", "installs.ini")
        end
        return nil, "LOCALAPPDATA and USERPROFILE are both unset; cannot locate installs.ini"
    else
        -- Linux (and anything else POSIX-y premake reports as not "windows")
        local home = os.getenv("HOME")
        if not home or home == "" then
            return nil, "HOME is unset; cannot locate installs.ini"
        end
        return path.join(home, ".basedengine", "installs.ini")
    end
end

-- ---------------------------------------------------------------------
-- Step 2: parse installs.ini's [Installs] section into a version->path map
-- ---------------------------------------------------------------------

local function parseInstallsIni(iniPath)
    local file, err = io.open(iniPath, "r")
    if not file then
        return nil, "could not open installs.ini at '" .. iniPath .. "': " .. tostring(err)
    end

    local installs = {}   -- versionString -> installPath
    local inInstallsSection = false

    for line in file:lines() do
        local trimmed = line:match("^%s*(.-)%s*$")  -- trim whitespace

        if trimmed ~= "" and not trimmed:match("^[;#]") then  -- skip blanks/comments
            local section = trimmed:match("^%[(.+)%]$")
            if section then
                inInstallsSection = (section:lower() == "installs")
            elseif inInstallsSection then
                local key, value = trimmed:match("^([^=]-)%s*=%s*(.-)$")
                if key and value then
                    installs[key] = value
                end
            end
        end
    end

    file:close()
    return installs
end

-- ---------------------------------------------------------------------
-- Step 3: find the .bproject file and read its EngineVersion
-- ---------------------------------------------------------------------

local function findBprojectEngineVersion(searchDir)
    local matches = os.matchfiles(path.join(searchDir, "*.bproject"))
    if not matches or #matches == 0 then
        return nil, "no .bproject file found in '" .. searchDir .. "'"
    end

    local bprojectPath = matches[1]
    local file, err = io.open(bprojectPath, "r")
    if not file then
        return nil, "could not open .bproject file '" .. bprojectPath .. "': " .. tostring(err)
    end

    local contents = file:read("*a")
    file:close()

    local engineVersion = extractJsonStringField(contents, "EngineVersion")
    if not engineVersion then
        return nil, "could not find \"EngineVersion\" field in '" .. bprojectPath .. "'"
    end

    return engineVersion
end

-- ---------------------------------------------------------------------
-- Step 4: find the highest installed version matching the constraint
-- ---------------------------------------------------------------------

local function findHighestMatchingInstall(installs, constraintStr)
    local constraint = parseVersion(constraintStr)

    local bestVersionStr = nil
    local bestVersionParts = nil
    local bestPath = nil

    for versionStr, installPath in pairs(installs) do
        local candidate = parseVersion(versionStr)
        if versionMatchesConstraint(candidate, constraint) then
            if not bestVersionParts or versionGreaterThan(candidate, bestVersionParts) then
                bestVersionStr = versionStr
                bestVersionParts = candidate
                bestPath = installPath
            end
        end
    end

    return bestPath, bestVersionStr
end

-- ---------------------------------------------------------------------
-- Public entry point
-- ---------------------------------------------------------------------

-- Returns: installDir (string) on success, or nil + error message on failure.
-- searchDir defaults to the directory of the calling premake5.lua (_SCRIPT_DIR)
-- if not explicitly provided.
function M.GetEngineInstallDir(searchDir)
    searchDir = searchDir or _SCRIPT_DIR or os.getcwd()

    local iniPath, iniErr = getInstallsIniPath()
    if not iniPath then
        return nil, false, iniErr
    end

    local installs, parseErr = parseInstallsIni(iniPath)
    if not installs then
        return nil, false, parseErr
    end

    local engineVersion, verErr = findBprojectEngineVersion(searchDir)
    if not engineVersion then
        return nil, false, verErr
    end

    local installDir, matchedVersion = findHighestMatchingInstall(installs, engineVersion)
    if not installDir then
        return nil, false, "no installed engine version matching '" .. engineVersion .. "' found in " .. iniPath
    end

    return installDir, true, matchedVersion
end

Utils = M