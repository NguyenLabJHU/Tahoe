/* $Id: BridgingScaleT.cpp,v 1.31 2003-03-31 23:16:47 paklein Exp $ */
#include "BridgingScaleT.h"

#include <iostream.h>
#include <iomanip.h>

#include "ShapeFunctionT.h"
#include "fstreamT.h"
#include "iAutoArrayT.h"
#include "OutputSetT.h"
#include "AutoFill2DT.h"
#include "RaggedArray2DT.h"
#include "iGridManagerT.h"
#include "iNodeT.h"
#include "nArrayGroupT.h"
#include "PointInCellDataT.h"

using namespace Tahoe;

/* constructor */
BridgingScaleT::BridgingScaleT(const ElementSupportT& support, 
	const FieldT& field,
	const SolidElementT& solid):
	ElementBaseT(support, field),
	fSolid(solid),
	fElMatU(ShapeFunction().ParentDomain().NumNodes(), ElementMatrixT::kSymmetric),
	fLocInitCoords(LocalArrayT::kInitCoords),
	fLocDisp(LocalArrayT::kDisp),
	fDOFvec(NumDOF()),
	fGlobalMass(support.Output(),1),
	fConnect(solid.NumElements(), solid.NumElementNodes()),
	fWtempU(ShapeFunction().ParentDomain().NumNodes(), NumDOF())
{

}

/* map coordinates into elements */
void BridgingScaleT::MaptoCells(const iArrayT& points_used, const dArray2DT* init_coords, 
	const dArray2DT* curr_coords, PointInCellDataT& cell_data)
{
	const char caller[] = "BridgingScaleT::MaptoCells";

	/* map data */
	cell_data.SetContinuumElement(fSolid);
	RaggedArray2DT<int>& point_in_cell = cell_data.PointInCell();
	RaggedArray2DT<double>& point_in_cell_coords = cell_data.PointInCellCoords();

	/* point coordinates */
	if (curr_coords && init_coords) ExceptionT::GeneralFail(caller, "cannot pass both init and curr coords");
	if (!curr_coords && !init_coords) ExceptionT::GeneralFail(caller, "must define init or curr coords");
	const dArray2DT& point_coordinates = (init_coords != NULL) ? *init_coords : *curr_coords;

	/* cell coordinates */
	const dArray2DT& cell_coordinates = (init_coords != NULL) ?
		ElementSupport().InitialCoordinates() :
		ElementSupport().CurrentCoordinates();
	LocalArrayT::TypeT coord_type = (init_coords != NULL) ? 
		LocalArrayT::kInitCoords : 
		LocalArrayT::kCurrCoords;

	/* stream */
	ostream& out = ElementSupport().Output();

	/* configure search grid */
	iGridManagerT grid(10, 100, point_coordinates, &points_used);
	grid.Reset();

	/* check all cells for points */
	const ParentDomainT& parent = ShapeFunction().ParentDomain();
	AutoFill2DT<int> auto_fill(fSolid.NumElements(), 1, 10, 10);
	dArrayT x_atom, centroid;
	LocalArrayT loc_cell_coords(coord_type, fSolid.NumElementNodes(), NumSD());
	loc_cell_coords.SetGlobal(cell_coordinates);
	for (int i = 0; i < fSolid.NumElements(); i++) {
	
		/* gives domain (global) nodal coordinates */
		loc_cell_coords.SetLocal(fSolid.ElementCard(i).NodesX());

		/* centroid and radius */
		double radius = parent.AverageRadius(loc_cell_coords, centroid);

		/* candidate points */
		const AutoArrayT<iNodeT>& hits = grid.HitsInRegion(centroid.Pointer(), 1.01*radius);

		/* check if points are within the element domain */
		for (int j = 0; j < hits.Length(); j++)
		{
			x_atom.Set(NumSD(), hits[j].Coords());
			if (parent.PointInDomain(loc_cell_coords, x_atom)) 
				auto_fill.Append(i, hits[j].Tag());
		}
	}

	/* copy/compress contents */
	point_in_cell.Copy(auto_fill);
	auto_fill.Free();
	
	/* verbose output */
	if (ElementSupport().PrintInput()) {
		iArrayT tmp(point_in_cell.Length(), point_in_cell.Pointer());
		out << "\n Particles in cells:\n"
		    << setw(kIntWidth) << "no." << '\n';
		tmp++;
		point_in_cell.WriteNumbered(out);
		tmp--;
		out.flush();
	}

	if (point_in_cell.Length() > points_used.Length()) {
		cout << '\n' << caller << ": WARNING: number of particles in cells " 
		     << point_in_cell.Length() << " exceeds\n" 
		     <<   "     the total number of particles " << points_used.Length() << endl;
	}

	/* map points in every cell to parent domain coordinates */
	dArrayT mapped(NumSD()), point;
	AutoFill2DT<double> inverse(point_in_cell.MajorDim(), 1, 10, 10);
	for (int i = 0; i < point_in_cell.MajorDim(); i++) 
	{
		/* cell coordinates */
		loc_cell_coords.SetLocal(fSolid.ElementCard(i).NodesX()); 

		/* run through list and map to parent domain */
		int* particles = point_in_cell(i);
		for (int j = 0; j < point_in_cell.MinorDim(i); j++) 
		{
			point_coordinates.RowAlias(particles[j], point);
			if (parent.MapToParentDomain(loc_cell_coords, point, mapped))
				inverse.Append(i, mapped);
			else
				ExceptionT::GeneralFail(caller, "mapping to parent domain failed");
		}
	}

	/* copy compress coordinate list */
	point_in_cell_coords.Copy(inverse);
	inverse.Free();
	
	/* verbose output */
	if (ElementSupport().PrintInput()) {

		int nsd = NumSD();
		out << "\n Mapped coordinates of particles in elements:\n";
		
		/* run though element and dump coordinates of particles in parent domain */
		dArray2DT inv_coords;
		for (int i = 0; i < point_in_cell.MajorDim(); i++) {
		
			int np = point_in_cell.MinorDim(i);
			out << "element = " << i+1 << '\n'
			    << "  count	= " << np << '\n';

			/* shallow copy of inverse coords */
			inv_coords.Set(np, nsd, point_in_cell_coords(i));

			/* write coordinates */
			int* p_list = point_in_cell(i);
			for (int j = 0; j < np; j++)
			{
				out << setw(kIntWidth) << p_list[j]+1 << ": ";
				inv_coords.PrintRow(j, out);
			}
		}
	}
}

