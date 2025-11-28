if not _G.CommandManagerLoaded then
    _G.CommandManagerLoaded = true
    if not pcall(require, "commandmanager") then
        print("CommandManager plugin not found, you can ignore this if you dont want this plugin.")
        return
    end
end

-- wrapper to create Lua command that checks player vs console
_RemoveCommand = _RemoveCommand or BlockCommand -- save our original callback first
function BlockCommand(cmdName)
    _RemoveCommand(cmdName)

    concommand.Add(cmdName, function(ply, cmd, args, argStr)
        if IsValid(ply) then ply:PrintMessage(HUD_PRINTCONSOLE, "Unknown command: " .. cmdName) return end -- if its a player then we dont want it so return early
        ExecuteBlockedCommand(cmdName, argStr or "") -- call C++ to execute the original command
    end)
end

function RemoveCommand(cmdName)
    _RemoveCommand(cmdName)
    concommand.Add(cmdName, function(ply, cmd, args, argStr)
        if IsValid(ply) then ply:PrintMessage(HUD_PRINTCONSOLE, "Unknown command: " .. cmdName) return -- if its a player then we dont want it so return early
		else print(string.format('Unknown command: "%s"', cmdName)) return end
    end)
end

-- block the 'status' command (only console can run this)
BlockCommand("status")

-- unblock the 'status' command (players can use it again)
--RestoreCommand("status")

-- to completely remove a command
--RemoveCommand("lua_dumptimers_sv")

-- restore removed command
-- RestoreCommand("lua_dumptimers_sv")

-- completely remove a command with no way to restore!
_RemoveCommand("lua_dumptimers_sv")

-- remove cheat flag from command (server-side commands only)
--UncheatCommand("sv_showlagcompensation")

-- restore cheat flag
--RecheatCommand("sv_showlagcompensation")
