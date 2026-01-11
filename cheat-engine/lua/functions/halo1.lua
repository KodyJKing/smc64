local module = {}

local forms = require("lua/functions/forms")
local plot = require("lua/functions/plot")

function module.relocatedMapBase()
    return readQword("halo1.dll+2D9CE10")
end

function module.mapBase()
    return readQword("halo1.dll+2EA3410")
end

function module.mapOffset()
    return module.relocatedMapBase() - module.mapBase()
end

function fromMapAddress(address)
    return address + module.mapOffset()
end

function toMapAddress(absoluteAddress)
    return absoluteAddress - module.mapOffset()
end

-- Get vertex array of the 0th BSP of the 0th node from a collision geometry tag
-- Todo: Determine size of node and BSP structures so this can be generalized.
function module.collisionGeometryVertexAddress(tagAddress)
    local nodesRel = readInteger(tagAddress + 0x290)
    local bspsRel = readInteger(fromMapAddress(nodesRel) + 0x38)
    local verticesRel = readInteger(fromMapAddress(bspsRel) + 0x58)
    local vertexCount = readInteger(fromMapAddress(bspsRel) + 0x54)
    return fromMapAddress(verticesRel), vertexCount
end

forms.addMenuItem(
    getMainForm().Menu.Items,
    {"Halo CE", "Memory", "To absolute address"},
    function(item)
        item.OnClick = function(sender)
            local addressStr = inputQuery("Map Address to Absolute Address", "Enter the map address (hex):", "")
            if not addressStr then return end
            local address = tonumber(addressStr, 16)
            local absoluteAddress = fromMapAddress(address)
            print(string.format("Absolute Address: %X", absoluteAddress))
        end
    end
)

forms.addMenuItem(
    getMainForm().Menu.Items,
    {"Halo CE", "Memory", "From absolute address"},
    function(item)
        item.OnClick = function(sender)
            local addressStr = inputQuery("Absolute Address to Map Address", "Enter the absolute address (hex):", "")
            if not addressStr then return end
            local absoluteAddress = tonumber(addressStr, 16)
            local mapAddress = toMapAddress(absoluteAddress)
            print(string.format("Map Address: %X", mapAddress))
        end
    end
)

forms.addMenuItem(
    getMainForm().Menu.Items,
    {"Halo CE", "Memory", "Get collision geometry vertex address"},
    function(item)
        item.OnClick = function(sender)
            local tagAddressStr = inputQuery("Collision Geometry Vertex Address", "Enter the collision tag address (hex):", "")
            if not tagAddressStr then return end
            local tagAddress = tonumber(tagAddressStr, 16)
            local vertexAddress, vertexCount = module.collisionGeometryVertexAddress(tagAddress)
            print(string.format("Vertex Address: %X", vertexAddress))
            print(string.format("Vertex Count: %d", vertexCount))
        end
    end
)

return module
