/* $Id: SimoQ1P0Axi.cpp,v 1.3 2004-04-27 07:24:47 paklein Exp $ */
#include "SimoQ1P0Axi.h"

#include "ShapeFunctionT.h"
#include "SolidMaterialT.h"
#include "SolidMatListT.h"

const double Pi2 = 2.0*acos(-1.0);
const int kRadialDirection = 0; /* x <-> r */

using namespace Tahoe;

/* constructor */
SimoQ1P0Axi::SimoQ1P0Axi(const ElementSupportT& support, const FieldT& field):
	UpdatedLagrangianAxiT(support, field),
	fF_tmp(NumSD())
{

}

/* data initialization */
void SimoQ1P0Axi::Initialize(void)
{
	const char caller[] = "SimoQ1P0Axi::Initialize";

	/* inherited */
	UpdatedLagrangianAxiT::Initialize();

	/* check geometry code and number of element nodes -> Q1 */
	if (GeometryCode() == GeometryT::kQuadrilateral) {
		if (NumElementNodes() != 4) 
			ExceptionT::BadInputValue(caller, "expecting 4 node quad: %d", NumElementNodes());
	}
	else if (GeometryCode() == GeometryT::kHexahedron) {
		if (NumElementNodes() != 8) 
			ExceptionT::BadInputValue(caller, "expecting 8 node hex: %d", NumElementNodes());
	}
	else
		ExceptionT::BadInputValue(caller, "expecting hex or quad geometry: %d", GeometryCode());
	
	/* need to store last deformed element volume */
	fElementVolume.Dimension(NumElements());	
	fElementVolume = 0.0;
	fElementVolume_last.Dimension(NumElements());
	fElementVolume_last = 0.0;
	
	/* element pressure */
	fPressure.Dimension(NumElements());
	fPressure = 0.0;
	
	/* determinant of the deformation gradient */
	fJacobian.Dimension(NumIP());
	fJacobian = 1.0;
	
	/* dimension work space */
	fMeanGradient.Dimension(NumSD(), NumElementNodes());
	fNEEmat.Dimension(fLHS);
	fdiff_b.Dimension(fGradNa);
	fb_bar.Dimension(fGradNa);
	fb_sig.Dimension(fGradNa);

	/* need to initialize previous volume */
	Top();
	while (NextElement())
	{
		/* inherited - computes gradients and standard 
		 * deformation gradients */
		UpdatedLagrangianAxiT::SetGlobalShape();

		/* compute mean of shape function gradients */
		double H; /* reference volume */
		double& v = fElementVolume_last[CurrElementNumber()];
		SetMeanGradient(fMeanGradient, H, v);
	}
}

/* finalize current step - step is solved */
void SimoQ1P0Axi::CloseStep(void)
{
	/* inherited */
	UpdatedLagrangianAxiT::CloseStep();
	
	/* store converged solution */
	fElementVolume_last = fElementVolume;
}
	
/* restore last converged state */
GlobalT::RelaxCodeT SimoQ1P0Axi::ResetStep(void)
{
	/* inherited */
	GlobalT::RelaxCodeT relax = UpdatedLagrangianAxiT::ResetStep();
	
	/* store converged solution */
	fElementVolume = fElementVolume_last;

	return relax;
}

/* read restart information from stream */
void SimoQ1P0Axi::ReadRestart(istream& in)
{
	/* inherited */
	UpdatedLagrangianAxiT::ReadRestart(in);
	
	/* read restart data */
	in >> fElementVolume;
	
	/* reset last state */
	fElementVolume_last = fElementVolume;
}

/* write restart information from stream */
void SimoQ1P0Axi::WriteRestart(ostream& out) const
{
	/* inherited */
	UpdatedLagrangianAxiT::WriteRestart(out);
	
	/* read restart data */
	out << fElementVolume << '\n';
}

/***********************************************************************
 * Protected
 ***********************************************************************/

/* form shape functions and derivatives */
void SimoQ1P0Axi::SetGlobalShape(void)
{
	/* current element number */
	int elem = CurrElementNumber();

	/* inherited - computes gradients and standard 
	 * deformation gradients */
	UpdatedLagrangianAxiT::SetGlobalShape();

	/* compute mean of shape function gradients */
	double H; /* reference volume */
	double& v = fElementVolume[elem];
	SetMeanGradient(fMeanGradient, H, v);
	
	/* last deformed volume */
	double& v_last = fElementVolume_last[elem];

	/* what needs to get computed */
	int material_number = CurrentElement().MaterialNumber();
	bool needs_F = Needs_F(material_number);
	bool needs_F_last = Needs_F_last(material_number);

	/* loop over integration points */
	for (int i = 0; i < NumIP(); i++)
	{
		/* deformation gradient */
		if (needs_F)
		{
			/* "replace" dilatation */
			dMatrixT& F = fF_List[i];
			double J = F.Det();
			if (J <= 0.0) ExceptionT::BadJacobianDet("SimoQ1P0Axi::SetGlobalShape");			
			F *= pow(v/(H*J), 1.0/3.0);
			
			/* store Jacobian */
			fJacobian[i] = J;
		}

		/* "last" deformation gradient */
		if (needs_F_last)
		{
			/* "replace" dilatation */
			dMatrixT& F = fF_last_List[i];
			double J = F.Det();
			F *= pow(v_last/(H*J), 1.0/3.0);
		}
	}
}

