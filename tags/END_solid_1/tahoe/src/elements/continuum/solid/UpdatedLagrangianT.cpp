/* $Id: UpdatedLagrangianT.cpp,v 1.2 2001-07-03 01:34:53 paklein Exp $ */
/* created: paklein (07/03/1996)                                          */

#include "UpdatedLagrangianT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>

#include "fstreamT.h"
#include "Constants.h"
#include "FEManagerT.h"
#include "StructuralMaterialT.h"
#include "MaterialList2DT.h"
#include "MaterialList3DT.h"
#include "ShapeFunctionT.h"

/* constructor */
UpdatedLagrangianT::UpdatedLagrangianT(FEManagerT& fe_manager):
	FiniteStrainT(fe_manager),
	fCurrShapes(NULL),
	fCauchyStress(fNumSD),
	fLocCurrCoords(LocalArrayT::kCurrCoords)
{
	/* disable any strain-displacement options */
	if (fStrainDispOpt != 0)
	{
		cout << "\nUpLag_FDElasticT::UpdatedLagrangianT: no strain-displacement options\n" << endl;
		fStrainDispOpt = 0;
	}

	/* consistency check */
	if (fAnalysisCode == GlobalT::kLinStatic ||
	    fAnalysisCode == GlobalT::kLinDynamic)
	{
		cout << "\nUpLag_FDElasticT::UpdatedLagrangianT: no current coordinates required\n" << endl;
		fLocCurrCoords.SetType(LocalArrayT::kInitCoords);
	}	
}

/* destructors */
UpdatedLagrangianT::~UpdatedLagrangianT(void)
{
	delete fCurrShapes;
	fCurrShapes = NULL;
}

/* data initialization */
void UpdatedLagrangianT::Initialize(void)
{
	/* inherited */
	FiniteStrainT::Initialize();

	/* dimension */
	fGradNa.Allocate(fNumSD, fNumElemNodes);
	fStressStiff.Allocate(fNumElemNodes);
	fTemp2.Allocate(fNumElemNodes*fNumDOF);
}

/***********************************************************************
* Protected
***********************************************************************/

/* initialize local arrays */
void UpdatedLagrangianT::SetLocalArrays(void)
{
	/* inherited */
	FiniteStrainT::SetLocalArrays();

	/* allocate and set source */
	fLocCurrCoords.Allocate(fNumElemNodes, fNumSD);
	fFEManager.RegisterLocal(fLocCurrCoords);
}

/* initialization functions */
void UpdatedLagrangianT::SetShape(void)
{
	/* inherited */
	FiniteStrainT::SetShape();

	/* linked shape functions */
	fCurrShapes = new ShapeFunctionT(*fShapes, fLocCurrCoords);
	if (!fCurrShapes) throw eOutOfMemory ;

	fCurrShapes->Initialize();
}

/* form shape functions and derivatives */
void UpdatedLagrangianT::SetGlobalShape(void)
{
	/* inherited */
	FiniteStrainT::SetGlobalShape();

	/* shape function wrt current config */
	SetLocalX(fLocCurrCoords);
	fCurrShapes->SetDerivatives();
}

/* construct materials manager and read data */
MaterialListT* UpdatedLagrangianT::NewMaterialList(int size) const
{
	if (fNumSD == 2)
		return new MaterialList2DT(size, *this);
	else if (fNumSD == 3)
		return new MaterialList3DT(size, *this);
	else
		return NULL;			
}

