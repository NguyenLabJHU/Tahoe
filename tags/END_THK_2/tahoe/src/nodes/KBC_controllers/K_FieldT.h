/* $Id: K_FieldT.h,v 1.6 2002-07-05 22:28:31 paklein Exp $ */
/* created: paklein (09/05/2000) */

#ifndef _K_FIELD_T_H_
#define _K_FIELD_T_H_

/* base class */
#include "KBC_ControllerT.h"

/* direct members */
#include "dArrayT.h"
#include "iArrayT.h"
#include "ScheduleT.h"
#include "dArray2DT.h"

namespace Tahoe {

/* forward declarations */
class ElementBaseT;
class IsotropicT;
class Material2DT;

/** K-field displacements */
class K_FieldT: public KBC_ControllerT
{
public:

	/* constructor */
	K_FieldT(NodeManagerT& node_manager);

	/* initialize data - called immediately after construction */
	virtual void Initialize(ifstreamT& in);
	virtual void WriteParameters(ostream& out) const;

	/* initial condition/restart functions
	 *
	 * Set to initial conditions.  The restart functions
	 * should read/write any data that overrides the default
	 * values */
	virtual void InitialCondition(void);
	virtual void ReadRestart(istream& in);
	virtual void WriteRestart(ostream& out) const;

	/* initialize/finalize/reset step */
	virtual void InitStep(void);
	virtual void CloseStep(void);
	virtual void Reset(void);

	/* returns true if the internal force has been changed since
	 * the last time step */
	virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/* output current configuration */
	virtual void WriteOutput(ostream& out) const;

protected:

	/* determine the new tip coordinates */
	void GetNewTipCoordinates(dArrayT& tip_coords);

	/* resolve element info to isotropic material */
	void ResolveMaterialReference(int element_group, int material_num,
		const IsotropicT** piso, const Material2DT** pmat) const;

	/* compute K-field displacement factors */
	virtual void ComputeDisplacementFactors(const dArrayT& tip_coords);
	
	/* set BC cards with current displacement field */
	void SetBCCards(void);
	
protected:

	/* K-field specifications */
	int    fnumLTf1;
	double fK1;
	int    fnumLTf2;
	double fK2;

	/* crack tip coordinates */
	dArrayT fInitTipCoords;
	
	/* crack extension parameters */
	dArrayT fGrowthDirection;

	/* near tip group */
	int    fNearTipGroupNum;   // -1: no nearfield group
	int    fNearTipOutputCode; // variable to locate crack tip
	int    fTipColumnNum;      // column of output variable to locate tip
	double fMaxGrowthDistance;
	int    fMaxGrowthSteps;
		
	/* BC nodes */
	int     fFarFieldGroupNum;
	int     fFarFieldMaterialNum;
	ArrayT<StringT> fID_List;
	iArrayT fNodes;
	
	/* external links */
	const IsotropicT*  fIsotropic;
	const Material2DT* fMaterial2D;

	/* runtime data */
	ScheduleT fDummySchedule;
	const ScheduleT* fLTf1;
	const ScheduleT* fLTf2;   	
	dArray2DT fK1Disp;
	dArray2DT fK2Disp;
	int fGrowthCount;

	/* tip coordinates */
	dArrayT fTipCoords;
	dArrayT fLastTipCoords;
};

} // namespace Tahoe 
#endif /* _K_FIELD_T_H_ */
