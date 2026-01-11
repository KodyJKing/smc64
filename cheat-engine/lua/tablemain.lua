function reloadPackage(path)
    package.loaded[path] = nil
    return require(path)
end

dofile("lua/functions/globals.lua")
dofile("lua/highlight_entity_memview.lua")
dofile("lua/ipc.lua")
dofile("lua/functions/plot.lua")
dofile("lua/functions/halo1.lua")

local forms = reloadPackage("lua/functions/forms")

forms.addMenuItem(
    getMainForm().Menu.Items,
    {"Halo CE", "Reload", "Script"},
    function(item)
        item.OnClick = function(sender)
            reopenProcess()
            dofile("lua/tablemain.lua")
        end
    end
)

forms.addMenuItem(
    getMainForm().Menu.Items,
    {"Halo CE", "Reload", "Cheat Engine"},
    function(item)
        item.OnClick = function(sender)
            reloadCheatEngineAndTable()
        end
    end
)