/* form the element stiffness matrix */
void UpdatedLagrangianT::FormStiffness(double constK)
{		
	/* matrix format */
	dMatrixT::SymmetryFlagT format =
		(fLHS.Format() == ElementMatrixT::kNonSymmetric) ?
		dMatrixT::kWhole :
		dMatrixT::kUpperOnly;

	/* integration */
	const double* Det    = fCurrShapes->IPDets();
	const double* Weight = fCurrShapes->IPWeights();

	/* initialize */
	fStressStiff = 0.0;
	
	fShapes->TopIP();
	while ( fShapes->NextIP() )
	{
		/* double scale factor */
		double scale = constK*(*Det++)*(*Weight++);
	
	/* S T R E S S   S T I F F N E S S */			
		/* compute Cauchy stress */
		(fCurrMaterial->s_ij()).ToMatrix(fCauchyStress);
	
		/* integration constants */		
		fCauchyStress *= scale;

		/* get shape function gradients matrix */
		fCurrShapes->GradNa(fGradNa);
	
		/* using the stress symmetry */
		fStressStiff.MultQTBQ(fGradNa, fCauchyStress,
			format, dMatrixT::kAccumulate);

	/* M A T E R I A L   S T I F F N E S S */									
		/* strain displacement matrix */
		fCurrShapes->B(fB);

		/* get D matrix */
		fD.SetToScaled(scale, fCurrMaterial->c_ijkl());
						
		/* accumulate */
		fLHS.MultQTBQ(fB, fD, format, dMatrixT::kAccumulate);	
	}
						
	/* stress stiffness into fLHS */
	fLHS.Expand(fStressStiff, fNumDOF);
}

//DEV - Rayleigh damping should be added to the constitutive level
#if 0
/*
* Compute the effective acceleration and velocities based
* on the algorithmic flags formXx and the given constants
* constXx.
*
*      acc_eff  = constMa acc  + constCv a vel
*      vel_eff  = constCv b vel;
*      disp_eff = constKd disp
*
* where a and b are the Rayleigh damping coefficients.
*
*        ***The effective displacement does not include
*           velocity since the internal force is a nonlinear
*           function of the displacements
*/
void UpdatedLagrangianT::ComputeEffectiveDVA(int formBody,
	int formMa, double constMa, int formCv, double constCv,
	int formKd, double constKd)
{
//DEV - same as Total Lagrangian -> move to base class

	/* acceleration */
	if (formMa || formBody)
	{
		if (formMa)
			SetLocalU(fLocAcc);
		else
			fLocAcc = 0.0;
		
		if (formBody) AddBodyForce(fLocAcc);

		fLocAcc *= constMa;	
	}
	else
		fLocAcc = 0.0;
	
	/* displacement */
	if (formKd)
	{
		SetLocalU(fLocDisp);
		fLocDisp *= constKd;	
	}
	else
		fLocDisp = 0.0;
	
	/* Rayleigh damping */
	if (formCv)
	{
		SetLocalU(fLocVel);
		fLocVel *= constCv;
		
		/* effective a */
		fLocAcc.AddScaled(fCurrMaterial->MassDamping(), fLocVel);
		
		/* effective v */
		fLocVel *= fCurrMaterial->StiffnessDamping();
	}
	else
		fLocVel = 0.0;
}	
#endif

/* calculate the damping force contribution ("-c*v") */
void UpdatedLagrangianT::FormCv(double constC)
{
//DEV - same as Total Lagrangian -> move to base class

//TEMP
//This is approximate.  No nonlinear Rayleigh damping

	/* clear workspace */
	fLHS = 0.0;
	fStressStiff = 0.0;

	/* form tangent stiffness */
	FormStiffness(constC);
	fLHS.CopySymmetric();

	/* reorder */
	fLocVel.ReturnTranspose(fTemp2);
	
	/* C*v */
	fLHS.MultTx(fTemp2, fNEEvec);
	
	/* Accumulate */
	fRHS += fNEEvec;
}

/* calculate the internal force contribution ("-k*d") */
void UpdatedLagrangianT::FormKd(double constK)
{
	const double* Det    = fCurrShapes->IPDets();
	const double* Weight = fCurrShapes->IPWeights();

	fCurrShapes->TopIP();
	while ( fCurrShapes->NextIP() )
	{
		/* get strain-displacement matrix */
		fCurrShapes->B(fB);

		/* B^T * Cauchy stress */
		fB.MultTx(fCurrMaterial->s_ij(), fNEEvec);

		/* accumulate */
		fRHS.AddScaled(constK*(*Weight++)*(*Det++), fNEEvec);
	}	
}