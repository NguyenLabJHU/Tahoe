/* $Id: RodT.h,v 1.9 2002-07-02 20:59:14 hspark Exp $ */
/* created: paklein (10/22/1996) */

#ifndef _ROD_T_H_
#define _ROD_T_H_

/* base class */
#include "ElementBaseT.h"

/* direct members */
#include "RodMaterialT.h"
#include "LocalArrayT.h"
/* templates */
#include "pArrayT.h"

/** \note the RodT class doesn't provide complete support for the
 * different time integration schemes implemented using the
 * controller classes. need to add something like the
 * ComputeEffectiveDVA functions from the continuum element
 * classes to provide contributions to the global equations
 * which are consistent with the time integration algorithm.
 * PAK (05/30/1999) */

namespace Tahoe {

class RodT: public ElementBaseT
{
public:

	/** spring potential types */
	enum PotentialCodeT {
        kQuad = 1, /**< linear spring */
       kLJ612 = 2 /**< Lennard-Jones 6-12 potential */
		};

	/** constructor */
	RodT(const ElementSupportT& support, const FieldT& field);
	
	/** initialization */
	virtual void Initialize(void);

	/** form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/* NOT implemented. Returns an zero force vector */
	virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force);
			
	/* returns the energy as defined by the derived class types */
	virtual double InternalEnergy(void);
	
	/* writing output */
	virtual void RegisterOutput(void);
	virtual void WriteOutput(IOBaseT::OutputModeT mode);

	/* compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode);

	/* initialize/finalize time increment */
	virtual void InitStep(void);
	virtual void CloseStep(void);

	/* Element type parameters */
	static const int kRodTndof; /* number of degrees of freedom per node */
	static const int  kRodTnsd; /* number of spatial dimensions */
	 			  	
protected: /* for derived classes only */
	 	
	/* called by FormRHS and FormLHS */
	virtual void LHSDriver(void);
	virtual void RHSDriver(void);

	/* increment current element */
	virtual bool NextElement(void);
		
	/* element data */
	virtual void ReadMaterialData(ifstreamT& in);	
	virtual void WriteMaterialData(ostream& out) const;

	/** return true if connectivities are changing */
	virtual bool ChangingGeometry(void) const { return false; };

private: /* MD related computational functions */

	void ComputeInstKE(void);
	void ComputeAvgKE(void);
	void ComputeInstPE(void);
	void ComputeAvgPE(void);
	void ComputeInstTotalE(void);
	void ComputeAvgTotalE(void);
	void ComputeInstTemperature(void);
	void ComputeAvgTemperature(void);
	void ComputeInstPressure(void);
	void ComputeAvgPressure(void);

protected:

	/** reference ID for sending output */
	int fOutputID;

	/* material data */
	pArrayT<RodMaterialT*> fMaterialsList; 	
	RodMaterialT*	       fCurrMaterial;

private:

	/** \name work space */
	/*@{*/
	/** constant matrix needed to compute the stiffness */
	dMatrixT fOneOne;

	/** current pair vector */
	dArrayT fBond;

	/** reference pair vector */
	dArrayT fBond0;

	/** current coordinates for one pair bond */
//	dArray2DT fPairCoords;
	/*@}*/

	/* MD related variables */
	double fKb;
	double fInstKE, fInstPE, fInstTotalE, fInstTemp, fInstPressure;
	double fAvgKE, fAvgPE, fAvgTotalE, fAvgTemp, fAvgPressure;
	double fSumKE, fSumPE, fSumTotalE, fSumTemp, fSumPressure;
	LocalArrayT fLocVel;

	const int& fStepNumber;
};

} // namespace Tahoe 
#endif /* _ROD_T_H_ */
