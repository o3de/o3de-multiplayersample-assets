{
     "ProgramSettings" : 
     {
         "EntryPoints":
         [
             {
                 "name": "DepthPassVS",
                 "type" : "Vertex"
             }
         ] 
     },

    "Source" : "DepthPassMesh.azsl",

    "DepthStencilState" : { 
        "Depth" : { "Enable" : true, "CompareFunc" : "GreaterEqual" }
    },

    "RasterState" :
    {
        "CullMode" : "None"
    },

    "DrawList" : "depth"
}