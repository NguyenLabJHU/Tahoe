/* $Id: FS_SCNIMFT.cpp,v 1.2 2004-04-12 16:51:47 cjkimme Exp $ */
#include "FS_SCNIMFT.h"

//#define VERIFY_B

#include "ArrayT.h"
#include "ofstreamT.h"
#include "ifstreamT.h"
#include "fstreamT.h"
#include "eIntegratorT.h"
#include "OutputSetT.h"
#include "ElementSupportT.h"
#include "CommManagerT.h"
#include "CommunicatorT.h"
#include "BasicFieldT.h"

#include "MeshFreeNodalShapeFunctionT.h"
#include "ContinuumMaterialT.h"
#include "SolidMaterialT.h"
#include "FSSolidMatT.h"
#include "SolidMatSupportT.h"

/* materials lists */
#include "SolidMatList1DT.h"
#include "SolidMatList2DT.h"
#include "SolidMatList3DT.h"

using namespace Tahoe;

/* constructors */
FS_SCNIMFT::FS_SCNIMFT(const ElementSupportT& support, const FieldT& field):
	SCNIMFT(support, field),
	fFSMatSupport(NULL)
{
	SetName("fd_mfparticle");
}

FS_SCNIMFT::FS_SCNIMFT(const ElementSupportT& support):
	SCNIMFT(support)
{
	SetName("fd_mfparticle");
}

/* destructor */
FS_SCNIMFT::~FS_SCNIMFT(void)
{
	delete fFSMatSupport;
}

/* generate labels for output data */
void FS_SCNIMFT::GenerateOutputLabels(ArrayT<StringT>& labels)
{
  	/* Reference Configuration */
	const char* ref[3] = {"X", "Y", "Z"};

	/* displacement labels */
	const char* disp[3] = {"D_X", "D_Y", "D_Z"};
	int num_labels = 2*fSD; // displacements
	int num_stress=0;

	const char* stress[6];
	const char* strain[6];
	
	stress[0] = "s11";
	stress[1] = "s22";
	strain[0] = "e11";
	strain[1] = "e22";
	num_stress = 3;
	
	if (fSD == 3)
	{
		num_stress=6;
		stress[2] = "s33";
	  	stress[3] = "s23";
	 	stress[4] = "s13";
	  	stress[5] = "s12";
	  	strain[2] = "e33";
	  	strain[3] = "e23";
	  	strain[4] = "e13";
	  	strain[5] = "e12";
	} 
	
	if (fSD == 2) 
	{
	  	stress[2] = "s12";
	  	strain[2] = "e12";
	}
		
	num_labels += 2 * num_stress + 1; 
	labels.Dimension(num_labels);
	int dex = 0;
	for (dex = 0; dex < fSD; dex++)
		labels[dex] = ref[dex];
	for (int ns = 0 ; ns < fSD; ns++)
	  	labels[dex++] = disp[ns];
	labels[dex++] = "mass";
	for (int ns = 0 ; ns < num_stress; ns++)
		labels[dex++] = strain[ns];
	for (int ns = 0 ; ns < num_stress; ns++)
		labels[dex++] = stress[ns];
}

