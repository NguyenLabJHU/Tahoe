/* $Id: ContactElementT.cpp,v 1.40 2003-02-03 04:40:18 paklein Exp $ */
#include "ContactElementT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>

#include "ofstreamT.h"
#include "fstreamT.h"
#include "IOBaseT.h"
#include "iGridManager2DT.h"
#include "XDOF_ManagerT.h"
#include "ExodusT.h"
#include "ModelFileT.h"
#include "SurfaceT.h"
#include "ContactSearchT.h"
#include "ContactNodeT.h"

/* parameters */ // unfortunately these are also in the derived classes

using namespace Tahoe;

static const int kMaxNumFaceNodes = 4; // 4node quads
static const int kMaxNumFaceDOF   = 12; // 4node quads in 3D

/* constructor */
ContactElementT::ContactElementT
(const ElementSupportT& support, const FieldT& field, int num_enf_params):
    ElementBaseT(support, field),
    LHS(ElementMatrixT::kNonSymmetric),
    tmp_LHS(ElementMatrixT::kNonSymmetric),
    fContactSearch(NULL),
    fXDOF_Nodes(NULL)
{
    fNumEnfParameters = num_enf_params;
    fNumMultipliers = 0;
    ReadControlData();
}

ContactElementT::ContactElementT
(const ElementSupportT& support, const FieldT& field, int num_enf_params, XDOF_ManagerT* xdof_nodes):
    ElementBaseT(support, field),
    fXDOF_Nodes(xdof_nodes),
    LHS(ElementMatrixT::kNonSymmetric),
    tmp_LHS(ElementMatrixT::kNonSymmetric),
    fContactSearch(NULL)
{
    fNumEnfParameters = num_enf_params;
    if (!fXDOF_Nodes) throw ExceptionT::kGeneralFail;
    ReadControlData();
}


/* destructor */
ContactElementT::~ContactElementT(void) 
{ 
	delete fContactSearch;
}

/* form of tangent matrix */
GlobalT::SystemTypeT ContactElementT::TangentType(void) const
{
	return GlobalT::kNonSymmetric; 
}

/* initialization after construction */
void ContactElementT::Initialize(void)
{
	/* inherited, calls EchoConnectivityData */
	ElementBaseT::Initialize();

	/* initialize surfaces, connect nodes to coordinates */
	for (int i = 0; i < fSurfaces.Length(); i++) {
		fSurfaces[i].Initialize(ElementSupport(), fNumMultipliers);
	}

    for (int i = 0; i < fSurfaces.Length(); i++) {
    	ArrayT<ContactNodeT*>& nodes = fSurfaces[i].ContactNodes();
    	for(int n = 0; n < nodes.Length(); n++){
			nodes[n]->EnforcementStatus() = -10;
		}
    }

#if 0
        /* set console access */
        iAddVariable("penalty_parameter", fpenalty);
#endif

	/* create search object */
	fContactSearch = 
	  new ContactSearchT(fSurfaces, fSearchParameters);

	/* workspace matrices */
	SetWorkspace();

	/* for bandwidth reduction in the case of no contact 
	 * make node-to-node pseudo-connectivities to link all bodies */
	int num_surfaces = fSurfaces.Length();
	if (num_surfaces > 1)
	{
		fSurfaceLinks.Dimension(num_surfaces - 1, 2);
		for (int i = 0; i < num_surfaces - 1; i++)
		{
			fSurfaceLinks(i,0) = fSurfaces[i  ].GlobalNodes()[0];
			fSurfaceLinks(i,1) = fSurfaces[i+1].GlobalNodes()[0];
		}
	}

	if (fXDOF_Nodes) {
		iArrayT numDOF(fSurfaces.Length());// the number of tag-sets
		numDOF = fNumMultipliers;
		/* this calls GenerateElementData */
		/* register with node manager */
		ElementSupport().XDOF_Manager().XDOF_Register(this, numDOF);
	}
	else {
		/* set initial contact configuration */
		bool changed = SetContactConfiguration();	
	}


}

