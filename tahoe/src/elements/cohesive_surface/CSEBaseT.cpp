/* $Id: CSEBaseT.cpp,v 1.11 2002-08-22 20:45:08 paklein Exp $ */
/* created: paklein (11/19/1997) */

#include "CSEBaseT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>

#include "fstreamT.h"
#include "Constants.h"
#include "SurfaceShapeT.h"
#include "iAutoArrayT.h"
#include "OutputSetT.h"
#include "ElementSupportT.h"
#include "ModelManagerT.h"

/* initialize static data */

using namespace Tahoe;

const int CSEBaseT::NumNodalOutputCodes = 5;
const int CSEBaseT::NumElementOutputCodes = 3;

/* constructor */
CSEBaseT::CSEBaseT(const ElementSupportT& support, const FieldT& field):
	ElementBaseT(support, field),
	fLocInitCoords1(LocalArrayT::kInitCoords),
	fLocCurrCoords(LocalArrayT::kCurrCoords),
	fFractureArea(0.0),
	fShapes(NULL)
{
	/* read control parameters */
	ifstreamT& in = ElementSupport().Input();

	in >> fGeometryCode;
	in >> fNumIntPts;
	in >> fCloseSurfaces;
	in >> fOutputArea;

	/* checks */
	if (NumSD() == 2 && fGeometryCode != GeometryT::kLine)
	{
		cout << "\n CSEBaseT::CSEBaseT: expecting geometry code "
		     << GeometryT::kLine<< " for 2D: " << fGeometryCode << endl;
		throw eBadInputValue;
	}
	else if (NumSD() == 3 &&
	         fGeometryCode != GeometryT::kQuadrilateral &&
	         fGeometryCode != GeometryT::kTriangle)
	{
		cout << "\n CSEBaseT::CSEBaseT: expecting geometry code " << GeometryT::kQuadrilateral
		     << " or\n" <<   "     " << GeometryT::kTriangle << " for 3D: "
		     << fGeometryCode << endl;
		throw eBadInputValue;
	}
	
	if (fCloseSurfaces != 0 &&
	    fCloseSurfaces != 1) throw eBadInputValue;
	if (fOutputArea != 0 &&
	    fOutputArea != 1) throw eBadInputValue;
}

/* destructor */
CSEBaseT::~CSEBaseT(void)
{
	delete fShapes;
	fShapes = NULL;
}

