# Kithbash3D (KB3D) Fix Up

This document describes the process of fixing up a KB3D kit for Open 3D Engine.

## Pass 1

We downloaded the native kit:

`o3de-multiplayersample-assets\Gems\kb3d_mps\.src\KB3D_HighTechStreets\KB3D_HiTechStreets-Native.ma`

This is one scene, with all of the objects spread out.  What we need, is this scene broken out into individual objects, each centered at the origin.

First, we saved as a binary working file, and made sure things like relative texture paths were in place.

`o3de-multiplayersample-assets\Gems\kb3d_mps\.src\KB3D_HighTechStreets\KB3D_HighTechStreets.mb`

Next, we made a pruned set, of just the objects we were going to use in a modified form in this project.

`o3de-multiplayersample-assets\Gems\kb3d_mps\.src\KB3D_HighTechStreets\KB3D_HighTechStreets_prune.mb`

This file was also exported as a FBX `KB3D_HighTechStreets_prune.fbx`

We ran a exporter tool on this `Maya > O3DE > Scene Exporter`

And this generated all of the individual object directories, which include a main FBX source file and generated Atom (the renderer) material files for Open3Dengine (O3DE)

`o3de-multiplayersample-assets\Gems\kb3d_mps\Assets\KB3D_HighTechStreets\Objects\HTS_Bench_A\*`

`o3de-multiplayersample-assets\Gems\kb3d_mps\Assets\KB3D_HighTechStreets\Objects\HTS_BldgLG_D\*`

`... etc`

Those all contain per-object materials for assignment, but the materials all reshare the game textures from the shared folder:

`o3de-multiplayersample-assets\Gems\kb3d_mps\Assets\KB3D_HighTechStreets\KB3DTextures`

Next we may need to do additional work on each asset to prepare them for full 'out of the box' use in O3DE.

## Pass 2

Here are some of the things we may need to do to condition assets:

- We may need to generate custom collision proxy geo (_phys) 

- We may need to split up a model and reassemble it in a prefab

- We may need to separate transparent meshes for draw order

### Getting Started

1. Start Maya from "o3de-multiplayersample-assets\Gems\kb3d_mps\Tools\Launch_Maya.bat"
- This will use the Gem as the project root (workspace.mel)

- This ensures portable/relative asset paths for textures

- Make sure Maya prefs are Z-up (the exported KB3D fbx files are z-up)

### Fixing up a Kitbash3D (KB3D) object

Using this object as an example:

`o3de-multiplayersample-assets\Gems\kb3d_mps\Assets\KB3D_HighTechStreets\Objects\HTS_BldgLG_D`

1. Open the per-object source FBX file (opening FBX in Maya, is the same as importing into an empty scene.)

2. Save as a Maya binary (.mb), we used the following naming convention: `Maya2023_HTS_BldgLG_D.mb`

#### Game Exporter, Scene Hierarchy

We are going to set this file up so we can use the game exporter, which can either 'export all', 'export selected' or 'export Object Set'.

We are going to use 'Export Object Set', as this will allow us to split the model such that multiple FBX files will be output.

Here is what the hierarchy looks like we prepared for export

<img src="../assets/img/posts/KB3D_Fix_Up-assets/2023-02-13-15-02-56-image.png" title="" alt="" width="325">

Here is a descriptions of how we rearranged the model:

1. DO_NOT_EXPORT
   
   1. Planters and other object were put into this group, to be excluded from the Object Export Set.  These can be separate modular pieces, and using nested prefabs can be optionally brought back in (this is an optimization for polycount.)
   
   2. One Copy of the Planter, was moved to `o3de-multiplayersample-assets\Gems\kb3d_mps\Assets\KB3D_HighTechStreets\Objects\HTS_Planter_B` so we could use it this way.

2. ObservationDeck
   
   1. This is everything in the Observation deck except the glass and transparent objects were split off (so we can sort their draw order)
   
   2. An optimized collision mesh was generated: ObservationDeck_phys

3. ObservationDeck_draw0
   
   1. This is the inner layer of transparent screens

4. ObservationDeck_draw1
   
   1. This is the secound outer layer of glass (draws over the screens)

5. Main
   
   1. Just the tower
   
   2.  An optimized collision mesh was generated: Main_phys

6. HoloA
   
   1. The holo screen above the tower door

#### Game Exporter

And the Maya Game Exporter is configured like this

![](../assets/img/posts/KB3D_Fix_Up-assets/2023-02-13-15-03-13-image.png)

Things to make note of:

1. Even though we split it up, we are going to leave as-is the original source `HTS_BldgLG_D.fbx` (we are just not going to utilize it.)

2. We are exporting as multiple FBX files into the same folder, with the setting `Export to Multiple Files`

3. The prefix for the FBX exports is: `HTS_BldgLG_D_`

4. That will be combined with the node name for each in the Export Object Set, so we should end up with:
   
   1. HTS_BldgLG_D_ObservationDeck.fbx
   
   2. HTS_BldgLG_D_ObservationDeck_draw0.fbx
   
   3. HTS_BldgLG_D_ObservationDeck_draw1.fbx
   
   4. HTS_BldgLG_D_Main.fbx
   
   5. HTS_BldgLG_D_HoloA.fbx

## O3DE Prefab

Now we can reconstruct the parts into a prefab in the O3DE Editor.

### Per-FBX Piece

I basically do all of this for each prefab piece we exported in the last step.

1. Delete the existing prefab file if it's there (we are going to replace it), in this case: `o3de-multiplayersample-assets\Gems\kb3d_mps\Assets\KB3D_HighTechStreets\Objects\HTS_BldgLG_D\HTS_BldgLG_D.prefab`

2. You can validate each fbx that was exported, and what it's contents are. Find the fbx in the Asset Browser, select it, and use the action `Edit Scene Settings ...` to launch the tool.
   
   1. I generally use the tab labeled `Procedrual Prefab` and remove the procprefab rule (it'll just generate extra rules and product outputs we don't need.)
   
   2. These models have one or more color per-vertex (CPV) sets, which look like they came from 3dsMax (probably when the objects were created), we aren't using them so you can also disable the `Vertex Color Stream` option.

3. You'll have something like this afterwards

4. ![](../assets/img/posts/KB3D_Fix_Up-assets/2023-02-13-15-17-25-image.png)

### Prefab Hierarchy

Now you'll assemble a nested set of entities that assemble the FBX pieces into a single assembly we will save to replace the original prefab.  These generally will be a mix of the following components: mesh (load the fbx piece), physics components (make rigid body, load custom phys meshes), and the Material component (assign the shared materials to the surface assignment slots.)  For the sake of brevity, I am not going to log each individual entity and step, you can go dissect the prefab we provide.




