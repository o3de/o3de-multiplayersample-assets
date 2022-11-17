{
    "Source" : "Billboard_Legacy.azsl",

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
