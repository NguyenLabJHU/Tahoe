#include <iostream.h>
#include "BoxT.h"
#include "iArrayT.h"
#include "dArrayT.h"
#include "dArray2DT.h"
#include "VolumeT.h"
#include "CrystalLatticeT.h"

#include "FCCT.h"
#include "BCCT.h"
#include "DIAT.h"
#include "HEXT.h"
#include "CORUNT.h"

#include "nArrayT.h"
#include "ifstreamT.h"

   using namespace Tahoe;

    class PolyT : public BoxT 
   {
   	      
   private:
   
   	//info about the grains in this polycrystalline structure
      int NumberofGrains;
      nArrayT<dArrayT> GrainCenters;
      
   
   public:
   
   //Constructor
      PolyT(int dim, dArray2DT len, dArrayT lattice_parameter,
       iArrayT which_sort, StringT slt, iArrayT per, int NumberofGrains);
   
	PolyT(int dim, iArrayT cel, dArrayT lattice_parameter,
       iArrayT which_sort, StringT slt, iArrayT per, int NumberofGrains);
   
   //Destructor
       ~PolyT(){};
   
   // Copy constructor
      PolyT(const PolyT& source);
   
      void CreateLattice(CrystalLatticeT* templateLattice); 
   
  protected:
      CrystalLatticeT* GenerateLattice(CrystalLatticeT* templateLattice);
   };


