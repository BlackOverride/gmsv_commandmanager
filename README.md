# CommandManager

a gmod binary module that gives you full control over engine commands - block them from players, remove them completely, or even bypass cheat restrictions without enabling sv_cheats.

## The problem

as a server developer, you've probably run into these annoying issues:

- **can't block certain commands from players** - stuff like `status`, or other commands that you want only server console to run? tough luck with vanilla gmod
- **cheat commands are locked behind sv_cheats 1** - want to use something like `sv_showlagcompensation` for debugging on your live server? you'd have to enable sv_cheats which opens up a whole can of worms and security issues
- **no way to completely remove unwanted commands** - some commands you just don't want available at all, but the engine doesn't give you an easy way to nuke them

basically, you're stuck with whatever valve decided to give you. until now.

## What this does

commandmanager acts as a bridge between lua and the engine's command system, giving you programmatic control over any console command. it lets you:

- **block commands from players** while still allowing server console to use them
- **completely remove commands** from existence (even console can't use them)
- **bypass cheat restrictions** on specific commands without enabling sv_cheats globally
- **restore everything back** to default state when needed

it's all done through simple lua functions that you can call from your server scripts.

## How it works

the plugin hooks directly into the source engine's command system (ICvar/ConCommand) and manipulates:
- command registration (to remove/restore commands)  
- command flags (to remove FCVAR_CHEAT requirements)

## Installation

1. download the appropriate binary for your server:
   - `gmsv_commandmanager_linux.dll` for 32-bit linux servers
   - `gmsv_commandmanager_linux64.dll` for 64-bit linux servers

2. place it in `garrysmod/lua/bin/`

3. copy the lua `garrysmod/addons/cmdmanager/lua/autorun/server/` with your command configurations:


## Available functions

### BlockCommand(command)
blocks a command from being executed by players, but server console can still use it.

```lua
BlockCommand("status")  -- players can't run status anymore
```

### RemoveCommand(command)
removes a command from the server. nobody can use it, not even console.

```lua
RemoveCommand("echo")  -- echo command is disabled for everyone
```

### RestoreCommand(command)
restore a command back if it was removed or blocked.

```lua
RestoreCommand("echo")  -- echo is back
```

### DestroyCommand(command)
destroy the command from everything including server console (cannot be restored)

```lua
DestroyCommand("echo")  -- echo command is gone completely
```

### UncheatCommand(command)
removes the cheat requirement from a command so it works without sv_cheats 1.

**important note:** this only works for server-side commands. client-side cheat commands (like `vcollide_wireframe`) won't work because the client does its own cheat checking.

```lua
UncheatCommand("sv_showlagcompensation")  -- now works without sv_cheats
```

### RecheatCommand(command)
restores the cheat requirement back to a command.

```lua
RecheatCommand("sv_showlagcompensation")  -- requires sv_cheats again
```

## Notes

- using RemoveCommand(cmd) and then BlockCommand(cmd) or vice versa will override each other.
- uncheat only works on server-side commands, not client cheat commands
- uncheat / recheat also works on cvars not only commands
- may also use as combination for example calling UncheatCommand("sv_showlagcompensation") and then calling BlockCommand("sv_showlagcompensation") to block from client console 
  (just incase your not sure if command can be executed from client side directly to server)


## Windows builds

i only needed linux builds for my servers, so that's all i compiled (x86 and x86_64). however, the code is cross-platform and should compile perfectly fine on windows using the same garrysmod_common setup. if you need windows builds, just run premake and compile it yourself - it'll work the same way.

## Building from source

you'll need [garrysmod_common](https://github.com/danielga/garrysmod_common) set up.

```bash
# generate project files
./premake5 --os=linux gmake --gmcommon=/path/to/garrysmod_common/

# compile for 64-bit
cd projects/linux/gmake
make config=release_x86_64

# compile for 32-bit  
make config=release_x86
```
