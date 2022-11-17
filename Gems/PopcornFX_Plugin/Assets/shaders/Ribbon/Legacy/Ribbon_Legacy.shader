{
    "Source" : "Ribbon_Legacy.azsl",

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
          "name": "RibbonVS",
          "type": "Vertex"
        },
        {
          "name": "RibbonFS",
          "type": "Fragment"
        }
      ]
    },

    "DrawList" : "transparent"
}
