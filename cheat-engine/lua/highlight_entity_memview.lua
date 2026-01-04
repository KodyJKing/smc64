local forms = reloadPackage("lua/functions/forms")

function createMemoryViewFromAddressExpression(expression)
    local mv = createMemoryView()
    mv.Caption = "Memory View: " .. expression
    mv:show()
    local t = createTimer(mv)
    t.Interval = 100
    t.Enabled = true
    t.OnTimer = function(timer)
        local address = nil
        pcall(function()
            address = getAddress(expression)
        end)
        if address then
            mv.HexadecimalView.Address = address
        end
    end

    local originalOnClose = mv.OnClose
    mv.OnClose = function(sender)
        t.Enabled = false
        t.destroy()
        return originalOnClose(sender)
    end
end

-- forms.printComponentTree(getMainForm().Menu)

local mi = forms.addMenuItem(
    getMainForm().Menu.Items, 
    {"Halo CE", "Watch in Memory View", "Looking at Object"}, 
    function(item)
        item.OnClick = function(sender)
            createMemoryViewFromAddressExpression("[smc64.highlightEntity]")
        end
    end
)