/* form the element stiffness matrix */
void SimoQ1P0Axi::FormStiffness(double constK)
{
	/* matrix format */
	dMatrixT::SymmetryFlagT format =
		(fLHS.Format() == ElementMatrixT::kNonSymmetric) ?
		dMatrixT::kWhole :
		dMatrixT::kUpperOnly;

	/* current element info */
	int el = CurrElementNumber();
	double v = fElementVolume[el];
	double p_bar = fPressure[el];

	/* integration */
	const double* Det    = fCurrShapes->IPDets();
	const double* Weight = fCurrShapes->IPWeights();

	/* initialize */
	fStressStiff = 0.0;
	fNEEvec = 0.0;

	int  nsd = NumSD();
	int ndof = NumDOF();
	int nen  = NumElementNodes();
	fCurrShapes->GradNa(fMeanGradient, fb_bar);	
	fShapes->TopIP();
	while ( fShapes->NextIP() )
	{
		int ip = fShapes->CurrIP();
		double r = fRadius_x[ip];

		/* scale factor */
		double scale = Pi2*r*constK*(*Det++)*(*Weight++);

		/* collect array of nodal shape functions */
		const double* Na_u = fCurrShapes->IPShapeU();
		fIPShape.Alias(nen, Na_u);
		double* u_r = fNEEvec.Pointer(kRadialDirection);
		for (int a = 0; a < nen; a++) {
			*u_r = *Na_u++;
			u_r += ndof;
		}

	/* S T R E S S   S T I F F N E S S */			
		/* compute Cauchy stress */
		const dSymMatrixT& cauchy = fCurrMaterial->s_ij();
		cauchy.ToMatrix(fStressMat);
		fMat2D.Rank2ReduceFrom3D(fStressMat);
		
		/* determinant of modified deformation gradient */
		double J_bar = DeformationGradient().Det();
		
		/* detF correction */
		double J_correction = J_bar/fJacobian[CurrIP()];
		double p = J_correction*cauchy.Trace()/3.0;

		/* get shape function gradients matrix */
		fCurrShapes->GradNa(fGradNa);
		fb_sig.MultAB(fMat2D, fGradNa); //contribution from out-of-plane stress??

		/* integration constants */		
		fMat2D *= scale*J_correction;
	
		/* using the stress symmetry */
		fStressStiff.MultQTBQ(fGradNa, fMat2D, format, dMatrixT::kAccumulate);

		/* contribution from out-of-plane stress */
		fLHS.Outer(fNEEvec, fNEEvec, scale*J_correction*fStressMat(2,2)/(r*r), dMatrixT::kAccumulate);

	/* M A T E R I A L   S T I F F N E S S */									
		/* strain displacement matrix */
		Set_B_bar_axi(fIPShape, fCurrShapes->Derivatives_U(), fMeanGradient, r, fB);

		/* get D matrix */
		fD.Rank4ReduceFrom3D(fCurrMaterial->c_ijkl());
		fD *= scale;
						
		/* accumulate */
		fLHS.MultQTBQ(fB, fD, format, dMatrixT::kAccumulate);

		/* add axisymmetric contribution to b */
		Na_u = fCurrShapes->IPShapeU();
		double* b_r = fGradNa.Pointer(kRadialDirection);
		for (int a = 0; a < nen; a++) {
			*b_r += (*Na_u++)/r;
			b_r += nsd;
		}
		
		/* $div div$ term */	
		fNEEmat.Outer(fGradNa, fGradNa);
		fLHS.AddScaled(p_bar*scale, fNEEmat);

		fdiff_b.DiffOf(fGradNa, fb_bar);
		fNEEmat.Outer(fdiff_b, fdiff_b);
		fLHS.AddScaled(scale*2.0*p/3.0, fNEEmat);
		
		fNEEmat.Outer(fb_sig, fdiff_b);
		fNEEmat.Symmetrize();
		fLHS.AddScaled(-J_correction*scale*4.0/3.0, fNEEmat);

		bSp_bRq_to_KSqRp(fGradNa, fNEEmat);
		fLHS.AddScaled(scale*(p - p_bar), fNEEmat);
	}
						
	/* stress stiffness into fLHS */
	fLHS.Expand(fStressStiff, NumDOF(), dMatrixT::kAccumulate);
	
	/* $\bar{div}\bar{div}$ term */
	fNEEmat.Outer(fb_bar, fb_bar);
	fLHS.AddScaled(-p_bar*v, fNEEmat);
}

