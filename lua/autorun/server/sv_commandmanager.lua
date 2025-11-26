if not pcall(require, "commandmanager") then
    print("CommandManager plugin not found, you can ignore this if you dont want this plugin.")
    return
end

-- block the 'status' command (only console can run this)
BlockCommand("status")

-- unblock the 'status' command (players can use it again on their console)
--UnblockCommand("status")

-- remove the 'lua_dumptimers_sv' command (cannot be run on server anymore even by console)
RemoveCommand("lua_dumptimers_sv")

-- restore the 'lua_dumptimers_sv' command we deleted
--RestoreCommand("lua_dumptimers_sv")

-- remove cheat flag from command so we can use it without sv_cheats 1 (this will only work on commands that can be executed by server no on client realm.)
--UncheatCommand("sv_showlagcompensation")

-- put the cheat flag back on command (requires sv_cheats 1 again)
--RecheatCommand("sv_showlagcompensation")