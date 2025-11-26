#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/InterfacePointers.hpp>
#include <tier1/iconvar.h>
#include <tier1/convar.h>
#include <string>
#include <map>

// helper struct to access private m_nFlags member
struct ConCommandBaseAccessor {
    void* vtable;
    ConCommandBase* m_pNext;
    bool m_bRegistered;
    const char* m_pszName;
    const char* m_pszHelpString;
    int m_nFlags;
};

std::map<std::string, ConCommandBase*> removedCommands; 
std::map<std::string, FnCommandCallback_t> blockedCommands;
std::map<std::string, int> uncheatCommands;

void BlockingCallback(const CCommand &args) {
    // silently block the command
}

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
        return;
    }
    
    ConCommand* cmd = static_cast<ConCommand*>(cmdBase);
    
    if (blockedCommands.find(command) == blockedCommands.end()) {
        FnCommandCallback_t* callbackPtr = reinterpret_cast<FnCommandCallback_t*>(
            reinterpret_cast<char*>(cmd) + sizeof(ConCommandBase)
        );
        
        blockedCommands[command] = *callbackPtr;
        *callbackPtr = &BlockingCallback;
    }
}

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
        if (removedCommands.find(command) == removedCommands.end()) {
            removedCommands[command] = cmd;
            icvar->UnregisterConCommand(cmd);
        }
    }
}

void UnblockCommand(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }

    ICvar *icvar = InterfacePointers::Cvar();
    if (!icvar) {
        return;
    }

    auto it = blockedCommands.find(command);
    if (it != blockedCommands.end()) {
        ConCommandBase* cmdBase = icvar->FindCommandBase(command);
        if (cmdBase && cmdBase->IsCommand()) {
            ConCommand* cmd = static_cast<ConCommand*>(cmdBase);
            
            FnCommandCallback_t* callbackPtr = reinterpret_cast<FnCommandCallback_t*>(
                reinterpret_cast<char*>(cmd) + sizeof(ConCommandBase)
            );
            *callbackPtr = it->second;
            
            blockedCommands.erase(it);
        }
    }
}

void RestoreCommand(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }

    ICvar *icvar = InterfacePointers::Cvar();
    if (!icvar) {
        return;
    }

    auto it = removedCommands.find(command);
    if (it != removedCommands.end()) {
        ConCommandBase* cmd = it->second;
        if (cmd) {
            icvar->RegisterConCommand(cmd);
            removedCommands.erase(it);
        }
    }
}

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
        if (uncheatCommands.find(command) == uncheatCommands.end()) {
            ConCommandBaseAccessor* accessor = reinterpret_cast<ConCommandBaseAccessor*>(cmd);
            
            uncheatCommands[command] = accessor->m_nFlags;
            accessor->m_nFlags &= ~FCVAR_CHEAT;
        }
    }
}

void RecheatCommand(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }

    ICvar *icvar = InterfacePointers::Cvar();
    if (!icvar) {
        return;
    }

    auto it = uncheatCommands.find(command);
    if (it != uncheatCommands.end()) {
        ConCommandBase* cmd = icvar->FindCommandBase(command);
        if (cmd) {
            ConCommandBaseAccessor* accessor = reinterpret_cast<ConCommandBaseAccessor*>(cmd);
            accessor->m_nFlags = it->second;
            uncheatCommands.erase(it);
        }
    }
}

LUA_FUNCTION(BlockCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        BlockCommand(command);
    }
    
    return 0;
}

LUA_FUNCTION(RemoveCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        RemoveCommand(command);
    }
    
    return 0;
}

LUA_FUNCTION(UnblockCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        UnblockCommand(command);
    }
    
    return 0;
}

LUA_FUNCTION(RestoreCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        RestoreCommand(command);
    }
    
    return 0;
}

LUA_FUNCTION(UncheatCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        UncheatCommand(command);
    }
    
    return 0;
}

LUA_FUNCTION(RecheatCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* command = LUA->GetString(1);

    if (command && strlen(command) > 0) {
        RecheatCommand(command);
    }
    
    return 0;
}

GMOD_MODULE_OPEN() {
    Msg("\n");
    Msg("=========================================\n");
    Msg("  CommandManager v1.0\n");
    Msg("  Created by Linda Black\n");
    Msg("=========================================\n");
    Msg("\n");
    
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

GMOD_MODULE_CLOSE() {
    for (const auto& pair : blockedCommands) {
        UnblockCommand(pair.first.c_str());
    }
    blockedCommands.clear();
    
    for (const auto& pair : removedCommands) {
        RestoreCommand(pair.first.c_str());
    }
    removedCommands.clear();
    
    for (const auto& pair : uncheatCommands) {
        RecheatCommand(pair.first.c_str());
    }
    uncheatCommands.clear();
    
    return 0;
}