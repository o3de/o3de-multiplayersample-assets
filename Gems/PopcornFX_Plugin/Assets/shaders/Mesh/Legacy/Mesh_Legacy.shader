{
    "Source" : "Mesh_Legacy.azsl",

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
          "name": "MeshVS",
          "type": "Vertex"
        },
        {
          "name": "MeshFS",
          "type": "Fragment"
        }
      ]
    },

    "DrawList" : "transparent"
}
