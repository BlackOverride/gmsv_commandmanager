if not _G.CommandManagerLoaded then
    _G.CommandManagerLoaded = true
    if not pcall(require, "commandmanager") then
        print("CommandManager plugin not found, you can ignore this if you dont want this plugin.")
        return
    end
end

__BlockCommand = __BlockCommand or BlockCommand -- dont remove this line..

-- localize
local IsValid, print, HUD_PRINTCONSOLE, __BlockCommand = IsValid, print, HUD_PRINTCONSOLE, __BlockCommand

-- careful when tweaking this section --
--

function BlockCommand(cmdName)
    __BlockCommand(cmdName)
    local msg = "Unknown command: " .. cmdName
    concommand.Add(cmdName, function(ply, cmd, args, argStr)
        if IsValid(ply) then 
            ply:PrintMessage(HUD_PRINTCONSOLE, msg) 
            return 
        end
        ExecuteBlockedCommand(cmdName, argStr or "")
    end)
end

function RemoveCommand(cmdName)
    __BlockCommand(cmdName)
    local msgPly = "Unknown command: " .. cmdName
    local msgCon = string.format('Unknown command: "%s"', cmdName)
    concommand.Add(cmdName, function(ply)
        if IsValid(ply) then 
            ply:PrintMessage(HUD_PRINTCONSOLE, msg) 
        else 
            print(msgCon)
        end
    end)
end

-- careful when tweaking this section --
--

--
-- usage examples --

-- block the 'status' command (only console can run this)
BlockCommand("status")

-- unblock the 'status' command (players can use it again)
--RestoreCommand("status")

-- to completely remove a command from server console too
--RemoveCommand("lua_dumptimers_sv")

-- restore removed command
-- RestoreCommand("lua_dumptimers_sv")

-- completely remove a command with no way to restore!
DestroyCommand("lua_dumptimers_sv")

-- remove cheat flag from command (server-side commands only)
--UncheatCommand("sv_showlagcompensation")

-- restore cheat flag
--RecheatCommand("sv_showlagcompensation")
