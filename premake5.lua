-- Change to 3 if compiling to x86_64
PROJECT_GENERATOR_VERSION = 2

newoption({
    trigger = "gmcommon",
    description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
    value = "../garrysmod_common"
})

local gmcommon = _OPTIONS["gmcommon"] or os.getenv("GARRYSMOD_COMMON")
if gmcommon == nil then
    error("you didn't provide a path to your garrysmod_common directory")
end

include(gmcommon)

CreateWorkspace({name = "commandmanager", abi_compatible = true})
    CreateProject({serverside = true, manual_files = false})
        IncludeSDKCommon()
        IncludeSDKTier0()
        IncludeSDKTier1()
        IncludeHelpersExtended()

        files({
            "source/main.cpp"
        })
        
        filter({})
