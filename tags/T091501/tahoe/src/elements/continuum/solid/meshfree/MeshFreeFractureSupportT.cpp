/* $Id: MeshFreeFractureSupportT.cpp,v 1.1.1.1 2001-01-29 08:20:39 paklein Exp $ */
/* created: paklein (02/15/2000)                                          */

#include "MeshFreeFractureSupportT.h"

#include "StringT.h"
#include "fstreamT.h"
#include "MeshFreeShapeFunctionT.h"
#include "StructuralMaterialT.h"
#include "FDStructMatT.h"

/* crack growth */
#include "Front2DT.h"
#include "Front3DT.h"
#include "FrontNodeT.h"
#include "SamplingSurfaceT.h"

/* constructor */
MeshFreeFractureSupportT::MeshFreeFractureSupportT(ifstreamT& in):
	MeshFreeElementSupportT(in),
	fCriterion(kNoCriterion)
{

}

/* destructor */
MeshFreeFractureSupportT::~MeshFreeFractureSupportT(void)
{
	for (int i = 0; i < fFrontList.Length(); i++)
		delete fFrontList[i];
	fFrontList = NULL;

	for (int j = 0; j < fSamplingSurfaces.Length(); j++)
		delete fSamplingSurfaces[j];
	fSamplingSurfaces = NULL;
}

/* write output */
void MeshFreeFractureSupportT::WriteOutput(ostream& out)
{
	out << " Crack path data:\n";
	fFacets.WriteNumbered(out);
}

/* initialize/finalize time increment */
void MeshFreeFractureSupportT::CloseStep(void)
{
	/* set all marked to off */
	for (int i = 0; i < fSamplingSurfaces.Length(); i++)
		fSamplingSurfaces[i]->ChangeFlags(kMarked, kOFF);
}

void MeshFreeFractureSupportT::ResetStep(void)
{
	/* reset all marked to on */
	for (int i = 0; i < fSamplingSurfaces.Length(); i++)
		fSamplingSurfaces[i]->ChangeFlags(kMarked, kON);
		
//TEMP - need to reset cutting surfaces and restore all
//       shape function databases
cout << "\n MeshFreeFractureSupportT::ResetStep: not fully implemented" << endl;
throw eGeneralFail;		
}

/***********************************************************************
* Protected
***********************************************************************/

istream& operator>>(istream& in, MeshFreeFractureSupportT::FractureCriterionT& criterion)
{
	int i_criterion = -1;
	in >> i_criterion;
	switch(i_criterion)
	{
		case MeshFreeFractureSupportT::kNoCriterion:
			criterion = MeshFreeFractureSupportT::kNoCriterion;
			break;
		case MeshFreeFractureSupportT::kMaxHoopStress:
			criterion = MeshFreeFractureSupportT::kMaxHoopStress;
			break;
		case MeshFreeFractureSupportT::kMaxTraction:
			criterion = MeshFreeFractureSupportT::kMaxTraction;
			break;
		case MeshFreeFractureSupportT::kAcoustic:
			criterion = MeshFreeFractureSupportT::kAcoustic;
			break;
		default:
			cout << "\n operator>>MeshFreeFractureSupportT::FractureCriterionT: bad value: "
			     << i_criterion << endl;
			throw eBadInputValue;	
	}
	return in;
}