void FS_SCNIMFT::WriteOutput(void)
{
	ofstreamT& out = ElementSupport().Output();

	const char caller[] = "FS_SCNIMFT::WriteOutput";
	
	/* muli-processor information */
	CommManagerT& comm_manager = ElementSupport().CommManager();
	const ArrayT<int>* proc_map = comm_manager.ProcessorMap();
	int rank = ElementSupport().Rank();

	/* dimensions */
	int ndof = NumDOF();
	int num_stress = fSD == 2 ? 3 : 6;
	int num_output = 2*ndof + 1 + 2 * num_stress; /* ref. coords, displacements, mass, e, s */

	/* number of nodes */
	const ArrayT<int>* partition_nodes = comm_manager.PartitionNodes();
	int non = (partition_nodes) ? partition_nodes->Length() : 
								ElementSupport().NumNodes();

	/* map from partition node index */
	const InverseMapT* inverse_map = comm_manager.PartitionNodes_inv();

	/* output arrays length number of active nodes */
	dArray2DT n_values(non, num_output), e_values;
	n_values = 0.0;

	/* global coordinates */
	const dArray2DT& coords = ElementSupport().CurrentCoordinates();

	/* the field */
	const FieldT& field = Field();
	const dArray2DT* velocities = NULL;
	if (field.Order() > 0) velocities = &(field[1]);

	/* check 2D */
	if (NumDOF() != 2) ExceptionT::GeneralFail(caller, "2D only: %d", NumDOF());

	/* For now, just one material. Grab it */
	ContinuumMaterialT *mat = (*fMaterialList)[0];
	SolidMaterialT* fCurrMaterial = TB_DYNAMIC_CAST(SolidMaterialT*,mat);
	if (!fCurrMaterial)
	{
		ExceptionT::GeneralFail("FS_SCNIMFT::RHSDriver","Cannot get material\n");
	}
	
	int nNodes = fNodes.Length();

	const RaggedArray2DT<int>& nodeSupport = fNodalShapes->NodeNeighbors();
	
	ArrayT<dMatrixT> Flist(1);
	Flist[0].Dimension(fSD);
	dMatrixT& Fdef = Flist[0];
	dMatrixT BJ(fSD*fSD, fSD);
	
	/* displacements */
	const dArray2DT& u = Field()(0,0);

	/* collect displacements */
	dArrayT vec, values_i;
	for (int i = 0; i < nNodes; i++) 
	{
		int   tag_i = (partition_nodes) ? (*partition_nodes)[i] : i;
		int local_i = (inverse_map) ? inverse_map->Map(tag_i) : tag_i;

		/* values for particle i */
		n_values.RowAlias(local_i, values_i);

		/* copy in */
		vec.Set(ndof, values_i.Pointer());
		coords.RowCopy(tag_i, vec);
		vec.Set(ndof, values_i.Pointer() + ndof);
		vec = 0.;
			
		LinkedListT<int>& supp_i = fNodalSupports[i];
		LinkedListT<double>& phi_i = fNodalPhi[i];
		supp_i.Top(); phi_i.Top();
		while (supp_i.Next() && phi_i.Next()) 
		{
			vec.AddScaled(*(phi_i.CurrentValue()), u(*(supp_i.CurrentValue())));
		}
		
		// Compute smoothed deformation gradient
		Fdef = 0.0;
		LinkedListT<dArrayT>& bVectors_i = facetWorkSpace[i];
		LinkedListT<int>& nodeSupport_i = nodeWorkSpace[i];
		nodeSupport_i.Top(); bVectors_i.Top();
		while (nodeSupport_i.Next() && bVectors_i.Next())
		//for (int j = 0; j < nodeSupport.MinorDim(i); j++)
		{
			bVectorToMatrix(bVectors_i.CurrentValue()->Pointer(), BJ);
			BJ.Multx(u(*(nodeSupport_i.CurrentValue())), Fdef.Pointer(), 1.0, 1);
		}	
		Fdef.PlusIdentity();
		fFSMatSupport->SetDeformationGradient(&Flist);
		
		const double* stress = fCurrMaterial->s_ij().Pointer();

		double* inp_val = values_i.Pointer() + 2*ndof;
		
		*inp_val++ = fVoronoiCellVolumes[i];

		for (int j = 0; j < num_stress; j++)
			*inp_val++ = Fdef[j];
		for (int j = 0; j < num_stress; j++)
			*inp_val++ = stress[j]; 

	}

	/* send */
	ElementSupport().WriteOutput(fOutputID, n_values, e_values);

}

/* compute specified output parameter(s) */
void FS_SCNIMFT::SendOutput(int kincode)
{
#pragma unused(kincode)
	//TEMP: for now, do nothing
}

/* trigger reconfiguration */
GlobalT::RelaxCodeT FS_SCNIMFT::RelaxSystem(void)
{
	/* inherited */
	GlobalT::RelaxCodeT relax = ElementBaseT::RelaxSystem();

	return relax;
}

/* write restart data to the output stream */
void FS_SCNIMFT::WriteRestart(ostream& out) const
{
	ElementBaseT::WriteRestart(out);
	
}

/* read restart data to the output stream */
void FS_SCNIMFT::ReadRestart(istream& in)
{
	ElementBaseT::ReadRestart(in);
}

/* contribution to the nodal residual forces */
//const dArray2DT& FS_SCNIMFT::InternalForce(int group)
//{
	/* check */
//	if (group != Group())
//		ExceptionT::GeneralFail("FS_SCNIMFT::InternalForce", 
//			"expecting solver group %d not %d", Group(), group);
//	return fForce;
//}

/***********************************************************************
 * Protected
 ***********************************************************************/

