/* $Id: RootedLevelT.cpp,v 1.6 2002-10-20 22:39:01 paklein Exp $ */
/* created: paklein (08/05/1996) */

#include "RootedLevelT.h"
#include "GraphT.h"
#include "iArrayT.h"

/* constructor */

using namespace Tahoe;

RootedLevelT::RootedLevelT(void):
	fNumNodes(0),
	fLevels(NULL),
	fNumLevels(0),
	fMaxWidth(0),
	fWidths(NULL),
	fRoot(0),
	fNodesOnLevel(NULL)
{

}

/* destructor */
RootedLevelT::~RootedLevelT(void)
{
	delete[] fLevels;
	delete[] fWidths;
	delete[] fNodesOnLevel;
}

/* generate the level rooted structure from the given graph
* rooted at the given node */
void RootedLevelT::MakeRootedLevel(const GraphT& graph, int rootnode)
{
//TEMP - only allocate if size has changed
	fNumNodes = graph.NumNodes();
	if (rootnode < 0 || rootnode >= fNumNodes)
	{
		cout << "\nRootedLevelT::MakeRootedLevel: incorrect root node number:";
		cout << rootnode << '\n' << endl;
		throw ExceptionT::kGeneralFail;
	}

	/* free previous */
	if (fLevels != NULL) delete[] fLevels;

	fLevels = new int[fNumNodes];
	if (!fLevels) throw ExceptionT::kOutOfMemory;

	/* initialize levels */
	int* p = fLevels;
	for (int i = 0; i < fNumNodes; i++)
		*p++ = -1;

	/* determine levels */
	fLevels[rootnode] = 0;
	fNumLevels = 1;
	
	int nodecount = 1;
	while (nodecount < fNumNodes)
	{
		int newonlevel = 0;
		for (int i = 0; i < fNumNodes; i++)
			if (fLevels[i] == fNumLevels - 1)
			{
				int  degree = graph.Degree(i);
				int* edges  = graph.Edges(i);
				
				for (int j = 0; j < degree; j++)
				{	
					int edgenode = *edges;

#if __option (extended_errorcheck)
					if (edgenode < 0 || edgenode >= fNumNodes) throw ExceptionT::kGeneralFail;
#endif
					/* assign if free */
					if (fLevels[edgenode] == -1)
					{
						fLevels[edgenode] = fNumLevels;
						nodecount++;
						newonlevel++;
					}
					edges++;
				}
			}
			
		/* graph is not continuously connected */	
		if (newonlevel == 0)
		{
			cout << "\n RootedLevelT::MakeRootedLevel: structure is disconnected" << endl;

			/* write node numbers along the cut */
			iArrayT levels(fNumNodes, fLevels);
			int num_cut = levels.Count(fNumLevels-1);
			iArrayT cut_nodes(num_cut);
			num_cut = 0;
			for (int i = 0; i < fNumNodes; i++)
				if (fLevels[i] == fNumLevels-1)
					cut_nodes[num_cut++] = i;
			cout << " graph nodes along cut:\n" << cut_nodes.wrap(10) << endl;
			throw ExceptionT::kGeneralFail;
		}
		else
			/* next */
			fNumLevels++;
	}
	
	/* check no unassigned */
	p = fLevels;
	for (int j = 0; j < fNumNodes; j++)
		if (*p++ == -1)
		{
			cout << "\n RootedLevelT::MakeRootedLevel: node " << j << " has no level" << endl;
			throw ExceptionT::kGeneralFail;
		}
	
	/* finalize */
	ComputeWidths();
	fRoot = rootnode;
}

/* generate the level rooted structure from the given graph
* rooted at the given node */
void RootedLevelT::MakePartialRootedLevel(const GraphT& graph, int rootnode,
	bool clear)
{
//TEMP - only allocate if size has changed
	fNumNodes = graph.NumNodes();
	if (rootnode < 0 || rootnode >= fNumNodes)
	{
		cout << "\nRootedLevelT::MakeRootedLevel: incorrect root node number:";
		cout << rootnode << '\n' << endl;
		throw ExceptionT::kGeneralFail;
	}

	/* free previous */
	delete[] fLevels;
	fLevels = new int[fNumNodes];
	if (!fLevels) throw ExceptionT::kOutOfMemory;

	/* restart */
	if (clear)
	{
		/* initialize levels */
		int* p = fLevels;
		for (int i = 0; i < fNumNodes; i++)
			*p++ = -1;
			
		/* restart */
		fNumLevels = 1;
	}

	/* start */
	fLevels[rootnode] = fNumLevels - 1;
	
	/* set levels */
	int  nodecount = 1;
	int newonlevel = 1;
	while (newonlevel > 0)
	{
		newonlevel = 0;	
		for (int i = 0; i < fNumNodes; i++)
			if (fLevels[i] == fNumLevels - 1)
			{
				int degree = graph.Degree(i);
				int* edges = graph.Edges(i);
				
				for (int j = 0; j < degree; j++)
				{	
					int edgenode = *edges;

#if __option (extended_errorcheck)
					if (edgenode < 0 || edgenode >= fNumNodes) throw ExceptionT::kGeneralFail;
#endif
				
					/* assign if free */
					if (fLevels[edgenode] == -1)
					{
						fLevels[edgenode] = fNumLevels;
						nodecount++;
						newonlevel++;
					}
					edges++;
				}
			}
			
		/* next */
		if (newonlevel > 0) fNumLevels++;
	}
	
	/* finalize */
	ComputeWidths();
	fRoot = rootnode;
}
	