void ContactElementT::SetWorkspace(void)
{ 	/* workspace matrices */  // ARE THESE CORRECT?
	n1.Dimension(NumSD());
   	RHS_man.SetWard    (kMaxNumFaceDOF,RHS);
   	tmp_RHS_man.SetWard(kMaxNumFaceDOF,tmp_RHS);
   	LHS_man.SetWard    (kMaxNumFaceDOF,LHS);
   	tmp_LHS_man.SetWard(kMaxNumFaceDOF,tmp_LHS);
   	N1_man.SetWard     (kMaxNumFaceDOF,N1);
   	N2_man.SetWard     (kMaxNumFaceDOF,N2);
   	N1n_man.SetWard    (kMaxNumFaceDOF,N1n);
   	N2n_man.SetWard    (kMaxNumFaceDOF,N2n);
   	eqnums1_man.SetWard(kMaxNumFaceDOF,eqnums1,NumSD());
   	eqnums2_man.SetWard(kMaxNumFaceDOF,eqnums2,NumSD());
   	weights_man.SetWard(kMaxNumFaceNodes,weights);
	
	if (fXDOF_Nodes) {
	P1_man.SetWard     (kMaxNumFaceNodes*fNumMultipliers,P1);
	P2_man.SetWard     (kMaxNumFaceNodes*fNumMultipliers,P2);
	P1values_man.SetWard(kMaxNumFaceNodes*fNumMultipliers,P1values);
	P2values_man.SetWard(kMaxNumFaceNodes*fNumMultipliers,P2values);
	xRHS_man.SetWard   (kMaxNumFaceNodes*fNumMultipliers,xRHS);
   	xeqnums1_man.SetWard(kMaxNumFaceDOF,xeqnums1,fNumMultipliers);
   	xeqnums2_man.SetWard(kMaxNumFaceDOF,xeqnums2,fNumMultipliers);
   	xconn1_man.SetWard(kMaxNumFaceDOF,xconn1);
   	xconn2_man.SetWard(kMaxNumFaceDOF,xconn2);
	}
}

/* done once per time-step */
GlobalT::RelaxCodeT ContactElementT::RelaxSystem(void)
{
   if (fXDOF_Nodes) {
	/* override - handled by DOFElement::Reconfigure */
	return GlobalT::kNoRelax;
   }
   else {
        /* inherited */
        GlobalT::RelaxCodeT relax = ElementBaseT::RelaxSystem();

        /* generate contact element data */
        bool contact_changed = SetContactConfiguration();

        /* minimal test of new-ness */
        if (!contact_changed)
                return relax;
        else
                return GlobalT::MaxPrecedence(relax, GlobalT::kReEQ);
   }
}

/* returns 1 if group needs to reconfigure DOF's, else 0 */
int ContactElementT::Reconfigure(void)
{ // this overrides Relax
	return 1; // always reconfigure, since SetCont.Conf. is in Gen.El.Data
#if 0
	/* inherited */
        GlobalT::RelaxCodeT relax = ElementBaseT::RelaxSystem();

        /* generate contact element data */
        bool contact_changed = SetContactConfiguration();

        /* minimal test of new-ness */
        if (contact_changed)
                relax = GlobalT::MaxPrecedence(relax, GlobalT::kReEQ);

        if (relax != GlobalT::kNoRelax)
                return 1;
        else
                return 0;
#endif
}

/* this sequence supplants Relax */
/* (1) sizes the DOF tags array needed for the current timestep */
void ContactElementT::SetDOFTags(void)
{ 
	bool changed = fContactSearch->SetInteractions();

	/* Initialize */
	for (int i = 0; i < fSurfaces.Length(); i++) {
		/* form potential connectivity for step */
 		fSurfaces[i].SetPotentialConnectivity();

		fSurfaces[i].InitializeMultiplierMap();
	}

	/* Tag potentially active nodes */
	for (int i = 0; i < fSurfaces.Length(); i++) {
		fSurfaces[i].DetermineMultiplierExtent();
	}
	
	/* Number active nodes and total */
	/* Store last dof tag and value */
	/* Resize DOF tags array for number of potential contacts */
	for (int i = 0; i < fSurfaces.Length(); i++) {
	    fSurfaces[i].AllocateMultiplierTags();
	}
}

/* (2) this function allows the external manager to set the Tags */
iArrayT& ContactElementT::DOFTags(int tag_set)
{
        return fSurfaces[tag_set].MultiplierTags(); 
}

/* (3) generates connectivity based on current tags */
void ContactElementT::GenerateElementData(void)
{ 
	for (int i = 0; i < fSurfaces.Length(); i++) {
		/* hand off location of multipliers */
		const dArray2DT& multipliers = ElementSupport().XDOF_Manager().XDOF(this, i);
		fSurfaces[i].AliasMultipliers(multipliers);

		/* form potential connectivity for step */
 		fSurfaces[i].SetMultiplierConnectivity();
 	}
}

/* set DOF values to the last converged solution, this is called after SetDOF */
void ContactElementT::ResetDOF(dArray2DT& XDOF, int tag_set) const
{
	fSurfaces[tag_set].ResetMultipliers(XDOF);
}

/* return the displacement-ghost node pairs to avoid pivoting*/
const iArray2DT& ContactElementT::DOFConnects(int tag_set) const
{ 
	return fSurfaces[tag_set].DisplacementMultiplierNodePairs();
}