/* form group contribution to the stiffness matrix */
void FS_SCNIMFT::LHSDriver(GlobalT::SystemTypeT sys_type)
{
#pragma unused(sys_type)
	/* time integration parameters */
	double constK = 0.0;
	double constM = 0.0;
	int formK = fIntegrator->FormK(constK);
	int formM = fIntegrator->FormM(constM);
	
	/* quick exit */
	if ((formM == 0 && formK == 0) ||
	    (fabs(constM) < kSmall &&
	     fabs(constK) < kSmall)) return;

	/* assemble particle mass */
	if (formM) {
		//AssembleParticleMass(mass);
	}
	
	if (formK)
	{
		/* hold the smoothed strain */
		ArrayT<dMatrixT> Flist(1);
		Flist[0].Dimension(fSD);
		dMatrixT& Fdef = Flist[0];
		
		/* displacements */
		const dArray2DT& u = Field()(0,0);
	
		/* For now, just one material. Grab it */
		ContinuumMaterialT *mat = (*fMaterialList)[0];
		SolidMaterialT* fCurrMaterial = TB_DYNAMIC_CAST(SolidMaterialT*,mat);
		if (!fCurrMaterial)
		{
			ExceptionT::GeneralFail("FS_SCNIMFT::LHSDriver","Cannot get material\n");
		}

		int nNodes = fNodes.Length();

		/* assembly information */
		int group = Group();
		int ndof = NumDOF();
		fLHS.Dimension(ndof);
		const ElementSupportT& support = ElementSupport();
		
		const iArray2DT& field_eqnos = Field().Equations();
		iArrayT row_eqnos(ndof); 
		iArrayT col_eqnos(ndof);
		dMatrixT BJ(fSD*fSD, ndof), BK(fSD*fSD, ndof), FTs(fSD), fStress(fSD);
		dMatrixT Tijkl(fSD*fSD), BJTCijkl(fSD, fSD == 2 ? 3 : 6), K_JK;
		K_JK.Alias(fLHS);
		LinkedListT<dArrayT> bVectors_j;
		LinkedListT<int> nodeSupport_j;
		for (int i = 0; i < nNodes; i++)
		{	
			double w_i = fVoronoiCellVolumes[i]*constK; // integration weights
			
			// Compute smoothed deformation gradient
			Fdef = 0.0;
			LinkedListT<dArrayT>& bVectors_i = facetWorkSpace[i];
			LinkedListT<int>& nodeSupport_i = nodeWorkSpace[i];
			nodeSupport_i.Top(); bVectors_i.Top();
			while (nodeSupport_i.Next() && bVectors_i.Next())
			{
				bVectorToMatrix(bVectors_i.CurrentValue()->Pointer(), BJ);
				BJ.Multx(u(*(nodeSupport_i.CurrentValue())), Fdef.Pointer(), 1.0, 1);
			}	
			Fdef.PlusIdentity(); // convert to F
			fFSMatSupport->SetDeformationGradient(&Flist);
		
			const dMatrixT& cijkl = fCurrMaterial->c_ijkl();
			fCurrMaterial->s_ij().ToMatrix(fStress);
		
			// FTs = F^T sigma
			FTs.MultATB(Fdef, fStress);
			// T_11 = T_22 = FTs_11 T_13 = T_24 = FTs_12
			// T_31 = T_42 = FTs_21 T_33 = T_44 = FTs_22
			Tijkl.Expand(FTs, fSD, dMatrixT::kOverwrite);
			// Tijkl = 0.; used to debug material stiffness
			// use brute force, low-level until I find optimal way
			Tijkl[0] += cijkl[0]; // += C_11
			Tijkl[1] += cijkl[6]; // += C_13
			Tijkl[2] += cijkl[6]; // += C_13
			Tijkl[3] += cijkl[3]; // += C_12
			Tijkl[4] += cijkl[6]; // += C_13
			Tijkl[5] += cijkl[8]; // += C_33
			Tijkl[6] += cijkl[8]; // += C_33
			Tijkl[7] += cijkl[7]; // += C_23
			Tijkl[8] += cijkl[6]; // += C_13
			Tijkl[9] += cijkl[8]; // += C_33
			Tijkl[10] += cijkl[8]; // += C_33
			Tijkl[11] += cijkl[7]; // += C_23
			Tijkl[12] += cijkl[3]; // += C_12
			Tijkl[13] += cijkl[7]; // += C_23
			Tijkl[14] += cijkl[7]; // += C_23
			Tijkl.Last() += cijkl[4]; // += C_22
			
			// sum over pairs to get contribution to stiffness
			nodeSupport_i.Top(); bVectors_i.Top();
			while (nodeSupport_i.Next() && bVectors_i.Next())
			{
				bVectorToMatrix(bVectors_i.CurrentValue()->Pointer(), BJ);
				//BJTCijkl.MultATB(BJ, cijkl, 0); // material stiffness alone
				
				/* simultanesouly compute material and stress stiffnesses */
				BJTCijkl.MultATB(BJ, Tijkl, 1); // accumulate stress stiffness
				col_eqnos.Copy(field_eqnos(*(nodeSupport_i.CurrentValue())));
				
				bVectors_j.Alias(bVectors_i);
				nodeSupport_j.Alias(nodeSupport_i);
				do // this is do..while to calculate the i == j term before advancing to the next node
				{
					bVectorToMatrix(bVectors_j.CurrentValue()->Pointer(), BK);
					
					// K_JK = BT_J x Cijkl x B_K 
					K_JK.MultAB(BJTCijkl, BK, 0);
					
					K_JK *= w_i*constK;
					
					/* assemble */
					row_eqnos.Copy(field_eqnos(*(nodeSupport_j.CurrentValue())));
					support.AssembleLHS(group, fLHS, row_eqnos, col_eqnos);
				} while (nodeSupport_j.Next() && bVectors_j.Next());
			}	
		}
	}
}

