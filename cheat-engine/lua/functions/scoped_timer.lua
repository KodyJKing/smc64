if timers == nil then
    timers = {}
end

function destroyOldTimers()
    for k, v in pairs(timers) do
        v.destroy()
    end
    timers = {}
end

destroyOldTimers()

function createScopedTimer(owner, enabled)
    local timer = createTimer(owner, enabled)
    timer.Interval = interval
    timer.OnTimer = callback
    table.insert(timers, timer)
    return timer
end
