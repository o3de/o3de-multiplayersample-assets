{
    "Source" : "MeshLit_Legacy.azsl",

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
            },
            "BackFace" :
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
          "name": "MeshVS",
          "type": "Vertex"
        },
        {
          "name": "MeshFS",
          "type": "Fragment"
        }
      ]
    },

    "DrawList" : "forward"
}