void FS_SCNIMFT::RHSDriver(void)
{
	/* function name */
	const char caller[] = "ParticlePairT::RHSDriver2D";

	/* check 2D */
	if (NumDOF() != 2) ExceptionT::GeneralFail(caller, "2D only: %d", NumDOF());

	/* time integration parameters */
	double constMa = 0.0;
	double constKd = 0.0;
	int formMa = fIntegrator->FormMa(constMa);
	int formKd = fIntegrator->FormKd(constKd);

	/* For now, just one material. Grab it */
	ContinuumMaterialT *mat = (*fMaterialList)[0];
	SolidMaterialT* fCurrMaterial = TB_DYNAMIC_CAST(SolidMaterialT*,mat);
	if (!fCurrMaterial)
	{
		ExceptionT::GeneralFail("FS_SCNIMFT::RHSDriver","Cannot get material\n");
	}
	
	int nNodes = fNodes.Length();

	if (formMa) 
	{
		if (Field().Order() < 2)
			ExceptionT::GeneralFail(caller,"Field's Order does not have accelerations\n");
	
		fLHS = 0.0; // fLHS.Length() = nNodes * fSD;
		const dArray2DT& a = Field()(0,2); // accelerations
		double* ma = fLHS.Pointer();
		const double* acc;
		int* nodes = fNodes.Pointer();
		double* volume = fVoronoiCellVolumes.Pointer();
		for (int i = 0; i < nNodes; i++)
		{
			acc = a(*nodes++);
			for (int j = 0; j < fSD; j++)
				*ma++ = *volume * *acc++;
			volume++;
		}
		fLHS *= fCurrMaterial->Density();
	}

	fForce = 0.0;
	ArrayT<dMatrixT> Flist(1);
	Flist[0].Dimension(fSD);
	dMatrixT& Fdef = Flist[0];
	dMatrixT BJ(fSD*fSD, fSD), fStress(fSD);
	
	/* displacements */
	const dArray2DT& u = Field()(0,0);
	for (int i = 0; i < nNodes; i++)
	{
		double w_i = fVoronoiCellVolumes[i]; // integration weight

		// Compute smoothed deformation gradient
		Fdef = 0.0;
		LinkedListT<dArrayT>& bVectors_i = facetWorkSpace[i];
		LinkedListT<int>& nodeSupport_i = nodeWorkSpace[i];
		nodeSupport_i.Top(); bVectors_i.Top();
		int jcount = 0;
		while (nodeSupport_i.Next() && bVectors_i.Next())
		{
			bVectorToMatrix(bVectors_i.CurrentValue()->Pointer(), BJ);
			BJ.Multx(u(*(nodeSupport_i.CurrentValue())), Fdef.Pointer(), 1.0, 1);
		}
		Fdef.PlusIdentity(); // convert to F 	
		fFSMatSupport->SetDeformationGradient(&Flist);
		
		fCurrMaterial->s_ij().ToMatrix(fStress);
		
		nodeSupport_i.Top(); bVectors_i.Top();
		while (nodeSupport_i.Next() && bVectors_i.Next())
		{
			bVectorToMatrix(bVectors_i.CurrentValue()->Pointer(), BJ);
			double* fint = fForce(*(nodeSupport_i.CurrentValue()));
			BJ.MultTx(fStress.Pointer(), fint, w_i, 1);      
		}	
	}
	
	fForce *= -constKd;

	/* assemble */
	ElementSupport().AssembleRHS(Group(), fForce, Field().Equations());

}