/* calculate the internal force contribution ("-k*d") */
void SimoQ1P0Axi::FormKd(double constK)
{
	const double* Det    = fCurrShapes->IPDets();
	const double* Weight = fCurrShapes->IPWeights();

	/* collect incremental heat */
	bool need_heat = fElementHeat.Length() == fShapes->NumIP();

	/* current element number */
	int elem = CurrElementNumber();

	/* constant pressure */
	double& p_bar = fPressure[elem];
	p_bar = 0.0;

	int nen = NumElementNodes();
	fCurrShapes->TopIP();
	while ( fCurrShapes->NextIP() )
	{
		int ip = fShapes->CurrIP();
		double r = fRadius_x[ip];

		/* collect array of nodal shape functions */
		fIPShape.Alias(nen, fShapes->IPShapeU());
	
		/* strain displacement matrix */
		Set_B_bar_axi(fIPShape, fCurrShapes->Derivatives_U(), fMeanGradient, r, fB);

		/* translate Cauchy stress to axisymmetric */
		const dSymMatrixT& cauchy = fCurrMaterial->s_ij();
		fStress2D_axi.ReduceFrom3D(cauchy);
		
		/* B^T * Cauchy stress */
		fB.MultTx(fStress2D_axi, fNEEvec);
		
		/* determinant of modified deformation gradient */
		double J_bar = DeformationGradient().Det();
		
		/* detF correction */
		double J_correction = J_bar/fJacobian[CurrIP()];
		double vol = Pi2*r*(*Weight++)*(*Det++)*J_correction;
		
		/* integrate pressure */
		p_bar += vol*cauchy.Trace()/3.0;
		
		/* accumulate */
		fRHS.AddScaled(constK*vol, fNEEvec);

		/* incremental heat generation */
		if (need_heat) 
			fElementHeat[fShapes->CurrIP()] += fCurrMaterial->IncrementalHeat();
	}
	
	/* volume averaged */
	p_bar /= fElementVolume[CurrElementNumber()];
}

/* read materials data */
void SimoQ1P0Axi::ReadMaterialData(ifstreamT& in)
{
	/* inherited */
	UpdatedLagrangianAxiT::ReadMaterialData(in);

	/* make sure 2D materials are plane strain */
	if (StructuralMaterialList().HasPlaneStress()) 
		ExceptionT::BadInputValue("SimoQ1P0Axi::ReadMaterialData", "2D materials must be plane strain");
}

/***********************************************************************
 * Private
 ***********************************************************************/

/* compute mean shape function gradient, Hughes (4.5.23) */
void SimoQ1P0Axi::SetMeanGradient(dArray2DT& mean_gradient, double& H, double& v) const
{
	/* assume same integration rule defined for current and references
	 * shape functions */
	int nip = NumIP();
	const double*   det = fCurrShapes->IPDets();
	const double* det_0 = fShapes->IPDets();
	const double*     w = fShapes->IPWeights();

	/* H and current volume */
	H = 0.0;
	v = 0.0;
	for (int i = 0; i < nip; i++) {
		H += Pi2*fRadius_X[i]*w[i]*det_0[i];
		v += Pi2*fRadius_x[i]*w[i]*det[i];
	}

	/* initialize */
	mean_gradient = 0.0;			

	/* integrate */
	int nen = mean_gradient.MinorDim();
	for (int i = 0; i < nip; i++) {

		double r = fRadius_x[i];
		double dv_by_v = Pi2*r*w[i]*det[i]/v;

		mean_gradient.AddScaled(dv_by_v, fCurrShapes->Derivatives_U(i));
		
		/* contribution from out-of-plane component */
		double* mean_r = mean_gradient(kRadialDirection);
		const double* pNaU = fCurrShapes->IPShapeU(i);
		for (int a = 0; a < nen; a++)
			*mean_r++ += dv_by_v*(*pNaU++)/r;
	}
}

void SimoQ1P0Axi::bSp_bRq_to_KSqRp(const dMatrixT& b, dMatrixT& K) const
{
#if __option(extended_errorcheck)
	/* dimension check */
	if (b.Length() != K.Rows() ||
	    K.Rows() != K.Cols()) ExceptionT::SizeMismatch("SimoQ1P0Axi::bSp_bRq_to_KSqRp");
#endif

	int dim = K.Rows();
	int sub_dim = b.Rows();
	int S = 0;
	int p = 0;
	for (int i = 0; i < dim; i++)
	{
		int R = 0;
		int q = 0;
		for (int j = 0; j < dim; j++)
		{
			K(i,j) = b(q,S)*b(p,R);
		
			q++;
			if (q == sub_dim) {
				R++;
				q = 0;
			}
		}
		p++;
		if (p == sub_dim) {
			S++;
			p = 0;
		}
	}	
}