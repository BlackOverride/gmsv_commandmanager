#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/InterfacePointers.hpp>
#include <tier1/iconvar.h>
#include <tier1/convar.h>
#include <string>
#include <map>

struct ConCommandBaseAccessor {
    void* vtable;
    ConCommandBase* m_pNext;
    bool m_bRegistered;
    const char* m_pszName;
    const char* m_pszHelpString;
    int m_nFlags;
};

std::map<std::string, FnCommandCallback_t> blockedCommands;
std::map<std::string, int> uncheatCommands;

static ICvar* g_pCvar = nullptr;

void BlockCommand(const char* command) {
    if (!command || strlen(command) == 0 || !g_pCvar) {
        return;
    }

    ConCommandBase* cmdBase = g_pCvar->FindCommandBase(command);
    if (!cmdBase || !cmdBase->IsCommand()) {
        return;
    }
    
    ConCommand* cmd = static_cast<ConCommand*>(cmdBase);
    
    if (blockedCommands.find(command) == blockedCommands.end()) {
        FnCommandCallback_t* callbackPtr = reinterpret_cast<FnCommandCallback_t*>(
            reinterpret_cast<char*>(cmd) + sizeof(ConCommandBase)
        );
        
        blockedCommands[command] = *callbackPtr;
        g_pCvar->UnregisterConCommand(cmd);
    }
}

void RestoreCommand(const char* command) {
    if (!command || strlen(command) == 0 || !g_pCvar) {
        return;
    }

    auto it = blockedCommands.find(command);
    if (it != blockedCommands.end()) {
        ConCommandBase* cmdBase = g_pCvar->FindCommandBase(command);
        if (cmdBase && cmdBase->IsCommand()) {
            ConCommand* cmd = static_cast<ConCommand*>(cmdBase);
            
            g_pCvar->RegisterConCommand(cmd);
            blockedCommands.erase(it);
        }
    }
}

void UncheatCommand(const char* command) {
    if (!command || strlen(command) == 0 || !g_pCvar) {
        return;
    }

    ConCommandBase* cmd = g_pCvar->FindCommandBase(command);
    if (cmd) {
        if (uncheatCommands.find(command) == uncheatCommands.end()) {
            ConCommandBaseAccessor* accessor = reinterpret_cast<ConCommandBaseAccessor*>(cmd);
            
            uncheatCommands[command] = accessor->m_nFlags;
            accessor->m_nFlags &= ~FCVAR_CHEAT;
        }
    }
}

void RecheatCommand(const char* command) {
    if (!command || strlen(command) == 0 || !g_pCvar) {
        return;
    }

    auto it = uncheatCommands.find(command);
    if (it != uncheatCommands.end()) {
        ConCommandBase* cmd = g_pCvar->FindCommandBase(command);
        if (cmd) {
            ConCommandBaseAccessor* accessor = reinterpret_cast<ConCommandBaseAccessor*>(cmd);
            accessor->m_nFlags = it->second;
            uncheatCommands.erase(it);
        }
    }
}

LUA_FUNCTION(BlockCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    BlockCommand(LUA->GetString(1));
    return 0;
}

LUA_FUNCTION(RestoreCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    RestoreCommand(LUA->GetString(1));
    return 0;
}

LUA_FUNCTION(UncheatCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    UncheatCommand(LUA->GetString(1));
    return 0;
}

LUA_FUNCTION(RecheatCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    RecheatCommand(LUA->GetString(1));
    return 0;
}

LUA_FUNCTION(ExecuteBlockedCommandLua) {
    LUA->CheckType(1, GarrysMod::Lua::Type::String);
    const char* cmdName = LUA->GetString(1);
    const char* args = nullptr;

    if (LUA->IsType(2, GarrysMod::Lua::Type::String))
        args = LUA->GetString(2);

    auto it = blockedCommands.find(cmdName);
    if (it == blockedCommands.end()) return 0;

    FnCommandCallback_t cb = it->second;

    CCommand ccmd;
    if (args) ccmd.Tokenize(args);

    cb(ccmd);
    return 0;
}

GMOD_MODULE_OPEN() {
    g_pCvar = InterfacePointers::Cvar();

    Msg("\n");
    Msg("=========================================\n");
    Msg("  CommandManager v1.1\n");
    Msg("  Created by Linda Black\n");
    Msg("=========================================\n");
    Msg("\n");
    
    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
        LUA->PushCFunction(BlockCommandLua);
        LUA->SetField(-2, "BlockCommand");
        
        LUA->PushCFunction(RestoreCommandLua);
        LUA->SetField(-2, "RestoreCommand");
        
        LUA->PushCFunction(UncheatCommandLua);
        LUA->SetField(-2, "UncheatCommand");
        
        LUA->PushCFunction(RecheatCommandLua);
        LUA->SetField(-2, "RecheatCommand");
        
        LUA->PushCFunction(ExecuteBlockedCommandLua);
        LUA->SetField(-2, "ExecuteBlockedCommand");
    LUA->Pop();

    return 0;
}

GMOD_MODULE_CLOSE() {
    for (const auto& pair : blockedCommands) {
        RestoreCommand(pair.first.c_str());
    }
    blockedCommands.clear();
    
    for (const auto& pair : uncheatCommands) {
        RecheatCommand(pair.first.c_str());
    }
    uncheatCommands.clear();
    
    return 0;
}