/* append element equations numbers to the list */
void ContactElementT::Equations(AutoArrayT<const iArray2DT*>& eq_1,
                AutoArrayT<const RaggedArray2DT<int>*>& eq_2)
{
#pragma unused(eq_1)
	
  /* send potential connectivity */
  for (int i = 0; i < fSurfaces.Length(); i++) {
	const RaggedArray2DT<int>& connectivities   
		= fSurfaces[i].Connectivities(); 
	RaggedArray2DT<int>& equation_numbers 
		= fSurfaces[i].EqNums();
        /* get local equations numbers for u nodes from NodeManager */
	/* Connectivities generated in SetConfiguration */
	if (!fXDOF_Nodes ) {
		Field().SetLocalEqnos(connectivities, equation_numbers);
	}
	else {
		ElementSupport().XDOF_Manager().XDOF_SetLocalEqnos(Group(), connectivities, equation_numbers);
	}

        /* add to list */
        eq_2.Append(&equation_numbers);
  }

}


/* appends group connectivities to the array for graph-based algorithms */
void ContactElementT::ConnectsU(AutoArrayT<const iArray2DT*>& connects_1,
	AutoArrayT<const RaggedArray2DT<int>*>& connects_2) const
{
	/* inherited */
	/* base class uses fConnectivities to create profile */
//ElementBaseT::ConnectsU(connects_1, connects_2);

	/* link surfaces with fictious node-to-node pairs*/
	/* only necessary for bodies out-of-contact */
	connects_1.AppendUnique(&fSurfaceLinks);
	
	/* add node-face interactions */
	for (int i = 0; i < fSurfaces.Length(); i++) {
	  const RaggedArray2DT<int>& connectivities   
		= fSurfaces[i].Connectivities(); 
	  connects_2.Append(&connectivities);
	}
}

/* returns no (NULL) geometry connectivies */
void ContactElementT::ConnectsX(AutoArrayT<const iArray2DT*>& connects) const
{
#pragma unused (connects)
	connects.Append(NULL);
}

void ContactElementT::WriteOutput(void)
{
// look at EXODUS output in continuumelementT
        /* contact statistics */
        ostream& out = ElementSupport().Output();
        out << "\n Contact tracking: group "
                << ElementSupport().ElementGroupNumber(this) + 1 << '\n';
        out << " Time                           = "
                << ElementSupport().Time() << '\n';

        /* output files */
        StringT filename;
        filename.Root(ElementSupport().Input().filename());
        filename.Append(".", ElementSupport().StepNumber());
        filename.Append("of", ElementSupport().NumberOfSteps());

        for(int s = 0; s < fSurfaces.Length(); s++) {
            const ContactSurfaceT& surface = fSurfaces[s];

           if (fOutputFlags[kGaps]) {
                StringT gap_out;
                gap_out = gap_out.Append(filename,".gap");
                gap_out = gap_out.Append(s);
                ofstream gap_file (gap_out);
                surface.PrintGaps(gap_file);
           }

           if (fOutputFlags[kNormals]) {
                StringT normal_out;
                normal_out = normal_out.Append(filename,".normal");
                normal_out = normal_out.Append(s);
                ofstream normal_file (normal_out);
                surface.PrintNormals(normal_file);
           }

           if (fOutputFlags[kStatus]) {
				surface.PrintStatus(cout);
           }

           if (fOutputFlags[kMultipliers]) {
//		surface.PrintMultipliers(cout);
                StringT pressure_out;
                pressure_out = pressure_out.Append(filename,".pre");
                pressure_out = pressure_out.Append(s);
                ofstream pressure_file (pressure_out);
                surface.PrintMultipliers(pressure_file);
           }

           if (fOutputFlags[kArea]) {
              surface.PrintContactArea(cout);
           }
		}
}

/* compute specified output parameter and send for smoothing */
void ContactElementT::SendOutput(int kincode)
{
#pragma unused(kincode)
//not implemented: contact tractions/forces
}

/* writing output - nothing to write */
void ContactElementT::RegisterOutput(void) {}

/* solution calls */
void ContactElementT::AddNodalForce(const FieldT& field, int node, dArrayT& force)
{
#pragma unused(field)
#pragma unused(node)
#pragma unused(force)
//not implemented
}

/* Returns the energy as defined by the derived class types */
double ContactElementT::InternalEnergy(void)
{
//not implemented
        return 0.0;
}


/***********************************************************************
* Protected
***********************************************************************/

