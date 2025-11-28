#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/InterfacePointers.hpp>
#include <tier1/iconvar.h>
#include <tier1/convar.h>
#include <string>
#include <map>

struct ConCommandAccessor {
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
    if (!command || !g_pCvar) return;

    ConCommandBase* cmdBase = g_pCvar->FindCommandBase(command);
    if (!cmdBase || !cmdBase->IsCommand()) return;
    
    if (blockedCommands.find(command) != blockedCommands.end()) return;
    
    ConCommand* cmd = static_cast<ConCommand*>(cmdBase);
    FnCommandCallback_t* callbackPtr = reinterpret_cast<FnCommandCallback_t*>(
        reinterpret_cast<char*>(cmd) + sizeof(ConCommandBase)
    );
    
    blockedCommands[command] = *callbackPtr;
    g_pCvar->UnregisterConCommand(cmd);
}

void RestoreCommand(const char* command) {
    if (!command || !g_pCvar) return;

    auto it = blockedCommands.find(command);
    if (it == blockedCommands.end()) return;
    
    ConCommandBase* cmdBase = g_pCvar->FindCommandBase(command);
    if (!cmdBase || !cmdBase->IsCommand()) return;
    
    ConCommand* cmd = static_cast<ConCommand*>(cmdBase);
    FnCommandCallback_t* callbackPtr = reinterpret_cast<FnCommandCallback_t*>(
        reinterpret_cast<char*>(cmd) + sizeof(ConCommandBase)
    );
    
    *callbackPtr = it->second;
    g_pCvar->RegisterConCommand(cmd);
    blockedCommands.erase(it);
}

void UncheatCommand(const char* command) {
    if (!command || !g_pCvar) return;

    ConCommandBase* cmd = g_pCvar->FindCommandBase(command);
    if (!cmd) return;
    
    if (uncheatCommands.find(command) != uncheatCommands.end()) return;
    
    ConCommandAccessor* accessor = reinterpret_cast<ConCommandAccessor*>(cmd);
    uncheatCommands[command] = accessor->m_nFlags;
    accessor->m_nFlags &= ~FCVAR_CHEAT;
}

void RecheatCommand(const char* command) {
    if (!command || !g_pCvar) return;

    auto it = uncheatCommands.find(command);
    if (it == uncheatCommands.end()) return;
    
    ConCommandBase* cmd = g_pCvar->FindCommandBase(command);
    if (!cmd) return;
    
    ConCommandAccessor* accessor = reinterpret_cast<ConCommandAccessor*>(cmd);
    accessor->m_nFlags = it->second;
    uncheatCommands.erase(it);
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
    
    auto it = blockedCommands.find(cmdName);
    if (it == blockedCommands.end()) return 0;

    CCommand ccmd;
    if (LUA->IsType(2, GarrysMod::Lua::Type::String)) {
        const char* args = LUA->GetString(2);
        if (args) ccmd.Tokenize(args);
    }

    it->second(ccmd);
    return 0;
}

GMOD_MODULE_OPEN() {
    g_pCvar = InterfacePointers::Cvar();

    Msg("\n=========================================\n");
    Msg("  CommandManager v1.1\n");
    Msg("  Created by Linda Black\n");
    Msg("=========================================\n\n");
    
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
