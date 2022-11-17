#----------------------------------------------------------------------------
# Copyright Persistant Studios, SARL. All Rights Reserved.
# https://www.popcornfx.com/terms-and-conditions/
#----------------------------------------------------------------------------

set(POPCORNFX_VERSION 2.14.1)
set(POPCORNFX_LICENSE O3DE)

if (PK_O3DE_MAJOR_VERSION GREATER_EQUAL 2205)
    o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_name}")
else()
    ly_get_list_relative_pal_filename(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} ${gem_restricted_path} ${gem_path} ${gem_name})
endif()

include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake) #for PAL_TRAIT_POPCORNFX_SUPPORTED Traits

if(NOT PAL_TRAIT_POPCORNFX_SUPPORTED)
    return()
endif()

add_compile_definitions(PK_O3DE_MAJOR_VERSION=${PK_O3DE_MAJOR_VERSION})
add_compile_definitions(PK_O3DE_PATCH_VERSION=${PK_O3DE_PATCH_VERSION})

ly_add_target(
    NAME PopcornFX.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        popcornfx_autogen_files.cmake
        popcornfx_shared_files.cmake
        ../Assets/popcornfx_asset_files.cmake
    COMPILE_DEFINITIONS
        PUBLIC
            O3DE_USE_PK
            PK_USE_PHYSX
            PK_USE_EMOTIONFX
            PK_USE_STARTINGPOINTINPUT
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzFramework
            AZ::AzCore
            AZ::AtomCore
            3rdParty::PopcornFX
            Gem::Atom_Utils.Static
            Gem::Atom_RPI.Public
            Gem::Atom_RHI.Public
            Gem::Atom_RHI.Reflect
            Gem::AtomLyIntegration_CommonFeatures.Static
            Gem::LyShine.Static
            Gem::LmbrCentral
            Gem::ScriptCanvas
            Gem::StartingPointInput.Static
            Gem::EMotionFXStaticLib
            Gem::PhysX.Static
    AUTOGEN_RULES
        *.ScriptCanvasGrammar.xml,ScriptCanvasGrammar_Header.jinja,$path/$fileprefix.generated.h
        *.ScriptCanvasGrammar.xml,ScriptCanvasGrammar_Source.jinja,$path/$fileprefix.generated.cpp
        *.ScriptCanvasNodeable.xml,ScriptCanvasNodeable_Header.jinja,$path/$fileprefix.generated.h
        *.ScriptCanvasNodeable.xml,ScriptCanvasNodeable_Source.jinja,$path/$fileprefix.generated.cpp
)

ly_add_target(
    NAME PopcornFX ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        popcornfx_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::PopcornFX.Static
    RUNTIME_DEPENDENCIES
        Gem::LmbrCentral
)

ly_create_alias(NAME PopcornFX.Clients NAMESPACE Gem TARGETS Gem::PopcornFX)
#ly_create_alias(NAME PopcornFX.Servers NAMESPACE Gem TARGETS Gem::PopcornFX)

if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_target(
        NAME PopcornFX.Editor GEM_MODULE
        NAMESPACE Gem
        AUTOUIC
        AUTOMOC
        AUTORCC
        FILES_CMAKE
            popcornfx_editor_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
                .
            PUBLIC
                Include
        COMPILE_DEFINITIONS
            PRIVATE
                POPCORNFX_EDITOR
        BUILD_DEPENDENCIES
            PRIVATE
                AZ::AssetBuilderSDK
                AZ::AzToolsFramework
                3rdParty::PopcornFXEditor
                Gem::PopcornFX.Static
        RUNTIME_DEPENDENCIES
            Gem::LmbrCentral.Editor
    )

    ly_create_alias(NAME PopcornFX.Builders NAMESPACE Gem TARGETS Gem::PopcornFX.Editor)
    ly_create_alias(NAME PopcornFX.Tools NAMESPACE Gem TARGETS Gem::PopcornFX.Editor)
endif()