void FS_SCNIMFT::bVectorToMatrix(double *bVector, dMatrixT& BJ)
{
#if __option(extended_errorcheck)
	if (BJ.Rows() != fSD*2) 
		ExceptionT::SizeMismatch("SCNIMFT::bVectorToMatrix","Matrix has bad majorDim");
	if (BJ.Cols() != fSD) 
		ExceptionT::SizeMismatch("SCNIMFT::bVectorToMatrix","Matrix has bad minorDim");
#endif

	double* Bptr = BJ.Pointer();
	
	BJ = 0.;
	Bptr[0] = *bVector;
	if (fSD == 2)
	{
		Bptr[5] = *bVector++;
		Bptr[2] = *bVector;
		Bptr[7] = *bVector;
	}
	else // fSD == 3
	{   // I haven't changed this yet
		Bptr[11] = Bptr[16] = *bVector++;
		Bptr[7] = *bVector;
		Bptr[5] = Bptr[15] = *bVector++;
		Bptr[14] = *bVector;
		Bptr[4] = Bptr[9] = *bVector;
	}
}

void FS_SCNIMFT::ReadMaterialData(ifstreamT& in)
{
	/* base class */
	SCNIMFT::ReadMaterialData(in);

	/* offset to class needs flags */
	fNeedsOffset = fMaterialNeeds[0].Length();
	
	/* set material needs */
	for (int i = 0; i < fMaterialNeeds.Length(); i++)
	{
		/* needs array */
		ArrayT<bool>& needs = fMaterialNeeds[i];

		/* resize array */
		needs.Resize(needs.Length() + 2, true);

		/* casts are safe since class contructs materials list */
		ContinuumMaterialT* pcont_mat = (*fMaterialList)[i];
		FSSolidMatT* mat = (FSSolidMatT*) pcont_mat;

		/* collect needs */
		needs[fNeedsOffset] = mat->Need_F();
		needs[fNeedsOffset + 1] = mat->Need_F_last();
		
		/* consistency */
		needs[0] = needs[0] || needs[fNeedsOffset];
		needs[2] = needs[2] || needs[fNeedsOffset + 1];
	}
}

/* use in conjunction with ReadMaterialData */
void FS_SCNIMFT::WriteMaterialData(ostream& out) const
{
	fMaterialList->WriteMaterialData(out);

	/* flush buffer */
	out.flush();
}

/* return a pointer to a new material list */
MaterialListT* FS_SCNIMFT::NewMaterialList(int nsd, int size)
{
	/* full list */
	if (size > 0)
	{
		/* material support */
		if (!fFSMatSupport) {
			fFSMatSupport = new FSMatSupportT(fSD, fSD, 1);
			
			if (!fFSMatSupport)
				ExceptionT::GeneralFail("FS_SCNIMFT::NewMaterialList","Could not instantiate material support\n");
				
			/* ElementSupportT sources */
			const ElementSupportT& e_support = ElementSupport();
			fFSMatSupport->SetRunState(e_support.RunState());
			fFSMatSupport->SetStepNumber(e_support.StepNumber());
			fFSMatSupport->SetTime(e_support.Time());                              
			fFSMatSupport->SetTimeStep(e_support.TimeStep());
			fFSMatSupport->SetNumberOfSteps(e_support.NumberOfSteps());
		}

		if (nsd == 1)
			return new SolidMatList1DT(size, *fFSMatSupport);
		else if (nsd == 2)
			return new SolidMatList2DT(size, *fFSMatSupport);
		else if (nsd == 3)
			return new SolidMatList3DT(size, *fFSMatSupport);
		else
			return NULL;
	}
	else
	{
		if (nsd == 1)
			return new SolidMatList1DT;
		else if (nsd == 2)
			return new SolidMatList2DT;
		else if (nsd == 3)
			return new SolidMatList3DT;
		else
			return NULL;
	}	
}
	
// XML stuff below

/* describe the parameters needed by the interface */
void FS_SCNIMFT::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	ElementBaseT::DefineParameters(list);

}

/* information about subordinate parameter lists */
void FS_SCNIMFT::DefineSubs(SubListT& sub_list) const
{
	/* inherited */
	ElementBaseT::DefineSubs(sub_list);

}

/* return the description of the given inline subordinate parameter list */
void FS_SCNIMFT::DefineInlineSub(const StringT& sub, ParameterListT::ListOrderT& order, 
	SubListT& sub_sub_list) const
{
	/* inherited */
	ElementBaseT::DefineInlineSub(sub, order, sub_sub_list);
}

/* a pointer to the ParameterInterfaceT of the given subordinate */
ParameterInterfaceT* FS_SCNIMFT::NewSub(const StringT& list_name) const
{
	/* inherited */
	return ElementBaseT::NewSub(list_name);
}