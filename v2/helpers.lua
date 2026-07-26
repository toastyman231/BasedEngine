function GetPythonCommand()
    -- 1. Try running 'python3 --version' and capture the stdout and status code
    -- Note: os.outputof redirects stderr to stdout automatically in Premake
    local output, exit_code = os.outputof("python3 --version")

    -- 2. Validate the exit code and verify that 'Python' is in the string
    -- (Protects against broken Microsoft Store stub shortcuts)
    if exit_code == 0 and output and string.find(output, "Python") then
        return "python3"
    end

    -- 3. Fallback safely to "python"
    return "python"
end