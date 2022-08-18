{
    "Source" : "./default_ForwardPass.azsl",

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

    "ProgramSettings":
    {
      "EntryPoints":
      [
        {
          "name": "default_MainPassVS",
          "type": "Vertex"
        },
        {
          "name": "default_MainPassPS",
          "type": "Fragment"
        }
      ]
    },

    "DrawList" : "forward"
}