/* allocates space and reads connectivity data */
void CSEBaseT::Initialize(void)
{
	/* inherited */
	ElementBaseT::Initialize();

	/* dimensions */
	int num_facet_nodes = NumElementNodes()/2;

	/* initialize local arrays */
	fLocInitCoords1.Allocate(num_facet_nodes, NumSD());
	fLocCurrCoords.Allocate(NumElementNodes(), NumSD());
	ElementSupport().RegisterCoordinates(fLocInitCoords1);
	ElementSupport().RegisterCoordinates(fLocCurrCoords);

	/* construct surface shape functions */
	fShapes = new SurfaceShapeT(fGeometryCode, fNumIntPts, NumElementNodes(), NumDOF(),
		fLocInitCoords1);
	if (!fShapes) throw eOutOfMemory;
	fShapes->Initialize();

	/* work space */
	fNodes1.Allocate(num_facet_nodes);
	int nee = NumElementNodes()*NumDOF();
	fNEEvec.Allocate(nee);
	fNEEmat.Allocate(nee);

	/* echo output codes (one at a time to allow comments) */
	fNodalOutputCodes.Allocate(NumNodalOutputCodes);
	ifstreamT& in = ElementSupport().Input();
	ostream&   out = ElementSupport().Output();
	for (int i = 0; i < fNodalOutputCodes.Length(); i++)
	{
		in >> fNodalOutputCodes[i];

		/* convert all to "at print increment" */
		if (fNodalOutputCodes[i] != IOBaseT::kAtNever)
			fNodalOutputCodes[i] = IOBaseT::kAtInc;		
	}

	/* echo */
	out << " Number of nodal output codes. . . . . . . . . . = " << fNodalOutputCodes.Length() << '\n';
	out << "    [" << fNodalOutputCodes[NodalCoord   ] << "]: initial nodal coordinates\n";
	out << "    [" << fNodalOutputCodes[NodalDisp    ] << "]: nodal displacements\n";
	out << "    [" << fNodalOutputCodes[NodalDispJump] << "]: nodal gap (magnitude)\n";
	out << "    [" << fNodalOutputCodes[NodalTraction] << "]: nodal traction magnitude\n";
	out << "    [" << fNodalOutputCodes[MaterialData ] << "]: constitutive output data\n";

	/* check */
	if (fNodalOutputCodes.Min() < IOBaseT::kAtFinal ||
	    fNodalOutputCodes.Max() > IOBaseT::kAtInc) throw eBadInputValue;

	fElementOutputCodes.Allocate(NumElementOutputCodes);
	fElementOutputCodes = IOBaseT::kAtNever;

//TEMP - backward compatibility
	if (StringT::versioncmp(ElementSupport().Version(), "v3.01") < 1)
	{
		/* message */
		cout << "\n CSEBaseT::Initialize: use input file version newer than v3.01\n" 
		     <<   "     to enable element output control" << endl;
		out << "\n CSEBaseT::Initialize: use input file version newer than v3.01\n" 
		    <<   "     to enable element output control" << endl;
	}
	else
	{
//TEMP - BACK
		int num_codes = fElementOutputCodes.Length();
		if (StringT::versioncmp(ElementSupport().Version(), "v3.02") < 1)
		{
			cout << "\n CSEBaseT::Initialize: use input file version newer than v3.02\n"
		         <<   "     to enable output control of element averaged traction" << endl;
			out << "\n CSEBaseT::Initialize: use input file version newer than v3.02\n"
		         <<   "     to enable output control of element averaged traction" << endl;
			num_codes = 2;
		}
	
		/* read in at a time to allow comments */
		for (int j = 0; j < num_codes; j++)
		{
			in >> fElementOutputCodes[j];
		
			/* convert all to "at print increment" */
			if (fElementOutputCodes[j] != IOBaseT::kAtNever)
				fElementOutputCodes[j] = IOBaseT::kAtInc;
		}	

		/* checks */
		if (fElementOutputCodes.Min() < IOBaseT::kAtFail ||
		    fElementOutputCodes.Max() > IOBaseT::kAtInc) throw eBadInputValue;
	}

	/* echo */
	out << " Number of element output codes. . . . . . . . . = " << fElementOutputCodes.Length() << '\n';
	out << "    [" << fElementOutputCodes[Centroid      ] << "]: centroid\n";
	out << "    [" << fElementOutputCodes[CohesiveEnergy] << "]: dissipated cohesive energy\n";
	out << "    [" << fElementOutputCodes[Traction      ] << "]: average traction\n";
	
	/* close surfaces */
	if (fCloseSurfaces) CloseSurfaces();

	/* output stream */
	if (fOutputArea == 1)
	{
		/* generate file name */
		StringT name = (ElementSupport().Input()).filename();
		name.Root();
		name.Append(".grp", ElementSupport().ElementGroupNumber(this) + 1);
		name.Append(".fracture");
		
		/* open stream */
		farea_out.open(name);
	}
}

/* initial condition/restart functions (per time sequence) */
void CSEBaseT::InitialCondition(void)
{
	/* inherited */
	ElementBaseT::InitialCondition();

	/* initialize element status flags */
	int nel = NumElements();
	for (int i = 0; i < nel; i++)
		fElementCards[i].Flag() = kON;
}

/* finalize time increment */
void CSEBaseT::CloseStep(void)
{
	/* inherited */
	ElementBaseT::CloseStep();

	/* deactivate marked elements */
	int nel = NumElements();
	for (int i = 0; i < nel; i++)
	{
		int& flag = fElementCards[i].Flag();
		flag = (flag == kMarked) ? kOFF : flag;
	}
}

/* resets to the last converged solution */
void CSEBaseT::ResetStep(void)
{
	/* inherited */
	ElementBaseT::ResetStep();

	/* unset marks */
	int nel = NumElements();
	for (int i = 0; i < nel; i++)
	{
		int& flag = fElementCards[i].Flag();
		flag = (flag == kMarked) ? kON : flag;
	}
}

/* solution calls */
void CSEBaseT::AddNodalForce(const FieldT& field, int node, dArrayT& force)
{
//TEMP
#pragma unused(field)
#pragma unused(node)
#pragma unused(force)
	//not implemented
}

/* returns the energy as defined by the derived class types */
double CSEBaseT::InternalEnergy(void) { return 0.0; } //not implemented

