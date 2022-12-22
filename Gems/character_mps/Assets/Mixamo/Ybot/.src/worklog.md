# Mixamo, YBot, Worklog

This is a worklog of changes made to the YBot for MPS

## The files in /.src

These are production working files for the YBot model.  As production stages of art creation, often include file related to cleaning up work, creating baking workflows (to generate texture channels). We want to include these file for end-to-end examples of the work we did (for instance, a baking tutorial.)

| File                                  | Description                                                                                                                                   |
| ------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------- |
| native_mesh.fbx                       | The native Mixamo source file                                                                                                                 |
| Native_mesh_working.mb                | Used to generate smooth HIGH for baking                                                                                                       |
| Native_smooth_HIGH.fbx                | The HIGH poly exported out of Native_mesh_working.mb                                                                                          |
| Alpha_mesh_working.mb                 | This is the working file for the in-game mesh, it has the game optimized LOW with clean UV's for baking                                       |
| Alpha_mesh_LOW.fbx                    | The LOW poly exported from Alpha_mesh_working.mb                                                                                              |
| Bake_set_working.mb                   | Imports the HIGH and LOW to put into a import friendly hierarchy for Marmoset Toolbag (baking tool)                                           |
| Alpha_Bake_Set.fbx                    | Exported from Bake_set_working.mb, and is the file imported into Marmoset Toolbag directly                                                    |
| Marmoset_TB_Alpha_Bake.tbscene        | High to Low baking scene, no joints. This is used to bake normals on the body meshes.                                                         |
| Marmoset_TB_Alpha_Joints_Bake.tbscene | This bake scene includes the joints, so we can bake good AO, etc.                                                                             |
| Alpha_mesh_final.fbx                  | This is an export of the final cleaned up low poly game model ready for import into the actual runtime model (in the root folder above /.src) |

## [0.0.3] - 2022-12-21

Added section of this document above, to describe /.src files usage.

in Native_mesh_working.mb:

- I wanted to retain the original faceting of the native mesh (hard edges), so I redid the smoothing to generate the HIGH

- Split the head and body, so the bake set now has:
  
  - Alpha_Joints_HIGH
  
  - Native_Body_HIGH
  
  - Native_Head_HIGH

- Exported bake_set as:  Native_smooth_HIGH.fbx
  
  - Disabled the FBX triangulation flag, to retain nGons, for better subdivision compatibility during baking workflows.
  
  - This can be utilized as the HIGH for baking.

in Alpha_mesh_working.mb:

- Splitting head to it's own 0...1 UV shell, and separate material.
  
  - Alpha_Mesh_LOW|Alpha_Mesh_Head_LOW
  
  - Alpha_Mesh_LOW|Alpha_Mesh_Body_LOW

- Alpha_Mesh_Head_LOW: re-packed UV into it's own shell/tile
  
  - Used 'poke' on nGons to make sure we have a clean mesh, on both parts (head and body)
  
  - StandardPBR doesn't have configurable texture samplers yet, so made sure to leave some padding on the UV shell edges, avoid sampling over with wrapping in lower mips (this avoids visible seams from appearing.)
  
  - Packed for a 1024x map. (will actually bake 2048, we can always optimize.)

- Alpha_Mesh_Body_LOW: re-packed, to optimize remaining shells (no head)
  
  - Packed for 2048x (will bake at 4096)
  
  - Also fixed nGons, also padded slightly

- Exported Alpha_Mesh_LOW (group)
  
  - Need to ensure FBX triangulate flag is enabled, because we are going to allow Marmoset to calculate/utilize MikkT tangent space and we want to ensure consistent triangle splitting occurs. (same flag will be set for game export.)