/* initialization */
void MeshFreeFractureSupportT::InitSupport(ifstreamT& in, ostream& out,
	AutoArrayT<ElementCardT>& elem_cards, const iArrayT& surface_nodes,
	int numDOF, int max_node_num, const StringT& model_file, IOBaseT::FileTypeT format)
{
	/* inherited */
	MeshFreeElementSupportT::InitSupport(in, out, elem_cards, surface_nodes,
		numDOF, max_node_num, model_file, format);

	/* unitialized */
	fNumFacetNodes = -1;

	/* field cutting facets (and active crack fronts) */
	InitCuttingFacetsAndFronts(in, out);

	/* field sampling surfaces */
	InitSamplingSurfaces(in, out);
	
	/* insertion criterion */
	if (fFrontList.Length() > 0 || fSamplingSurfaces.Length() > 0)
	{
		in >> fCriterion
		   >> fs_u;
		out << " Failure criterion . . . . . . . . . . . . . . . = " << fCriterion << '\n';
		out << " Failure initiation stress . . . . . . . . . . . = " << fs_u  << '\n';

		/* check */
		if (fCriterion == kNoCriterion)
		{
			out.flush();
			cout << "\n MeshFreeFractureSupportT::InitSupport: criterion does not admit growth: "
			     << kNoCriterion << endl;
			throw eBadInputValue;
		}
		
		/* allocate work space */
		int nsd = fMFShapes->NumSD();
		ftmp_nsd.Allocate(nsd);
	}
}

bool MeshFreeFractureSupportT::CheckGrowth(StructuralMaterialT& material,
	LocalArrayT& disp, bool verbose)
{
	/* clear facets to reset */
	fResetFacets.Allocate(0);
	fInitTractionMan.SetMajorDimension(0, false);

	/* steps in checking growth */
	bool growth_on_front = CheckFronts(material, disp, verbose);
	bool growth_on_surfs = CheckSurfaces(material, disp, verbose);

	/* write cutting facets */
	if (verbose)
	{
		cout << "\n Crack path data:\n";
		fFacets.WriteNumbered(cout);
		cout << endl;
	}

	if (growth_on_front || growth_on_surfs)
	{
		/* send to shapes */
		fMFShapes->ResetFacets(fResetFacets);
		
		/* reset sampling surface databases */
		iArrayT reset_nodes;
		reset_nodes.Alias(fMFShapes->ResetNodes());
		if (reset_nodes.Length() > 0)
		{
			for (int i = 0; i < fSamplingSurfaces.Length(); i++)
				fSamplingSurfaces[i]->ResetSamplingPoints(reset_nodes);
		}		
		return true;
//NOTE: need to return the nodes that were affected by the
//      the insertion of new facets so that the sampling
//      surface DB can be updated.
//
//      Should immediately trigger recalculation to ensure
//      that the nodal support sizes are current, instead of
//      waiting until the next call to fetch value out of
//      the database?? Reset d_max, but not the shape functions?
	}
	else return false;
}

/***********************************************************************
* Private
***********************************************************************/

/* steps in InitSupport() */
void MeshFreeFractureSupportT::InitCuttingFacetsAndFronts(ifstreamT& in,
	ostream& out)
{
	int num_facets = -99;
	in >> num_facets;
	out << " Number of cutting facets. . . . . . . . . . . . = "
	    << num_facets << '\n';
	if (num_facets < 0)
	{
		if (num_facets == -99)
			cout << "\n MeshFreeFractureSupportT::InitSupport: error reading\n"
			     <<   "     number of cutting facets" << endl;
		else
			cout << "\n MeshFreeFractureSupportT::InitSupport: number of cutting facets\n"
			     <<   "     must be >= 0: " << num_facets << endl;
		out.flush();
		throw eBadInputValue;
	}

	if (num_facets > 0)
	{
		/* number of cutting facet nodes */
		int num_facet_nodes;
		in >> num_facet_nodes;
		if (fNumFacetNodes != -1 && fNumFacetNodes != num_facet_nodes)
		{
			cout << "\n MeshFreeFractureSupportT::InitCuttingFacetsAndFronts: number of\n"
			     <<   "    cutting facet nodes " << num_facet_nodes
			     << " does not match previously given value " << fNumFacetNodes
			     << endl;
			throw eBadInputValue;
		}
		else if (fNumFacetNodes == -1)
			InitFacetDatabase(num_facet_nodes);
		
		/* read facet data */
		int nsd = fMFShapes->NumSD();
		fResetFacets.Allocate(0);
		fFacetman.SetMajorDimension(num_facets, false);
		dArray2DT facet;
		dArrayT x;
		for (int i = 0; i < num_facets; i++)
		{
			/* set pointers */
			fFacets.RowAlias(i, x);
			facet.Set(fNumFacetNodes, nsd, fFacets(i));
		
			/* echo facet */
			in >> x;
			out << x.wrap(nsd) << '\n';
	   			
			/* new facets list */
			fResetFacets.Append(i);
		}

		/* set initialization tractions */
		fInitTractionMan.SetMajorDimension(num_facets, false);
		fInitTractions = 0.0; // initial cutting facets are traction free

		/* send initial facets */
		fMFShapes->ResetFacets(fResetFacets);

		/* construct fronts from stream data */
		InitializeFronts(in, out);
	}
	else
	{
		/* dummy values */
		fs_i  = 0.0;
		fda   = 0.0;
		fda_s = 0.0;
		fcone = 0.0;
		fn_s  = 0;
	}
}

