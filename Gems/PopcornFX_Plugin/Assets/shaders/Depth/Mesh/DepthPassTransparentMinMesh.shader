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

    "RasterState": { "CullMode": "None" },

    "DepthStencilState" : { 
        "Depth" : { "Enable" : true, "CompareFunc" : "GreaterEqual" }
    },
    
    "DrawList" : "depthTransparentMin"
} 