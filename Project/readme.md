# MPS: Asset Test Project

This is a light-weight O3DE project for MPS asset contributors, so they can work on new assets, and test, and maintain them independently from a single repository.

< image >

The [primary readme.md](https://github.com/o3de/o3de-multiplayersample-assets/blob/main/readme.md) at the root of this repository has more general instructions and information on using this repository and asset gems.  This page is just a brief on getting the test project up and running.

### Activate/Register Asset Source Gems

If you are a developer, make sure your development branch engine is built and then also registered:  

`c:\path\to\o3de\scripts\o3de register --this-engine`  

If you are using an installer build, that step may not be necessary.

Currently, you must register these asset gems with the engine manually (in the future, you will be able to do this via the project manager)  

`c:\path\to\o3de\scripts\o3de register --all-gems-path c:\path\to\o3de-multiplayersample-assets\Gems`  

Once this is done, you should be able to use the o3de.exe (project manager) to browse the gems and add them to a game project, whether that be your own test project or the [o3de-multiplayersample](https://github.com/o3de/o3de-multiplayersample)  

Note: there may be a way in the future to add/register the Gems in the Project Manager GUI, when that works we will update this document.

### Setup MPS Asset Test Project in O3DE

This MPS Asset Test Project is already enabled with the MPS Asset Gems you activated above (but your engine still needs them registered, so do not skip that step.)  

1. Start ‘O3DE Project Manager’
   
   1. If you are using an installed version, this will be in the Windows Start menu.
   
   2. Or you can find it on disk in a location such as:
      
      1. “C:\O3DE\0.0.0.0\bin\Windows\profile\Default\o3de.exe”

2. Use the blue button in the upper right labeled “New Project …”
   
   1. With the pull down, select “Open Existing Project”

3. Browse to and add the MPS Asset Test Project folder:
   
   1. C:\path\to\repo\o3de-multiplayersample-assets\Project

4. You may be prompted to rebuild the project within the Project Manager GUI
   
   1. There are other options for building as a developer not outlined in this document, so if you need help with an alternate path, please let us know.

5. Launch the Editor for the project (this will start the AP and Editor.exe)

# License

For terms please see the LICENSE*.TXT files within this distribution.
