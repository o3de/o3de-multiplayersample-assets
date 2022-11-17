#----------------------------------------------------------------------------
# Copyright Persistant Studios, SARL. All Rights Reserved.
# https://www.popcornfx.com/terms-and-conditions/
#----------------------------------------------------------------------------

import os
import sys
from shutil import copyfile

# Parse arguments
if len(sys.argv) != 3:
    print('Incorrect number of args')
    exit()

engine_path = sys.argv[1]
if not os.path.exists(engine_path):
    print(f'Given path {engine_path} does not exist')
    exit()

project_path = sys.argv[2]
if not os.path.exists(project_path):
    print(f'Given path {project_path} does not exist')
    exit()

sys.path.insert(0, os.path.join(engine_path, 'Gems/Atom/RPI/Tools/'))

from atom_rpi_tools.pass_data import PassTemplate
import atom_rpi_tools.utils as utils

# Folder of this py file
dir_name = os.path.dirname(os.path.realpath(__file__))

# Copy vector texture preset

srcPresetPath = os.path.join(dir_name, 'PopcornVectors.preset')
pathToFolder = os.path.join(project_path, 'Config/AtomImageBuilder')
try:
    os.stat(pathToFolder)
except:
    os.mkdir(pathToFolder) 
dstPresetPath = os.path.join(pathToFolder, 'PopcornVectors.preset')
try:
    copyfile(srcPresetPath, dstPresetPath)
except:
    print('Error copying vector texture preset')
    exit()
print('Copying vector texture preset to project')

# Patch render pipeline (add distortion)

## Gem::Atom_Feature_Common gem's path since default render pipeline is comming from this gem
source_gem = 'Gem::Atom_Feature_Common'
gem_assets_path = os.path.join(engine_path,'Gems/Atom/feature/Common/Assets/')

pipeline_relatvie_path = 'Passes/MainPipeline.pass'
srcRenderPipeline = os.path.join(gem_assets_path, pipeline_relatvie_path)
destRenderPipeline = os.path.join(project_path, pipeline_relatvie_path)
## If the project doesn't have a customized main pipeline
## copy the default render pipeline from Atom_Common_Feature gem to same path in project folder
utils.find_or_copy_file(destRenderPipeline, srcRenderPipeline)

## Load project render pipeline
renderPipeline = PassTemplate(destRenderPipeline)

## Skip if DistortionPass already exist
newPassName = 'DistortionPass'
if renderPipeline.find_pass(newPassName)>-1:
    print('Skip merging. DistortionPass already exists')
    exit()
 
## Insert DistortionPass after TransparentPass
refPass = 'TransparentPass'
## The data file for new pass request is in the same folder of the py file
newPassRequestFilePath = os.path.join(dir_name, 'DistortionPass.data') 
newPassRequestData = utils.load_json_file(newPassRequestFilePath)
insertIndex = renderPipeline.find_pass(refPass) + 1
if insertIndex>-1:
    renderPipeline.insert_pass_request(insertIndex, newPassRequestData)
else:
    print('Failed to find ', refPass)
    exit()
 
## Update connections for all the passes after DistortionPass
renderPipeline.replace_references_after('DistortionPass', 'TransparentPass', 'InputOutput', 'DistortionPass', 'OutputColor')

## Save the updated render pipeline
renderPipeline.save()