/* return the node numbers on the specified level */
void RootedLevelT::NodesOnLevel(iArrayT& nodes, int level) const
{
	/* no mistakes */
	if (level > fNumLevels) throw ExceptionT::kGeneralFail;
	
	int count = 0;	
	for (int i = 0; i < fNumNodes; i++)
		if (fLevels[i] == level)
		{
#if __option (extended_errorcheck)
			if (count >= fMaxWidth) throw ExceptionT::kGeneralFail;
#endif
			fNodesOnLevel[count++] = i;
		}
			
	if (count != Width(level))
		throw ExceptionT::kGeneralFail;
	
	/* return value */
	nodes.Set(count, fNodesOnLevel);
}	

/* return list of nodes used */
void RootedLevelT::NodesUsed(iArrayT& nodes_used) const
{
	/* count */
	int count = 0;
	int* plevel = fLevels;
	for (int i = 0; i < fNumNodes; i++)
		if (*plevel++ > -1)
			count++;
	
	/* allocate */
	nodes_used.Dimension(count);
	
	/* collect */
	count = 0;
	plevel = fLevels;
	for (int j = 0; j < fNumNodes; j++)
		if (*plevel++ > -1)
			nodes_used[count++] = j;
}

/* initialize the priorities as specified in Sloan.  Assumes DIRECT
* INDEXING in priorities */
void RootedLevelT::InitializePriorities(iArrayT& priorities, int W2) const
{
	/* check */
	if (fNumNodes != priorities.Length()) throw ExceptionT::kGeneralFail;

	for (int i = 0; i < fNumNodes; i++)
		priorities[i] += fLevels[i]*W2; //OFFSET
}

/* copy in the labelled branches of the rooted level structure */
void RootedLevelT::CopyIn(const RootedLevelT& source)
{
	/* initialize */
	if (fNumNodes == 0)
	{
		fNumNodes = source.fNumNodes;
		
		/* free previous */
		delete[] fLevels;
		fLevels = new int[fNumNodes];
		if (!fLevels) throw ExceptionT::kOutOfMemory;

		/* initialize levels */
		int* p = fLevels;
		for (int i = 0; i < fNumNodes; i++)
			*p++ = -1;		
	}
	else if (fNumNodes != source.fNumNodes)
		throw ExceptionT::kSizeMismatch;
	
	/* store root node */
	fRoot = source.fRoot;
	
	/* copy in */
	int* pthis = fLevels;
	int*  psrc = source.fLevels;
	for (int i = 0; i < fNumNodes; i++)
	{
		if (*psrc > -1)
		{
			/* already has level! */
			if (*pthis > -1)
			{
				cout << "\n RootedLevelT::CopyIn: structures overlap" << endl;
				throw ExceptionT::kGeneralFail;
			}
			else
				*pthis = *psrc + fNumLevels;
		}
		
		pthis++; psrc++;	
	}
	
	/* update number of levels */
	fNumLevels += source.Depth();
	
	/* (re-) compute widths */
	ComputeWidths();
}

/************************************************************************
* Private
************************************************************************/

/* compute the number of nodes in each level */
void RootedLevelT::ComputeWidths(void)
{
	/* free existing space */
	if (fWidths != NULL) delete[] fWidths;
	
	/* allocate new space */
	fWidths = new int[fNumLevels];
	if (!fWidths) throw ExceptionT::kOutOfMemory;

	/* clear space */
	int* p = fWidths;
	for (int i = 0; i < fNumLevels; i++)
		*p++ = 0;

	/* count nodes on each level */
	for (int j = 0; j < fNumNodes; j++)
	{
		/* node not in level structure */
		int level = fLevels[j];
		if (level > -1)
		{
#if __option (extended_errorcheck)
			if (level < 0 || level >= fNumLevels) throw ExceptionT::kGeneralFail;
#endif		
			fWidths[level]++;
		}
	}
		
	/* compute the maximum width */
	int old_max = fMaxWidth;
	fMaxWidth = 0;
	p = fWidths;
	for (int k = 0; k < fNumLevels; k++)
	{
		fMaxWidth = ( (*p > fMaxWidth) ? *p : fMaxWidth );
		p++;
	}
	
	/* allocate space for return values */
	if (fMaxWidth != old_max)
	{
		delete[] fNodesOnLevel;
		fNodesOnLevel = new int[fMaxWidth];
		if (!fNodesOnLevel) throw ExceptionT::kOutOfMemory;
	}
}