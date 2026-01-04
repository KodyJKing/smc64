local module = {}

local forms = reloadPackage("lua/functions/forms")
local memoryView = reloadPackage("lua/functions/memory_view")

local MessageTypes = {
    OPEN_HEX_VIEW = 1,
}

function readQwordFromBytes(bytes, startIndex)
    local value = 0
    for i = 0, 7 do
        value = value | (bytes[startIndex + i] << (i * 8))
    end
    return value
end

function readStringFromBytes(bytes, startIndex)
    local str = ''
    local i = 0
    while true do
        local byte = bytes[startIndex + i]
        if byte == 0 then
            break
        end
        str = str .. string.char(byte)
        i = i + 1
    end
    return str
end

function module.handleMessage(message)
    local bytes = message.data
    local messageType = bytes[1] -- Technically this is 8-byte, but we'll handle that later.
    if (messageType == MessageTypes.OPEN_HEX_VIEW) then
        local address = readQwordFromBytes(bytes, 9)
        -- print("Address:", address)
        local caption = readStringFromBytes(bytes, 17)
        -- print("Caption:", caption)
        local mv = memoryView.createHexView(address)
        if (string.len(caption) > 0) then
            mv.Caption = caption
        end
        mv:show()
    else
        print("Unknown message type:", messageType)
        pprint(message)
    end
end

return module
