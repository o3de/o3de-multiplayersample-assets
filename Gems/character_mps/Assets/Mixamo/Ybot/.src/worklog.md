# Mixamo, YBot, Worklog

This is a worklog of changes made to the YBot for MPS

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
