/* $Id: PenaltyCylinderT.cpp,v 1.2 2003-10-04 19:14:05 paklein Exp $ */
#include "PenaltyCylinderT.h"

#include <iostream.h>
#include <iomanip.h>

#include "toolboxConstants.h"
#include "FEManagerT.h"
#include "fstreamT.h"
#include "eIntegratorT.h"

using namespace Tahoe;

/* constructor */
PenaltyCylinderT::PenaltyCylinderT(FEManagerT& fe_manager,
	int group,
	const iArray2DT& eqnos,
	const dArray2DT& coords,
	const dArray2DT& disp,
	const dArray2DT* vels):
	PenaltyRegionT(fe_manager, group, eqnos, coords, disp, vels),
	fDirection(rCoords.MinorDim()),
	fR(rCoords.MinorDim()),
	fv_OP(rCoords.MinorDim()),
	fLHS(eqnos.MinorDim(), ElementMatrixT::kSymmetric)
{
	SetName("sphere_cylinder");
}

/* input processing */
void PenaltyCylinderT::EchoData(ifstreamT& in, ostream& out)
{
	/* inherited */
	PenaltyRegionT::EchoData(in, out);

	/* echo parameters */
	in >> fRadius; if (fRadius < 0.0) ExceptionT::BadInputValue("PenaltyCylinderT::EchoData");
	in >> fDirection;
	fDirection.UnitVector();

	out << " Cylinder radius . . . . . . . . . . . . . . . . . = " << fRadius << '\n';
	out << " Cylinder direction. . . . . . . . . . . . . . . . =\n" << fDirection << '\n';
}

/* form of tangent matrix */
GlobalT::SystemTypeT PenaltyCylinderT::TangentType(void) const
{
	/* symmetric tangent (frictionless) */
	return GlobalT::kSymmetric;
}

/* tangent term */
void PenaltyCylinderT::ApplyLHS(GlobalT::SystemTypeT sys_type)
{
#pragma unused(sys_type)

	double constK = 0.0;
	int formK = fIntegrator->FormK(constK);
	if (!formK) return;

	/* node by node */
	for (int i = 0; i < fNumContactNodes; i++)
	{
		double gap  = fGap[i];
		double dist = gap + fRadius;

		/* active */
		if (gap < 0.0)
		{
			/* get force vector (has normal direction) */
			fContactForce2D.RowAlias(i,fd_sh);
			
			double dPhi = gap*fk;
			fLHS.Outer(fd_sh, fd_sh, constK*((fk/dPhi) - (1.0/dist))/dPhi);
			fLHS.PlusIdentity(constK*dPhi/dist);
			fLHS.Outer(fDirection, fDirection, -constK*dPhi/dist, dMatrixT::kAccumulate);
		
			/* assemble */
			rEqnos.RowAlias(fContactNodes[i], fi_sh);
			fFEManager.AssembleLHS(fGroup, fLHS, fi_sh);
		}
	}
}

/* describe the parameters needed by the interface */
void PenaltyCylinderT::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	PenaltyRegionT::DefineParameters(list);
	
	list.AddParameter(fRadius, "radius");
}

/**********************************************************************
 * Protected
 **********************************************************************/

/* compute the nodal contribution to the residual force vector */
void PenaltyCylinderT::ComputeContactForce(double kforce)
{
	/* loop over strikers */
	fContactForce2D = 0.0;	
	for (int i = 0; i < fNumContactNodes; i++)
	{
		/* center to striker */
		rCoords.RowCopy(fContactNodes[i], fv_OP);
		fv_OP -= fx;

		/* vector in radial direction */
		fR.SetToCombination(1.0, fv_OP, -dArrayT::Dot(fv_OP, fDirection), fDirection);
		
		/* penetration */
		double dist = fR.Magnitude();
		double pen  = dist - fRadius;
		if (pen < 0.0)
		{
			/* convert to force*outward normal */
			fR *= (-pen*fk*kforce/dist);
		
			/* accumulate */
			fContactForce2D.SetRow(i, fR);
		}

		/* store */
		fGap[i] = pen;
	}
}
