/* $Id: PenaltyContactElement2DT.cpp,v 1.24 2002-06-08 20:20:20 paklein Exp $ */
#include "PenaltyContactElement2DT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>

#include "ContactNodeT.h"
#include "Linear.h"
#include "ModSmithFerrante.h"
#include "GreenwoodWilliamson.h"

/* vector functions */
#include "vector2D.h"

/* constants */
const double PI = 2.0*acos(0.0);

/* parameters */
static const int kMaxNumFaceNodes = 4;
static const int kMaxNumFaceDOF   = 12;

/* constructor */
PenaltyContactElement2DT::PenaltyContactElement2DT(const ElementSupportT& support, const FieldT& field):
	ContactElementT(support, field, kNumEnfParameters)
{
}

void PenaltyContactElement2DT::Initialize(void)
{
    ContactElementT::Initialize();

	/* set up Penalty functions */
	int num_surfaces = fSearchParameters.Rows();
	fPenaltyFunctions.Dimension(num_surfaces*(num_surfaces-1));
    for (int i = 0; i < num_surfaces ; i++)
    {
        for (int j = 0 ; j < num_surfaces ; j++)
        {
          dArrayT& parameters = fEnforcementParameters(i,j);
		  if (parameters.Length()) {
			switch ((int) parameters[kPenaltyType]) 
			{
			case PenaltyContactElement2DT::kLinear:
				// Macauley bracket:  <-x> ???
				fPenaltyFunctions[LookUp(i,j,num_surfaces)] = new Linear(1.0);
				break;
			case PenaltyContactElement2DT::kModSmithFerrante:
				{
                double A = parameters[kSmithFerranteA];
                double B = parameters[kSmithFerranteB];
				fPenaltyFunctions[LookUp(i,j,num_surfaces)] = new ModSmithFerrante(A,B);
				//parameters[kPenalty] = 1.0; // overwrite pen value
				}
				break;
			case PenaltyContactElement2DT::kGreenwoodWilliamson:
				{
                /* parameters for Greenwood-Williamson load formulation */
                double gw_m = parameters[kAsperityHeightMean];
                double gw_s = parameters[kAsperityHeightStandardDeviation];
                double gw_dens = parameters[kAsperityDensity];
                double gw_mod = parameters[kHertzianModulus];
                double gw_rad = parameters[kAsperityTipRadius];
                double material_coeff=(4.0/3.0)*gw_dens*gw_mod*sqrt(gw_rad);
          		double area_coeff = PI*gw_dens*gw_rad;
				parameters[kPenalty] *= material_coeff; // overwrite pen value
				fPenaltyFunctions[LookUp(i,j,num_surfaces)]
                                        = new GreenwoodWilliamson(1.5,gw_m,gw_s);
				}
				break;
			default:
				throw eBadInputValue;
			}
		  }
		}
	}

	/* subsidary data for GW models */
    fRealArea.Allocate(fSurfaces.Length());
	fRealArea = 0;
}

/* print/compute element output quantities */
void PenaltyContactElement2DT::WriteOutput(IOBaseT::OutputModeT mode)
{
	/* call base class */
	ContactElementT::WriteOutput(mode);
	
	cout << "\n";
	for (int i=0; i<fSurfaces.Length(); i++)
		cout << "real contact area = " << fRealArea[i] << "\n";
}

/***********************************************************************
 * Protected
 ***********************************************************************/