/* print element group data */
void ContactElementT::ReadControlData(void)
{
    /* streams */
    ifstreamT& in = ElementSupport().Input(); 
    ostream&  out = ElementSupport().Output(); 

    /* print flags */
    fOutputFlags.Dimension(kNumOutputFlags);
    for (int i = 0; i < fOutputFlags.Length(); i++) {
        in >> fOutputFlags[i];
    }
	out << " Print gaps                 = " << fOutputFlags[kGaps] << '\n';
	out << " Print normals              = " << fOutputFlags[kNormals] << '\n';
	out << " Print status               = " << fOutputFlags[kStatus] << '\n';
	out << " Print multipliers          = " << fOutputFlags[kMultipliers] << '\n';
    out << " Print contact area         = " << fOutputFlags[kArea] << '\n';


    int num_surfaces;
    in >> num_surfaces;
    if (num_surfaces < 1) throw ExceptionT::kBadInputValue;
	out << " Number of contact surfaces. . . . . . . . . . . = "
	    << num_surfaces << '\n';

    int num_pairs;
    in >> num_pairs;
    if (num_pairs < 1 || num_pairs > num_surfaces*(num_surfaces-1))
        throw ExceptionT::kBadInputValue;
	out << " Number of surface pairs with data . . . . . . . = "
	    << num_pairs << '\n';

	/* parameters */
	out << " Number of search parameters . . . . . . . . . . = "
	    << kSearchNumParameters << '\n';
	out << " Number of enforcement parameters. . . . . . . . = "
	    << fNumEnfParameters << '\n';
    fSearchParameters.Dimension(num_surfaces);
    fEnforcementParameters.Dimension(num_surfaces);
    int s1, s2;
    for (int i = 0; i < num_pairs ; i++)
    {
        in >> s1 >> s2;
        s1--; s2--;

        dArrayT& search_parameters = fSearchParameters(s1,s2);
        /* general parameters for search */
        search_parameters.Allocate (kSearchNumParameters);

        dArrayT& enf_parameters    = fEnforcementParameters(s1,s2);
        /* parameters specific to enforcement */
        enf_parameters.Allocate (fNumEnfParameters);
        for (int j = 0 ; j < search_parameters.Length() ; j++)
        {
            in >> search_parameters[j];
        }
        for (int j = 0 ; j < enf_parameters.Length() ; j++)
        {
            in >> enf_parameters[j];
        }
    }
    fSearchParameters.CopySymmetric();
    fEnforcementParameters.CopySymmetric();

}

/* print element group data */
void ContactElementT::PrintControlData(ostream& out) const
{
	/* inherited */
	ElementBaseT::PrintControlData(out);

}

/* echo contact surfaces */
void ContactElementT::EchoConnectivityData(ifstreamT& in, ostream& out)
{
	int num_surfaces = fSearchParameters.Rows();
	/* surfaces */
	out << " Surface connectivity data .........................\n";
	fSurfaces.Dimension(num_surfaces); 
	for (int i = 0; i < fSurfaces.Length(); i++)
	{
		int spec_mode;
		in >> spec_mode;
		ContactSurfaceT& surface = fSurfaces[i];
		surface.SetTag(i);
		/* read connectivity data */
		switch (spec_mode)
		{
			case kSideSets:
				surface.InputSideSets(ElementSupport(), in, out);
				break;
			
			default:
				cout << "\n ContactElementT::EchoSurfaceData:"
                                     << " unknown surface specification\n";
				cout <<   "     mode " << spec_mode 
                                     << " for surface " << i+1 << '\n';
				throw ExceptionT::kBadInputValue;
		}
		surface.PrintConnectivityData(out);
	}
}

/* generate contact element data - return true if configuration has
 * changed since the last call */
/* generate connectivity data based on current node-face pairs */
bool ContactElementT::SetContactConfiguration(void)
{
	bool changed = fContactSearch->SetInteractions();
	
        if (changed) { 
		/* form potential connectivity for step */
  		for (int i = 0; i < fSurfaces.Length(); i++) {
			fSurfaces[i].SetPotentialConnectivity();
  		}
	}

	return changed;
}

bool ContactElementT::UpdateContactConfiguration(void)
{
	bool changed = fContactSearch->UpdateInteractions();
	return changed;
}

int ContactElementT::PassType (int s1, int s2) const
{
    dArrayT& parameters = fSearchParameters(s1,s2);
    int pass_code = (int) parameters[kPass];
    if (s1 == s2 || pass_code == 0) {
        return kSymmetric;
    }
    else if (s1 < s2) {
        switch (pass_code) {
            case  1: return kPrimary; break;
            case  2: return kSecondary; break;
            case -1: return kDeformable; break;
            case -2: return kRigid; break;
        }
    }
    else {//(s1 > s2)
        switch (pass_code) {
            case  2: return kPrimary; break;
            case  1: return kSecondary; break;
            case -2: return kDeformable; break;
            case -1: return kRigid; break;
        }
    }

    /* dummy return value */
    return kPrimary;
}