void MeshFreeFractureSupportT::InitSamplingSurfaces(ifstreamT& in, ostream& out)
{
	int num_sampling_surfaces = -99;
	in >> num_sampling_surfaces;
	out << " Number of sampling surfaces . . . . . . . . . . = "
	    << num_sampling_surfaces << '\n';
	if (num_sampling_surfaces < 0)
	{
		if (num_sampling_surfaces == -99)
			cout << "\n MeshFreeFractureSupportT::InitSupport: error reading\n"
			     <<   "     number of sampling surfaces" << endl;
		else
			cout << "\n MeshFreeFractureSupportT::InitSupport: number of sampling surfaces\n"
			     <<   "     must be >= 0: " << num_sampling_surfaces << endl;
		out.flush();
		throw eBadInputValue;
	}
	
	if (num_sampling_surfaces > 0)
	{
		fSamplingSurfaces.Allocate(num_sampling_surfaces);

		//TEMP - only support in line data
		for (int i = 0; i < fSamplingSurfaces.Length(); i++)
		{
			/* surface parameters */
			GeometryT::CodeT code;
			int num_facet_nodes;
			int num_samples_per_facet;
			in >> code >> num_facet_nodes >> num_samples_per_facet;

			/* echo parameters */
			out << "\n Surface number. . . . . . . . . . . . . . . . . = " << i+1 << '\n';
			out << " Facet geometry code . . . . . . . . . . . . . . = " << code << '\n';
			out << " Number of facet nodes . . . . . . . . . . . . . = " << num_facet_nodes << '\n';
			out << " Number of sampling points per facet . . . . . . = " << num_samples_per_facet << '\n';
			out.flush();

	   		if (fNumFacetNodes != -1 && fNumFacetNodes != num_facet_nodes)
	   		{
	   			cout << "\n MeshFreeFractureSupportT::InitSamplingSurfaces: number of\n"
	   			     <<   "    cutting facet nodes " << num_facet_nodes
	   			     << " does not match previously given value " << fNumFacetNodes
	   			     << endl;
	   			throw eBadInputValue;
	   		}
	   		else if (fNumFacetNodes == -1)
	   			InitFacetDatabase(num_facet_nodes);

			fSamplingSurfaces[i] = new SamplingSurfaceT(code, num_facet_nodes,
				num_samples_per_facet, fMFShapes->MeshFreeSupport());
			if (!fSamplingSurfaces[i]) throw eOutOfMemory;

			/* surface geometry data */
			int nnd;
			in >> nnd;
			dArray2DT facet_coords(nnd, fMFShapes->NumSD());
			facet_coords.ReadNumbered(in);
			
			int nel;
			in >> nel;
			iArray2DT facet_connects(nel, num_facet_nodes);
			facet_connects.ReadNumbered(in);
			
			/* convert numbering */
			facet_connects--;

			/* echo geometry */
			out << " Number of surface nodes . . . . . . . . . . . . = " << facet_coords.MajorDim() << '\n';
			facet_coords.WriteNumbered(out);
			out << " Number of surface facets. . . . . . . . . . . . = " << facet_connects.MajorDim() << '\n';
			facet_connects.WriteNumbered(out);

			/* set sampling point database */
			fSamplingSurfaces[i]->SetSamplingPoints(facet_coords, facet_connects);
			
			/* set facet flags */
			fSamplingSurfaces[i]->SetFlags(kON);
			
			/* write storage statistics */
			fSamplingSurfaces[i]->WriteStatistics(out);
		}
	}
}