- Make the Alpha_mesh_final (group):
  
  - duplicated a copy of the head and body into this group
  
  - duplicate the body again for the right-hand side, mirror/flip this, and remove the center body polys so we just have a copy of the arm and leg
  
  - then create a cube
    
    - select the cube, then select the original body, the select the dup/mirrored arm and leg, and combine them.  after combine, remove the 6 faces of the cube and rename the object, then delete history, rename and parent back into group to keep organized.
    
    - if you don't combine into a cube (or another clean mesh), your mesh might be incorrectly flipped in another app (like Marmoset), but combining into another clean mesh like a cube is a hack that internally adds all the polys to the existing mesh and internally corrects the facing, normals, etc.
  
  - Export group as Alpha_mesh_final.fbx
    
    - Do not need to enable the FBX triangulation flag, this is not the actual runtime model

- In Bake_set_working.mb (where we build a baking friendly file)
  
  - imported the new Alpha_Mesh_LOW.fbx
  
  - imported the new Native_smooth_HIGH.fbx
  
  - organized into hierarchy groups
  
  - export to Alpha_Bake_Set.fbx (for import to Marmoset as single file)

- In Marmoset_TB_Alpha_Bake.tbscene
  
  - It should automatically refresh the reference/import of Alpha_Bake_Set.fbx
    
    - all the HIGH and LOW meshes are in this group
    
    - In this file no joints, we won't use Alpha_Joints_HIGH in any Bake Project!
  
  - Renamed Bake Project 1, to Bake Project Head
    
    - Duplicated all HIGHs and LOWs into this, and set up the Bake Group
      
      - The LOW only needs: Alpha_Mesh_Head_LOW
    
    - Tangent Space should be MikkT, and Per-Vertex for all meshes
    
    - Updated this group so the bake output goes to:
      
      - Texture_Bake_Channels/No_Joints/Alpha_bake_head.psd
  
  - Duplicated, named second Bake Project Body
    
    - Mostly the same steps as above...
    
    - But we will use Alpha_Joints_HIGH in bake projects
    
    - The LOW only needs: Alpha_Mesh_Body_LOW
    
    - Output switches to:
      
      - Texture_Bake_Channels/No_Joints/Alpha_bake_Body.psd
  
  - Baked both Bake Projects (head, body)
  
  - Set up Marmoset set materials Head_Mat and Body_MAT, so we can preview baked texture channels on the LOW meshes.
  
  - Baked texture channels
  
  - Previewed, tweaked bake cage, baked again

- In Marmoset_TB_Alpha_Joints_Bake.tbscene
  
  - All of the same type of update alteration as above (or you could 'save as' this file over the other, then update bake projects)
  
  - With these additional alterations ...
  
  - Both bake projects need Alpha_Joints_HIGH to be included in HIGH
  
  - Bake project outputs, switch to: Texture_Bake_Channels/**With_Joints**/*

- /Ybot (the asset folder)
  
  - in ybot_yup_working:
    
    - set the scene settings to centimeter
    
    - cleaned out the rig/model, to be replaced
    
    - imported the /.src/native_mesh.fbx (want native rig)
    
    - parented rig under Armature (locator)
    
    - moved the original body mesh into REFERENCE
    
    - imported the /.src/Alpha_mesh_final.fbx
    
    - included the meshes in a group: body, head, joints, logo plate
    
    - made sure mesh transforms were frozen, then deleted history on them
    
    - then used 'bind skin' on them, using mixamorig:Hips as the selected root node
    
    - then with only the 'mesh group' selected, use the edit > LOD > generate LOD meshes.  This creates a 'LOD_Group_1' node, with grouped hierarchy of decimated LOD meshes beneath it (this can also propagate skinning, which is why I skinned first)
    
    - NOTE: you really need to clean up the node names after creating the 'LOD group node', as they don't match the default soft naming conventions:
      
      - LOD_0 changed to LOD0, as an example
      
      - the on the sub-meshes also added _lod0
      
      - this helps make sure that all the automated mesh groups in scene settings are handled correctly
  
  - selected the YBot_LOD_Group and Armature, used the Game Exporter, to export the FBX, with skinning, etc.  make sure that the triangulation flag is on.
    
    - exported as YBot.fbx
  
  - In Editor
    
    - opened the Material Editor and fixed-up the ybot_body_base and ybot_head_base materials (corrected texture assignment and such.)
    
    - Then put the Ybot ActorAsset into a Entity with Actor Component to load the ybot, and then a Simple Motion Component - and validated several mixamo animations (the skinning needs to be improve, letting Alex D handle that.)  What's important (Ithink), is that the motions work without 'retarget' enabled.

