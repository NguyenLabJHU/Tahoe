/* $Id: TiedNodesT.cpp,v 1.12.2.1 2002-10-17 04:42:00 paklein Exp $ */
#include "TiedNodesT.h"
#include "AutoArrayT.h"
#include "NodeManagerT.h"
#include "ElementBaseT.h"
#include "BasicFieldT.h"
#include "FEManagerT.h"

//TEMP
#include "ofstreamT.h"

using namespace Tahoe;

/* constructor */
TiedNodesT::TiedNodesT(NodeManagerT& node_manager, BasicFieldT& field):
	KBC_ControllerT(node_manager),
	fField(field),
	fDummySchedule(1.0),
	fFEManager(node_manager.FEManager())
{

}

/* initialize data. Must be called immediately after construction */
void TiedNodesT::Initialize(ifstreamT& in)
{
	/* inherited */
	KBC_ControllerT::Initialize(in);
	
	/* read "leader" nodes */
	iArrayT leader_nodes;
	ReadNodes(in, fLeaderIds, leader_nodes);

	/* read "follower" nodes */
	iArrayT follower_nodes;
	ReadNodes(in, fFollowerIds, follower_nodes);

	/* echo "follower" has one "leader" */
	fNodePairs.Dimension(follower_nodes.Length(), 2);
	fNodePairs = -1;
	fNodePairs.SetColumn(0, follower_nodes);
	fPairStatus.Dimension(follower_nodes);
	fPairStatus = kFree;
	fPairStatus_last = fPairStatus;

	/* read more parameters */
	ReadParameters(in);
	
	/* initialize tied pairs */
	InitTiedNodePairs(leader_nodes, follower_nodes);
	
	/* check */
	int free_count = fPairStatus.Count(kFree);
	if (free_count != 0) {
		cout << "\n TiedNodesT::Initialize: " << free_count
		     << " follower nodes without leaders" << endl;
//		throw ExceptionT::kGeneralFail;
//NOTE: for MP calculations, followers nodes that are external
//      may not have their (external) leader nodes reproduced
//      on this processor. Therefore, leader-less followers are
//      not necessarily an error, though they should remain kFree
//      and rely on other processors to enforce the tied constraint.
//      Also, pairs that involve _only_ external nodes should be
//      removed since any operations involving them on this
//      processor are redundant.
	}
	
	/* generate BC cards */
	SetBCCards();
}

/* inform controller of external nodes */
void TiedNodesT::SetExternalNodes(const iArrayT& ex_nodes) const
{
	if (ex_nodes.Length() > 0)
		cout << "\n TiedNodesT::SetExternalNodes: not implemented" << endl;

	//pair interactions that involve external nodes that are followers
	//can be removed from the list
}

void TiedNodesT::WriteParameters(ostream& out) const
{
	/* inherited */
	KBC_ControllerT::WriteParameters(out);

	out << "\n T i e d   n o d e   p a r a m e t e r s :\n\n";
	out << " Number of leader node set ids . . . . . . . . . = " << fLeaderIds.Length() << '\n';
	for (int i = 0; i < fLeaderIds.Length(); i++)
		out << '\t' << fLeaderIds[i] << '\n';
	out << " Number of followers node set ids. . . . . . . . = " << fFollowerIds.Length() << '\n';
	for (int i = 0; i < fFollowerIds.Length(); i++)
		out << '\t' << fFollowerIds[i] << '\n';
	out << " Follower-leader pairs:\n";
	iArray2DT tmp;
	tmp.Alias(fNodePairs);
	tmp++;
	fNodePairs.WriteNumbered(out);
	tmp--;	
	out << endl;
}

/* set to initial conditions. Reset all conditions to tied. */
void TiedNodesT::InitialCondition(void)
{
	/* inherited */
	KBC_ControllerT::InitialCondition();
	
	/* tie all */
	fPairStatus = kTied;
}

void TiedNodesT::ReadRestart(istream& in)
{
	/* inherited */
	KBC_ControllerT::ReadRestart(in);
	
	/* read pair status */
	int num_pairs = -1;
	in >> num_pairs;
	if (num_pairs != fPairStatus.Length()) {
		cout << "\n TiedNodesT::ReadRestart: number of values read from the\n" 
		     <<   "     restart file " << num_pairs
		     << " does not match the number of pairs "
		     << fPairStatus.Length() << endl;
		throw ExceptionT::kGeneralFail;
	}
	in >> fPairStatus;
	
	/* reset history */
	fPairStatus_last = fPairStatus;
}

