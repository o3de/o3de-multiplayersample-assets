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