## [0.0.2] - 2022-12-14

Added the right hand side (flip/mirror) of arms and legs into the HIGH bake model set, so they are accounted for in the AO bake pass (inner thighs, etc.) 

Tweaked the bake settings in the marmoset files

Adjusted the normals on the bake set geometry.

## [0.0.2] - 2022-12-13

Updated the tangent orientation settings in marmoset to left handed

Updated the bake settings in marmoset from per-pixel to per-vertex

Rebake of texture channels and texture updates

## [0.0.1] - 2022-12-13

Baked textures with Marmoset Toolbag (bake scene files included in .src)

    set the HIGH with 3 subD levels to smooth out sampling anomlies

    baked one set with Joints (for occlusion)

    baked second set without joints (so joints aren't sampled in normals)

Added ybot\base.material with basecolor, normals and AO

Added mixamo logo textures

## [0.0.0] - 2022-12-09

created nest .src\ directory

modified the Gems asset processor setreg, to exclude scanning .src\

Added \Tools dir to the Gem root (so I can start Maya, with Gem as project root)

downloaded the original native YBot into .src\

    .src\YBot.fbx

opened our alpha Gems\character_mps\Assets\Mixamo\Ybot\ybot.fbx (optimized)

exported the Alpha_Surface, and Alpha_Joints meshes, into .src\

    - no triangulation (keep quads)

    - no animation (I am just doing UVs, and material baking)

    .src\alpha_mesh.fbx

opened the alpha_mesh.fbx

    - removed armature (will have to re-skin, transfer weights anyway)

    - grouped together

    - deleted all history

    - saved as alpha_mesh.mb (working maya file)

Then did the same with the original native YBot

    - saved as native_mesh.mb

Going to set up the native_mesh as High-res (for baking)

    - we probably don't need to bake the joint meshes, so just leave in place

    - going to avoid mirror on head/chest

    - but going to encourage mirroring arms/legs (so remove one side)

    - see 00_native_mesh.png

The meshes have both soft/hard edges, I want a 'clean bake'

    - going to smooth with creases, then soften.

    - see 01_smooth_mesh.png

    - duplicated group of smoothed meshes, clean combined: 

        - combined with 'world origin'

        - after combine, deleted all history, named Native_Smooth_HIGH

Exported this single combined mesh to: .src\Native_Smooth_HIGH.fbx

    - exported as FBX

    - with triangulation enabled

    - without tangent/binormals (going to bake MikkT)

Now can close the .src\native_mesh.mb (what we needed, was the HIGH poly to bake against)

Move onto UV unwrapping our customized mesh ...

Open our Gems\character_mps\Assets\Mixamo\Ybot\.src\alpha_mesh.mb (working file, with the optimized model, no rig)

- Hiding the joints spheres (for now, will still need to unwrap them)
  
  - I just don't think we will need baked normals, so delaying

- Duplicated the Alpha_surface mesh
  
  - Separated it, then renamed this group: Alpha_Surface_UVs
  
  - Deleted all history

- Remove the right arm/leg (to match the HIGH poly)

- Now UV unwrapping each piece (26 total)

The orignal UVs are kind of a mess (see original_UVs.png)

- unwrapped each piece (unfold, move/sew, etc)

- layout, packed UVs (see clean_UVs.png)

- made all edges soft (preserves creases in HIGH, smooth meshes bake better with MikkT workflow)

- duplicated group of UV meshes, clean combined:

        - combined with 'world origin'

        - after combine, deleted all history, named Alpha_Smooth_LOW

Exported this single combined mesh to: .src\Alpha_Smooth_LOW.fbx

    - exported as FBX

    - with triangulation enabled

    - without tangent/binormals (going to bake MikkT in Marmoset)