/* initialize interpolation data */
void BridgingScaleT::InitInterpolation(const iArrayT& points_used, 
	PointInCellDataT& cell_data) const
{
	/* dimension return value */
	dArray2DT& weights = cell_data.InterpolationWeights();
	weights.Dimension(points_used.Length(), fSolid.NumElementNodes());
	iArrayT& cell = cell_data.InterpolatingCell();
	cell.Dimension(points_used.Length());
	cell = -1;
	
	/* global to local map */
	InverseMapT& global_to_local = cell_data.GlobalToLocal();
	global_to_local.SetMap(points_used);

	/* cell shape functions */
	int nsd = NumSD();
	const ParentDomainT& parent = ShapeFunction().ParentDomain();

	/* point in cells data */
	const RaggedArray2DT<int>& point_in_cell = cell_data.PointInCell();
	const RaggedArray2DT<double>& point_in_cell_coords = cell_data.PointInCellCoords();

	/* run through cells */
	dArrayT Na;
	dArrayT point_coords;
	dArray2DT mapped_coords;
	for (int i = 0; i < point_in_cell.MajorDim(); i++)
	{
		int np = point_in_cell.MinorDim(i);
		if (np > 0)
		{
			int* points = point_in_cell(i);
		
			/* mapped coordinates of points in this cell */
			mapped_coords.Set(np, nsd, point_in_cell_coords(i));
			
			/* run through points */
			for (int j = 0; j < np; j++)
			{
				int dex = global_to_local.Map(points[j]);
				if (cell[dex] == -1) /* not set yet */
				{
					cell[dex] = i;
					
					/* fetch point data */
					mapped_coords.RowAlias(j, point_coords);
					weights.RowAlias(dex, Na);
					
					/* evaluate shape functions */
					parent.EvaluateShapeFunctions(point_coords, Na);
				}
			}
		}
	}
}

void BridgingScaleT::InterpolateField(const StringT& field, const PointInCellDataT& cell_data,
	dArray2DT& point_values) const
{
	int nen = fSolid.NumElementNodes();

	/* get the field */
	const FieldT* the_field = ElementSupport().Field(field);
	LocalArrayT loc_field(LocalArrayT::kDisp, nen, the_field->NumDOF());
	loc_field.SetGlobal((*the_field)[0]); /* take zeroth order field */
	
	/* interpolation data */
	const dArray2DT& weights = cell_data.InterpolationWeights();
	const iArrayT& cell = cell_data.InterpolatingCell();
	
	/* dimension return value */
	point_values.Dimension(weights.MajorDim(), the_field->NumDOF());

	/* loop over points */
	for (int i = 0; i < point_values.MajorDim(); i++)
	{
		/* element nodes */
		int element = cell[i];
		const iArrayT& nodes = fSolid.ElementCard(element).NodesU();

		/* collect local values */
		loc_field.SetLocal(nodes);
		
		/* interpolate */
		for (int j = 0; j < point_values.MinorDim(); j++)
			point_values(i,j) = weights.DotRow(i, loc_field(j));
	}
}

