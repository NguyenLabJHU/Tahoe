/* $Id: MultiplierContactElement2DT.cpp,v 1.14 2002-11-21 01:13:36 paklein Exp $ */
// created by : rjones 2001

// DEVELOPMENT

#include "MultiplierContactElement2DT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>


#include "MultiplierContactElement2DT.h"
#include "ContactNodeT.h"
#include "ElementSupportT.h"
#include "XDOF_ManagerT.h"

/* vector functions */
#include "vector2D.h"

/* parameters */

using namespace Tahoe;

static const int kMaxNumFaceNodes = 4;
static const int kMaxNumFaceDOF   = 12;

/* constructor */
MultiplierContactElement2DT::MultiplierContactElement2DT
(const ElementSupportT& support, const FieldT& field):
	ContactElementT(support, field, kNumEnfParameters, &support.XDOF_Manager())
{
	fNumMultipliers = 1;
}

/***********************************************************************
 * Protected
 ***********************************************************************/

/* print element group data */
void MultiplierContactElement2DT::PrintControlData(ostream& out) const
{
	ContactElementT::PrintControlData(out);
	/* warning : req. solver that pivots */
	// CHECK SOLVER TYPE

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
              out << "  gap tolerance:      "
                    << search_parameters[kGapTol] << '\n';
              out << "  xi tolerance :      "
                    << search_parameters[kXiTol] << '\n';
			  out << "  pass flag    :      "
                    << (int) search_parameters[kPass] << '\n';
              out << "  consistent tangent: "
                    << (int) enf_parameters[kConsistentTangent] << '\n';
              out << "  penalty :           "
                    << enf_parameters[kPenalty] << '\n';
              out << "  gap scale :         "
                    << enf_parameters[kGScale] << '\n';
              out << "  pressure scale:     "
                    << enf_parameters[kPScale] << '\n';
              out << "  pressure tolerance: "
                    << enf_parameters[kTolP] << '\n';
			}
		}
	}

}

/* called before LHSDriver during iteration process */
void MultiplierContactElement2DT::SetStatus(void)
{ 
  int opp_surf_tag;
  for(int surf_tag = 0; surf_tag < fSurfaces.Length(); surf_tag++) {
	ContactSurfaceT& surface = fSurfaces[surf_tag];
	ArrayT<ContactNodeT*>& nodes = surface.ContactNodes();
	for(int n = 0; n < nodes.Length(); n++){	
		ContactNodeT* node = nodes[n];
		node->EnforcementStatus() = kNoP;//initialize
		if (node->HasMultiplier()){
			double& pre = node->Pressure(); 
			if (node->HasProjection() ){
				opp_surf_tag = node->OpposingSurface()->Tag();
				dArrayT& parameters 
					= fEnforcementParameters(surf_tag,opp_surf_tag);
				int ipass = PassType(surf_tag,opp_surf_tag);
				double tolP = parameters[kTolP];
				if (ipass == kSecondary) { /* pressure collocation */
					node->EnforcementStatus() = kPJump;	
				}
				else if(pre > -tolP) { /* contact */
					node->EnforcementStatus() = kGapZero;	
				}
				else { /* no contact */
					node->EnforcementStatus() = kPZero;	
				}
			}
			else { /* no contact */
					node->EnforcementStatus() = kPZero;	
			}
		}
	}
// DEBUG
#if 0
	cout << "\n\n";
	surface.PrintGaps(cout);
	surface.PrintNormals(cout);
	surface.PrintMultipliers(cout);
	surface.PrintStatus(cout);
#endif
  }
}