void MeshFreeFractureSupportT::InitializeFronts(ifstreamT& in, ostream& out)
{
	/* crack tip parameters */
	in >> fs_i >> fda >> fda_s >> fcone >> fn_s;
	
	/* check parameters */
	if (fabs(fcone) < kSmall && fn_s != -1) fn_s = 0;
	
	out << " Fraction of failure stress at insertion . . . . = " << fs_i  << '\n';
	out << " Crack extension increment . . . . . . . . . . . = " << fda   << '\n';
	out << " Crack sampling point (per increment da) . . . . = " << fda_s << '\n';
	out << " Sampling angle cone (+/- degrees) . . . . . . . = " << fcone << '\n';
	out << " Number of side sampling points (symmetric). . . = " << fn_s  << endl;

	/* checks */
	if (fs_i < 0.0)
	{
		cout << "\n MeshFreeFractureSupportT::InitializeFronts: fraction of\n"
		     <<   "     failure stress at insertion must be >= 0: " << fs_i << endl;
		throw eBadInputValue;
	}

	/* number of crack fronts */
	int num_fronts;
	in >> num_fronts; if (num_fronts < 0) throw eBadInputValue;
	out << " Number of active crack fronts . . . . . . . . . = " << num_fronts << endl;
			
	/* allocate tip space */	
	if (fn_s > -1 && num_fronts > 0)
	{
		/* allocate list */
		fFrontList.Allocate(num_fronts);
		int numSD = fMFShapes->NumSD();
		for (int i = 0; i < fFrontList.Length(); i++)
		{
			out << "\n Initial configuration: front " << i+1 << '\n';

			int num_seg;
			in >> num_seg;
			out << " Number of edges . . . . . . . . . . . . . . . . = " << num_seg << '\n';
		
			/* echo front specification */
			out << setw(kIntWidth) << "facet";
			out << setw(kIntWidth) << "edge" << '\n';
			iArrayT fr_facets(num_seg), fr_edges(num_seg);
			for (int j = 0; j < num_seg; j++)
			{
				in >> fr_facets[j] >> fr_edges[j];
				out << setw(kIntWidth) << fr_facets[j];
				out << setw(kIntWidth) <<  fr_edges[j] << '\n';
			}
		
			/* correct offset */
			fr_facets--;
			fr_edges--;
		
			if (numSD == 2)
			{
				Front2DT* front2D = new Front2DT(fcone, fda, fda_s, fn_s);
				if (!front2D) throw eOutOfMemory;			

		  		/* store */
		  		fFrontList[i] = front2D;
			}
			else
			{
				Front3DT* front3D = new Front3DT(fcone, fda, fda_s, fn_s);
				if (!front3D) throw eOutOfMemory;			

		  		/* set minimum included angle for kinks on the front */
		  		front3D->SetKinkAngle(90.0);		

		  		/* store */
		  		fFrontList[i] = front3D;
			}
				
	  		fFrontList[i]->Initialize(fFacets, fr_facets, fr_edges);
		}	  	

		/* allocate workspace */
		fhoop.Allocate(numSD);
	}		
}

