/* $Id: ThermalSurfaceT.cpp,v 1.1.2.6 2002-05-17 01:29:56 paklein Exp $ */
#include "ThermalSurfaceT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>

#include "ElementSupportT.h"
#include "fstreamT.h"
#include "Constants.h"
#include "SurfaceShapeT.h"
#include "eControllerT.h"

/* constructor */
ThermalSurfaceT::ThermalSurfaceT(const ElementSupportT& support, const FieldT& field):
	CSEBaseT(support, field),
	fLocTemperatures(LocalArrayT::kDisp)
{

}

/* form of tangent matrix */
GlobalT::SystemTypeT ThermalSurfaceT::TangentType(void) const
{
	/* tangent matrix is not symmetric */
	return GlobalT::kSymmetric;
}

void ThermalSurfaceT::Initialize(void)
{
	/* inherited */
	CSEBaseT::Initialize();

	/* check output codes */
	if (fNodalOutputCodes[MaterialData])
	{
		cout << "\n ThermalSurfaceT::Initialize: material outputs not supported, overriding" << endl;
		fNodalOutputCodes[MaterialData] = IOBaseT::kAtNever;
	}

	/* initialize local array */
	fLocTemperatures.Allocate(NumElementNodes(), NumDOF());
	Field().RegisterLocal(fLocTemperatures);

	/* read conduction parameters */
	ifstreamT& in = ElementSupport().Input();
	int num_sets = -1;
	in >> num_sets;
	fConduction.Dimension(num_sets, 2);
	fConduction = -1.0;

	/* sets of parameters as 
	 * [set #] [K_0] [D_c] */
	fConduction.ReadNumbered(in);
	
	/* check */
	if (fConduction.Min() < 0) throw eBadInputValue;
	
	/* echo */	
	ostream& out = ElementSupport().Output();
	int d_width = OutputWidth(out, fConduction.Pointer());
	out << "\n Surface conduction:\n";
	out << setw(kIntWidth) << "set"
	    << setw(d_width) << "K_0"
	    << setw(d_width) << "d_c" << '\n';
	fConduction.WriteNumbered(out);
	out.flush();
}

/***********************************************************************
* Protected
***********************************************************************/

/* called by FormRHS and FormLHS */
void ThermalSurfaceT::LHSDriver(void)
{
	/* algorithmic constants */
	double constK = 0.0;
	int     formK = fController->FormK(constK);
	if (!formK) return;

	/* node map of facet 1 */
	iArrayT facet1;
	(fShapes->NodesOnFacets()).RowAlias(0, facet1);

	/* work space */
	dArrayT gap(fLocCurrCoords.MinorDim());
	dArrayT jump_Na;
	dMatrixT K_temp(fLHS.Rows());
	
	Top();
	while (NextElement())
	{
		/* current element */
		const ElementCardT& element = CurrentElement();
	
		/* conduction parameters */
		int set = element.MaterialNumber();
		double K_0 = fConduction(set, kK_0);
		double d_c = fConduction(set, kd_c);

		/* get ref geometry (1st facet only) */
		fNodes1.Collect(facet1, element.NodesX());
		fLocInitCoords1.SetLocal(fNodes1);

		/* get current geometry */
		SetLocalX(fLocCurrCoords);

	  	/* initialize */
		fLHS = 0.0;
	
		/* loop over integration points */
		fShapes->TopIP();
		while (fShapes->NextIP())
		{
			/* gap vector from facet1 to facet2 */
			fShapes->InterpolateJump(fLocCurrCoords, gap);
			double d = gap.Magnitude();
			
			/* current conduction */
			double K = (d > d_c) ? 0.0 : K_0*(1.0 - (d/d_c));
			
			/* integration weights */
			double j = fShapes->Jacobian();
			double w = fShapes->IPWeight();

			/* integration point jump shape functions */
			fShapes->JumpShapes(jump_Na);
			
			/* accumulate */
			K_temp.Outer(jump_Na, jump_Na);
			fLHS.AddScaled(j*w*constK*K, K_temp);
		}
									
		/* assemble */
		AssembleLHS();
	}
}

