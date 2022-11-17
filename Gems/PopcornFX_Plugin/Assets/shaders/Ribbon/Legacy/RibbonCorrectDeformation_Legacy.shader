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
          "name": "RibbonVS_CorrectDeformation",
          "type": "Vertex"
        },
        {
          "name": "RibbonFS_CorrectDeformation",
          "type": "Fragment"
        }
      ]
    },

    "DrawList" : "transparent"
}