/* steps in checking growth */
bool MeshFreeFractureSupportT::CheckFronts(StructuralMaterialT& material,
	LocalArrayT& disp, bool verbose)
{
#pragma unused(verbose)

	/* checks */
	if (!fLocGroup.IsRegistered(disp) ||
		disp.Type() != LocalArrayT::kDisp) throw eGeneralFail;

#ifdef __NO_RTTI__
	cout << "\n MeshFreeFractureSupportT::CheckSurfaces: requires RTTI" << endl;
	throw eGeneralFail;
#endif

	/* check for finite deformation */
	FDStructMatT* FD_material = dynamic_cast<FDStructMatT*>(&material);
	
	/* dimensions */
	int nsd = fMFShapes->NumSD();
	
	/* work space */
	dArrayT  x_s;
	dMatrixT Q_s;
	AutoArrayT<int> ex_pts, ex_dir;
	dMatrixT F_inv(nsd);
	dArrayT t_max(nsd), n(nsd), t(nsd);
	
	/* sampling point data */
	dArrayT samples;

	/* loop over fronts */
	bool growth = false;
	for (int k = 0; k < fFrontList.Length(); k++)
	{
		FrontT* front = fFrontList[k];
		const AutoArrayT<FrontNodeT*>& frontnodes = front->FrontNodes();
	
		/* propagation points/directions */
		for (int i = 0; i < frontnodes.Length(); i++)
		{
			/* front node */
			const FrontNodeT& fr_node = *frontnodes[i];
		
			/* sampling point info */
			const dArray2DT& x_list = fr_node.SampleCoords();
			const dArray2DT& Q_list = fr_node.SampleTransForm();

			/* loop over sampling points */
			double max_sample;
			int max_dex = -1;
			int dex = nsd - 1; // "last" component
			samples.Allocate(x_list.MajorDim());
			for (int j = 0; j < x_list.MajorDim(); j++)
			{
				/* fetch coords and transformation tensor */
				x_s.Set(nsd, x_list(j));
				Q_s.Set(nsd, nsd, Q_list(j));
				
				/* set field */
				if (fMFShapes->SetDerivativesAt(x_s))
				{
					/* collect local displacements */
					const iArrayT& neighbors = fMFShapes->Neighbors();
					fLocGroup.SetNumberOfNodes(neighbors.Length());
					disp.SetLocal(neighbors);
					
					/* reference normal */
					Q_s.CopyColumn(nsd - 1, n);

					/* transform to current for finite deformation */
					if (FD_material)
					{
						/* deformation gradient */
						F_inv.Inverse(FD_material->F());

						/* current unit normal */
						F_inv.MultTx(n, t);
						n.UnitVector(t);
					}
					
					/* compute criterion */
					double sample = ComputeCriterion(material, Q_s, n, fCriterion, fs_u, t);
					samples[j] = sample;
					
					/* find max */
					if (max_dex == -1 || sample > max_sample)
					{
						/* store */
						max_dex = j;
						max_sample = sample;
						t_max = t;
					}
				}
				else
				{
					cout << "\n MeshFreeFractureSupportT::CheckFronts: error at sampling point: ";
					cout << x_s.no_wrap() << endl;
					throw eGeneralFail;
				}
			}
		
			/* (relaxed) fracture initation criterion */
			if (max_sample >= fs_u*fs_i)
			{
				/* record */
				ex_pts.Append(i);
				ex_dir.Append(max_dex);
								
				/* store initiation traction */
				fInitTractionMan.SetMajorDimension(ex_pts.Length(), true);
				fInitTractions.SetRow(ex_pts.Length() - 1, t_max);
			}
		}

		/* check for growth */
		if (ex_pts.Length() > 0)
		{
			/* get new facets/update front */
			const dArray2DT& new_facets = front->NewFacets(ex_pts, ex_dir);
	
			/* add to cutting surfaces */
			int num_new_facets = new_facets.MajorDim();
			int num_facets     = fFacets.MajorDim();
			fFacetman.SetMajorDimension(num_facets + num_new_facets, true);
			dArray2DT facet;
			for (int i = 0; i < num_new_facets; i++)
			{
				int facet_dex = num_facets + i;
			
				/* alias */			
				facet.Set(nsd, nsd, fFacets(facet_dex));
			
				/* copy/store */
				new_facets.RowCopy(i, facet);
			
				/* keep list */
				fResetFacets.Append(facet_dex);
			}
	
			/* set flag */
			growth = true;
			
			/* write stress distributions */
			cout << "\n MeshFreeFractureSupportT::CheckFronts:\n";
			cout <<   "    front: " << k+1 << '\n';
			cout <<   "  ext pts: " << ex_pts.Length() << '\n';
			dArrayT tmp;
			int d_width = OutputWidth(cout, n.Pointer());
			for (int j = 0; j < ex_pts.Length(); j++)
			{
				/* front node */
				const FrontNodeT& fr_node = *frontnodes[j];
				tmp.Set(nsd, (double*) fr_node.Coords());
				cout <<   "  front x: " << tmp.no_wrap() << '\n';
		
				/* sampling point info */
				const dArray2DT& x_list = fr_node.SampleCoords();
				const dArray2DT& Q_list = fr_node.SampleTransForm();

				/* header */
				cout << setw(kIntWidth) << "i"
				     << setw(d_width) << "sample"
				     << setw(d_width) << "x" << '\n';

				/* loop over sampling points */
				int dex = nsd - 1; // "last" component
				for (int k = 0; k < x_list.MajorDim(); k++)
				{
					/* fetch coords and transformation tensor */
					x_s.Set(nsd, x_list(k));
						
					/* output */
					cout << setw(kIntWidth) << k+1
					     <<   setw(d_width) << samples[k]
					     <<   setw(d_width) << x_s.no_wrap() << '\n';
				}
				cout << endl;
			}
		}
	}

	return growth;
}