/* compute the projection matrix */
void BridgingScaleT::InitProjection(const PointInCellDataT& cell_data)
{
	/* projected part of the mesh */
	const iArrayT& cell_nodes = cell_data.CellNodes();
	const iArray2DT& cell_connects = cell_data.CellConnectivities();

	/* cell connectivities are (matrix equations) - 1 */
	iArrayT tmp_shift;
	tmp_shift.Alias(cell_connects);
	tmp_shift++;

	/* configure the matrix */
	int num_projected_nodes = cell_nodes.Length();
	fGlobalMass.AddEquationSet(cell_connects);
	fGlobalMass.Initialize(num_projected_nodes, num_projected_nodes, 1);
	fGlobalMass.Clear();

	/* points in cell data */
	const RaggedArray2DT<int>& point_in_cell = cell_data.PointInCell();
	const dArray2DT& weights = cell_data.InterpolationWeights();
	const InverseMapT& global_to_local = cell_data.GlobalToLocal();

	/* loop over mesh */
	int cell_dex = 0;
	iArrayT cell_eq;
	dArrayT Na;
	for (int i = 0; i < point_in_cell.MajorDim(); i++)
	{
		int np = point_in_cell.MinorDim(i);
		if (np > 0)
		{
			fElMatU = 0.0;
			int* points = point_in_cell(i);
			for (int j = 0; j < np; j++)
			{
				/* fetch interpolation weights */
				int point_dex = global_to_local.Map(points[j]);
				weights.RowAlias(point_dex, Na);
			
				/* element mass */
				fElMatU.Outer(Na, Na, 1.0, dMatrixT::kAccumulate);
			}

			/* equations of cell in projector */
			cell_connects.RowAlias(cell_dex++, cell_eq);

			/* assemble into matrix */
			fGlobalMass.Assemble(fElMatU, cell_eq);
		}
	}
	
	/* shift back */
	tmp_shift--;
	
//TEMP - write mass matrix to file
#if 0
ostream& out = ElementSupport().Output();
out << "\n weights =\n" << weights << endl;
out << "\n mass matrix =\n" << fGlobalMass << endl;
#endif
//TEMP
}

/* project the point values onto the mesh */
void BridgingScaleT::ProjectField(const StringT& field, const PointInCellDataT& cell_data,
	const dArray2DT& values, dArray2DT& projection)
{
#pragma unused(field)

	/* projected part of the mesh */
	const iArrayT& cell_nodes = cell_data.CellNodes();
	const iArray2DT& cell_connects = cell_data.CellConnectivities();

	/* points in cell data */
	const RaggedArray2DT<int>& point_in_cell = cell_data.PointInCell();
	const dArray2DT& weights = cell_data.InterpolationWeights();
	const InverseMapT& global_to_local = cell_data.GlobalToLocal();

	/* initialize return value */
	projection.Dimension(cell_nodes.Length(), values.MinorDim());
	projection = 0.0;

	/* loop over mesh */
	int cell_dex = 0;
	iArrayT cell_eq;
	dArrayT Na, point_value;
	dMatrixT Nd(cell_connects.MinorDim(), values.MinorDim());
	for (int i = 0; i < point_in_cell.MajorDim(); i++)
	{
		int np = point_in_cell.MinorDim(i);
		if (np > 0)
		{
			int* points = point_in_cell(i);
			Nd = 0.0;
			for (int j = 0; j < np; j++)
			{
				int point = points[j];
			
				/* fetch interpolation weights */
				int point_dex = global_to_local.Map(point);
				weights.RowAlias(point_dex, Na);

				/* source values of the point */
				values.RowAlias(point, point_value);
			
				/* rhs during projection */
				Nd.Outer(Na, point_value, 1.0, dMatrixT::kAccumulate);
			}

			/* equations of cell in projector */
			cell_connects.RowAlias(cell_dex++, cell_eq);

			/* assemble */
			for (int j = 0; j < Nd.Cols(); j++)
				projection.Accumulate(j, cell_eq, Nd(j));
		}
	}

//TEMP - write mass matrix to file
#if 0
ostream& out = ElementSupport().Output();
out << "\n values =\n" << values << endl;
out << "\n residual =\n" << projection << endl;
#endif
//TEMP

	/* calculate projection - requires global matrix that supports 
	 * multiple sovles */
	dArrayT u_tmp(projection.MajorDim());
	for (int i = 0; i < projection.MinorDim(); i++)
	{
		projection.ColumnCopy(i, u_tmp);
		fGlobalMass.Solve(u_tmp);
		projection.SetColumn(i, u_tmp);
	}
	u_tmp.Free();

	/* initialize return values */
	fFineScale.Dimension(values);
	fFineScale = 0.0;
	fCoarseScale.Dimension(values);
	fCoarseScale = 0.0;

	cell_dex = 0;
	iArrayT cell_connect;
	dArray2DT cell_projection(cell_connects.MinorDim(), projection.MinorDim());
	for (int i = 0; i < point_in_cell.MajorDim(); i++)
	{
		int np = point_in_cell.MinorDim(i);
		if (np > 0)
		{
			/* gather cell information */
			cell_connects.RowAlias(cell_dex++, cell_connect);
			cell_projection.RowCollect(cell_connect, projection);
		
			int* points = point_in_cell(i);
			for (int j = 0; j < np; j++)
			{
				int point = points[j];
			
				/* fetch interpolation weights */
				int point_dex = global_to_local.Map(point);
				weights.RowAlias(point_dex, Na);

				/* interpolate to point */
				for (int k = 0; k < projection.MinorDim(); k++)
				{
					/* interpolate coarse scale to point */
					fCoarseScale(point, k) = cell_projection.DotColumn(k, Na);

					/* error = source - projection */
					fFineScale(point, k) = values(point, k) - fCoarseScale(point, k);
				}
			}
		}
	}
}