void TiedNodesT::WriteRestart(ostream& out) const
{
	/* inherited */
	KBC_ControllerT::WriteRestart(out);

	/* write pair status */
	out << fPairStatus.Length() << '\n'
		<< fPairStatus.wrap_tight(10) << endl;
}

/* computing residual force */
void TiedNodesT::FormRHS(void) 
{
	/* copy data from leaders to followers */
	CopyKinematics();
}

/* initialize the current step */
void TiedNodesT::InitStep(void)
{
	/* inherited */
	KBC_ControllerT::InitStep();
	
	/* save history */
	fPairStatus_last = fPairStatus;
}

/* signal that the solution has been found */
void TiedNodesT::CloseStep(void)
{
	/* inherited */
	KBC_ControllerT::CloseStep();

	/* copy data from leaders to followers */
	CopyKinematics();

	/* update history */
	fPairStatus_last = fPairStatus;
}

/* solution for the current step failed. */
void TiedNodesT::Reset(void)
{
	/* inherited */
	KBC_ControllerT::Reset();

	/* reset status */
	fPairStatus = fPairStatus_last;
}

/* see if pair status has changed */
GlobalT::RelaxCodeT TiedNodesT::RelaxSystem(void)
{
	/* inherited */
	GlobalT::RelaxCodeT code = KBC_ControllerT::RelaxSystem();

	/* copy data from leaders to followers */
	CopyKinematics();
	
	/* check pair status */
	if (ChangeStatus())
	{
		/* reset BC cards */
		SetBCCards();
		
		return GlobalT::MaxPrecedence(code, GlobalT::kReEQ);
	}
	else return code;
}

/* append connectivities generated by the controller */
void TiedNodesT::Connectivities(AutoArrayT<const iArray2DT*>& connects) const
{
	/* inherited */
	KBC_ControllerT::Connectivities(connects);
	
	if (!connects.AppendUnique(&fNodePairs)) {
		cout << "\n TiedNodesT::Connects: pair connects not added" << endl;
		throw ExceptionT::kGeneralFail;	
	}
}

/* append equation number sets generated by the controller */
void TiedNodesT::Equations(AutoArrayT<const iArray2DT*>& equations) const
{
	/* inherited */
	KBC_ControllerT::Equations(equations);

	/* copy in the equation numbers - note that the number of active equations
	 * is not changed. */
	iArray2DT& eqnos = fField.Equations();
	for (int i = 0; i < fPairStatus.Length(); i++)
		if (fPairStatus[i] == kTied)
		{
			/* destination and source */
			int follower = fNodePairs(i,0);
			int leader   = fNodePairs(i,1);

			/* copy */
			eqnos.CopyRowFromRow(follower, leader);
		}
}

/* append equation number sets generated by the controller */
void TiedNodesT::SetEquations(void) 
{
	/* inherited */
	KBC_ControllerT::SetEquations();

	/* copy in the equation numbers - note that the number of active equations
	 * is not changed. */
	iArray2DT& eqnos = fField.Equations();
	for (int i = 0; i < fPairStatus.Length(); i++)
		if (fPairStatus[i] == kTied)
		{
			/* destination and source */
			int follower = fNodePairs(i,0);
			int leader   = fNodePairs(i,1);

			/* copy */
			eqnos.CopyRowFromRow(follower, leader);
		}
}

/* output current configuration */
void TiedNodesT::WriteOutput(ostream& out) const
{
	/* inherited */
	KBC_ControllerT::WriteOutput(out);
	
	/* not quite so const */
	TiedNodesT* non_const_this = const_cast<TiedNodesT*>(this);
	non_const_this->CopyKinematics();

       	out <<"\n T i e d  N o d e s  O u t p u t  D a t a : \n\n";
	out  <<"   " << fPairStatus.Count(kTied) << " tied nodes\n";
	out <<" Index   Pair Status ( 0 = Free, 1 = Tied, 4 = External Pair) \n";
	for (int i = 0; i < fNodePairs.MajorDim(); i++) 
       	    out <<i+1<<" "<< (fPairStatus[i] == kFree ? 0 : 1 )<<"\n";

}

/**********************************************************************
 * Protected
 **********************************************************************/