bool MeshFreeFractureSupportT::CheckSurfaces(StructuralMaterialT& material,
	LocalArrayT& disp, bool verbose)
{
#pragma unused(verbose)

	/* checks */
	if (!fLocGroup.IsRegistered(disp) || disp.Type() != LocalArrayT::kDisp)
		throw eGeneralFail;

#ifdef __NO_RTTI__
	cout << "\n MeshFreeFractureSupportT::CheckSurfaces: requires RTTI" << endl;
	throw eGeneralFail;
#endif

	/* check for finite deformation */
	FDStructMatT* FD_material = dynamic_cast<FDStructMatT*>(&material);

	/* sampling point data */
	iArrayT neighbors;
	dArrayT field;
	dArray2DT Dfield;
	
	/* work space */
	int nsd = fMFShapes->NumSD();
	dArrayT tmp_nsd(nsd), n_(nsd), t_(nsd);
	dMatrixT F_inv(nsd), Q_(nsd);
	dArray2DT coordinates;

	/* loop over surfaces */
	bool growth = false;
	for (int i = 0; i < fSamplingSurfaces.Length(); i++)
	{
		/* dimensions */
		SamplingSurfaceT& surface = *(fSamplingSurfaces[i]);
		int num_facets = surface.NumberOfFacets();
		int num_points = surface.SamplesPerFacet();
		for (int facet = 0; facet < num_facets; facet++)
			if (surface.Flag(facet) == kON)
			{		
				bool fail = false;			
				for (int point = 0; !fail && point < num_points; point++)
				{
					/* load field data */
					surface.LoadSamplingPoint(facet, point, neighbors, field, Dfield);
				
					/* set shape functions */
					fMFShapes->UseDerivatives(neighbors, Dfield);

					/* collect local displacements */
					fLocGroup.SetNumberOfNodes(neighbors.Length());
					disp.SetLocal(neighbors);
	
					/* surface coordinate frame */
					surface.ReferenceConfiguration(facet, point, Q_, n_);
					
					/* transform for finite deformation */
					if (FD_material)
					{
						/* deformation gradient */
						F_inv.Inverse(FD_material->F());

						/* current unit normal */
						F_inv.MultTx(n_, tmp_nsd);
						n_.UnitVector(tmp_nsd);
					}
					
					/* compute criterion */
					double sample = ComputeCriterion(material, Q_, n_, fCriterion, fs_u, t_);
					
					/* initiation criterion */
					if (sample > fs_u*fs_i)
					{
						/* set flags */
						fail = true;
						growth = true;
											
						/* store initiation traction */
						int num_new_facets = fInitTractionMan.MajorDim() + 1;
						fInitTractionMan.SetMajorDimension(num_new_facets, true);
						fInitTractions.SetRow(num_new_facets - 1, t_);
						//NOTE: are only keeping one (the "first") traction
						//      sampled on the surface
						
						/* retrieve facet coordinates */
						coordinates.Allocate(surface.NodesPerFacet(), nsd);
						surface.FacetCoordinates(facet, coordinates);
						
						/* store facet coordinates */
						int tot_num_facets = fFacetman.MajorDim() + 1;
						fFacetman.SetMajorDimension(tot_num_facets, true);
						fFacets.SetRow(tot_num_facets - 1, coordinates);
						
						/* list of new facets */
						fResetFacets.Append(tot_num_facets - 1);
						
						/* don't sample anymore */
						surface.Flag(facet) = kMarked;
					}
				}
			}
	}
	return growth;
}