/* writing output */
void CSEBaseT::RegisterOutput(void)
{
//NOTE: could loop over each output mode and register
//      it with the output separately. for now just register
//      "kAtInc"

	/* "deformed" geometry */
	GeometryT::CodeT geo_code;
	switch (fGeometryCode)
	{
		case GeometryT::kLine:		
			geo_code = GeometryT::kQuadrilateral;
			break;

		case GeometryT::kQuadrilateral:
			geo_code = GeometryT::kHexahedron;
			break;

		case GeometryT::kTriangle:
			geo_code = GeometryT::kPentahedron;
			break;

		default:	

		cout << "\n CSEBaseT::RegisterOutput: could not translate\n";
		cout << "     geometry code " << fGeometryCode
			 << " to a pseudo-geometry code for the volume." << endl;
		throw eGeneralFail;	
	}	

	/* nodal output */
	iArrayT n_counts;
	SetNodalOutputCodes(IOBaseT::kAtInc, fNodalOutputCodes, n_counts);

	/* element output */
	iArrayT e_counts;
	SetElementOutputCodes(IOBaseT::kAtInc, fElementOutputCodes, e_counts);
	ArrayT<StringT> block_ID(fBlockData.Length());
	for (int i = 0; i < block_ID.Length(); i++)
		block_ID[i] = fBlockData[i].ID();

	/* collect variable labels */
	ArrayT<StringT> n_labels(n_counts.Sum());
	ArrayT<StringT> e_labels(e_counts.Sum());
	GenerateOutputLabels(n_counts, n_labels, e_counts, e_labels);

	/* set output specifier */
	StringT set_ID;
	set_ID.Append(ElementSupport().ElementGroupNumber(this) + 1);
	OutputSetT output_set(set_ID, geo_code, block_ID, fOutput_Connectivities, 
		n_labels, e_labels, false);
		
	/* register and get output ID */
	fOutputID = ElementSupport().RegisterOutput(output_set);
}

//NOTE - this function is identical to ContinuumElementT::WriteOutput
void CSEBaseT::WriteOutput(IOBaseT::OutputModeT mode)
{
//TEMP - not handling general output modes yet
	if (mode != IOBaseT::kAtInc)
	{
		cout << "\n CSEBaseT::WriteOutput: only handling \"at increment\"\n"
		     <<   "     print mode. SKIPPING." << endl;
		return;
	}

	/* fracture area */
	if (fOutputArea)
	{
		farea_out << setw(kDoubleWidth) << ElementSupport().Time();
		farea_out << setw(kDoubleWidth) << fFractureArea << endl;
	}

	/* map output flags to count of values */
	iArrayT n_counts;
	SetNodalOutputCodes(mode, fNodalOutputCodes, n_counts);
	iArrayT e_counts;
	SetElementOutputCodes(mode, fElementOutputCodes, e_counts);

	/* calculate output values */
	dArray2DT n_values;
	dArray2DT e_values;
	ComputeOutput(n_counts, n_values, e_counts, e_values);

	/* send to output */
	ElementSupport().WriteOutput(fOutputID, n_values, e_values);
}

/* compute specified output parameter and send for smoothing */
void CSEBaseT::SendOutput(int kincode)
{
	/* output flags */
	iArrayT flags(fNodalOutputCodes.Length());

	/* set flags to get desired output */
	flags = IOBaseT::kAtNever;
	switch (kincode)
	{
		case NodalDisp:
		    flags[NodalDisp] = NumDOF();
			break;
		case NodalDispJump:
		    flags[NodalDispJump] = 1;
			break;
		case NodalTraction:
		    flags[NodalTraction] = 1;
			break;
		default:
			cout << "\n CSEBaseT::SendKinematic: invalid output code: ";
			cout << kincode << endl;
	}

	/* number of output values */
	iArrayT n_counts;
	SetNodalOutputCodes(IOBaseT::kAtInc, flags, n_counts);

	/* reset averaging workspace */
	ElementSupport().ResetAverage(n_counts.Sum());

	/* set flags for no element output */
	iArrayT e_counts(fElementOutputCodes.Length());
	e_counts = 0;

	/* generate nodal values */
	dArray2DT e_values, n_values;
	ComputeOutput(n_counts, n_values, e_counts, e_values);
}

/***********************************************************************
* Protected
***********************************************************************/

