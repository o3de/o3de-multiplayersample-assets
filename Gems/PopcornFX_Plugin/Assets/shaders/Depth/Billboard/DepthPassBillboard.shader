{
    "Source" : "DepthPassBillboard.azsl",

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

    "RasterState": { "CullMode": "None" },

    "DepthStencilState" : { 
        "Depth" : { "Enable" : true, "CompareFunc" : "GreaterEqual" }
    },

    "DrawList" : "depth"
}