void ThermalSurfaceT::RHSDriver(void)
{
	/* time-stepping parameters */
	double constKd = 0.0;
	int     formKd = fController->FormKd(constKd);
	if (!formKd) return;
	
	/* node map of facet 1 */
	iArrayT facet1;
	(fShapes->NodesOnFacets()).RowAlias(0, facet1);

	/* work space */
	dArrayT gap(fLocCurrCoords.MinorDim());
	dArrayT Na;

	/* block info - needed for source terms */
	int block_dex = 0;
	const ElementBlockDataT* block_data = fBlockData.Pointer(block_dex);
	const dArray2DT* block_source = Field().Source(block_data->ID());
	dArrayT ip_source;
	if (block_source) ip_source.Dimension(fShapes->NumIP());
	int block_count = 0;

ostream& out = ElementSupport().Output();
out << "\n ThermalSurfaceT::RHSDriver: source: \n";
block_source->WriteNumbered(out);
out << endl;
	
	Top();
	while (NextElement())
	{
		/* reset block info */
		if (block_count == block_data->Dimension()) {
			block_data = fBlockData.Pointer(++block_dex);
			block_source = Field().Source(block_data->ID());
			block_count = 0;
		}

		/* convert heat increment/volume to rate */
		if (block_source) {
			block_source->RowCopy(block_count, ip_source);
			ip_source /= ElementSupport().TimeStep();
		}
		block_count++;
	
		/* current element */
		const ElementCardT& element = CurrentElement();
	
		/* conduction parameters */
		int set = element.MaterialNumber();
		double K_0 = fConduction(set, kK_0);
		double d_c = fConduction(set, kd_c);

		/* get ref geometry (1st facet only) */
		fNodes1.Collect(facet1, element.NodesX());
		fLocInitCoords1.SetLocal(fNodes1);

		/* get current geometry */
		SetLocalX(fLocCurrCoords);

		/* get nodal temperatures */
		SetLocalU(fLocTemperatures);

		/* initialize */
		fRHS = 0.0;
	
		/* loop over integration points */
		fShapes->TopIP();
		while (fShapes->NextIP())
		{
			/* gap vector from facet1 to facet2 */
			fShapes->InterpolateJump(fLocCurrCoords, gap);
			double d = gap.Magnitude();
			
			/* current conduction */
			double K = (d > d_c) ? 0.0 : K_0*(1.0 - (d/d_c));
			
			/* temperature jump */
			const dArrayT& temp_jump = fShapes->InterpolateJumpU(fLocTemperatures);
			double j = fShapes->Jacobian();
			double w = fShapes->IPWeight();

			/* integration point jump shape functions */
			fShapes->JumpShapes(Na);
			
			/* accumulate */
			fRHS.AddScaled(-j*w*constKd*K*temp_jump[0], Na);
			
			/* temperature source */
			if (block_source) {

				/* integration point shape functions */
				fShapes->Shapes(Na);
			
				/* accumulate - split heat half onto each face */
				fRHS.AddScaled(0.5*j*w*constKd*ip_source[fShapes->CurrIP()], Na);			
			}
		}
									
		/* assemble */
		AssembleRHS();
	}
}

