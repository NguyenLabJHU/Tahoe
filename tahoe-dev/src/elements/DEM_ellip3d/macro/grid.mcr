#!MC 1000
$!VarSet |MFBD| = ''
$!ACTIVEFIELDMAPS = [1]
$!FIELDLAYERS SHOWMESH = YES
$!FIELDLAYERS SHOWEDGE = NO
$!REDRAWALL 
$!FIELDMAP [1-500]  MESH{COLOR = GREEN}
$!FIELDMAP [1-500]  MESH{LINETHICKNESS = 0.3}
$!FIELDMAP [1-500]  SHADE{SHOW = NO}
$!FIELDMAP [1-500]  SURFACES{SURFACESTOPLOT = EXPOSEDCELLFACES}
$!REDRAWALL 
$!VIEW SETMAGNIFICATION
  MAGNIFICATION = 1
$!RemoveVar |MFBD|
