#include "FSDielectricElastomerT.h"
#include "FSDEMatSupportT.h"
#include "FSDEMatT.h"
#include "ParameterContainerT.h"
#include "OutputSetT.h"
#include "ShapeFunctionT.h"

//
// materials lists (3D only)
//
#include "FSSolidMatList3DT.h"

namespace Tahoe {

  //
  //	constructor
  //
/* constructor */
// FSDielectricElastomerT::FSDielectricElastomerT(const ElementSupportT& support):
// {
// 
// }

/* Destructor */
  FSDielectricElastomerT::~FSDielectricElastomerT()
  {
	  if (0 != fFSDEMatSupport) delete fFSDEMatSupport;
  }

  //
  // specify parameters needed by the interface
  //
  void FSDielectricElastomerT::DefineParameters(ParameterListT& list) const
  {
    // inherited
    FiniteStrainT::DefineParameters(list);

    // additional fields
//    list.AddParameter(ParameterT::Word, "electric_field_name");
  }

  /* describe the parameters needed by the interface */
  void FSDielectricElastomerT::DefineSubs(SubListT& sub_list) const
  {
	  /* inherited */
	  SolidElementT::DefineSubs(sub_list);

	  /* Dielectric elastomer parameters */
	  sub_list.AddSub("Dielectric_Elastomer");
  }

  //
  // accept parameter list
  //
  void FSDielectricElastomerT::TakeParameterList(const ParameterListT& list)
  {
    //
    // inherited
    //
    FiniteStrainT::TakeParameterList(list);

    /* Define matrix sizes */
    int nen = NumElementNodes();
    int nsd = NumSD();
    int nel = nen;	// # electrical DOFs per element
    int nme = nen * nsd;	// # of mechanical DOFs per element
    int dof = nsd + 1;	// total # of DOFs per node (mech + elec)
    int neq = nen * dof;	// total # of DOFs per element (mech + elec)

    fAmm.Dimension(nme, nme);
    fAme.Dimension(nme, nel);
    fAem.Dimension(nel, nme);
    fAee.Dimension(nel, nel);

    fLHS.Dimension(neq);
    fRHS.Dimension(neq);

	fGradNa.Dimension(nsd, nen);
  }

  //
  // PROTECTED
  //

  //
  // construct a new material support and return a pointer
  //
  MaterialSupportT*
  FSDielectricElastomerT::NewMaterialSupport(MaterialSupportT* p) const
  {
    //
    // allocate
    //
    if (!p) p = new FSDEMatSupportT(NumDOF(), NumIP());

    //
    // inherited initializations
    //
    FiniteStrainT::NewMaterialSupport(p);

    //
    // set parent class fields
    //
    FSDEMatSupportT* ps = dynamic_cast<FSDEMatSupportT*> (p);

//     if (ps != 0) {
// 
//       ps->SetElectricDisplacement(&fD_List);
// 
//     }

    return p;
  }

  //
  // construct materials manager and read data
  //
  MaterialListT*
  FSDielectricElastomerT::NewMaterialList(const StringT& name, int size)
  {
    if (name != "large_strain_material_3D") {
      return 0;
    }
    MaterialListT* mlp = 0;

    if (size > 0) {

      //
      // material support
      //
      if (0 == fFSDEMatSupport) {
        fFSDEMatSupport = dynamic_cast<FSDEMatSupportT*> (NewMaterialSupport());

        if (0 == fFSDEMatSupport) {
          ExceptionT::GeneralFail("FSDielectricElastomerT::NewMaterialList");
        }
      }

      mlp = new FSSolidMatList3DT(size, *fFSDEMatSupport);

    } else {
      mlp = new FSSolidMatList3DT;
    }
    return mlp;

  }

  //
  // form shape functions and derivatives; for ComputeOutput
  //
  void FSDielectricElastomerT::SetGlobalShape()
  {

    //
    // inherited
    //
    FiniteStrainT::SetGlobalShape();


  }