/* print element group data */
void PenaltyContactElement2DT::PrintControlData(ostream& out) const
{
	ContactElementT::PrintControlData(out);
    /* write out search parameter matrix */
	out << " Interaction parameters ............................\n";
	int num_surfaces = fSearchParameters.Rows();
    for (int i = 0; i < num_surfaces ; i++)
	{
		for (int j = i ; j < num_surfaces ; j++)
		{
			dArrayT& search_parameters = fSearchParameters(i,j);
			dArrayT& enf_parameters = fEnforcementParameters(i,j);
			/* only print allocated parameter arrays */
			if (search_parameters.Length() == kSearchNumParameters) {
		  	  out << "  surface pair: ("  << i << "," << j << ")\n" ;
			  out << "  gap tolerance:    "
					<< search_parameters[kGapTol] << '\n';
			  out << "  xi tolerance :    "
					<< search_parameters[kXiTol] << '\n';
			  out << "  pass flag    :    "
					<< (int) search_parameters[kPass] << '\n';
			  out << "  consis. tangent:  "
                    << (int) enf_parameters[kConsistentTangent] << '\n';
			  out << "  penalty :         "
					<< enf_parameters[kPenalty] << '\n';
			  out << "  penalty types:\n"
				  << "     Linear              " 
 				  << PenaltyContactElement2DT::kLinear << "\n"
				  << "     ModSmithFerrante    " 
				  << PenaltyContactElement2DT::kModSmithFerrante << "\n"
				  << "     GreenwoodWilliamson " 
			      << PenaltyContactElement2DT::kGreenwoodWilliamson << "\n";
			  out << "  penalty Type :         "
					<< (int) enf_parameters[kPenaltyType] << '\n';
			  switch ((int) enf_parameters[kPenaltyType]) 
			  {
			  case kLinear: // no other parameters
			    out << "  <no parameters> \n";
				break;	
			  case kModSmithFerrante:
				out << "  Smith-Ferrante A : "
					<< enf_parameters[kSmithFerranteA] << '\n';
				out << "  Smith-Ferrante B : "
					<< enf_parameters[kSmithFerranteB] << '\n';
				break;	
			  case kGreenwoodWilliamson:
				out << "  Average asperity height            : "
					<< enf_parameters[kAsperityHeightMean] << '\n';
				out << "  Asperity height standard deviation : "
					<< enf_parameters[kAsperityHeightStandardDeviation] << '\n';
				out << "  Asperity density                   : "
					<< enf_parameters[kAsperityDensity] << '\n';
				out << "  Asperity Radius                    : "
					<< enf_parameters[kAsperityTipRadius] << '\n';
				out << "  Hertzian Modulus                   : "
					<< enf_parameters[kHertzianModulus] << '\n';
				break;	
			  default:
				throw eBadInputValue;
		  	  }
			}
		}
	}
	out <<'\n';
}

/* called before LHSDriver during iteration process */
void PenaltyContactElement2DT::RHSDriver(void)
{ /* form RESIDUAL */ 
  /* update kinematic data */
  UpdateContactConfiguration();

  bool in_contact = 0;
  ContactNodeT* node;
  double gap, pre;
  int num_nodes,s2;


  int num_surfaces = fSurfaces.Length(); 
  int nsd = NumSD();
  /* residual */
  for(int s = 0; s < num_surfaces; s++) {
	ContactSurfaceT& surface = fSurfaces[s];
	/* all faces on a surface the same size */
	const ArrayT<FaceT*>& faces = surface.Faces();
	const ArrayT<ContactNodeT*>& nodes = surface.ContactNodes();
	num_nodes = surface.NumNodesPerFace();
	RHS_man.SetLength(num_nodes*nsd,false);
	tmp_RHS_man.SetLength(num_nodes*nsd,false);
	N1_man.SetDimensions(num_nodes*nsd, nsd);
	weights_man.SetLength(num_nodes,false);
	eqnums1_man.SetMajorDimension(num_nodes,false);
	fRealArea[s] = 0;
		
	/*form residual for this surface */
	for (int f = 0;  f < faces.Length(); f++) {
	  const FaceT* face = faces[f];
	  face->Quadrature(points,weights);
	  /* primary face */
	  const iArrayT& conn1 = face->GlobalConnectivity();
	  RHS = 0.0;
	  in_contact = 0;
	  /*loop over (nodal) quadrature points */
	  /*NOTE: these CORRESPOND to local node numbers */
	  for (int i = 0 ; i < weights.Length() ; i++) {
		node = nodes[face->Node(i)];
		if (node->Status() > ContactNodeT::kNoProjection) {
		  in_contact = 1;
		  s2 = node->OpposingFace()->Surface().Tag();
		  gap =  node->Gap();
		  dArrayT& parameters = fEnforcementParameters(s,s2);
		  /* First derivative represents force */
          pre  = -parameters[kPenalty] * fPenaltyFunctions[LookUp(s,s2,num_surfaces)]->DFunction(gap);
		  node->nPressure() = pre; // store value on ContactNode for output
		  face->ComputeShapeFunctions(points(i),N1);
		  for (int j =0; j < nsd; j++) {n1[j] = node->Normal()[j];}
		  N1.Multx(n1, tmp_RHS);
		  /* pressure = -e <g> and t = - p n   */
		  tmp_RHS.SetToScaled(-pre*weights[i], tmp_RHS);
		  RHS += tmp_RHS;


		  /* real area computation */
		  if (parameters[kPenaltyType] 
				== PenaltyContactElement2DT::kGreenwoodWilliamson) {
			double gw_m = parameters[kAsperityHeightMean];
			double gw_s = parameters[kAsperityHeightStandardDeviation];
			double gw_dens = parameters[kAsperityDensity];
			double gw_mod = parameters[kHertzianModulus];
			double gw_rad = parameters[kAsperityTipRadius];

			GreenwoodWilliamson GWArea(1.0,gw_m,gw_s); 	
  		  	double area_coeff = PI*gw_dens*gw_rad;
		  	fRealArea[s] += (area_coeff*GWArea.Function(gap)*weights[i]);
		  }
		}
	  } 
          /* get equation numbers */
          Field().SetLocalEqnos(conn1, eqnums1);
          /* assemble */
          if (in_contact) ElementSupport().AssembleRHS(Group(), RHS, eqnums1);
	}
  }
}

