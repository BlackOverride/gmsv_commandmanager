#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/InterfacePointers.hpp>
#include <GarrysMod/Symbol.hpp>
#include <tier1/iconvar.h>
#include <tier1/convar.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

// helper to access private m_nFlags member
struct ConCommandBaseAccessor {
    void* vtable;
    ConCommandBase* m_pNext;
    bool m_bRegistered;
    const char* m_pszName;
    const char* m_pszHelpString;
    int m_nFlags;  // this is what we want to access
};

// store removed commands so we can restore them later if needed
std::map<std::string, ConCommandBase*> removedCommands; 

// keep track of blocked commands with their original callbacks
std::map<std::string, FnCommandCallback_t> blockedCommands;

// keep track of uncheated commands and their original flags
std::map<std::string, int> uncheatCommands;

// our blocking callback - does nothing (blocks execution)
void BlockingCallback(const CCommand &args) {
    // silently block the command
}

// block a command from players (they can't run it, but console still can)
void BlockCommand(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }

    ICvar *icvar = InterfacePointers::Cvar();
    if (!icvar) {
        return;
    }

    ConCommandBase* cmdBase = icvar->FindCommandBase(command);
    if (!cmdBase || !cmdBase->IsCommand()) {
        return; // only works on commands, not convars
    }
    
    ConCommand* cmd = static_cast<ConCommand*>(cmdBase);
    
    // make sure were not blocking the same command twice
    if (blockedCommands.find(command) == blockedCommands.end()) {
        // get the callback function pointer
        // the callback is stored in m_fnCommandCallback at offset after ConCommandBase
        FnCommandCallback_t* callbackPtr = reinterpret_cast<FnCommandCallback_t*>(
            reinterpret_cast<char*>(cmd) + sizeof(ConCommandBase)
        );
        
        // store original callback
        blockedCommands[command] = *callbackPtr;
        
        // replace with our blocking callback
        *callbackPtr = &BlockingCallback;
    }
}

// completely remove a command from the server
void RemoveCommand(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }

    ICvar *icvar = InterfacePointers::Cvar();
    if (!icvar) {
        return;
    }

    ConCommandBase* cmd = icvar->FindCommandBase(command);
    if (cmd) {
        // dont remove if we already removed it
        if (removedCommands.find(command) == removedCommands.end()) {
            removedCommands[command] = cmd;
            icvar->UnregisterConCommand(cmd);
        }
    }
}

// unblock a command that was previously blocked
void UnblockCommand(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }

    ICvar *icvar = InterfacePointers::Cvar();
    if (!icvar) {
        return;
    }

    // check if this command was actually blocked by us
    auto it = blockedCommands.find(command);
    if (it != blockedCommands.end()) {
        ConCommandBase* cmdBase = icvar->FindCommandBase(command);
        if (cmdBase && cmdBase->IsCommand()) {
            ConCommand* cmd = static_cast<ConCommand*>(cmdBase);
            
            // restore original callback
            FnCommandCallback_t* callbackPtr = reinterpret_cast<FnCommandCallback_t*>(
                reinterpret_cast<char*>(cmd) + sizeof(ConCommandBase)
            );
            *callbackPtr = it->second;
            
            blockedCommands.erase(it);
        }
    }
}

// restore a command that was removed
void RestoreCommand(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }

    ICvar *icvar = InterfacePointers::Cvar();
    if (!icvar) {
        return;
    }

    // check if we actually removed this command
    auto it = removedCommands.find(command);
    if (it != removedCommands.end()) {
        ConCommandBase* cmd = it->second;
        if (cmd) {
            icvar->RegisterConCommand(cmd);
            removedCommands.erase(it);
        }
    }
}

// remove cheat flag from a command so it works without sv_cheats 1
void UncheatCommand(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }

    ICvar *icvar = InterfacePointers::Cvar();
    if (!icvar) {
        return;
    }

    ConCommandBase* cmd = icvar->FindCommandBase(command);
    if (cmd) {
        // make sure were not uncheating the same command twice
        if (uncheatCommands.find(command) == uncheatCommands.end()) {
            // cast to our accessor to read and modify private member
            ConCommandBaseAccessor* accessor = reinterpret_cast<ConCommandBaseAccessor*>(cmd);
            
            // store original flags
            uncheatCommands[command] = accessor->m_nFlags;
            
            // only remove the cheat flag
            accessor->m_nFlags &= ~FCVAR_CHEAT;
        }
    }
}

// restore cheat flag to a command (back to requiring sv_cheats 1)
void RecheatCommand(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }

    ICvar *icvar = InterfacePointers::Cvar();
    if (!icvar) {
        return;
    }

    // check if this command was actually uncheated by us
    auto it = uncheatCommands.find(command);
    if (it != uncheatCommands.end()) {
        ConCommandBase* cmd = icvar->FindCommandBase(command);
        if (cmd) {
            // cast to our accessor and restore original flags
            ConCommandBaseAccessor* accessor = reinterpret_cast<ConCommandBaseAccessor*>(cmd);
            accessor->m_nFlags = it->second;
            uncheatCommands.erase(it);
        }
    }
}

// lua function to block a command
LUA_FUNCTION(BlockCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        BlockCommand(command);
    }
    
    return 0;
}

// lua function to remove a command
LUA_FUNCTION(RemoveCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        RemoveCommand(command);
    }
    
    return 0;
}

// lua function to unblock a command
LUA_FUNCTION(UnblockCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        UnblockCommand(command);
    }
    
    return 0;
}

// lua function to restore a removed command
LUA_FUNCTION(RestoreCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        RestoreCommand(command);
    }
    
    return 0;
}

// lua function to remove cheat flag from a command
LUA_FUNCTION(UncheatCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        UncheatCommand(command);
    }
    
    return 0;
}

// lua function to restore cheat flag to a command
LUA_FUNCTION(RecheatCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        RecheatCommand(command);
    }
    
    return 0;
}

// module entry point
GMOD_MODULE_OPEN() {
    // print version and credits
    Msg("\n");
    Msg("=========================================\n");
    Msg("  CommandManager v1.0\n");
    Msg("  Created by Linda Black\n");
    Msg("=========================================\n");
    Msg("\n");
    
    // setup our lua functions so they can be called from server scripts
    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
        LUA->PushCFunction(BlockCommandLua);
        LUA->SetField(-2, "BlockCommand");
        
        LUA->PushCFunction(RemoveCommandLua);
        LUA->SetField(-2, "RemoveCommand");
        
        LUA->PushCFunction(UnblockCommandLua);
        LUA->SetField(-2, "UnblockCommand");
        
        LUA->PushCFunction(RestoreCommandLua);
        LUA->SetField(-2, "RestoreCommand");
        
        LUA->PushCFunction(UncheatCommandLua);
        LUA->SetField(-2, "UncheatCommand");
        
        LUA->PushCFunction(RecheatCommandLua);
        LUA->SetField(-2, "RecheatCommand");
    LUA->Pop();

    return 0;
}

// cleanup when module gets unloaded - restore everything back to normal
GMOD_MODULE_CLOSE() {
    // put back any commands we blocked
    for (const auto& pair : blockedCommands) {
        UnblockCommand(pair.first.c_str());
    }
    blockedCommands.clear();
    
    // put back any commands we completely removed
    for (const auto& pair : removedCommands) {
        RestoreCommand(pair.first.c_str());
    }
    removedCommands.clear();
    
    // put back cheat flags on any commands we uncheated
    for (const auto& pair : uncheatCommands) {
        RecheatCommand(pair.first.c_str());
    }
    uncheatCommands.clear();
    
    return 0;
}