  //
  // write all current element information to the stream
  //
  void FSDielectricElastomerT::CurrElementInfo(ostream& out) const
  {

    //
    // inherited
    //
    FiniteStrainT::CurrElementInfo(out);

  }

// 
//  increment current element - for ComputeOutput
// 
  bool FSDielectricElastomerT::NextElement()
  {

    bool isThereNext = FiniteStrainT::NextElement();

    if (isThereNext == true) {

      const int index = CurrentElement().MaterialNumber();

      ContinuumMaterialT* pMaterial = (*fMaterialList)[index];

      fCurrMaterial = dynamic_cast<FSDEMatT*> (pMaterial);

    }

    return isThereNext;

  }

  //
  //
  //
  void FSDielectricElastomerT::Equations(AutoArrayT<const iArray2DT*>& eq_1,
      AutoArrayT<const RaggedArray2DT<int>*>& eq_2)
  {
    for (int i = 0; i < fEqnos.Length(); ++i) {

      const int ndf = NumDOF();
      const int nen = fConnectivities[i]->MinorDim();
      const int offset = ndf * nen;

//       fElectricVectorPotentialField->SetLocalEqnos(*fConnectivities[i],
//           fEqnos[i], offset);

    }
	
    ElementBaseT::Equations(eq_1, eq_2);
  }

/* calculate the LHS of residual, or element stiffness matrix */
  void FSDielectricElastomerT::FormStiffness(double constK)
  {
    //
    // matrix format
    //
    dMatrixT::SymmetryFlagT format = (fLHS.Format()
        == ElementMatrixT::kNonSymmetric)
        ? dMatrixT::kWhole
        : dMatrixT::kUpperOnly;

    //
    // integrate over element
    //
    const int nsd = NumSD();
    const int nen = NumElementNodes();

    fAmm = 0.0;
    fAme = 0.0;
    fAem = 0.0;
    fAee = 0.0;

    fShapes->TopIP();

    while (fShapes->NextIP() != 0) {

      //
      // scale/weighting factor for integration
      //
      const double w = constK * fShapes->IPDet() * fShapes->IPWeight();

      /* prepare derivatives of shape functions in reference configuration */
      const dArray2DT& DNaX = fShapes->Derivatives_X();
	  Set_B(DNaX, fB);
	  fShapes->GradNa(DNaX, fGradNa);	  

	  /* A potential issue for all stiffnesses:  the free energy function depends upon
	     the stretch tensor, deformation gradient, and electric field.  The problem is with
	     the E-field, since it's the gradient of the potential, and so somehow, the value
	     of the potential (which are nodal values) will have to be obtained here */

	  //
      // get all stiffnesses needed for LHS of residual
      //
      dMatrixT C = fCurrMaterial->C_IJKL();
      dMatrixT H = fCurrMaterial->E_IJK();
      dMatrixT HT(3,6);	// may need to initialize
      H.Transpose(HT);
      dMatrixT B = fCurrMaterial->B_IJ();
      dSymMatrixT S = fCurrMaterial->S_IJ();
	  dArrayT D = fCurrMaterial->D_I();	

	  /* mechanical-mechanical stiffness (24 x 24 matrix for the 8-node 3D element) */
 	  C *= w;
 	  fAmm.MultQTBQ(fB, C, format, dMatrixT::kAccumulate);
	  
 	  /* Not sure if these next two mechanical-electrical MultATBC are correct */
 	  /* mechanical-electrical stiffness (24 x 8 matrix for 8-node 3D element) */
 	  H *= w;
 	  fAme.MultATBC(fB, H, fGradNa, dMatrixT::kWhole, dMatrixT::kAccumulate);
	  
 	  /* electrical-mechanical stiffness (8 x 24 matrix for 8-node 3D element) */
 	  /* This may be redundant since it should be symmetric of mechanical-electrical */
// 	  fAem.MultATBC(fB, HT, fGradNa, dMatrixT::kWhole, dMatrixT::kAccumulate);	  
 	  fAme.Transpose(fAem);
 
 	  /* electrical-electrical stiffness (8 x 8 matrix for 8-node 3D element) */
 	  B *= w;
 	  fAee.MultQTBQ(fGradNa, B, format, dMatrixT::kAccumulate);
    }

	/* Assemble into fLHS, or element stiffness matrix */
	fLHS.AddBlock(0, 0, fAmm);
	fLHS.AddBlock(fAmm.Rows(), fAmm.Cols(), fAee);
	fLHS.AddBlock(0, fAmm.Cols(), fAme);
	fLHS.AddBlock(fAmm.Rows(), 0, fAem);
  }

/* Compute RHS, or residual of element equations */
  void FSDielectricElastomerT::FormKd(double constK)
  {
    const int nsd = NumSD();
    const int nen = NumElementNodes();
    
    /* Define mechanical and electrical residuals */
	dArrayT Rtotal((nsd+1)*nen);
	dMatrixT RMech(nen, nsd);
	RMech = 0.0;
	Rtotal = 0.0;
	dArrayT RElec(nen);
	RElec = 0.0;

    fShapes->TopIP();

    while (fShapes->NextIP() != 0) {

	  /* integration weight */
      const double w = constK * fShapes->IPDet() * fShapes->IPWeight();

      const dArray2DT & DNaX = fShapes->Derivatives_X();
	  fShapes->GradNa(DNaX, fGradNa);	
	  
	  /* Mechanical stress */
	  dSymMatrixT S = fCurrMaterial->S_IJ();	
	  fGradNa.MultTx(S, RMech);	// 8x3 matrix RMech2; check mult by dSymMatrixT
	  RMech *= -1.0;	// need for right sign for residual
	  
	  /* Electrical displacement - assume it is defined in FSDEMatT somehow */
	  dArrayT D = fCurrMaterial->D_I();	// electrical displacement vector 3 x 1
	  fGradNa.MultTx(D, RElec);	// 3x1 vector of shape function gradient * D
	  
	  /* NOTE:  mechanical inertia term, mechanical body force term, mechanical
	  surface traction term, electrical body force (charge), electrical surface 
	  traction not accounted for here */
	  
	}
	
	/* Transfer RMech2 from dMatrixT to arrays */
	dArrayT temp1, temp2, temp3;
	RMech.CopyColumn(0, temp1);
	RMech.CopyColumn(1, temp2);
	RMech.CopyColumn(2, temp3);
	Rtotal.CopyIn(0, temp1);
	Rtotal.CopyIn(nen, temp2);
	Rtotal.CopyIn(2*nen, temp3);
	Rtotal.CopyIn(3*nen, RElec);
	
	fRHS += Rtotal;
  }

