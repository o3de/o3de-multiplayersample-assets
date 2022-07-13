# O3DE Terrain Experiment: Gaea (Quadspinner)

I wanted to make some terrain for Game Jam July 2022 

In the past, I used World Machine (WM) a lot, but it's old and slow.

I used the Gaea beta and wanted to re-acquaint myself with the latest version (so I can make prototype assets faster)

- It is node base, and from that satndapoint is pretty similar to WM

- But it's got a modern slick UX that feels better

- It's got pretty fast preview updates (I think much faster then WM)

- Preview and terrain details are more resolution independant, meaning the high-res build output actually looks like the preview (in world machine, sometimes things were resolution dependant. the final output didn't jive with previews and this could be frustrating.)

## Things to know

- Gaea doesn't have soemthing like 'world size' and dealing with feature detail feels a bit wonky.  You might want to familize yourself with the terrain scale and how it deals with resolution:
  
  - [Terrain Scale - Gaea Documentation - QuadSpinner](https://docs.quadspinner.com/Guide/Build/Scale.html)

- Building Gaea output is configurable, which is great, you'll also want to familiarize yourself with this if you want your terrain generator files as well as output to be in your project source contol (for instance both the generator and build output in a asset gem)
  
  - [Build Manager - Gaea Documentation - QuadSpinner](https://docs.quadspinner.com/Guide/Build/Manager.html)

- Gaea is set by default to bake output into a location in your user folder like Documents/Gaea/Builds/[Filename]

- So I made some changes ....

## Set up the terrain project file (.tor)...

I created a folder within my asset gem with a pattern like this:

`C:\o3de-multiplayersample-assets\Gems\landscape_mps\.src\Quadspinner-Gaea`

Gaea as a terrain generator, can always render and reproduce output files, and to not muddy the project assets, I can place the generator and any input source files *outside of an assets folder*.

I created the **< gem root >/.src/** folder, because this would allow me to make a end-to-end sample Asset Gem (with documentation) that is self-contained, holding all of the upstream source files (for terrain generation) as well as the downstream terrain assets (trrain asset output).  The user would be able to follow along and start by loading the source terrain generator.

So I created a simple sliff side test, which looks like this.

![](C:\Users\gallowj\AppData\Roaming\marktext\images\2022-07-12-16-13-07-image.png)

And I saved it in this location:

`< gem root >/.src/Quadspinner-Gaea/Cliff-test/cliff.tor`

### Set up output nodes ...

To output files from the generator, you need to follow these steps:

You can output most nodes, then manage how that node type output is managed from the build tab.  I did not do this, I prefered to be more explicit.

The node type output defines graph exit points to a file on disk.

1. Place an <u>output node</u>, wire it into the graph

2. Rename the node, select the outpt node and select rename
   
   1. The name you give it will inform the name of the image output file name

3. Select the outpt node, right-click and select ''Mark for export"
   
   1. Nodes marked for export will create image files during a build

I made several outputs for a macro_color, normalmap, height, and some placement maps (aka splatmaps that we can use for blending masks and other things)

My graph looks like this, I circled the output nodes

![](C:\Users\gallowj\AppData\Roaming\marktext\images\2022-07-12-16-33-42-image.png)

#### Set up output node file types ...

Each output node can specify the bit-depth (8-bit, 16 or 32), and the image file type that is written out (.ong, .exr, etc.)

This table is how I configured each type:

| Node Name   | Bit Depth | Image Type | Usage                    |
|:----------- |:---------:| ----------:| ------------------------ |
| macro_color | 16-bit    | .png       | macro color              |
| height      | 32-bit    | .exr       | height displacement      |
| normalmap   | 16-bit    | .png       | world space normals      |
| wear        | 8-bit     | .png       | mask / splat / placement |
| deposit     | 8-bit     | .png       | mask / splat / placement |
| flow        | 8-bit     | .png       | mask / splat / placement |

#### Set up the build output

The <u>Build Tab</u> allows you to configure where and how a build is output.

As mentioned above, if you have marked other nodes in the graph for output, the build tab will allow you to manage how that type is output (but I didn't do it this way, I created explicit output nodes.)

##### Terrain Definition (Scale)

Remember: terrain scale and height are a bit funky.

- Only a handful of nodes actually care what the

- Gaea defaults to an arbitrary size of 5000 m square in width, and 2600 m high.
  
  - I don't necissarily like this, I prefer to know what I am working with ... so I will set this to mimic how i will use them in-engine.  You can change this in the bottom portin of the build tab named "Terrain Definition".
    
    The UI here is a bit odd, it won't let you just type in numbers by double-clicking:
    
    - You can use the sliders to get it close,
    
    - Then ricght-click on the number and it brings up an odd control based on arrow buttons, that allows you to move the scale up/down in increments: half the value, 5 m, or 1 m at a time.
    
    - Or once you have right-clicked, then you can type in numerically.

- I set the Scale to 1024 m

- I set the Height to 512 m

##### Definition and options

I used the following options:

- Method: Normal Build
  
  - I am only generating a single chuk / tile

- Resolution: 4096
  
  - This will give me 0.25 m textels, because the terrain scale was set to 1024 m.

- Options:
  
  - Range: Normalized
    
    - This forces the terrain to use the full range between 0 ... 1, basically it ensures the best fidelity but will require that we manually scale the inputs on the o3de side (which I would do anyway because I like to be precise)
  
  - The rest of these I left at defaults

<img title="" src="file:///C:/Users/gallowj/AppData/Roaming/marktext/images/2022-07-12-17-07-42-image.png" alt="" width="234" data-align="center">

##### Build Destination

I set the build destination file pattern to:

   ` [FileLocation]/../../../Assets/[Filename]/`

This will build output image files directly into the equivelant asset folder:

```batch
< gem root >/Assets/cliff_test/macro_color.png
< gem root >/Assets/cliff_test/normalmap.png
etc ...
```

There are a numebr of hooks for the file tree pattern, see the docs here:

https://docs.quadspinner.com/Guide/Build/Manager.html

# O3DE Terrain workflow

Nates and things to know:

- The exr files for height that were generated out of Gaea failed in the AP.
  
  - This might be because it ran out of memory, this can happen especially with high resolution images.  Try to shut down and restart the AP and see if it makes it through.
  
  - Another option, is to open them in Photoshop (PS) and resave them as exr.
    
    - Make sure they are 32-bit RGB (our image builder may not yet know what to do with single channel / luminance exr images.)
  
  - note: I was able to get them to stop failing after a re-save from PS

- In Gaea, the easiest way to colorize terrain is a use of the **<u>SatMap Node</u>** which has a lot of color ramps (CLUTs, used as a ramp look up table.)  These color ramps are generated from satelite data... and they can look deceivingly good in Gaea, however O3de Atom is a PBR renderer and the Terrain rendering is a varation on the StandardPBR materialType; so to be accurate the albedo color (derived from the basecolor/macrocolor texture) should be within the correct luminance range.
  
  - Unity for insatnce has some built in validation tools: [Unity - Manual: Physically Based Rendering Material Validator](https://docs.unity3d.com/2018.3/Documentation/Manual/MaterialValidator.html)
  
  - And Substance Designer also has a validaor, this post covers various ways to validate: [ArtStation - PBR, Color space conversion and Albedo chart](https://www.artstation.com/blogs/shinsoj/Q9j6/pbr-color-space-conversion-and-albedo-chart)
  
  - And Substance has an Albedo correction node (note, not meant to always be accurate!): [PBR Albedo Safe Color | Substance 3D Designer](https://substance3d.adobe.com/documentation/sddoc/pbr-albedo-safe-color-159451045.html)
  
  - ... moving on, I could go deep into this rabbit hole.



The source image output for terrain is placed in the Gem assets folder such as

`o3de-multiplayersample-assets/Gems/landscape_mps/Assets/cliff`

`o3de-multiplayersample-assets/Gems/landscape_mps/Assets/Crater-barren`



Image files are processed as into runtime textures by the ImageBuilder in the AssetProcessor

C:/path/to/o3de/Gems/Atom/Asset/ImageProcessingAtom

There are two ways tha you can inform the ImageBuilder what processing and format to apply to the input image.  Option #1 Naming Conventions, Option #2 Image Settings

## Option #1 Naming Conventions

fooey

## Option #2 Image Settings

fooey

## Setting up a Level with Terrain

(make sure the terrain gems and landscape canvas are enabled in project)

Create a new level

Select the root Level entity from the outliner

Add Terrain World Component, here are my settings

![](C:\Users\gallowj\AppData\Roaming\marktext\images\2022-07-13-17-03-36-image.png)

Add the Terrain World Renderer Component (I left default settings, so no pic)

Created a entity, named it Terrain:

    moved to 0,0,0

    Added Box Shape Component

        Dimensions: 1024, 1024, 512

    Added Terrain Layer Spawner (set Layer priority: Background)

    Added Terrain Height Gradient List Component

        Hit + to add one sampler index

Created a child entity, named it Height

    Added Shape Reference (referenced the parent Terrain, to share world box)

    Added a Image Gradient Component

        Loaded 

       `o3de-multiplayersample-assets\Gems\landscape_mps\Assets\cliff\height.exr`

    



Note: my terrain looked stair stepped, so not sure exactly how the height.exr is processed or samples.

    Find the height.exr in the Asset Browser

    Right-Click and select "Texture Settings"

    Note: it was using albedo profile

        Not good, maybe just height doesn't work like _height convention?

            Probably wouldn't work the way we wanted it to anyway

                Height is for material height maps

        Switched to GSI32 (32-bit gradient signal, I know this is what I actually want)

        Hit Apply button (and waited, this is a big image)

            Whoops this crashed the editor!

            note: it did create the height.exr.assetinfo file on disk though

            Whoops my level changes are gone (have to redo, ack!)

            note: remember to save often






