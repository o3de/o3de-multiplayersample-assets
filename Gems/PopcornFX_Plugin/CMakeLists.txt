#----------------------------------------------------------------------------
# Copyright Persistant Studios, SARL. All Rights Reserved.
# https://www.popcornfx.com/terms-and-conditions/
#----------------------------------------------------------------------------

#List of supported O3DE versions, uncomment one of the line to force the version
#set(PK_O3DE_ENGINE_VERSION "2210.0")
#set(PK_O3DE_ENGINE_VERSION "2205.0")
#set(PK_O3DE_ENGINE_VERSION "2111.2")
#set(PK_O3DE_ENGINE_VERSION "2111.1")

#Latest supported O3DE version
set(PK_O3DE_ENGINE_VERSION_LATEST "2210.0")

if (${PK_O3DE_ENGINE_VERSION})
    set(o3de_build_number ${PK_O3DE_ENGINE_VERSION})
elseif (${INSTALLED_ENGINE})
    o3de_read_json_key(o3de_build_number ${LY_ROOT_FOLDER}/engine.json "O3DEBuildNumber")
    if (o3de_build_number STREQUAL "" OR o3de_build_number STREQUAL "0")
        message(FATAL_ERROR "Invalid O3DEBuildNumber in 'engine.json'. Please set PK_O3DE_ENGINE_VERSION in the PopcornFX CMakeLists.txt.")
    endif()
else()
    o3de_read_json_key(o3de_version ${LY_ROOT_FOLDER}/engine.json "O3DEVersion")
    if (o3de_version STREQUAL "0.1.0.0")
        set(PK_O3DE_DEVELOPMENT ON)
        set(o3de_build_number ${PK_O3DE_ENGINE_VERSION_LATEST})
    else ()
        message(FATAL_ERROR "Could not determine the engine version using O3DEVersion in 'engine.json'. Please set PK_O3DE_ENGINE_VERSION in the PopcornFX CMakeLists.txt.")
    endif()
endif()

string(REPLACE "." ";" o3de_version_list ${o3de_build_number})
list(GET o3de_version_list 0 PK_O3DE_MAJOR_VERSION)
list(GET o3de_version_list 1 PK_O3DE_PATCH_VERSION)

if (${PK_O3DE_DEVELOPMENT})
    MATH(EXPR PK_O3DE_MAJOR_VERSION "${PK_O3DE_MAJOR_VERSION}+1")
endif()

set(gem_path ${CMAKE_CURRENT_LIST_DIR})
set(gem_json ${gem_path}/gem.json)
o3de_restricted_path(${gem_json} gem_restricted_path gem_parent_relative_path)

if (PK_O3DE_MAJOR_VERSION GREATER_EQUAL 2205)
    o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")
else ()
    o3de_read_json_key(gem_name ${gem_json} "gem_name")
    ly_get_list_relative_pal_filename(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" ${gem_path} ${gem_name})
endif ()

# Now that we have the platform abstraction layer (PAL) folder for this folder, thats where we will find the
# project cmake for this platform.
include(${pal_dir}/${PAL_PLATFORM_NAME_LOWERCASE}_gem.cmake)

function(pk_patch_file old_string new_string path)
    unset(shader_content)
    ly_file_read(${path} shader_content)
    string(REPLACE ${old_string} ${new_string} shader_content "${shader_content}")
    file(WRITE ${path} "${shader_content}")
endfunction()

if (PK_O3DE_MAJOR_VERSION GREATER 2210)
    pk_patch_file("\"BlendState\"" "\"GlobalTargetBlendState\"" ${gem_path}/Assets/shaders/Billboard/Default/Billboard.shader)
    pk_patch_file("\"BlendState\"" "\"GlobalTargetBlendState\"" ${gem_path}/Assets/shaders/Billboard/Default/BillboardDistortion.shader)
    pk_patch_file("\"BlendState\"" "\"GlobalTargetBlendState\"" ${gem_path}/Assets/shaders/Ribbon/Default/RibbonDistortion.shader)
    pk_patch_file("\"BlendState\"" "\"GlobalTargetBlendState\"" ${gem_path}/Assets/shaders/PostProcess/DistortionPostProcess.shader)
    pk_patch_file("PK_O3DE_NEW_LIT 0" "PK_O3DE_NEW_LIT 1" ${gem_path}/Assets/shaders/Common/PopcornOptions.azsli)
    pk_patch_file("PK_O3DE_NEW_PBR 0" "PK_O3DE_NEW_PBR 1" ${gem_path}/Assets/shaders/Common/PopcornOptions.azsli)
elseif (PK_O3DE_MAJOR_VERSION EQUAL 2210)
    pk_patch_file("\"BlendState\"" "\"GlobalTargetBlendState\"" ${gem_path}/Assets/shaders/Billboard/Default/Billboard.shader)
    pk_patch_file("\"BlendState\"" "\"GlobalTargetBlendState\"" ${gem_path}/Assets/shaders/Billboard/Default/BillboardDistortion.shader)
    pk_patch_file("\"BlendState\"" "\"GlobalTargetBlendState\"" ${gem_path}/Assets/shaders/Ribbon/Default/RibbonDistortion.shader)
    pk_patch_file("\"BlendState\"" "\"GlobalTargetBlendState\"" ${gem_path}/Assets/shaders/PostProcess/DistortionPostProcess.shader)
    pk_patch_file("PK_O3DE_NEW_LIT 0" "PK_O3DE_NEW_LIT 1" ${gem_path}/Assets/shaders/Common/PopcornOptions.azsli)
    pk_patch_file("PK_O3DE_NEW_PBR 1" "PK_O3DE_NEW_PBR 0" ${gem_path}/Assets/shaders/Common/PopcornOptions.azsli)
else ()
    pk_patch_file("\"GlobalTargetBlendState\"" "\"BlendState\"" ${gem_path}/Assets/shaders/Billboard/Default/Billboard.shader)
    pk_patch_file("\"GlobalTargetBlendState\"" "\"BlendState\"" ${gem_path}/Assets/shaders/Billboard/Default/BillboardDistortion.shader)
    pk_patch_file("\"GlobalTargetBlendState\"" "\"BlendState\"" ${gem_path}/Assets/shaders/Ribbon/Default/RibbonDistortion.shader)
    pk_patch_file("\"GlobalTargetBlendState\"" "\"BlendState\"" ${gem_path}/Assets/shaders/PostProcess/DistortionPostProcess.shader)
    pk_patch_file("PK_O3DE_NEW_LIT 1" "PK_O3DE_NEW_LIT 0" ${gem_path}/Assets/shaders/Common/PopcornOptions.azsli)
    pk_patch_file("PK_O3DE_NEW_PBR 1" "PK_O3DE_NEW_PBR 0" ${gem_path}/Assets/shaders/Common/PopcornOptions.azsli)
endif ()

add_subdirectory(Code)
