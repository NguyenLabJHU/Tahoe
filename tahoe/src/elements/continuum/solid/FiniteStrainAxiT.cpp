/* $Id: FiniteStrainAxiT.cpp,v 1.1 2004-02-02 23:48:38 paklein Exp $ */
#include "FiniteStrainAxiT.h"

#include "ShapeFunctionT.h"
#include "FSSolidMatT.h"
#include "FSMatSupportT.h"

/* materials lists */
#include "SolidMatList3DT.h"

using namespace Tahoe;

const double Pi = acos(-1.0);
const int kRadialDirection = 0; /* the x direction is radial */
const int kNSD = 2;

/* constructor */
FiniteStrainAxiT::FiniteStrainAxiT(const ElementSupportT& support, const FieldT& field):
	FiniteStrainT(support, field),
	fF2D(kNSD),
	fLocCurrCoords(LocalArrayT::kCurrCoords)	
{
	SetName("large_strain_axi");
}

/* called immediately after constructor */
void FiniteStrainAxiT::Initialize(void)
{
	/* inherited */
	FiniteStrainT::Initialize();

	/* dimensions */
	int nip = NumIP();

	/* integration point radii over the current element */
	fRadius_X.Dimension(nip);
	fRadius_x.Dimension(nip);

	/* redimension with out-of-plane component */
	int nstrs = dSymMatrixT::NumValues(kNSD) + 1;
	fD.Dimension(nstrs);
	fB.Dimension(nstrs, NumSD()*NumElementNodes());

	/* allocate 3D deformation gradient list */
	if (fF_List.Length() > 0) {
		fF_all.Dimension(nip*3*3);
		for (int i = 0; i < nip; i++)
			fF_List[i].Set(3, 3, fF_all.Pointer(i*3*3));
	}
	
	/* allocate 3D "last" deformation gradient list */
	if (fF_last_List.Length() > 0) {
		fF_last_all.Dimension(nip*3*3);
		for (int i = 0; i < nip; i++)
			fF_last_List[i].Set(3, 3, fF_last_all.Pointer(i*3*3));
	}
}

/***********************************************************************
 * Protected
 ***********************************************************************/

/* initial local arrays */
void FiniteStrainAxiT::SetLocalArrays(void)
{
	/* inherited */
	FiniteStrainT::SetLocalArrays();

	/* allocate and set source */
	fLocCurrCoords.Dimension(NumElementNodes(), NumSD());
	ElementSupport().RegisterCoordinates(fLocCurrCoords);
}

/* construct a new material support and return a pointer */
MaterialSupportT* FiniteStrainAxiT::NewMaterialSupport(MaterialSupportT* p) const
{
	/* allocate */
	if (!p) p = new FSMatSupportT(3, NumDOF(), NumIP());

	/* inherited initializations */
	FiniteStrainT::NewMaterialSupport(p);

	return p;
}

/* construct materials manager and read data */
MaterialListT* FiniteStrainAxiT::NewMaterialList(int nsd, int size)
{
#pragma unused(nsd)

	if (size > 0)
	{
		/* material support */
		if (!fFSMatSupport) {
			fFSMatSupport = TB_DYNAMIC_CAST(FSMatSupportT*, NewMaterialSupport());
			if (!fFSMatSupport) ExceptionT::GeneralFail("FiniteStrainAxiT::NewMaterialList");
		}

		return new SolidMatList3DT(size, *fFSMatSupport);
	}
	else
		return new SolidMatList3DT;
}

/* form shape functions and derivatives */
void FiniteStrainAxiT::SetGlobalShape(void)
{
	/* skip call to FiniteStrainT::SetGlobalShape */
	SolidElementT::SetGlobalShape();

	/* what needs to get computed */
	int material_number = CurrentElement().MaterialNumber();
	bool needs_F = Needs_F(material_number);
	bool needs_F_last = Needs_F_last(material_number);
	
	/* get current element coordinates */
	SetLocalX(fLocCurrCoords);
	int nen = fLocCurrCoords.NumberOfNodes();
	int nun = fLocDisp.NumberOfNodes();
	
	/* loop over integration points */
	for (int i = 0; i < NumIP(); i++)
	{
		/* compute radii */
		const double* NaX = fShapes->IPShapeX(i);
		const double* NaU = fShapes->IPShapeU(i);
		const double* X_r = fLocInitCoords(kRadialDirection);
		const double* u_r = fLocDisp(kRadialDirection);
		const double* u_r_last = fLocLastDisp(kRadialDirection);
		double R = 0.0;
		double u = 0.0;
		double u_last = 0.0;
		if (nen == nun) 
		{
			for (int a = 0; a < nen; a++) {
				R += (*NaX)*(*X_r++);
				u += (*NaU)*(*u_r++);
				u_last += (*NaU)*(*u_r_last++);
				NaX++;
				NaU++;
			}
		}
		else /* separate loops for field and geometry */
		{
			for (int a = 0; a < nen; a++) {
				R += (*NaX)*(*X_r++);
				NaX++;
			}
			for (int a = 0; a < nun; a++) {
				u += (*NaU)*(*u_r++);
				u_last += (*NaU)*(*u_r_last++);
				NaU++;
			}		
		}
		double r = R + u;
		fRadius_X[i] = R;
		fRadius_x[i] = r;

		/* deformation gradient */
		if (needs_F)
		{
			/* 2D deformation gradient */
			fShapes->GradU(fLocDisp, fF2D, i);
			fF2D.PlusIdentity();

			/* make axisymmetric */
			dMatrixT& F3D = fF_List[i];
			F3D.Rank2ExpandFrom2D(fF2D);
			F3D(2,2) = r/R;
		}

		/* "last" deformation gradient */
		if (needs_F_last)
		{
			/* 2D deformation gradient */
			fShapes->GradU(fLocLastDisp, fF2D, i);
			fF2D.PlusIdentity();

			/* make axisymmetric */
			dMatrixT& F3D = fF_last_List[i];
			F3D.Rank2ExpandFrom2D(fF2D);
			F3D(2,2) = (R + u_last)/R;
		}
	}
}
