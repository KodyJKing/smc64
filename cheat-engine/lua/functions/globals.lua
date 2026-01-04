local json = reloadPackage("lua/json")

dofile("lua/functions/scoped_timer.lua")

function reopenProcess(memrec)
    openProcess(getOpenedProcessID())
end

function pprint(value)
    print(json.encode(value))
end

function prints(s)
    print(s)
    return s
end

function hex(x)
    return string.format("%x", x):upper()
end

function clearConsole()
    GetLuaEngine().MenuItem5.doClick()
end

function currentTablePath()
    return getMainForm().SaveDialog1.Filename
end

function reloadCheatEngineAndTable()
    local cePath = getCheatEngineDir() .. "cheatengine-x86_64.exe"
    local tablePath = currentTablePath()
    shellExecute(cePath, tablePath)
    closeCE()
end
