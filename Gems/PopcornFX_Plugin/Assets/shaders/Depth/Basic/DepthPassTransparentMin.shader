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

    "Source" : "DepthPass.azsl",

    "RasterState": { "CullMode": "None" },

    "DepthStencilState" : { 
        "Depth" : { "Enable" : true, "CompareFunc" : "GreaterEqual" }
    },
    
    "DrawList" : "depthTransparentMin"
} 