{
    "Source" : "Billboard.azsl",

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

    "GlobalTargetBlendState" :
    {
        "Enable" : true,
        "BlendSource" : "One",
        "BlendDest" : "AlphaSourceInverse",
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

    "DrawList" : "transparent"
}
