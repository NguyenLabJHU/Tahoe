/* $Id: OutputSetT.cpp,v 1.19 2003-11-04 01:21:12 paklein Exp $ */
/* created: paklein (03/07/2000) */
#include "OutputSetT.h"
#include "iArrayT.h"
#include "iArray2DT.h"

using namespace Tahoe;

namespace Tahoe {
/* array behavior */
DEFINE_TEMPLATE_STATIC const bool ArrayT<OutputSetT*>::fByteCopy = true;
}

/* constructor */
OutputSetT::OutputSetT(GeometryT::CodeT geometry_code,
	const ArrayT<StringT>& block_ID, 
	const ArrayT<const iArray2DT*>& connectivities, 
	const ArrayT<StringT>& n_labels, 
	const ArrayT<StringT>& e_labels, bool changing):
	fMode(kElementBlock),
	fPrintStep(-1),
	fID("1"), /* dummy ID */
	fChanging(changing),
	fGeometry(geometry_code),
	fBlockID(block_ID),
	fConnectivities(connectivities.Length()),
	fBlockNodesUsed(fConnectivities.Length()),
	fBlockIndexToSetIndexMap(fConnectivities.Length())
{
	if (fConnectivities.Length() != fBlockID.Length()) 
	  {
	    cout << "\n\nOutputSetT::OutputSetT size mismatch: \n";
	    cout << " fConnectivities.Length = " << fConnectivities.Length();
	    cout << "\n    fBlockID.Length = " << fBlockID.Length() << endl;
	    throw ExceptionT::kSizeMismatch;
	  }

	for (int i=0; i < fConnectivities.Length(); i++)
		fConnectivities[i] = connectivities[i];

	fNodeOutputLabels.Dimension(n_labels.Length());
	for (int i = 0; i < fNodeOutputLabels.Length(); i++)
	  {
		fNodeOutputLabels[i] = n_labels[i];
		fNodeOutputLabels[i].Replace (' ', '_');
	  }

	fElementOutputLabels.Dimension(e_labels.Length());
	for (int j = 0; j < fElementOutputLabels.Length(); j++)
	  {
		fElementOutputLabels[j] = e_labels[j];
		fElementOutputLabels[j].Replace (' ', '_');
	  }

	/* set the nodes used array */
	fChanging = true; // force calculation of nodes used
	NodesUsed();
	for (int i = 0; i < fConnectivities.Length(); i++)
		BlockNodesUsed(fBlockID[i]);
	fChanging = changing; // reset
}

OutputSetT::OutputSetT(GeometryT::CodeT geometry_code,
	const iArray2DT& connectivities, const ArrayT<StringT>& n_labels, bool changing):
	fMode(kFreeSet),
	fPrintStep(-1),
	fID("1"), /* dummy ID */
	fChanging(false),
	fGeometry(geometry_code),
	fConnectivities(1),
	fBlockNodesUsed(1),
	fBlockID(1),
	fBlockIndexToSetIndexMap(1)
{
	/* keep reference to connectivities */
	fConnectivities[0] = &connectivities;
	fBlockID[0] = fID; /* must give connectivities a reasonable ID for compatibility
	                    * with the output classes */

	/* copy node labels */
	fNodeOutputLabels.Dimension(n_labels.Length());
	for (int i = 0; i < fNodeOutputLabels.Length(); i++)
	  {
		fNodeOutputLabels[i] = n_labels[i];
		fNodeOutputLabels[i].Replace (' ', '_');
	  }

	/* set the nodes used array */
	fChanging = true; // force calculation of nodes used
	NodesUsed();
	fBlockNodesUsed[0].Alias(fNodesUsed);
	fChanging = changing;
}

OutputSetT::OutputSetT(const OutputSetT& source):
	fPrintStep(-1),
	fMode(source.fMode),
	fID(source.fID),
	fChanging(source.fChanging),
	fGeometry(source.fGeometry),
	fBlockID(source.fBlockID),
	fNodesUsed(source.fNodesUsed),
	fConnectivities(source.NumBlocks()),
	fBlockNodesUsed(fConnectivities.Length()),
	fBlockIndexToSetIndexMap(source.fBlockIndexToSetIndexMap)
{
	for (int i=0; i < fConnectivities.Length(); i++)
	        fConnectivities[i] = source.fConnectivities[i];

	fNodeOutputLabels.Dimension(source.fNodeOutputLabels.Length());
	for (int i = 0; i < fNodeOutputLabels.Length(); i++)
	  {
		fNodeOutputLabels[i] = source.fNodeOutputLabels[i];
		fNodeOutputLabels[i].Replace (' ', '_');
	  }

	fElementOutputLabels.Dimension(source.fElementOutputLabels.Length());
	for (int j = 0; j < fElementOutputLabels.Length(); j++)
	  {
		fElementOutputLabels[j] = source.fElementOutputLabels[j];
		fElementOutputLabels[j].Replace (' ', '_');
	  }

	if (fMode == kElementBlock &&
	    fConnectivities.Length() != fBlockID.Length()) throw ExceptionT::kSizeMismatch;
	
	/* set nodes used by blocks */
	if (fConnectivities.Length() == 1)
		fBlockNodesUsed[0].Alias(fNodesUsed);	
	else
		fBlockNodesUsed = source.fBlockNodesUsed;
}