  //
  // extrapolate from integration points and compute output nodal/element
  // values
  //
  void FSDielectricElastomerT::ComputeOutput(const iArrayT& n_codes,
      dArray2DT& n_values, const iArrayT& e_codes, dArray2DT& e_values)
  {
// 
//  number of output values
// 
//     int n_out = n_codes.Sum();
//     int e_out = e_codes.Sum();
// 
//  nothing to output
//     if (n_out == 0 && e_out == 0) return;
// 
//  dimensions
//     int nsd = NumSD();
//     int ndof = NumDOF();
//     int nen = NumElementNodes();
//     int nnd = ElementSupport().NumNodes();
//     int nstrs = StrainDim();
// 
//  reset averaging work space
//     ElementSupport().ResetAverage(n_out);
// 
//  allocate element results space
//     e_values.Dimension(NumElements(), e_out);
// 
//  nodal work arrays
//     dArray2DT nodal_space(nen, n_out);
//     dArray2DT nodal_all(nen, n_out);
//     dArray2DT coords, disp;
//     dArray2DT nodalstress, princstress, matdat;
//     dArray2DT energy, speed;
//     dArray2DT ndElectricVectorPotential;
//     dArray2DT ndDivergenceVectorPotential;
//     dArray2DT ndElectricDisplacement;
//     dArray2DT ndElectricField;
// 
//  ip values
//     dArrayT ipmat(n_codes[iMaterialData]), ipenergy(1);
//     dArrayT ipspeed(nsd), ipprincipal(nsd);
//     dMatrixT ippvector(nsd);
// 
//  set shallow copies
//     double* pall = nodal_space.Pointer();
//     coords.Alias(nen, n_codes[iNodalCoord], pall);
//     pall += coords.Length();
//     disp.Alias(nen, n_codes[iNodalDisp], pall);
//     pall += disp.Length();
// 
//     nodalstress.Alias(nen, n_codes[iNodalStress], pall);
//     pall += nodalstress.Length();
//     princstress.Alias(nen, n_codes[iPrincipal], pall);
//     pall += princstress.Length();
//     energy.Alias(nen, n_codes[iEnergyDensity], pall);
//     pall += energy.Length();
//     speed.Alias(nen, n_codes[iWaveSpeeds], pall);
//     pall += speed.Length();
//     matdat.Alias(nen, n_codes[iMaterialData], pall);
//     pall += matdat.Length();
// 
//     ndElectricVectorPotential.Alias(nen, n_codes[ND_ELEC_POT], pall);
//     pall += ndElectricVectorPotential.Length();
// 
//     ndDivergenceVectorPotential.Alias(nen, n_codes[ND_DIV_POT], pall);
//     pall += ndDivergenceVectorPotential.Length();
// 
//     ndElectricDisplacement.Alias(nen, n_codes[ND_ELEC_DISP], pall);
//     pall += ndElectricDisplacement.Length();
// 
//     ndElectricField.Alias(nen, n_codes[ND_ELEC_FLD], pall);
//     pall += ndElectricField.Length();
// 
//  element work arrays
//     dArrayT element_values(e_values.MinorDim());
//     pall = element_values.Pointer();
//     dArrayT centroid, ip_centroid, ip_mass;
//     dArrayT ip_coords(nsd);
//     if (e_codes[iCentroid]) {
//       centroid.Alias(nsd, pall);
//       pall += nsd;
//       ip_centroid.Dimension(nsd);
//     }
//     if (e_codes[iMass]) {
//       ip_mass.Alias(NumIP(), pall);
//       pall += NumIP();
//     }
//     double w_tmp, ke_tmp;
//     double mass;
//     double& strain_energy = (e_codes[iStrainEnergy])
//         ? *pall++
//         : w_tmp;
//     double& kinetic_energy = (e_codes[iKineticEnergy])
//         ? *pall++
//         : ke_tmp;
//     dArrayT linear_momentum, ip_velocity;
// 
//     if (e_codes[iLinearMomentum]) {
//       linear_momentum.Alias(ndof, pall);
//       pall += ndof;
//       ip_velocity.Dimension(ndof);
//     } else if (e_codes[iKineticEnergy]) ip_velocity.Dimension(ndof);
// 
//     dArray2DT ip_stress;
//     if (e_codes[iIPStress]) {
//       ip_stress.Alias(NumIP(), e_codes[iIPStress] / NumIP(), pall);
//       pall += ip_stress.Length();
//     }
//     dArray2DT ip_material_data;
//     if (e_codes[iIPMaterialData]) {
//       ip_material_data.Alias(NumIP(), e_codes[iIPMaterialData] / NumIP(), pall);
//       pall += ip_material_data.Length();
//       ipmat.Dimension(ip_material_data.MinorDim());
//     }
// 
//     dArray2DT ipElectricDisplacement;
//     if (e_codes[IP_ELEC_DISP]) {
//       ipElectricDisplacement.Alias(NumIP(), NumSD(), pall);
//       pall += NumIP() * NumSD();
//     }
// 
//     dArray2DT ipElectricField;
//     if (e_codes[IP_ELEC_FLD]) {
//       ipElectricField.Alias(NumIP(), NumSD(), pall);
//       pall += NumIP() * NumSD();
//     }
// 
//  check that degrees are displacements
//     int interpolant_DOF = InterpolantDOFs();
// 
//     Top();
//     while (NextElement()) {
// 
//       if (CurrentElement().Flag() == ElementCardT::kOFF) continue;
// 
//       // initialize
//       nodal_space = 0.0;
// 
//       // global shape function values
//       SetGlobalShape();
// 
//       // collect nodal values
//       if (e_codes[iKineticEnergy] || e_codes[iLinearMomentum]) {
//         if (fLocVel.IsRegistered())
//           SetLocalU(fLocVel);
//         else
//           fLocVel = 0.0;
//       }
// 
//       // coordinates and displacements all at once
//       if (n_codes[iNodalCoord]) fLocInitCoords.ReturnTranspose(coords);
//       if (n_codes[iNodalDisp]) {
//         if (interpolant_DOF)
//           fLocDisp.ReturnTranspose(disp);
//         else
//           NodalDOFs(CurrentElement().NodesX(), disp);
//       }
// 
//       // initialize element values
//       mass = strain_energy = kinetic_energy = 0;
//       if (e_codes[iCentroid]) centroid = 0.0;
//       if (e_codes[iLinearMomentum]) linear_momentum = 0.0;
//       const double* j = fShapes->IPDets();
//       const double* w = fShapes->IPWeights();
// 
//       // integrate
//       dArray2DT Na_X_ip_w;
//       fShapes->TopIP();
//       while (fShapes->NextIP() != 0) {
// // 
// //         // density may change with integration point
// //         double density = fCurrMaterial->Density();
// 
//         // element integration weight
//         double ip_w = (*j++) * (*w++);
// 
//         if (qNoExtrap) {
//           Na_X_ip_w.Dimension(nen, 1);
//           for (int k = 0; k < nen; k++) {
//             Na_X_ip_w(k, 0) = 1.;
//           }
//         }
// 
//         // get Cauchy stress
// //         const dSymMatrixT& stress = fCurrMaterial->s_ij();
//         dSymMatrixT strain;
// 
//         // stresses
//         if (n_codes[iNodalStress]) {
//           if (qNoExtrap) {
//             for (int k = 0; k < nen; k++) {
// //              nodalstress.AddToRowScaled(k, Na_X_ip_w(k, 0), stress);
//             }
//           } else {
// //            fShapes->Extrapolate(stress, nodalstress);
//           }
//         }
// 
//         if (e_codes[iIPStress]) {
//           double* row = ip_stress(fShapes->CurrIP());
//           strain.Set(nsd, row);
// //          strain = stress;
// //          row += stress.Length();
//           strain.Set(nsd, row);
// //           fCurrMaterial->Strain(strain);
//         }
// 
//           // integrate over element
//           if (e_codes[iStrainEnergy]) {
// //            strain_energy += ip_w * ip_strain_energy;
//           }
// 
//         }
// 
//         // material stuff
//         if (n_codes[iMaterialData] || e_codes[iIPMaterialData]) {
//           // compute material output
// //          fCurrMaterial->ComputeOutput(ipmat);
// 
//           // store nodal data
//           if (n_codes[iMaterialData]) {
//             if (qNoExtrap) {
//               for (int k = 0; k < nen; k++) {
//                 matdat.AddToRowScaled(k, Na_X_ip_w(k, 0), ipmat);
//               }
//             } else {
//               fShapes->Extrapolate(ipmat, matdat);
//             }
//           }
// 
//           // store element data
//           if (e_codes[iIPMaterialData]) {
//             ip_material_data.SetRow(fShapes->CurrIP(), ipmat);
//           }
//         }
// 
// 
//         // divergence of vector potential
//         dArrayT DivPhi(1);
// //        DivPhi = fCurrMaterial->DivPhi();
// 
//         if (n_codes[ND_DIV_POT]) {
//           if (qNoExtrap) {
//             for (int k = 0; k < nen; k++) {
//               ndDivergenceVectorPotential.AddToRowScaled(k, Na_X_ip_w(k, 0), DivPhi);
//             }
//           } else {
//             fShapes->Extrapolate(DivPhi, ndDivergenceVectorPotential);
//           }
//         }
// 
// 
//         // electric field
// //        const dArrayT& E = fCurrMaterial->E_I();
// 
//         if (n_codes[ND_ELEC_FLD]) {
//           if (qNoExtrap) {
//             for (int k = 0; k < nen; k++) {
// //              ndElectricField.AddToRowScaled(k, Na_X_ip_w(k, 0), E);
//             }
//           } else {
//  //           fShapes->Extrapolate(E, ndElectricField);
//           }
//         }
// 
//       }
// 
//       // copy in the cols
//       int colcount = 0;
//       nodal_all.BlockColumnCopyAt(disp, colcount);
//       colcount += disp.MinorDim();
// 
//       nodal_all.BlockColumnCopyAt(coords, colcount);
//       colcount += coords.MinorDim();
// 
//       if (qNoExtrap) {
//         double nip(fShapes->NumIP());
//         nodalstress /= nip;
//         princstress /= nip;
//         energy /= nip;
//         speed /= nip;
//         matdat /= nip;
//         ndElectricVectorPotential /= nip;
//         ndDivergenceVectorPotential /= nip;
//         ndElectricDisplacement /= nip;
//         ndElectricField /= nip;
//       }
//       nodal_all.BlockColumnCopyAt(nodalstress, colcount);
//       colcount += nodalstress.MinorDim();
// 
//       nodal_all.BlockColumnCopyAt(princstress, colcount);
//       colcount += princstress.MinorDim();
// 
//       nodal_all.BlockColumnCopyAt(energy, colcount);
//       colcount += energy.MinorDim();
// 
//       nodal_all.BlockColumnCopyAt(speed, colcount);
//       colcount += speed.MinorDim();
// 
//       nodal_all.BlockColumnCopyAt(matdat, colcount);
//       colcount += matdat.MinorDim();
// 
//       nodal_all.BlockColumnCopyAt(ndElectricVectorPotential, colcount);
//       colcount += ndElectricVectorPotential.MinorDim();
// 
//       nodal_all.BlockColumnCopyAt(ndDivergenceVectorPotential, colcount);
//       colcount += ndDivergenceVectorPotential.MinorDim();
// 
//       nodal_all.BlockColumnCopyAt(ndElectricDisplacement, colcount);
//       colcount += ndElectricDisplacement.MinorDim();
// 
//       nodal_all.BlockColumnCopyAt(ndElectricField, colcount);
//       colcount += ndElectricField.MinorDim();
// 
//       // accumulate - extrapolation done from ip's to corners => X nodes
//       ElementSupport().AssembleAverage(CurrentElement().NodesX(), nodal_all);
// 
//       // element values
//       if (e_codes[iCentroid]) centroid /= mass;
// 
//       // store results
//       e_values.SetRow(CurrElementNumber(), element_values);
// 
//     }
// 
//  get nodally averaged values
// //     const OutputSetT& output_set = ElementSupport().OutputSet(fOutputID);
// //     const iArrayT& nodes_used = output_set.NodesUsed();
// //     dArray2DT extrap_values(nodes_used.Length(), n_out);
// //     extrap_values.RowCollect(nodes_used, ElementSupport().OutputAverage());
// // 
// //     int tmpDim = extrap_values.MajorDim();
// //     n_values.Dimension(tmpDim, n_out);
// //     n_values.BlockColumnCopyAt(extrap_values, 0);
   }

} // namespace Tahoe