/* extrapolate the integration point stresses and strains and extrapolate */
void ThermalSurfaceT::ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
	const iArrayT& e_codes, dArray2DT& e_values)
{
	/* number of nodally smoothed values */
	int n_out = n_codes.Sum();
	int e_out = e_codes.Sum();

	/* nothing to output */
	if (n_out == 0 && e_out == 0) return;

	int nsd = NumSD();
	int ndof = NumDOF();
	int nen = NumElementNodes();

	/* reset averaging workspace */
	ElementSupport().ResetAverage(n_out);

	/* allocate element results space */
	e_values.Allocate(NumElements(), e_out);
	e_values = 0.0;

	/* work arrays */
	dArray2DT nodal_space(nen, n_out);
	dArray2DT nodal_all(nen, n_out);
	dArray2DT coords, disp;
	dArray2DT jump, Tmag;

	/* ip values */
	dArrayT ipjump(1), ipTmag(1);
	LocalArrayT loc_init_coords(LocalArrayT::kInitCoords, nen, nsd);
	LocalArrayT loc_disp(LocalArrayT::kDisp, nen, ndof);
	ElementSupport().RegisterCoordinates(loc_init_coords);
	Field().RegisterLocal(loc_disp);

	/* set shallow copies */
	double* pall = nodal_space.Pointer();
	coords.Set(nen, n_codes[NodalCoord], pall);
	pall += coords.Length();
	disp.Set(nen, n_codes[NodalDisp], pall);
	pall += disp.Length();
	jump.Set(nen, n_codes[NodalDispJump], pall);
	pall += jump.Length();
	Tmag.Set(nen, n_codes[NodalTraction], pall);

	/* element work arrays */
	dArrayT element_values(e_values.MinorDim());
	pall = element_values.Pointer();
	dArrayT centroid;
	if (e_codes[Centroid])
	{
		centroid.Set(nsd, pall); 
		pall += nsd;
	}
	double e_tmp, area;
	double& phi = (e_codes[CohesiveEnergy]) ? *pall++ : e_tmp;
	double& T_e = (e_codes[Traction]) ? *pall++ : e_tmp;

	/* node map of facet 1 */
	iArrayT facet1;
	(fShapes->NodesOnFacets()).RowAlias(0, facet1);

	dArrayT gap(fLocCurrCoords.MinorDim());
	Top();
	while (NextElement())
	{
		/* current element */
		ElementCardT& element = CurrentElement();

		/* conduction parameters */
		int set = element.MaterialNumber();
		double K_0 = fConduction(set, kK_0);
		double d_c = fConduction(set, kd_c);

		/* initialize */
	    nodal_space = 0.0;
	    element_values = 0.0;

		/* coordinates for whole element */
		if (n_codes[NodalCoord])
		{
			SetLocalX(loc_init_coords);
			loc_init_coords.ReturnTranspose(coords);
		}
		
		/* temperatures for whole element */
		if (n_codes[NodalDisp])
		{
			SetLocalU(loc_disp);
			loc_disp.ReturnTranspose(disp);
		}

		/* compute output */
		if (element.Flag() == kON)
		{
			/* get ref geometry (1st facet only) */
			fNodes1.Collect(facet1, element.NodesX());
			fLocInitCoords1.SetLocal(fNodes1);

			/* get current geometry */
			SetLocalX(fLocCurrCoords); //EFFECTIVE_DVA

			/* initialize element values */
			phi = area = 0.0;
			if (e_codes[Centroid]) centroid = 0.0;

			/* integrate */
			fShapes->TopIP();
			while (fShapes->NextIP())
			{
				/* element integration weight */
				double ip_w = fShapes->Jacobian()*fShapes->IPWeight();
				area += ip_w;

				/* gap */
				fShapes->InterpolateJump(fLocCurrCoords, gap);
				double d = gap.Magnitude();

				/* temperature jump */
				const dArrayT& temp_jump = fShapes->InterpolateJumpU(loc_disp);

				/* current conduction */
				double K = (d > d_c) ? 0.0 : K_0*(1.0 - (d/d_c));

				if (n_codes[NodalDispJump])
				{
					/* extrapolate ip values to nodes */
					ipjump[0] = temp_jump[0];
					fShapes->Extrapolate(ipjump, jump);				
				}

				/* traction */
				if (n_codes[NodalTraction] || e_codes[Traction])
				{
					/* compute flux */
					double t = K*temp_jump[0];
					
					/* extrapolate to nodes */
					if (n_codes[NodalTraction])
					{
						ipTmag[0] = t;
						fShapes->Extrapolate(ipTmag, Tmag);
					}
					
					/* element average */
					if (e_codes[Traction]) T_e += ip_w*t;
				}	
				
				/* moment */
				if (e_codes[Centroid])
					centroid.AddScaled(ip_w, fShapes->IPCoords());
				
//TEMP - what to do about this?
				/* cohesive energy */
				if (e_codes[CohesiveEnergy])
				{
					/* surface potential */
					double potential = 0.0;

					/* integrate */
					phi += potential*ip_w;
				}
			}
			
			/* element values */
			if (e_codes[Centroid]) centroid /= area;
			if (e_codes[Traction]) T_e /= area;
		}
		/* element has failed */
		else
		{
			/* can still be calculated */
			if (e_codes[Centroid] || e_codes[CohesiveEnergy])
			{
				/* get ref geometry (1st facet only) */
				fNodes1.Collect(facet1, element.NodesX());
				fLocInitCoords1.SetLocal(fNodes1);

				/* initialize element values */
				phi = area = 0.0;
				if (e_codes[Centroid]) centroid = 0.0;
	
				/* integrate */
				fShapes->TopIP();
				while (fShapes->NextIP())
				{
					/* element integration weight */
					double ip_w = fShapes->Jacobian()*fShapes->IPWeight();
					area += ip_w;
		
					/* moment */
					if (e_codes[Centroid])
						centroid.AddScaled(ip_w, fShapes->IPCoords());
				}
				
				/* element values */
				if (e_codes[Centroid]) centroid /= area;
				
//TEMP - what to do about this?
				/* cohesive energy */
				if (e_codes[CohesiveEnergy])
					phi = 0.0;
			}		
		}

		/* copy in the cols (in sequence of output) */
		int colcount = 0;
		nodal_all.BlockColumnCopyAt(disp  , colcount); colcount += disp.MinorDim();
		nodal_all.BlockColumnCopyAt(coords, colcount); colcount += coords.MinorDim();
		nodal_all.BlockColumnCopyAt(jump  , colcount); colcount += jump.MinorDim();
		nodal_all.BlockColumnCopyAt(Tmag  , colcount);

		/* accumulate - extrapolation done from ip's to corners => X nodes */
		ElementSupport().AssembleAverage(CurrentElement().NodesX(), nodal_all);
		
		/* store results */
		e_values.SetRow(CurrElementNumber(), element_values);		
	}

	/* get nodally averaged values */
	ElementSupport().OutputUsedAverage(n_values);
}