/* print element group data */
void CSEBaseT::PrintControlData(ostream& out) const
{
	/* inherited */
	ElementBaseT::PrintControlData(out);

	/* control parameters */
	out << " Element geometry code . . . . . . . . . . . . . = " << fGeometryCode << '\n';
	out << "    eq." << GeometryT::kLine          << ", line\n";
	out << "    eq." << GeometryT::kQuadrilateral << ", quadrilateral\n";
	out << "    eq." << GeometryT::kTriangle	  << ", triangle\n";
	out << " Number of integration points. . . . . . . . . . = " << fNumIntPts     << '\n';
	out << " Initial surface closure flag. . . . . . . . . . = " << fCloseSurfaces << '\n';
	out << " Output fracture surface area. . . . . . . . . . = " << fOutputArea    << '\n';
}

/* read element connectivity data */
void CSEBaseT::ReadConnectivity(ifstreamT& in, ostream& out)
{
	/* inherited */
	ElementBaseT::ReadConnectivity(in, out);

	/* write output over the original connectivities */
	fOutput_Connectivities = fConnectivities;

//TEMP - do not translate for MP calculations because neither
//       their is no way to join the output either at runtime
//       or as a post-analysis step
if (ElementSupport().Size() > 1) {

	int nsd = NumSD();
	int nen = NumElementNodes();
	if ((nsd == 2 && nen != 4 && nen != 6) || 
	    (nsd == 3 && nen != 8 && nen != 16)) {
		cout << "\n CSEBaseT::ReadConnectivity: TEMP: detected higher order elements,\n"
			 <<   "     but output not reduced to lower order MP calculations." << endl;    
	}
}
	/* check for higher order elements */
	int nsd = NumSD();
	int nen = NumElementNodes();
	if ((nsd == 2 && nen != 4 && nen != 6) || 
	    (nsd == 3 && nen != 8 && nen != 16))
	{
		/* message */
		ostream& out = ElementSupport().Output();
		cout << "\n CSEBaseT::ReadConnectivity: detected higher order elements\n";
		out  << "\n CSEBaseT::ReadConnectivity: detected higher order elements\n";

		/* the geometry manager */
		ModelManagerT& model = ElementSupport().Model();

		/* nen: 8 -> 6 */
		int map_2D[] = {0, 1, 2, 3, 4, 6}; 

		/* nen: 20 -> 16 */
		int map_3D[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}; 

		/* node map */
		iArrayT map((nsd == 2) ? 6 : 16, (nsd == 2) ? map_2D : map_3D); 

		/* loop over connectivity blocks */
		for (int b = 0; b < fBlockData.Length(); b++)
		{
			/* send new connectivities to model manager */
			ElementBlockDataT& block_data = fBlockData[b];
			const StringT& id = block_data.ID();
			StringT new_id = id;
			new_id.Append(b+1, 3);
			
			/* see if new_id is already present */
			int new_dex = model.ElementGroupIndex(new_id);
			if (new_dex == ModelManagerT::kNotFound)
			{
				/* message */
		     	cout << "     translating element block ID " << id << endl;
		     	out  << "     translating element block ID " << id << endl;

				/* translate */
				const iArray2DT& source = *(fOutput_Connectivities[b]);
				iArray2DT dest(source.MajorDim(), map.Length());
				for (int i = 0; i < dest.MajorDim(); i++)
				{
					int* a = dest(i);
					int* b = source(i);
					for (int j = 0; j < map.Length(); j++)
						*a++ = b[map[j]];	
				}

				/* send new connectivities to model manager */
				ElementBlockDataT& block_data = fBlockData[b];
				const StringT& id = block_data.ID();
				StringT new_id = id;
				new_id.Append(b+1, 3);
				if (!model.RegisterElementGroup (new_id, dest, GeometryT::kNone, true)) {
					cout << "\n CSEBaseT::ReadConnectivity: could not register element block ID: " << new_id << endl;
					throw eGeneralFail;
				}
			}

			/* message */
			cout << "     block ID " << id << " replaced by ID " << new_id << endl;
			out  << "     block ID " << id << " replaced by ID " << new_id << endl;

			/* set pointer to connectivity list */
			fConnectivities[b] = model.ElementGroupPointer(new_id);
			
			/* reset block data */
			int start = block_data.StartNumber();
			int dim = block_data.Dimension();
			int material = block_data.MaterialID();
			block_data.Set(new_id, start, dim, material);
		}
	}
}

void CSEBaseT::SetNodalOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
		iArrayT& counts) const
{
	/* initialize */
	counts.Allocate(flags.Length());
	counts = 0;
	
	if (flags[NodalCoord] == mode)
		counts[NodalCoord] = NumSD();
	if (flags[NodalDisp] == mode)
		counts[NodalDisp] = NumDOF();
	if (flags[NodalDispJump] == mode)
		counts[NodalDispJump] = 1;
	if (flags[NodalTraction] == mode)
		counts[NodalTraction] = 1;
}

