local module = {}

local forms = require("lua/functions/forms")

-- Data: [x0, y0, z0, x1, y1, z1, ...]
function module.plotPoints(data)
    local tmpfile = os.tmpname() .. ".csv"
    local f = io.open(tmpfile, "w")
    for i=1,#data,3 do
        f:write(string.format("%f,%f,%f\n", data[i], data[i+1], data[i+2]))
    end
    f:close()
    os.execute('start "python3" "./python/plot_csv.pyw" "' .. tmpfile .. '"')
end

function module.plotDataAtAddress(address, count, stride, maxValue)
    if maxValue == nil then maxValue = 1e6 end 
    local data = {}
    for i=0,(count-1) do
        local base = address + i*stride*4
        local x = readFloat(base)
        local y = readFloat(base + 4)
        local z = readFloat(base + 8)

        local fail = false
            or x~=x or math.abs(x) > maxValue
            or y~=y or math.abs(y) > maxValue
            or z~=z or math.abs(z) > maxValue
        
        if not fail then
            table.insert(data, x)
            table.insert(data, y)
            table.insert(data, z)
        end
    end
    module.plotPoints(data)
end

-- function testPlot()
--     local points = {}
--     for i=1,100 do
--         x = i
--         y = math.sin(i/10)*10
--         z = math.cos(i/10)*10
--         table.insert(points, x)
--         table.insert(points, y)
--         table.insert(points, z)
--         -- table.insert(points, {x=i, y=math.sin(i/10)*10, z=math.cos(i/10)*10})
--     end
--     module.plotPoints(points)
-- end

-- testPlot()

function module.promptToPlot(address)
    if not address then
        address = inputQuery("Plot Data", "Enter the base address (hex):", "")
    end
    if not address then return end
    address = tonumber(address, 16)

    local countStr = inputQuery("Plot Data", "Enter the number of points:", "100")
    if not countStr then return end
    local count = tonumber(countStr)

    local strideStr = inputQuery("Plot Data", "Enter the stride (in bytes):", "0C")
    if not strideStr then return end
    local stride = tonumber(strideStr, 16)

    module.plotDataAtAddress(address, count, stride)
end

forms.addMenuItem(
    getMainForm().Menu.Items,
    {"Halo CE", "Plot Data from Address"},
    function(item)
        item.OnClick = function(sender)
            module.promptToPlot()
        end
    end
)

return module