/* called before LHSDriver during iteration process */
void MultiplierContactElement2DT::RHSDriver(void)
{ /* form RESIDUAL */ 
  /* update kinematic data */
  UpdateContactConfiguration();

  /* set status of all surface nodes */
  SetStatus();

  bool elem_in_contact = 0;
  int opp_surf_tag=-1, status=-1;
  int num_nodes;
  ContactNodeT* node;
  double gap, /*pen,*/ pre, opp_pre=0.0;

	int nsd = NumSD();
  for(int surf_tag = 0; surf_tag < fSurfaces.Length(); surf_tag++) {
	ContactSurfaceT& surface = fSurfaces[surf_tag];
	/* all faces on a surface the same size */
	const ArrayT<FaceT*>& faces = surface.Faces();
	const ArrayT<ContactNodeT*>& nodes = surface.ContactNodes();
	num_nodes = surface.NumNodesPerFace();
	RHS_man.SetLength(num_nodes*nsd,false);
	xRHS_man.SetLength(num_nodes*fNumMultipliers,false);
	tmp_RHS_man.SetLength(num_nodes*nsd,false);
	N1_man.SetDimensions(num_nodes*nsd, nsd);
	P1_man.SetDimensions(num_nodes,fNumMultipliers);
	weights_man.SetLength(num_nodes,false);
	eqnums1_man.SetMajorDimension(num_nodes,false);
	xeqnums1_man.SetMajorDimension(num_nodes,false);
	xconn1_man.SetLength(num_nodes,false);
		
	/*form residual for this surface */
	for (int f = 0;  f < faces.Length(); f++) {
		const FaceT* face = faces[f];
		face->Quadrature(points,weights);
		/* primary face */
		const iArrayT& conn1 = face->GlobalConnectivity();
		surface.MultiplierTags(face->Connectivity(),xconn1);

		RHS = 0.0;
		xRHS = 0.0;
		elem_in_contact = 0;
		/*loop over (nodal) quadrature points */
		/*NOTE: these CORRESPOND to local node numbers */
		for (int i = 0 ; i < weights.Length() ; i++) {
			node = nodes[face->Node(i)];
			status = node->EnforcementStatus();
			if (status > kNoP )  {
				elem_in_contact = 1;
				const ContactSurfaceT* opp_surf = node->OpposingSurface();
				opp_surf_tag = opp_surf->Tag();
				dArrayT& parameters = 
				fEnforcementParameters(surf_tag,opp_surf_tag);

/* BLM: U dof on primary surface  -------------------------------------*/
				if (status == kGapZero || status == kPJump){
					/* pressure */
					pre = node->Pressure() ;
					/* gap */
					gap = node->Gap();
					/* pressure =  Lagrange multiplier + penalty */
					if (status == kGapZero) 
						{ pre += -parameters[kPenalty]*gap;}
					face->ComputeShapeFunctions(points(i),N1);
					for (int j =0; j < nsd; j++) {n1[j] = node->Normal()[j];}
					N1.Multx(n1, tmp_RHS);
					tmp_RHS.SetToScaled(-pre*weights[i], tmp_RHS);
					RHS += tmp_RHS;
				}

/* Constraint : X dof on primary surface ----------------------------- */
				face->ComputeShapeFunctions(points(i),P1);
				if (status == kGapZero) {
					gap = node->Gap();
					P1.SetToScaled(parameters[kGScale]*gap, P1);
				}
				else if (status == kPJump) {
					/* calculate pressure on opposing face */
					const double* opp_xi  = node->OpposingLocalCoordinates();
					P2values_man.SetLength(opp_surf->NumNodesPerFace()
						*fNumMultipliers,false);
					opp_surf->MultiplierValues(
						node->OpposingFace()->Connectivity(),P2values);	
					opp_pre = node->OpposingFace()->
						Interpolate(opp_xi,P2values);
					double pj = opp_pre - node->Pressure() ;
					P1.SetToScaled(parameters[kPScale]*pj, P1);
				}
				else if (status == kPZero) {
					double pj = - node->Pressure() ;
					P1.SetToScaled(parameters[kPScale]*pj, P1);
				}
				xRHS += P1;
			}
		} 
		/* assemble */
		if (elem_in_contact) {
			const ElementSupportT& support = ElementSupport();
		
			Field().SetLocalEqnos(conn1, eqnums1);
			support.AssembleRHS(Group(), RHS, eqnums1);
			support.XDOF_Manager().XDOF_SetLocalEqnos(Group(), xconn1, xeqnums1);
			support.AssembleRHS(Group(), xRHS, xeqnums1);
		}
	}
  }
}