/* initialize the cutting facet database */
void MeshFreeFractureSupportT::InitFacetDatabase(int num_facet_nodes)
{
	/* dimension */
	fNumFacetNodes = num_facet_nodes;
	
	/* initialize memory managers */
	int nsd = fMFShapes->NumSD();
	fFacetman.SetWard(25, fFacets, nsd*fNumFacetNodes);	
	fInitTractionMan.SetWard(25, fInitTractions, nsd);
	
	/* send to meshfree domain */
	fMFShapes->SetCuttingFacets(fFacets, fNumFacetNodes);
}

/* return the specified metric and returns the associated traction
* in the local frame defined by the transformation Q. n is the
* surface normal in the current configuration expressed in the
* global frame. Call only after configuring the meshfree field
* at the current point. Return value has sign convention that
* "more positive" is closer to failed */
double MeshFreeFractureSupportT::ComputeCriterion(StructuralMaterialT& material,
	const dMatrixT& Q, const dArrayT& n, FractureCriterionT criterion,
	double critical_value, dArrayT& t_local)
{
	double value = 0.0;
	if (criterion != kNoCriterion)
	{
		/* Cauchy stress */
		const dSymMatrixT& cauchy = material.s_ij();
		
		/* traction in global frame */
		cauchy.Multx(n, ftmp_nsd);
		
		/* convert to local frame */
		Q.MultTx(ftmp_nsd, t_local);
		
		/* resolve by criterion */
		if (criterion == kMaxHoopStress)
		{
			/* normal component is in last slot */
			value = t_local.Last();
			
			/* scale to match critical value */
			if (fabs(value) > kSmall)
				t_local *= critical_value/value;
		}
		else if (criterion == kMaxTraction)
		{
			value = t_local.Magnitude();
			
			/* scale to match critical value */
			if (fabs(value) > kSmall)
				t_local *= critical_value/value;
		}
		else if (criterion == kAcoustic)
		{
			/* reference normal */
			dArrayT N;
			Q.ColumnAlias(Q.Cols()-1, N);
		
			/* acoustical tensor */
			const dSymMatrixT& acoustical = material.AcousticalTensor(N);
			acoustical.PrincipalValues(ftmp_nsd);
		
			/* return (negative of) lowest speed */
			value = -ftmp_nsd.Min();
		}
		else
		{
			cout << "\n MeshFreeFractureSupportT::ComputeCriterion: unknown criterion: "
			     << criterion << endl;
			throw eGeneralFail;
		}
	}
	return value;
}

//TEMP
void MeshFreeFractureSupportT::SetStreamPrefs(ostream& out)
{
	out.precision(kPrecision);
	out.setf(ios::showpoint);
	out.setf(ios::right, ios::adjustfield);
	out.setf(ios::scientific, ios::floatfield); 	
}