void PenaltyContactElement2DT::LHSDriver(void)
{ /* form STIFFNESS */
  /* del g =  (n1.N2 * del u2 -  n1.N1 * del u1) */
  /* primary (X) primary block */
  /* primary (X) secondary block */

  bool in_contact;
  int consistent;
  int num_nodes,opp_num_nodes;
  int s2;
  ContactNodeT* node;
  double pre=0.0, dpre_dg=0.0;
  double gap=0.0;
  double lm2[3];
  dArrayT n1alphal1;
  n1alphal1.Allocate(NumSD());

  /* for consistent stiffness */    
   dArrayT N1nl;
   VariArrayT<double> N1nl_man(kMaxNumFaceDOF,N1nl);
   dMatrixT T1;
   nVariMatrixT<double> T1_man(kMaxNumFaceDOF,T1);
   dArrayT T1n;
   VariArrayT<double> T1n_man(kMaxNumFaceDOF,T1n);

   dMatrixT Perm(NumSD());
   Perm(0,0) = 0.0 ; Perm(0,1) = -1.0;
   Perm(1,0) = 1.0 ; Perm(1,1) =  0.0;
   double alpha;


	int nsd = NumSD();
    int num_surfaces = fSurfaces.Length(); 
	for(int s = 0; s < num_surfaces; s++) {
		ContactSurfaceT& surface = fSurfaces[s];
        /* all faces on a surface the same size */
        const ArrayT<FaceT*>& faces = surface.Faces();
        const ArrayT<ContactNodeT*>& nodes = surface.ContactNodes();
		num_nodes = surface.NumNodesPerFace();	
        LHS_man.SetDimensions(num_nodes*nsd);
        tmp_LHS_man.SetDimensions(num_nodes*nsd);
        N1_man.SetDimensions(num_nodes*nsd, nsd);
        N1n_man.SetLength(num_nodes*nsd,false);
		//if consistent
         N1nl_man.SetLength(num_nodes*nsd,false);
         T1_man.SetDimensions(num_nodes*nsd, nsd);
         T1n_man.SetLength(num_nodes*nsd,false);
        weights_man.SetLength(num_nodes,false);
        eqnums1_man.SetMajorDimension(num_nodes,false);
        /* form stiffness */
        for (int f = 0;  f < faces.Length(); f++) {
			const FaceT* face = faces[f];
			face->Quadrature(points,weights);
			/* primary face */
			const iArrayT& conn1 = face->GlobalConnectivity();
			/* get equation numbers */
			Field().SetLocalEqnos(conn1, eqnums1);
			LHS = 0.0;
			in_contact = 0;
			/*loop over (nodal) quadrature points */
			/*NOTE: these CORRESPOND to local node numbers */
			for (int i = 0 ; i < weights.Length() ; i++) {
				node = nodes[face->Node(i)];
                if (node->Status() > ContactNodeT::kNoProjection) {
					in_contact = 1;
					s2 = node->OpposingFace()->Surface().Tag();
					gap =  node->Gap();
					face->ComputeShapeFunctions(points(i),N1);
					dArrayT& parameters = fEnforcementParameters(s,s2);		
					pre  = 
					 parameters[kPenalty]*fPenaltyFunctions[LookUp(s,s2,num_surfaces)]->DFunction(gap);
					dpre_dg = 
					 parameters[kPenalty]*fPenaltyFunctions[LookUp(s,s2,num_surfaces)]->DDFunction(gap);
					consistent = (int) parameters[kConsistentTangent];
					const FaceT* opp_face = node->OpposingFace();
					opp_num_nodes = opp_face->NumNodes();
					N2_man.SetDimensions(opp_num_nodes*nsd, nsd);
					N2n_man.SetLength(opp_num_nodes*nsd,false);
					eqnums2_man.SetMajorDimension(opp_num_nodes,false);
					const iArrayT& conn2 = opp_face->GlobalConnectivity();
					opp_face->ComputeShapeFunctions
						(node->OpposingLocalCoordinates(),N2);
					const double* nm1 = node->Normal();
					for (int j =0; j < nsd; j++) {n1[j] = nm1[j];}
					N1.Multx(n1, N1n);
					N2.Multx(n1, N2n); 

					/* N1n (x) D g */
					/* Part:  dx1 (x) dx2 */
					tmp_LHS_man.SetDimensions(opp_num_nodes*nsd);
					tmp_LHS.Outer(N1n, N2n);
					tmp_LHS.SetToScaled(-dpre_dg*weights[i], tmp_LHS);

					/* get equation numbers */
					Field().SetLocalEqnos(conn2, eqnums2);
					/* assemble primary-secondary face stiffness */
					ElementSupport().AssembleLHS(Group(), tmp_LHS, eqnums1,eqnums2);

					/* Part:  dx1 (x) dx1 */
					if (consistent) {
						for (int j =0; j < nsd; j++) 
							{l1[j] =node->Tangent1()[j];}
						node->OpposingFace()->ComputeTangent1(points(i),lm2);
						alpha = Dot(node->Normal(),lm2) 
							/ Dot(l1.Pointer(),  lm2);
						for (int j =0; j < nsd; j++) 
							{n1alphal1[j] = n1[j] - alpha*l1[j];}
						N1.Multx(n1alphal1, N1nl);
						tmp_LHS.Outer(N1n, N1nl);
						tmp_LHS_man.SetDimensions(num_nodes*nsd);
						tmp_LHS.SetToScaled(dpre_dg*weights[i], tmp_LHS);
						LHS += tmp_LHS;

						face->ComputeShapeFunctionDerivatives(points(i),T1);
						double jac = face->ComputeJacobian(points(i));

						T1.Multx(n1, T1n);
						tmp_LHS.Outer(N1n, T1n);
						tmp_LHS.SetToScaled(pre*alpha*weights[i]/jac, tmp_LHS);
						LHS += tmp_LHS;

						T1.MultAB(T1,Perm);
						tmp_LHS.MultABT(N1, T1);
						tmp_LHS.SetToScaled(-pre*weights[i], tmp_LHS);
						LHS += tmp_LHS;
					} else {
						tmp_LHS.Outer(N1n, N1n);
						tmp_LHS.SetToScaled(dpre_dg*weights[i], tmp_LHS);
						LHS += tmp_LHS;
					}
				}
			}
          	/* assemble primary-primary face stiffness */
			if (in_contact) ElementSupport().AssembleLHS(Group(), LHS, eqnums1);
		}
	}
}

/***********************************************************************
 * Private
 ***********************************************************************/