void MultiplierContactElement2DT::LHSDriver(void)
{ /* form STIFFNESS */
  bool elem_in_contact = 0;
  int opp_surf_tag=-1, status=-1;
  int consistent, num_nodes, opp_num_nodes;
  ContactNodeT* node;
  double sfac, gap;
  double l1[3],lm2[3];
  dArrayT n1alphal1;
  n1alphal1.Dimension(NumSD());

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
  for(int surf_tag = 0; surf_tag < fSurfaces.Length(); surf_tag++) {
	ContactSurfaceT& surface = fSurfaces[surf_tag];
	/* all faces on a surface the same size */
	const ArrayT<FaceT*>& faces = surface.Faces();
	const ArrayT<ContactNodeT*>& nodes = surface.ContactNodes();
	num_nodes = surface.NumNodesPerFace();
	LHS_man.SetDimensions(num_nodes*nsd);
	N1_man.SetDimensions(num_nodes*nsd, nsd);
	N1n_man.SetLength(num_nodes*nsd,false);
	//if consistent
	N1nl_man.SetLength(num_nodes*nsd,false);
	T1_man.SetDimensions(num_nodes*nsd, nsd);
	T1n_man.SetLength(num_nodes*nsd,false);
	weights_man.SetLength(num_nodes,false);
	eqnums1_man.SetMajorDimension(num_nodes,false);
	xeqnums1_man.SetMajorDimension(num_nodes,false);

	/* form stiffness */
	for (int f = 0;  f < faces.Length(); f++) {
		/* primary face */
		const FaceT* face = faces[f];
		const iArrayT& conn1 = face->GlobalConnectivity();
		surface.MultiplierTags(face->Connectivity(),xconn1);
		ElementSupport().XDOF_Manager().XDOF_SetLocalEqnos(Group(), xconn1, xeqnums1);
		face->Quadrature(points,weights);
		/* get equation numbers */
		Field().SetLocalEqnos(conn1, eqnums1);
		LHS = 0.0;
		elem_in_contact = 0;
		/*loop over (nodal) quadrature points */
		/*NOTE: these CORRESPOND to local node numbers */
		for (int i = 0 ; i < weights.Length() ; i++) {
			node = nodes[face->Node(i)];
			status = node->EnforcementStatus();
			if (status > kNoP )  {
				elem_in_contact = 1;
				/* set-up */
				gap = node->Gap();
				const FaceT* opp_face = node->OpposingFace();
				opp_num_nodes = opp_face->NumNodes();
                const double* opp_xi  = node->OpposingLocalCoordinates();
				const ContactSurfaceT* opp_surf = node->OpposingSurface();
				opp_surf_tag = opp_surf->Tag();
				dArrayT& parameters =
                	fEnforcementParameters(surf_tag,opp_surf_tag);
				consistent = (int) parameters[kConsistentTangent];
				sfac = parameters[kPenalty];

				/* pressure shape function matrix */
				face->ComputeShapeFunctions(points(i),P1);

/* BLM ============================================================= */
/* K =  N1n (x) { P1 + pen (n1.N2 * D u2 -  n1.N1 * D u1) }  */
                if (status == kGapZero || status == kPJump){
					face->ComputeShapeFunctions(points(i),N1);				
					for (int j =0; j < NumSD(); j++) {n1[j] = node->Normal()[j];}
					N1.Multx(n1, N1n);

/* primary U (X) primary   P block */
					tmp_LHS_man.SetDimensions
						(num_nodes*NumSD(),num_nodes*fNumMultipliers);
					tmp_LHS.Outer(N1n, P1);
					ElementSupport().AssembleLHS(Group(), tmp_LHS, eqnums1,xeqnums1);

                	if (status == kGapZero){
/* primary U (X) primary   U block */
						if (consistent) { 
							/* slip */
							tmp_LHS_man.SetDimensions
								(num_nodes*NumSD(),num_nodes*NumSD());
							for (int j =0; j < NumSD(); j++) 
								{l1[j] = node->Tangent1()[j];}
							opp_face->ComputeTangent1(
							  node->OpposingLocalCoordinates(),lm2);
							alpha = Dot(node->Normal(),lm2) 
							      / Dot(l1,lm2);
							for (int j =0; j < NumSD(); j++) 
								{n1alphal1[j] = n1[j] - alpha*l1[j];}
							N1.Multx(n1alphal1, N1nl);
							face->ComputeShapeFunctionDerivatives(points(i),T1);
							double jac = face->ComputeJacobian(points(i));
							T1.Multx(n1, T1n);
							T1n.AddCombination(sfac*weights[i], N1nl,
								sfac*gap*alpha*weights[i]/jac,T1n);
							LHS.Outer(N1n, T1n);
							/* jacobian */
							T1.MultAB(T1,Perm);
							tmp_LHS.MultABT(N1, T1);
							tmp_LHS.SetToScaled
								(node->Pressure()-sfac*gap*weights[i], tmp_LHS);
							LHS += tmp_LHS;
						} else {
							LHS.Outer(N1n, N1n);
							LHS.SetToScaled(sfac*weights[i], LHS);
						}
						ElementSupport().AssembleLHS(Group(), LHS, eqnums1);

/* primary U (X) secondary U block */
						/* get connectivity */
						const iArrayT& conn2 = opp_face->GlobalConnectivity();

						N2_man.SetDimensions(opp_num_nodes*NumSD(), NumSD());
						N2n_man.SetLength(opp_num_nodes*NumSD(),false);
						eqnums2_man.SetMajorDimension(opp_num_nodes,false);
						opp_face->ComputeShapeFunctions (opp_xi,N2);
						N2.Multx(n1, N2n); 

						tmp_LHS_man.SetDimensions
							(num_nodes*NumSD(),opp_num_nodes*NumSD());
						tmp_LHS.Outer(N1n, N2n);
						tmp_LHS.SetToScaled(-sfac*weights[i], tmp_LHS);

						/* get equation numbers */
						Field().SetLocalEqnos(conn2, eqnums2);
						/* assemble primary-secondary face stiffness */
						ElementSupport().AssembleLHS(Group(), tmp_LHS, eqnums1,eqnums2);

					}
				} 
/* Constraint ====================================================== */
                if (status == kGapZero) {
					double gfac = parameters[kGScale];
/* primary P (X) primary   U block */
					tmp_LHS_man.SetDimensions
						(num_nodes*fNumMultipliers,num_nodes*NumSD());
					if (consistent) {
						/* T1n is the consistent D_u1 g(u1,u2) */
						tmp_LHS.Outer(P1, T1n);
					} else {
						tmp_LHS.Outer(P1, N1n);
					}
					tmp_LHS.SetToScaled(gfac, tmp_LHS);
					ElementSupport().AssembleLHS(Group(), tmp_LHS, xeqnums1,eqnums1);
/* primary P (X) secondary U block */
					tmp_LHS_man.SetDimensions
						(num_nodes*fNumMultipliers,opp_num_nodes*NumSD());
					tmp_LHS.Outer(P1, N2n);
					tmp_LHS.SetToScaled(-gfac, tmp_LHS);
					ElementSupport().AssembleLHS(Group(), tmp_LHS, xeqnums1,eqnums2);
                }
                else if (status == kPJump || status == kPZero) {
					double pfac = parameters[kPScale];
/* primary P (X) primary   P block */
					tmp_LHS_man.SetDimensions
						(num_nodes*fNumMultipliers,num_nodes*fNumMultipliers);
					tmp_LHS.Outer(P1, P1);
					tmp_LHS.SetToScaled(pfac, tmp_LHS);
					ElementSupport().AssembleLHS(Group(), tmp_LHS, xeqnums1,xeqnums1);
                	if (status == kPJump) {
/* primary P (X) secondary P block */		
					    xconn2_man.SetLength(opp_num_nodes,false);
						opp_surf->MultiplierTags
							(opp_face->Connectivity(),xconn2);
					    xeqnums2_man.SetMajorDimension(opp_num_nodes,false);
						ElementSupport().XDOF_Manager().XDOF_SetLocalEqnos(Group(), xconn2, xeqnums2);
						P2_man.SetDimensions
							(opp_num_nodes*fNumMultipliers,fNumMultipliers);
						opp_face->ComputeShapeFunctions(opp_xi,P2);
						tmp_LHS_man.SetDimensions (num_nodes*fNumMultipliers,
							opp_num_nodes*fNumMultipliers);
						tmp_LHS.Outer(P1, P2);
						tmp_LHS.SetToScaled(-pfac, tmp_LHS);
						ElementSupport().AssembleLHS(Group(), tmp_LHS, xeqnums1,xeqnums2);
					}
                }
			}
		}
	}
 }
}
