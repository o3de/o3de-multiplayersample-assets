{
    "Source" : "BillboardDistortion.azsl",

    "DepthStencilState" : 
    {
        "Depth" : 
        { 
            "Enable" : true, 
            "CompareFunc" : "GreaterEqual"
        }
    },

    "RasterState" :
    {
        "CullMode" : "None"
    },

    "GlobalTargetBlendState" : {
        "Enable" : true,
        "BlendSource" : "One",
        "BlendDest" : "One",
        "BlendOp" : "Add",
        "BlendAlphaSource" : "One",
        "BlendAlphaDest" : "One",
        "BlendAlphaOp" : "Add"
    },

    "ProgramSettings":
    {
      "EntryPoints":
      [
        {
          "name": "BillboardVS",
          "type": "Vertex"
        },
        {
          "name": "BillboardFS",
          "type": "Fragment"
        }
      ]
    },

    "DrawList" : "DistortionAccumulation"
}
