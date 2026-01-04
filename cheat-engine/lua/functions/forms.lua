local module = {}

function assertMenuItem(menuItem)
    if (menuItem.ClassName ~= "TMenuItem") then
        error("Error: expected TMenuItem, got " .. menuItem.ClassName)
    end
end

function module.findItemWithCaption(menuItem, caption)
    assertMenuItem(menuItem)

    for i=0, menuItem.Count-1 do
        local item = menuItem.Item[i]
        if item.Caption == caption then
            return item, i
        end
    end

    return nil, -1
end

function module.addMenuItem(menu, captions, initializeItem)
    assertMenuItem(menu)

    local currentMenu = menu
    for i=1, #captions-1 do
        local caption = captions[i]
        local item, _ = module.findItemWithCaption(currentMenu, caption)
        if not item then
            item = createMenuItem(currentMenu)
            item.Caption = caption
            currentMenu.add(item)
        end
        currentMenu = item
    end

    local finalCaption = captions[#captions]
    local finalItem, i = module.findItemWithCaption(currentMenu, finalCaption)
    if finalItem then
        currentMenu.delete(i)
    end

    finalItem = createMenuItem(currentMenu)
    initializeItem(finalItem)
    finalItem.Caption = finalCaption
    currentMenu.add(finalItem)
    return finalItem
end

function module.printComponentTree(comp, prefix, dent, visited)
    visited = visited or {}
    prefix = prefix or ""
    dent = dent or ""

    for i, v in ipairs(visited) do
        if v == comp then
            return
        end
    end
    table.insert(visited, comp)

    local caption = comp.Caption
    if caption then
        caption = "(" .. caption .. ")"
    else
        caption = ""
    end

    print(dent .. prefix .. comp.ClassName .. ": " .. comp.Name .. " " .. caption)

    local nextDent = dent .. "    "
    local isMenu = false
        or comp.ClassName == "TMenu"
        or comp.ClassName == "TPopupMenu"
        or comp.ClassName == "TMainMenu"
    local isMenuItem = comp.ClassName == "TMenuItem"

    if isMenu then
        for i = 0, comp.Items.Count - 1 do
            module.printComponentTree(
                comp.Items[i], "[" .. i .. "] ",
                nextDent, visited)
        end
    elseif isMenuItem then
        for i = 0, comp.Count - 1 do
            module.printComponentTree(
                comp.Item[i], "[" .. i .. "] ",
                nextDent, visited)
        end
    end

    for i = 0, comp.getComponentCount() - 1 do
        module.printComponentTree(
            comp.getComponent(i), "",
            nextDent, visited)
    end
end

return module
