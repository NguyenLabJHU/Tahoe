// file: CSEBaseT.h

// created: SAW 5/2/2000

#ifndef _CSEBASET_H_
#define _CSEBASET_H_

#include "ElementBaseT.h"

using namespace Tahoe;

class CSEBaseT : public ElementBaseT
{
 public:
  CSEBaseT (ostream& fMainOut, int ID);

  virtual void Initialize (GeometryT::CodeT code, int numregfacenodes);

  virtual void SetNodes (int e1local, const iArrayT& regelemnodes);

  virtual void CSElemFaces (iArrayT& faces) const;

 private:
  void CSEType (GeometryT::CodeT code, int numfacenodes);
  void Set3DNodes (int linearfacenodes);

 private:
  iArrayT fSurfaceFacets;
  iArrayT fNodeNumberingMap;
};

inline void CSEBaseT::CSElemFaces (iArrayT& faces) const { faces = fSurfaceFacets; }
#endif
