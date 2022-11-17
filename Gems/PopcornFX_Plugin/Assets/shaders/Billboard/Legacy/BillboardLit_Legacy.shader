{
    "Source" : "BillboardLit_Legacy.azsl",

    "DepthStencilState" :
    {
        "Depth" :
        {
            "Enable" : true,
            "CompareFunc" : "GreaterEqual"
        },
        "Stencil" :
        {
            "Enable" : true,
            "ReadMask" : "0x00",
            "WriteMask" : "0xFF",
            "FrontFace" :
            {
                "Func" : "Always",
                "DepthFailOp" : "Keep",
                "FailOp" : "Keep",
                "PassOp" : "Replace"
            }
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