/* dimensions */
int OutputSetT::NumBlockElements(const StringT& ID) const
{
	return fConnectivities[BlockIndex(ID)]->MajorDim();
}

int OutputSetT::NumElements(void) const 
{
  int num = 0;
  for (int i=0; i < fConnectivities.Length(); i++)
    num += fConnectivities[i]->MajorDim();
  return num; 
}

void OutputSetT::SetID(const StringT& id)
{
	fID = id;
	/* must give connectivities a reasonable ID for compatibility with the 
	 * output classes */
	if (Mode() == kFreeSet) {
		fBlockID[0] = fID;
	}
}

const iArray2DT* OutputSetT::Connectivities(const StringT& ID) const
{
	return fConnectivities[BlockIndex(ID)];
}

const iArrayT& OutputSetT::BlockNodesUsed(const StringT& ID)
{
	int index = BlockIndex(ID);
	if (fChanging) /* need to reset data */
	{
		/* just one set */
		if (fBlockNodesUsed.Length() == 1)
		{
			fBlockNodesUsed[index].Alias(fNodesUsed);
			fBlockIndexToSetIndexMap[0].Dimension(fNodesUsed.Length());
			fBlockIndexToSetIndexMap[0].SetValueToPosition();		
		}
		else /* more than one block */
		{
			/* determine nodes used by block */
			SetNodesUsed(*fConnectivities[index], fBlockNodesUsed[index]);
			
			/* block could be empty */
			if (fBlockNodesUsed[index].Length() > 0)
			{
				/* block to set index map */
				iArrayT& map = fBlockIndexToSetIndexMap[index];
				iArrayT& used = fBlockNodesUsed[index];
				map.Dimension(used.Length());
		
				/* range of nodes numbers */
				int min, max;
				fNodesUsed.MinMax(min, max);
				int range = max - min + 1;
			
				/* nodes used sequence */
				iArrayT sequence(range);
				sequence = -1;
			
				/* mark sequence */
				int dex = 0;
				for (int i = 0; i < fNodesUsed.Length(); i++)
					sequence[fNodesUsed[i] - min] = dex++;
					
				/* collect index list */
				for (int i = 0; i < map.Length(); i++)
				{
					int dex = sequence[used[i] - min];
					if (dex < 0) {
						cout << "\n OutputSetT::BlockNodesUsed: ERROR: block node used " << used[i]+1 
						     << " is not marked as used by the set" << endl;
						throw ExceptionT::kGeneralFail;
					}
					else
						map[i] = dex;
				}
			}
		}
	}
	
	return fBlockNodesUsed[index];
}

/* determine the nodes used */
void OutputSetT::SetNodesUsed(const iArray2DT& connects, iArrayT& nodes_used)
{
	nodes_used.Union(connects);
}

/* determine the nodes used */
void OutputSetT::SetNodesUsed(const ArrayT<const iArray2DT*>& connects_list, 
	iArrayT& nodes_used)
{
	/* compiler won't cast array type */
	ArrayT<const nArrayT<int>*> tmp(connects_list.Length(), 
		(const nArrayT<int>**) connects_list.Pointer());

	nodes_used.Union(tmp);
}

/*************************************************************************
* Private
*************************************************************************/

/* returns the index for the element block for the given */
int OutputSetT::BlockIndex(const StringT& ID) const
{
	if (fMode == kFreeSet)
		return 0;
	else
	{
		int index = -1;
		for (int i = 0; index == -1 && i < fBlockID.Length(); i++)
			if (fBlockID[i] == ID)
				index = i;

		if (index == -1) {
			cout << "\n OutputSetT::BlockIndex: block ID not found: " << ID << endl;
			throw ExceptionT::kGeneralFail;
		}
		return index;
	}
}
