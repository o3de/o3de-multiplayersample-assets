
# This will export the path to the directory containing the gem.json
# to the "SourcePaths" entry within the "cmake_dependencies.<project>.assetbuilder.setreg"
# which is generated when cmake is run
# This path is the gem root directory
if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_create_alias(NAME character_mps.Builders NAMESPACE Gem)
endif()
