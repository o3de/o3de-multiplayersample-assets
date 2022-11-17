{
    "Source" : "RibbonLit_Legacy.azsl",

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
          "name": "RibbonVS_AtlasBlend",
          "type": "Vertex"
        },
        {
          "name": "RibbonFS_AtlasBlend",
          "type": "Fragment"
        }
      ]
    },

    "DrawList" : "transparent"
}
