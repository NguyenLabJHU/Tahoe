/* $Id: iGridManagerT.cpp,v 1.3 2002-06-29 22:07:07 hspark Exp $ */
/* created: paklein (09/13/1998)                                          */
/* iNodeT grid with unified interface for 2D/3D                           */

#include "iGridManagerT.h"
#include "iGridManager1DT.h"
#include "iGridManager2DT.h"
#include "iGridManager3DT.h"
#include "iArrayT.h"

/* constructor */
iGridManagerT::iGridManagerT(const iArrayT& n_grid, const dArray2DT& coords,
	const iArrayT* nodes_used):
        fGrid1D(NULL),
	fGrid2D(NULL),
	fGrid3D(NULL)
{
	if (n_grid.Length() != coords.MinorDim()) throw eSizeMismatch;

	if (n_grid.Length() == 1)
	{
		fGrid1D = new iGridManager1DT(n_grid[0], coords, nodes_used);
		if (!fGrid1D) throw eOutOfMemory;	
	}
	else if (n_grid.Length() == 2)
	{
		fGrid2D = new iGridManager2DT(n_grid[0], n_grid[1], coords, nodes_used);
		if (!fGrid2D) throw eOutOfMemory;	
	}
	else if (n_grid.Length() == 3)
	{
		fGrid3D = new iGridManager3DT(n_grid[0], n_grid[1], n_grid[2], coords, nodes_used);
		if (!fGrid3D) throw eOutOfMemory;
	}
	else
		throw eOutOfRange;
}

	
/* destructor */
iGridManagerT::~iGridManagerT(void)
{
        delete fGrid1D;
	delete fGrid2D;
	delete fGrid3D;
}
	
/* reconfigure grid with stored coordinate data */
void iGridManagerT::Reset(void)
{
        if (fGrid1D)
	        fGrid1D->Reset();
	else if (fGrid2D)
		fGrid2D->Reset();
	else
		fGrid3D->Reset();
}

/* neighbors - returns neighbors coords(n) (SELF not included) */
void iGridManagerT::Neighbors(int n, double tol, AutoArrayT<int>& neighbors)
{
        if (fGrid1D)
	        fGrid1D->Neighbors(n, tol, neighbors);
	else if (fGrid2D)
		fGrid2D->Neighbors(n, tol, neighbors);
	else
		fGrid3D->Neighbors(n, tol, neighbors);
}

/* neighbors - returns neighbors coords(n) (SELF not included) */
void iGridManagerT::Neighbors(int n, const ArrayT<double>& tol_xyz, AutoArrayT<int>& neighbors)
{
        if (fGrid1D)
	        fGrid1D->Neighbors(n, tol_xyz, neighbors);
	else if (fGrid2D)
		fGrid2D->Neighbors(n, tol_xyz, neighbors);
	else
		fGrid3D->Neighbors(n, tol_xyz, neighbors);
}

/* return list of data falling within the defined region */
const AutoArrayT<iNodeT>& iGridManagerT::HitsInRegion(double* coords,
	double distance)
{
        if (fGrid1D)
		return fGrid1D->HitsInRegion(coords, distance);
	else if (fGrid2D)
		return fGrid2D->HitsInRegion(coords, distance);
	else
		return fGrid3D->HitsInRegion(coords, distance);
}

const AutoArrayT<iNodeT>& iGridManagerT::HitsInRegion(double* coords,
	int cell_span)
{
        if (fGrid1D)
		return fGrid1D->HitsInRegion(coords, cell_span);
	else if (fGrid2D)
		return fGrid2D->HitsInRegion(coords, cell_span);
	else
		return fGrid3D->HitsInRegion(coords, cell_span);
}

const AutoArrayT<iNodeT>& iGridManagerT::HitsInRegion(double* coords,
	const ArrayT<double>& tol_xyz)
{
        if (fGrid1D)
		return fGrid1D->HitsInRegion(coords, tol_xyz);
	else if (fGrid2D)
		return fGrid2D->HitsInRegion(coords, tol_xyz);
	else
		return fGrid3D->HitsInRegion(coords, tol_xyz);
}

/* the distance covered by the given cell span */
double iGridManagerT::CellSpan(int cell_span) const
{
        if (fGrid1D)
		return fGrid1D->CellSpan(cell_span);
	else if (fGrid2D)
		return fGrid2D->CellSpan(cell_span);
	else
		return fGrid3D->CellSpan(cell_span);
}

/* grid statistics */
void iGridManagerT::WriteStatistics(ostream& out) const
{
        if (fGrid1D)
		fGrid1D->WriteStatistics(out);
	else if (fGrid2D)
		fGrid2D->WriteStatistics(out);
	else
		fGrid3D->WriteStatistics(out);
}