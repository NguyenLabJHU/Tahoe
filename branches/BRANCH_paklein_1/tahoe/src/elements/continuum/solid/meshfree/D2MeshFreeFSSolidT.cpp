/* $Id: D2MeshFreeFSSolidT.cpp,v 1.7.4.1 2002-10-17 04:28:56 paklein Exp $ */
/* created: paklein (10/23/1999) */

#include "D2MeshFreeFSSolidT.h"

#include <iostream.h>
#include <iomanip.h>
#include <math.h>

#include "fstreamT.h"
#include "toolboxConstants.h"
#include "ExceptionT.h"
#include "D2MeshFreeShapeFunctionT.h"

//TEMP
#include "MaterialListT.h"
#include "D2VIB2D.h"

//TEMP - for RHS stuff
#include "eControllerT.h"

/* constructor */

using namespace Tahoe;

D2MeshFreeFSSolidT::D2MeshFreeFSSolidT(const ElementSupportT& support, const FieldT& field):
	MeshFreeFSSolidT(support, field),
	fD2MFShapes(NULL),

	/* work space */
	fDW(NumSD()),
	fDDW(NumSD(), dSymMatrixT::NumValues(NumSD())),
	fD2GradNa(dSymMatrixT::NumValues(NumSD()), 0), // need rows, but is dynamic
	fD2GradNa_wrap(10, fD2GradNa)
{
	//DEBUG
	DoPrint = 0;
}

/* check material's list */
void D2MeshFreeFSSolidT::Initialize(void)
{
//TEMP - must be a cleaner way
#ifdef __NO_RTTI__
	cout << "\n D2MeshFreeFSSolidT::Initialize: requires RTTI" << endl;
	throw ExceptionT::kGeneralFail;
#endif		

	/* inherited */
	MeshFreeFSSolidT::Initialize();

	/* check material's list */
	for (int i = 0; i < fMaterialList->Length(); i++)
	{
		ContinuumMaterialT* pcont_mat = (*fMaterialList)[i];
		StructuralMaterialT* pstruct_mat = (StructuralMaterialT*) pcont_mat;
		D2VIB2D* pmat = dynamic_cast<D2VIB2D*>(pstruct_mat);
		if (!pmat)
		{
			cout << "\n D2MeshFreeFSSolidT::Initialize: all materials must";
			cout << " be D2VIB2D" << endl;
			throw ExceptionT::kBadInputValue;
		}
	}
}

//DEV - no reason to override
#if 0
void D2MeshFreeFSSolidT::RHSDriver(void)
{
	/* skip down */
	ContinuumElementT::RHSDriver();

	/* element contribution */
	ElementRHSDriver();
}

/* form the residual force vector */
void D2MeshFreeFSSolidT::ElementRHSDriver(void)
{
	/* set components and weights */
	double constMa = 0.0;
	double constCv = 0.0;
	double constKd = 0.0;
	
	/* components dicated by the algorithm */
	int formMa = fController->FormMa(constMa);
	int formCv = fController->FormCv(constCv);
	int formKd = fController->FormKd(constKd);

	/* body forces */
	int formBody = 0;
	if (fMassType != kNoMass &&
	   (fBodyForceLTf > -1 && fBody.Magnitude() > kSmall))
	{	
		formBody = 1;
		if (!formMa) constMa = 1.0; //override controller value??
	}

	/* override controller */
	if (fMassType == kNoMass) formMa = 0;
	if (DoPrint)
		cout << " D2MeshFreeFSSolidT::ElementRHSDriver: force K*d" << '\n';

	Top();
	while (NextElement())
	{
		/* effective accelerations and displacements */
		ComputeEffectiveDVA(formBody, formMa, constMa, formCv, constCv, formKd, constKd);
	
		/* last check w/ effective a and d - override controller */
		int eformMa = fLocAcc.AbsMax() > 0.0;
		int eformCv = fLocVel.AbsMax() > 0.0;
		int eformKd = (fLocDisp.AbsMax() > 0.0 ||
		               fCurrMaterial->HasInternalStrain());

		if (eformMa || eformCv || eformKd)
		{
			/* initialize */
			fRHS = 0.0;
		
			/* global shape function values */
			SetGlobalShape();
			
			/* internal force contribution */	
			if (eformKd) FormKd(-1.0);
				
			/* damping */
			//if (eformCv) FormCv(-1.0);
			//DEV - compute at constitutive level

			/* inertia forces */
			if (eformMa) FormMa(fMassType, -(fCurrMaterial->Density()), fLocAcc);			  		
								
			/* assemble */
			AssembleRHS();
		}
	}
}
#endif

/***********************************************************************
* Protected
***********************************************************************/

/* initialization functions */
void D2MeshFreeFSSolidT::SetShape(void)
{
	/* only support single list of integration cells for now */
	if (fConnectivities.Length() > 1) {
		cout << "\n D2MeshFreeFSSolidT::SetShape: multiple element blocks within an"
		     <<   "     element group not supported. Number of blocks: " 
		     << fConnectivities.Length() << endl;
		throw ExceptionT::kGeneralFail;
	}

	/* constructors */
	fD2MFShapes = new D2MeshFreeShapeFunctionT(GeometryCode(), NumIP(),
		fLocInitCoords, ElementSupport().InitialCoordinates(), *fConnectivities[0], fOffGridNodes,
		fElementCards.Position(), ElementSupport().Input());
	if (!fD2MFShapes) throw ExceptionT::kOutOfMemory;
	
	/* initialize (set internal database) */
	fD2MFShapes->Initialize();
	
	/* set base class pointers */
	fShapes   = fD2MFShapes;
	fMFShapes = fD2MFShapes;
}