void CSEBaseT::SetElementOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
	iArrayT& counts) const
{
	/* initialize */
	counts.Allocate(flags.Length());
	counts = 0;

	if (flags[Centroid] == mode)
		counts[Centroid] = NumSD();
	if (flags[CohesiveEnergy] == mode)
		counts[CohesiveEnergy] = 1;
	if (flags[Traction] == mode)
		counts[Traction] = 1;
}

/* construct output labels array */
void CSEBaseT::GenerateOutputLabels(const iArrayT& n_codes, ArrayT<StringT>& n_labels,
	const iArrayT& e_codes, ArrayT<StringT>& e_labels) const
{
	/* allocate nodal output labels */
	n_labels.Allocate(n_codes.Sum());

	int count = 0;
	if (n_codes[NodalDisp])
	{
		/* labels from the field */
		const ArrayT<StringT>& labels = Field().Labels();
		for (int i = 0; i < labels.Length(); i++)
			n_labels[count++] = labels[i];
	}

	if (n_codes[NodalCoord])
	{
		const char* xlabels[] = {"x1", "x2", "x3"};
		for (int i = 0; i < NumSD(); i++)
			n_labels[count++] = xlabels[i];
	}

	if (n_codes[NodalDispJump]) n_labels[count++] = "jump";
	if (n_codes[NodalTraction]) n_labels[count++] = "Tmag";
	
	/* allocate nodal output labels */
	e_labels.Allocate(e_codes.Sum());
	count = 0;
	if (e_codes[Centroid])
	{
		const char* xlabels[] = {"xc_1", "xc_2", "xc_3"};
		for (int i = 0; i < NumSD(); i++)
			e_labels[count++] = xlabels[i];
	}
	if (e_codes[CohesiveEnergy]) e_labels[count++] = "phi";
	if (e_codes[Traction]) e_labels[count++] = "Tmag";
}

/* write all current element information to the stream */
void CSEBaseT::CurrElementInfo(ostream& out) const
{
	/* inherited */
	ElementBaseT::CurrElementInfo(out);
	dArray2DT temp;

	out <<   " initial coords:\n";
	temp.Allocate(fLocInitCoords1.NumberOfNodes(), fLocInitCoords1.MinorDim());
	fLocInitCoords1.ReturnTranspose(temp);
	temp.WriteNumbered(out);

	out <<   " current coords:\n";
	temp.Allocate(fLocCurrCoords.NumberOfNodes(), fLocCurrCoords.MinorDim());
	fLocCurrCoords.ReturnTranspose(temp);
	temp.WriteNumbered(out);
}

/***********************************************************************
* Private
***********************************************************************/

/* close surfaces to zero gap */
void CSEBaseT::CloseSurfaces(void) const
{
	/* get coordinates */
	const dArray2DT& init_coords = ElementSupport().InitialCoordinates();

	/* local nodes numbers on each facet */
	const iArray2DT& facetnodes = fShapes->NodesOnFacets();
		
	/* collapse elements */
	int nel = NumElements();
	for (int i = 0; i < nel; i++)
	{			
		int* pfacet1 = facetnodes(0);
		int* pfacet2 = facetnodes(1);
	        const iArrayT& elemnodes = fElementCards[i].NodesX();
		int* nodes = elemnodes.Pointer();

		for (int j = 0; j < facetnodes.MinorDim(); j++)
		{
			/* facet coordinates */		
			double* px1 = init_coords(nodes[*pfacet1++]);
			double* px2 = init_coords(nodes[*pfacet2++]);
				
			for (int k = 0; k < NumSD(); k++)
			{
				double x_mid = 0.5*(*px1 + *px2);
				*px1++ = x_mid;
				*px2++ = x_mid;
			}
		}		
	}
}

/* return the default number of element nodes */
int CSEBaseT::DefaultNumElemNodes(void) const
{
	/* return number of element nodes given the facet geometry */
	switch (fGeometryCode)
	{
		case GeometryT::kLine:
			return 4;
		case GeometryT::kQuadrilateral:
			return 8;
		case GeometryT::kTriangle:
			return 6;
		default:
			cout << "\n CSEBaseT::DefaultNumElemNodes: unknown geometry code: "
			     << fGeometryCode << endl;
			return 0;
	}
}
//NOTE: needed because ExodusII does not store ANY information about
//      empty element groups, which causes trouble for parallel execution
//      when a partition contains no element from a group.
