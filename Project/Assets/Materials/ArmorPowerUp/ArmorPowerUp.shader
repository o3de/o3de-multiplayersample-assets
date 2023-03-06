
{
    "Source" : "ArmorPowerUp.azsl",
	

    "RasterState": { "CullMode": "None" },

    "DepthStencilState" : { 
        //For a DoubleSided Object writemask zero and for see only througt object lessequal ine the compar func
        "Depth" : { "Enable" : true, "CompareFunc" : "GreaterEqual","WriteMask": "Zero" }
    },
		
    "GlobalTargetBlendState": {
        "Enable": true,
        "BlendSource": "AlphaSource",
        "BlendDest": "AlphaSourceInverse",
        "BlendOp": "Add"
    },

    "ProgramSettings":
    {
      "EntryPoints":
      [
        {
          "name": "MainVS",
          "type": "Vertex"
        },
        {
          "name": "MainPS",
          "type": "Fragment"
        }
      ]
    },

    "DrawList" : "transparent"
}
