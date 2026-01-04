local forms = reloadPackage("lua/functions/forms")
local messages = reloadPackage("lua/messages")

local IPC_BUFFER_SIZE = 128 -- (content size, not struct size)
local IPC_MESSAGE_SIZE = 1024

local processedUpToIndex = -1

function readMessage(address)
    local seq = readQword(address)
    local checksum = readQword(address + 8)
    local length = readInteger(address + 16)
    local data = readBytes(address + 20, length, true)
    return {
        seq = seq,
        checksum = checksum,
        length = length,
        data = data,
    }
end

function onMessage(message)
    -- print("Recieved message")
    -- pprint(message)

    -- -- Print as hex:
    -- local line = ''
    -- for i=1, message.length do
    --     line = line .. string.format("%02X ", message.data[i])
    -- end
    -- print(line)

    messages.handleMessage(message)
end

function processMessages(doNotFireMessages)
    local buffer = readQword("smc64.messageBuffer")
    local messageSize = readInteger("smc64.messageSize")

    function computeCheckSum(message)
        local checksum = 0
        for i = 1, message.length do
            checksum = (checksum + message.data[i]) & 0xFFFFFFFFFFFFFFFF
        end
        return checksum
    end

    for i = 0, IPC_BUFFER_SIZE - 1 do
        local messageAddress = buffer + i * messageSize
        local message = readMessage(messageAddress)
        if (message.length > 0) then
            local computedChecksum = computeCheckSum(message)
            if (computedChecksum ~= message.checksum) then
                print("  Invalid checksum! Computed:", hex(computedChecksum))
                break
            end
            if (message.seq > processedUpToIndex) then
                if not doNotFireMessages then
                    pcall(function() onMessage(message) end)
                end
                processedUpToIndex = message.seq
            end
        end
    end
end

function startIPCListener(interval)
    -- Mark pre-existing messages as processed
    pcall(function()
        processMessages(true)
    end)

    interval = interval or 100
    local timer = createScopedTimer(getMainForm())
    timer.Interval = interval
    timer.OnTimer = function()
        local success, err = pcall(processMessages)
        if not success then
            print("Error processing IPC messages:", err)
        end
    end
end

startIPCListener(100)

-- forms.addMenuItem(
--     getMainForm().Menu.Items, 
--     {"Halo CE", "Test", "Process Messages"}, 
--     function(item)
--         item.OnClick = function(sender)
--             processMessages()
--         end
--     end
-- )

-- forms.addMenuItem(
--     getMainForm().Menu.Items, 
--     {"Halo CE", "Test", "Start Process Messages"}, 
--     function(item)
--         item.OnClick = function(sender)
--             startIPCListener()
--         end
--     end
-- )
