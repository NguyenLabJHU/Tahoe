#!MC 1000
$!VarSet |MFBD| = ''
$!FIELDLAYERS SHOWBOUNDARY = NO
$!FIELDLAYERS SHOWSHADE = YES
$!FIELDLAYERS SHOWEDGE = YES
$!FIELDLAYERS SHOWMESH = NO
$!FRAMELAYOUT SHOWBORDER = NO
$!FIELDLAYERS USETRANSLUCENCY = NO
$!FIELD [1-500]  SHADE{COLOR = CUSTOM11}
$!FIELD [1-500]  SURFACEEFFECTS{LIGHTINGEFFECT = GOURAUD}
$!FIELD [1-500]  MESH{COLOR = CUSTOM41}
$!FIELDMAP [2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202]  EDGELAYER{COLOR = GREEN}
$!FIELDMAP [2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202]  EDGELAYER{LINETHICKNESS = 0.4}
$!ACTIVEFIELDMAPS += [2]
$!GLOBALTHREED LIGHTSOURCE{INCLUDESPECULAR = YES}
$!VIEW SETMAGNIFICATION
  MAGNIFICATION = 1
$!RemoveVar |MFBD|