/* current element operations */
bool D2MeshFreeFSSolidT::NextElement(void)
{
	/* inherited */
	int OK = MeshFreeFSSolidT::NextElement();
	
	/* resize */
	fD2GradNa_wrap.SetDimensions(fD2GradNa.Rows(), MeshFreeElementSupportT::NumElementNodes());

	/* set material pointer (cast checked above) */
	if (OK) pD2VIB2D = (D2VIB2D*) fCurrMaterial;

	return OK;
}

/* calculate the internal force contribution */
void D2MeshFreeFSSolidT::FormKd(double constK)
{
	/* set work space */
	dMatrixT fWP(NumDOF(), fStressStiff.Rows(), fNEEvec.Pointer());

	const double* Det    = fShapes->IPDets();
	const double* Weight = fShapes->IPWeights();
	fShapes->TopIP();
	while (fShapes->NextIP())
	{
		/* material internal stress terms */
		pD2VIB2D->StressTerms(fDW, fDDW);
	
		/* integration factor */
		double factor = constK*(*Weight++)*(*Det++);

		/* Wi,J PiJ */
		fD2MFShapes->GradNa(fGradNa);
		fWP.MultAB(fDW, fGradNa);

		/* accumulate */
		fRHS.AddScaled(factor, fNEEvec);

		/* Wi,JK PiJK */
		fD2MFShapes->D2GradNa(fD2GradNa);

		/* 3rd rank tensor (double) contraction */
		A_ijk_B_jkl(fDDW, fD2GradNa, fWP);

		/* accumulate */
		fRHS.AddScaled(factor, fNEEvec);
	}	
}

/***********************************************************************
* Private
***********************************************************************/

/* 3rd rank tensor (double) contraction */
void D2MeshFreeFSSolidT::A_ijk_B_jkl(const dMatrixT& A, const dMatrixT& B,
	dMatrixT& C)
{
#if __option (extended_errorcheck)
	/* dimension checks */
	if (C.Rows() != A.Rows() ||
	    C.Cols() != B.Cols() ||
	    A.Cols() != B.Rows()) throw ExceptionT::kSizeMismatch;
#endif		

	double scale_2D[3] = {1.0, 1.0, 2.0};
	double scale_3D[6] = {1.0, 1.0, 1.0, 2.0, 2.0, 2.0};
	double* scale = (NumSD() == 2) ? scale_2D : scale_3D;

	int        rows = C.Rows();
	int        cols = C.Cols();
	int    dotcount = A.Cols();
	double*	c       = C.Pointer();
	double*	BCol    = B.Pointer();

	register double sum;
	for (int Bcol = 0; Bcol < cols; Bcol++)
	{
		double* ARow = A.Pointer();
		 		 	
		for (int Arow = 0; Arow < rows; Arow++)
		{
			sum = 0.0;
			double* AR = ARow;
			double* BC = BCol;
			double* pscale = scale;
			for (int i = 0; i < dotcount; i++)
			{
				sum += (*AR)*(*BC++)*(*pscale++);
				AR += rows;
			}
			*c++ = sum;
			ARow++;
		}
		BCol += dotcount;
	}
}

/* write displacement field and gradients */
void D2MeshFreeFSSolidT::WriteField(void)
{
	cout << "\n D2MeshFreeFSSolidT::WriteField: writing full field" << endl;
	
	const dArray2DT& DOFs = Field()[0]; /* displacements */
	
	/* reconstruct displacement field and all derivatives */
	dArray2DT u;
	dArray2DT Du;
	dArray2DT DDu;
	iArrayT nodes;
	fD2MFShapes->NodalField(DOFs, u, Du, DDu, nodes);

	/* write data */
	ifstreamT& in = ElementSupport().Input();
	
	/* output filenames */
	StringT s_u, s_Du, s_DDu;
	s_u.Root(in.filename());
	s_Du.Root(in.filename());
	s_DDu.Root(in.filename());
	
	s_u.Append(".u.", ElementSupport().StepNumber());
	s_Du.Append(".Du.", ElementSupport().StepNumber());
	s_DDu.Append(".DDu.", ElementSupport().StepNumber());
	
	/* open output streams */
	ofstreamT out_u(s_u), out_Du(s_Du), out_DDu(s_DDu);

	/* write */
	for (int i = 0; i < nodes.Length(); i++)
	{
		out_u << setw(kIntWidth) << nodes[i] + 1;
		out_Du << setw(kIntWidth) << nodes[i] + 1;
		out_DDu << setw(kIntWidth) << nodes[i] + 1;

		u.PrintRow(i, out_u);		
		Du.PrintRow(i, out_Du);		
		DDu.PrintRow(i, out_DDu);		
	}	
	out_u << u;
	out_Du << Du;
	out_DDu << DDu;

	/* close */
	out_u.close();
	out_Du.close();
	out_DDu.close();
}