/* set initial tied node pairs */
void TiedNodesT::InitTiedNodePairs(const iArrayT& leader_nodes, 
	iArrayT& follower_nodes)
{
	/* coordinates */
	const dArray2DT& coords = fNodeManager.InitialCoordinates();
	
	/* get processor number */
	int np = fNodeManager.Rank();
	const iArrayT& pMap = fNodeManager.ProcessorMap();

	/* dumb search */
	int nsd = coords.MinorDim();

	/* Length may change during search if external nodes are removed */
	int FLength = follower_nodes.Length()-1;
	int LLength = leader_nodes.Length()-1;

	/*num of followers and leaders to be removed */	
	int Fct = 0, Lct = 0; 

	for (int i = 0; i <=FLength; i++)
	{
		double* x_f = coords(follower_nodes[i]);
		
		/*If a follower is external, flag it for removal from the list*/
		if (pMap[follower_nodes[i]] != np)
		{
		  fPairStatus[i] = kChangeF;
		}
		
		for (int j = 0; j < leader_nodes.Length(); j++)
		{
			double* x_l = coords(leader_nodes[j]);
			bool OK = true;
			for (int k = 0; OK && k < nsd; k++)
				OK = fabs(x_f[k] - x_l[k]) < kSmall;
				
			/* found */
			if (OK) 
			{
				fNodePairs(i,1) = leader_nodes[j];
				if (pMap[follower_nodes[i]] != np && 
								    pMap[leader_nodes[j]] != np)
	  			{
	  				/* Flag the pair as external */
				  fPairStatus[i] = kTiedExt;
	  			}
				else
				  fPairStatus[i] = kTied;
			}
		}
			
		if (fPairStatus[i]>kTied)
	  	{
	  		/* If something will get removed, send it to the
	  		 * end of the list and resize after the search
	  		 */
	    	follower_nodes[i] = follower_nodes[FLength];
	    	Fct++;
	    	fPairStatus[i] = kFree;
	    	fNodePairs(i,0) = fNodePairs(FLength,0);
		fNodePairs(i,1) = fNodePairs(FLength,1);
	    	if (i != follower_nodes.Length()-1)
	      		i--;
	    	FLength--;
	  	}
	}
	
	if (Fct > 0)
	{
		fPairStatus.Resize(fPairStatus.Length()-Fct);
		follower_nodes.Resize(follower_nodes.Length()-Fct);
		fNodePairs.Resize(fNodePairs.MajorDim()-Fct);
	}
}

/* check status of pairs */
bool TiedNodesT::ChangeStatus(void)
{
  /* To pass the benchmarks, the line below must be uncommented */
    	return false;

  	bool changeQ = false;
	ElementBaseT* surroundingGroup = fFEManager.ElementGroup(0);
  	if (!surroundingGroup)
        {
      //      	cout <<" Group 0 doesn't exist \n";
      	  throw ExceptionT::kGeneralFail;
        }
  	surroundingGroup->SendOutput(3);
  	dArray2DT fNodalQs = fNodeManager.OutputAverage();

  	for (int i = 0; i < fNodePairs.MajorDim();i++) 
        {  
	    dArrayT sigma(fNodalQs.MinorDim(),fNodalQs(fNodePairs(i,1)));
		if (fPairStatus[i] == kTied && TiedPotentialT::InitiationQ(sigma.Pointer()))     
		{ 
	  		fPairStatus[i] = kFree;
	  		changeQ = true;
		}
        }

  	return changeQ;

}

/**********************************************************************
 * Private
 **********************************************************************/

/* initialize the current step */
void TiedNodesT::SetBCCards(void)
{
	/* number of tied nodes */
	int n_tied = fPairStatus.Count(kTied);
	int ndof = fField.NumDOF();
	fKBC_Cards.Dimension(n_tied*ndof);
	//	cout<<"TiedNodesT::SetBCCards "<<n_tied<<" tied nodes\n"; 

	/* generate BC cards */
	if (n_tied > 0)
	{
		KBC_CardT* pcard = fKBC_Cards.Pointer();
		for (int i = 0; i < fNodePairs.MajorDim(); i++)
		{
		    if (fPairStatus[i] == kTied)
		    {
		    //			  cout<<"Pair "<<i<<" is tied "<<fNodePairs(i,0)<<" "<<fNodePairs(i,1)<<"\n";
				for (int j = 0; j < ndof; j++)
				{
					/* set values */
				  pcard->SetValues(fNodePairs(i,0), j, KBC_CardT::kDsp, 0,0.);
	
					/* dummy schedule */
				  pcard->SetSchedule(&fDummySchedule);
				  pcard++;
				} 
		    }
		}
	}
}

/* copy kinematic information from the leader nodes to the follower nodes */
void TiedNodesT::CopyKinematics(void)
{
	for (int i = 0; i < fPairStatus.Length(); i++)
		if (fPairStatus[i] == kTied)
		{
			/* destination and source */
			int follower = fNodePairs(i,0);
			int leader   = fNodePairs(i,1);

			/* kinematics */
			for (int j = 0; j <= fField.Order(); j++)
			{
				dArray2DT& u = fField[j];

				/* copy data from the leader */				
				u.CopyRowFromRow(follower, leader);
			}
		}
}