/* writing output */
void BridgingScaleT::RegisterOutput(void)
{
	/* collect variable labels */
	int ndof = NumDOF();
	if (ndof > 3) throw;
	ArrayT<StringT> n_labels(2*ndof);
	const char* coarse_labels[] = {"FE_X", "FE_Y", "FE_Z"};
	const char* fine_labels[] = {"fine_X", "fine_Y", "fine_Z"};
	int dex = 0;
	for (int i = 0; i < ndof; i++) n_labels[dex++] = coarse_labels[i];
	for (int i = 0; i < ndof; i++) n_labels[dex++] = fine_labels[i];

	/* register output at solid nodes */
#if 0
	OutputSetT output_set_solid(GeometryT::kPoint, fSolidNodesUsed, n_labels);
	fSolidOutputID = ElementSupport().RegisterOutput(output_set_solid);
#endif

#if 0
	/* register output at particles */
	OutputSetT output_set_particle(GeometryT::kPoint, fParticlesUsed, n_labels);
	fParticleOutputID = ElementSupport().RegisterOutput(output_set_particle);
#endif
}

//NOTE - this function is/was identical to CSEBaseT::WriteOutput
void BridgingScaleT::WriteOutput(void)
{
	/* calculate output values */
	dArray2DT n_values; // dimension: [number of output nodes] x [number of values]
//	ComputeOutput(n_counts, n_values);
//  rows in n_values correspond to the nodes listed in fSolidNodesUsed

	/* send to output */
//	ElementSupport().WriteOutput(fOutputID, n_values);
}

/***********************************************************************
* Protected
***********************************************************************/

/* initialize local arrays */
void BridgingScaleT::SetLocalArrays(void)
{
	/* dimension */
	fLocInitCoords.Dimension(NumElementNodes(), NumSD());
	fLocDisp.Dimension(NumElementNodes(), NumDOF());

	/* set source */
	ElementSupport().RegisterCoordinates(fLocInitCoords);
	Field().RegisterLocal(fLocDisp);	
}

/* print element group data */
void BridgingScaleT::PrintControlData(ostream& out) const
{
	/* inherited */
	ElementBaseT::PrintControlData(out);

//DEV
//	out << " Particle group number . . . . . . . . . . . . . = " << ElementSupport().ElementGroupNumber(&fParticle) + 1 << '\n';
	out << " Continuum group number. . . . . . . . . . . . . = " << ElementSupport().ElementGroupNumber(&fSolid) + 1 << '\n';
}

/* write all current element information to the stream */
void BridgingScaleT::CurrElementInfo(ostream& out) const
{
	/* inherited */
	ElementBaseT::CurrElementInfo(out);
	dArray2DT temp;
	temp.Dimension(fLocInitCoords.NumberOfNodes(), fLocInitCoords.MinorDim());
	
	out <<   " initial coords:\n";
	temp.Dimension(fLocInitCoords.NumberOfNodes(), fLocInitCoords.MinorDim());
	fLocInitCoords.ReturnTranspose(temp);
	temp.WriteNumbered(out);

	out <<   " displacements:\n";
	temp.Dimension(fLocDisp.NumberOfNodes(), fLocDisp.MinorDim());
	fLocDisp.ReturnTranspose(temp);
	temp.WriteNumbered(out);
}