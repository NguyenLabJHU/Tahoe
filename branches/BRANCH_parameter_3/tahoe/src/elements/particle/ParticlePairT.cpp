/* $Id: ParticlePairT.cpp,v 1.33 2004-04-02 16:48:22 jzimmer Exp $ */
#include "ParticlePairT.h"
#include "PairPropertyT.h"
#include "fstreamT.h"
#include "eIntegratorT.h"
#include "InverseMapT.h"
#include "CommManagerT.h"
#include "dSPMatrixT.h"
#include "ofstreamT.h"
#include "ifstreamT.h"
#include "ModelManagerT.h"
#include <iostream.h>
#include <iomanip.h>
#include <stdlib.h>
#include "dSymMatrixT.h"
#include "dArray2DT.h"
#include "iGridManagerT.h"

/* pair property types */
#include "LennardJonesPairT.h"
#include "HarmonicPairT.h"
#include "ParadynPairT.h"
#include "MatsuiPairT.h"

using namespace Tahoe;

/* parameters */
const int kMemoryHeadRoom = 15; /* percent */

/* constructor */
ParticlePairT::ParticlePairT(const ElementSupportT& support, const FieldT& field):
	ParticleT(support, field),
	fNeighbors(kMemoryHeadRoom),
	NearestNeighbors(kMemoryHeadRoom),
	RefNearestNeighbors(kMemoryHeadRoom),
	fEqnos(kMemoryHeadRoom),
	fForce_list_man(0, fForce_list)
{
	SetName("particle_pair");
	fopen = false;
}

/* constructor */
ParticlePairT::ParticlePairT(const ElementSupportT& support):
	ParticleT(support),
	fNeighbors(kMemoryHeadRoom),
	NearestNeighbors(kMemoryHeadRoom),
	RefNearestNeighbors(kMemoryHeadRoom),
	fEqnos(kMemoryHeadRoom),
	fForce_list_man(0, fForce_list)
{
	SetName("particle_pair");
	fopen = false;
}

/* collecting element group equation numbers */
void ParticlePairT::Equations(AutoArrayT<const iArray2DT*>& eq_1,
	AutoArrayT<const RaggedArray2DT<int>*>& eq_2)
{
#pragma unused(eq_1)

	/* dimension equations array */
	fEqnos.Configure(fNeighbors, NumDOF());

	/* get local equations numbers */
	Field().SetLocalEqnos(fNeighbors, fEqnos);

	/* add to list of equation numbers */
	eq_2.Append(&fEqnos);
}

/* class initialization */
void ParticlePairT::Initialize(void)
{
	/* inherited */
	ParticleT::Initialize();

	ParticleT::SetRefNN(NearestNeighbors,RefNearestNeighbors);

	/* dimension */
	int ndof = NumDOF();
	fLHS.Dimension(2*ndof);
	fRHS.Dimension(2*ndof);

	/* constant matrix needed to calculate stiffness */
	fOneOne.Dimension(fLHS);
	dMatrixT one(ndof);
	one.Identity();
	fOneOne.SetBlock(0, 0, one);
	fOneOne.SetBlock(ndof, ndof, one);
	one *= -1;
	fOneOne.SetBlock(0, ndof, one);
	fOneOne.SetBlock(ndof, 0, one);
}

/* collecting element geometry connectivities */
void ParticlePairT::ConnectsX(AutoArrayT<const iArray2DT*>& connects) const
{
	/* NOTE: do not add anything to the geometry connectivity list */
#pragma unused(connects)
}

/* collecting element field connectivities */
void ParticlePairT::ConnectsU(AutoArrayT<const iArray2DT*>& connects_1,
	AutoArrayT<const RaggedArray2DT<int>*>& connects_2) const
{
#pragma unused(connects_1)
	connects_2.AppendUnique(&fNeighbors);
}

void ParticlePairT::WriteOutput(void)
{
	const char caller[] = "ParticlePairT::WriteOutput";

	/* inherited */
	ParticleT::WriteOutput();

	/* muli-processor information */
	CommManagerT& comm_manager = ElementSupport().CommManager();
	const ArrayT<int>* proc_map = comm_manager.ProcessorMap();
	int rank = ElementSupport().Rank();

	/* dimensions */
	int ndof = NumDOF();
	int num_output = ndof + 2; /* displacement + PE + KE */

#ifndef NO_PARTICLE_STRESS_OUTPUT
	num_output++; /* includes centrosymmetry */
	num_output+=ndof; /* some more for slip vector */
#endif

	/* number of nodes */
	const ArrayT<int>* parition_nodes = comm_manager.PartitionNodes();
	int non = (parition_nodes) ? 
		parition_nodes->Length() : 
		ElementSupport().NumNodes();

	/* map from partition node index */
	const InverseMapT* inverse_map = comm_manager.PartitionNodes_inv();

#ifndef NO_PARTICLE_STRESS_OUTPUT
	dSymMatrixT vs_i(ndof), temp(ndof);
	int num_stresses = vs_i.NumValues(ndof);
	//dArray2DT vsvalues(non, num_stresses);
	num_output += num_stresses;
	num_output += num_stresses; //another for the strain
#endif

	/* output arrays length number of active nodes */
	dArray2DT n_values(non, num_output), e_values;
	n_values = 0.0;

	/* global coordinates */
	const dArray2DT& coords = ElementSupport().CurrentCoordinates();

	/* pair properties function pointers */
	int current_property = -1;
	PairPropertyT::EnergyFunction energy_function = NULL;	
	PairPropertyT::ForceFunction force_function = NULL;

	/* the field */
	const FieldT& field = Field();
	const dArray2DT& displacement = field[0];
	const dArray2DT* velocities = NULL;
	if (field.Order() > 0) velocities = &(field[1]);

	/* atomic volume */
  	double V0 = 0.0;
  	if (NumSD() == 1)
    	V0 = fLatticeParameter;
	else if (NumSD() == 2)
		V0 = sqrt(3.0)*fLatticeParameter*fLatticeParameter/2.0; /* 2D hex */
	else /* 3D */
		V0 = fLatticeParameter*fLatticeParameter*fLatticeParameter/4.0; /* FCC */

	/* collect mass per particle */
	dArrayT mass(fNumTypes);
	for (int i = 0; i < fNumTypes; i++)
		mass[i] = fPairProperties[fPropertiesMap(i,i)]->Mass();

	/* collect displacements */
	dArrayT vec, values_i;
	for (int i = 0; i < non; i++) {
		int   tag_i = (parition_nodes) ? (*parition_nodes)[i] : i;
		int local_i = (inverse_map) ? inverse_map->Map(tag_i) : tag_i;
		int  type_i = fType[tag_i];

		/* values for particle i */
		n_values.RowAlias(local_i, values_i);

		/* copy in */
		vec.Set(ndof, values_i.Pointer());
		displacement.RowCopy(tag_i, vec);

#ifndef NO_PARTICLE_STRESS_OUTPUT
		/* kinetic contribution to the virial */
		if (velocities) {
			velocities->RowAlias(tag_i, vec);
			temp.Outer(vec);
		 	for (int cc = 0; cc < num_stresses; cc++) {
				int ndex = ndof+2+cc;
		   		values_i[ndex] = -mass[type_i]*temp[cc]/V0;
		 	}
		}
#endif
	}
	
	/* run through neighbor list */
	iArrayT neighbors;
	dArrayT x_i, x_j, r_ij(ndof);

#ifndef NO_PARTICLE_STRESS_OUTPUT
	dArrayT SlipVector(ndof);
	dMatrixT Strain(ndof);
#endif
	for (int i = 0; i < fNeighbors.MajorDim(); i++)
	{	    
		/* row of neighbor list */
		fNeighbors.RowAlias(i, neighbors);

		/* tags */
		int   tag_i = neighbors[0]; /* self is 1st spot */
		int  type_i = fType[tag_i];
		int local_i = (inverse_map) ? inverse_map->Map(tag_i) : tag_i;
		
		/* values for particle i */
		n_values.RowAlias(local_i, values_i);

#ifndef NO_PARTICLE_STRESS_OUTPUT
		vs_i = 0.0;
#endif
		
		/* kinetic energy */
		if (velocities) 
		{
			velocities->RowAlias(tag_i, vec);
			values_i[ndof+1] = 0.5*mass[type_i]*dArrayT::Dot(vec, vec);
		}

		/* run though neighbors for one atom - first neighbor is self
		 * to compute potential energy */
		coords.RowAlias(tag_i, x_i);

		for (int j = 1; j < neighbors.Length(); j++)
		{
			/* tags */
			int   tag_j = neighbors[j];
			int  type_j = fType[tag_j];
			
			/* set pair property (if not already set) */
			int property = fPropertiesMap(type_i, type_j);
			if (property != current_property) 
			{
				energy_function = fPairProperties[property]->getEnergyFunction();
				force_function = fPairProperties[property]->getForceFunction();
				current_property = property;
			}
		
			/* global coordinates */
			coords.RowAlias(tag_j, x_j);

			/* connecting vector */
			r_ij.DiffOf(x_j, x_i);
			double r = r_ij.Magnitude();

			/* split interaction energy */
			double uby2 = 0.5*energy_function(r, NULL, NULL); 
			values_i[ndof] += uby2;
			
	      	/* interaction force */
			double F = force_function(r, NULL, NULL);
			double Fbyr = F/r;

#ifndef NO_PARTICLE_STRESS_OUTPUT
			temp.Outer(r_ij);
			vs_i.AddScaled(0.5*Fbyr, temp);
#endif

			/* second node may not be on processor */
			if (!proc_map || (*proc_map)[tag_j] == rank) 
			{
				int local_j = (inverse_map) ? inverse_map->Map(tag_j) : tag_j;	
				if (local_j < 0 || local_j >= n_values.MajorDim())
					cout << caller << ": out of range: " << local_j << '\n';
				else {

					/* potential energy */
					n_values(local_j, ndof) += uby2;

#ifndef NO_PARTICLE_STRESS_OUTPUT
			 		/* accumulate into stress into array */
		 			for (int cc = 0; cc < num_stresses; cc++) {
						int ndex = ndof+2+cc;
		   				n_values(local_j, ndex) += 0.5*Fbyr*temp[cc]/V0;		   
		 			}
#endif
				}
			}
		}


#ifndef NO_PARTICLE_STRESS_OUTPUT
		/* copy stress into array */
		for (int cc = 0; cc < num_stresses; cc++) {
		  int ndex = ndof+2+cc;
		  values_i[ndex] += (vs_i[cc]/V0);
		}
#endif
	}
#ifndef NO_PARTICLE_STRESS_OUTPUT
    int num_s_vals = num_stresses+1+ndof+1;
    dArray2DT s_values(non,num_s_vals);
    s_values = 0.0;

    /* flag for specifying Lagrangian (0) or Eulerian (1) strain */ 
    const int kEulerLagr = 0;
	/* calculate slip vector and strain */
	Calc_Slip_and_Strain(non,num_s_vals,s_values,RefNearestNeighbors,kEulerLagr);

    /* calculate centrosymmetry parameter */
	Calc_CSP(non,num_s_vals,s_values,NearestNeighbors);

	/* combine strain, slip vector and centrosymmetry parameter into n_values list */
	for (int i = 0; i < fNeighbors.MajorDim(); i++)
	{
		/* row of neighbor list */
		fNeighbors.RowAlias(i, neighbors);

		/* tags */
		int   tag_i = neighbors[0]; /* self is 1st spot */
		int  type_i = fType[tag_i];
		int local_i = (inverse_map) ? inverse_map->Map(tag_i) : tag_i;
									            
		int valuep = 0;
		for (int is = 0; is < num_stresses; is++)
		{
			n_values(local_i,ndof+2+num_stresses+valuep++) = s_values(local_i,is);
		}

		/* recover J, the determinant of the deformation gradient, for atom i
		 * and divide stress values by it */
		double J = s_values(local_i,num_stresses);
	    for (int is = 0; is < num_stresses; is++) n_values(local_i,ndof+2+is) /= J;

		for (int n = 0; n < ndof; n++)
			n_values(local_i, ndof+2+num_stresses+num_stresses+n) = s_values(local_i,num_stresses+1+n);

		n_values(local_i, num_output-1) = s_values(local_i,num_s_vals-1);
	}
#endif

#if 0
	/* temporary to calculate crack propagation velocity */
	ifstreamT& in = ElementSupport().Input();
	ModelManagerT& model = ElementSupport().Model();
	const ArrayT<StringT> id_list = model.NodeSetIDs();
	iArrayT nodelist;
	dArray2DT partial;
	nodelist = model.NodeSet(id_list[id_list.Length()-1]); // want last nodeset
	const StringT& input_file = in.filename();
	fsummary_file.Root(input_file);
	fsummary_file.Append(".crack");
	double xcoord = coords(nodelist[0],0);
	double ydispcrit = .13;
	const double& time = ElementSupport().Time();
	for (int i = 0; i < nodelist.Length(); i++)
	{
	    int node = nodelist[i];
	    if (fabs(displacement(node,1)) >= ydispcrit)
	      xcoord = coords(node,0);
	}

	if (fopen)
	{
		fout.open_append(fsummary_file);
	  	fout.precision(13);
	  	fout << xcoord 
	  	     << setw(25) << time
	  	     << endl;
	}
	else
	{
	  	fout.open(fsummary_file);
		fopen = true;
	  	fout.precision(13);
	  	fout << "x-coordinate"
	  	     << setw(25) << "Time"
	  	     << endl;
	  	fout << xcoord 
	  	     << setw(25) << time
	  	     << endl;
	}
#endif

#if 0
	/* Temporary to calculate MD energy history and write to file */
	ifstreamT& in = ElementSupport().Input();
	ModelManagerT& model = ElementSupport().Model();
	const ArrayT<StringT> id_list = model.NodeSetIDs();
	iArrayT nodelist;
	dArray2DT partial;
	nodelist = model.NodeSet(id_list[id_list.Length()-1]);
	nodelist = model.NodeSet(id_list[3]);  // id_list[3]
	partial.Dimension(nodelist.Length(), n_values.MinorDim());
	partial.RowCollect(nodelist, n_values);
	const StringT& input_file = in.filename();
	fsummary_file.Root(input_file);
	fsummary_file2.Root(input_file);
	fsummary_file.Append(".sum");
	fsummary_file2.Append(".full");
	if (fopen)
	{
	        fout.open_append(fsummary_file);
			fout2.open_append(fsummary_file2);
			fout.precision(13);
			fout2.precision(13);
			fout << n_values.ColumnSum(3) 
				 << setw(25) << n_values.ColumnSum(2)
				 << setw(25) << n_values.ColumnSum(3) + n_values.ColumnSum(2)
				 << endl;
			fout2 << partial.ColumnSum(3) 
				 << setw(25) << partial.ColumnSum(2)
				 << setw(25) << partial.ColumnSum(3) + partial.ColumnSum(2)
				 << endl;
	}
	else
	{
			fout.open(fsummary_file);
			fout2.open(fsummary_file2);
			fopen = true;
			fout.precision(13);
			fout2.precision(13);
			fout << "Kinetic Energy"
				 << setw(25) << "Potential Energy"
				 << setw(25) << "Total Energy"
				 << endl;
			fout << n_values.ColumnSum(3) 
				 << setw(25) << n_values.ColumnSum(2)
				 << setw(25) << n_values.ColumnSum(3) + n_values.ColumnSum(2)
				 << endl;
			fout2 << "Kinetic Energy"
				 << setw(25) << "Potential Energy"
				 << setw(25) << "Total Energy"
				 << endl;
			fout2 << partial.ColumnSum(3) 
				 << setw(25) << partial.ColumnSum(2)
				 << setw(25) << partial.ColumnSum(3) + partial.ColumnSum(2)
				 << endl;
	}
#endif

	/* send */
	ElementSupport().WriteOutput(fOutputID, n_values, e_values);
}

/* compute the part of the stiffness matrix */
void ParticlePairT::FormStiffness(const InverseMapT& col_to_col_eq_row_map,
	const iArray2DT& col_eq, dSPMatrixT& stiffness)
{
	const char caller[] = "ParticlePairT::FormStiffness";

	/* map should return -1 of out of range */
	if (col_to_col_eq_row_map.OutOfRange() != InverseMapT::MinusOne)
		ExceptionT::GeneralFail(caller, "inverse map out of range should return -1");

	/* assembly information */
	const ElementSupportT& support = ElementSupport();
	int group = Group();
	int ndof = NumDOF();
	fLHS.Dimension(2*ndof);
		
	/* global coordinates */
	const dArray2DT& coords = support.CurrentCoordinates();

	/* pair properties function pointers */
	int current_property = -1;
	PairPropertyT::ForceFunction force_function = NULL;
	PairPropertyT::StiffnessFunction stiffness_function = NULL;

	/* work space */
	dArrayT r_ij(NumDOF(), fRHS.Pointer());
	dArrayT r_ji(NumDOF(), fRHS.Pointer() + NumDOF());

	/* run through neighbor list */
	const iArray2DT& field_eqnos = Field().Equations();
	iArrayT row_eqnos, col_eqnos; 
	iArrayT neighbors;
	dArrayT x_i, x_j;
	for (int i = 0; i < fNeighbors.MajorDim(); i++)
	{
		/* row of neighbor list */
		fNeighbors.RowAlias(i, neighbors);

		/* type */
		int  tag_i = neighbors[0]; /* self is 1st spot */
		int type_i = fType[tag_i];
		
		/* particle equations */
		field_eqnos.RowAlias(tag_i, row_eqnos);

		/* run though neighbors for one atom - first neighbor is self */
		coords.RowAlias(tag_i, x_i);
		for (int j = 1; j < neighbors.Length(); j++)
		{
			/* global tag */
			int tag_j = neighbors[j];
			
			/* particle is a target column */
			int col_eq_index = col_to_col_eq_row_map.Map(tag_j);
			if (col_eq_index != -1)
			{
				/* more particle info */
				int type_j = fType[tag_j];

				/* particle equations */
				col_eq.RowAlias(col_eq_index, col_eqnos);

				/* set pair property (if not already set) */
				int property = fPropertiesMap(type_i, type_j);
				if (property != current_property)
				{
					force_function = fPairProperties[property]->getForceFunction();
					stiffness_function = fPairProperties[property]->getStiffnessFunction();
					current_property = property;
				}

				/* global coordinates */
				coords.RowAlias(tag_j, x_j);

				/* connecting vector */
				r_ij.DiffOf(x_j, x_i);
				double r = r_ij.Magnitude();
				r_ji.SetToScaled(-1.0, r_ij);

				/* interaction functions */
				double F = force_function(r, NULL, NULL);
				double K = stiffness_function(r, NULL, NULL);
				double Fbyr = F/r;

				/* 1st term */
				fLHS.Outer(fRHS, fRHS, (K - Fbyr)/r/r);

				/* 2nd term */				
				fLHS.AddScaled(Fbyr, fOneOne);

				/* assemble */
				for (int p = 0; p < row_eqnos.Length(); p++)
					for (int q = 0; q < col_eqnos.Length(); q++)
						stiffness.AddElement(row_eqnos[p]-1, col_eqnos[q]-1, fLHS(p,q));
			}
		}
	}
}

/* describe the parameters needed by the interface */
void ParticlePairT::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	ParticleT::DefineParameters(list);
}

/* information about subordinate parameter lists */
void ParticlePairT::DefineSubs(SubListT& sub_list) const
{
	/* inherited */
	ParticleT::DefineSubs(sub_list);

	/* the pair properties - array of choices */
	sub_list.AddSub("property_list", ParameterListT::OnePlus, true);
}

/* return the description of the given inline subordinate parameter list */
void ParticlePairT::DefineInlineSub(const StringT& sub, ParameterListT::ListOrderT& order, 
	SubListT& sub_sub_list) const
{
	if (sub == "property_list")
	{
		order = ParameterListT::Choice;
		
		/* harmonic pair potential */
		sub_sub_list.AddSub("harmonic");

		/* Lennard-Jones 6/12 */
		sub_sub_list.AddSub("Lennard_Jones");

		/* Paradyn pair potential */
		sub_sub_list.AddSub("Paradyn_pair");

		/* Matsui pair potential */
		sub_sub_list.AddSub("Matsui");
	}
	else /* inherited */
		ParticleT::DefineInlineSub(sub, order, sub_sub_list);
}

/* a pointer to the ParameterInterfaceT of the given subordinate */
ParameterInterfaceT* ParticlePairT::NewSub(const StringT& list_name) const
{
	/* try to construct potential */
	PairPropertyT* pair_property = New_PairProperty(list_name, false);
	if (pair_property)
		return pair_property;
	else /* inherited */
		return ParticleT::NewSub(list_name);
}

/***********************************************************************
 * Protected
 ***********************************************************************/

/* return a new pair property or NULL if the name is invalid */
PairPropertyT* ParticlePairT::New_PairProperty(const StringT& name, bool throw_on_fail) const
{
	if (name == "harmonic")
		return new HarmonicPairT;
	else if (name == "Lennard_Jones")
		return new LennardJonesPairT;
	else if (name == "Paradyn_pair")
		return new ParadynPairT;
	else if (name == "Matsui")
		return new MatsuiPairT;
	else if (throw_on_fail)
		ExceptionT::GeneralFail("ParticlePairT::New_PairProperty",
			"unrecognized potential \"%s\"", name.Pointer());
		
	return NULL;
}

/* generate labels for output data */
void ParticlePairT::GenerateOutputLabels(ArrayT<StringT>& labels) const
{
  int ndof=NumDOF();
	if (ndof > 3) ExceptionT::GeneralFail("ParticlePairT::GenerateOutputLabels");

	/* displacement labels */
	const char* disp[3] = {"D_X", "D_Y", "D_Z"};
	const char* SV[3] = {"SV_X", "SV_Y", "SV_Z"};
	int num_labels =
		ndof // displacements
		+ 2;     // PE and KE

#ifndef NO_PARTICLE_STRESS_OUTPUT
	int num_stress=0;
	const char* stress[6];
	const char* strain[6];
	if (ndof==3){
	  num_stress=6;
	  stress[0]="s11";
	  stress[1]="s22";
	  stress[2]="s33";
	  stress[3]="s23";
	  stress[4]="s13";
	  stress[5]="s12";
	  }
	  else if (ndof==2) {
	   num_stress=3;
	  stress[0]="s11";
	  stress[1]="s22";
	  stress[2]="s12";
	  }
	  else if (ndof==1) {
	   num_stress=1;
	  stress[0] = "s11";
	  }
	if (ndof==3){
	  
	  strain[0]="e11";
	  strain[1]="e12";
	  strain[2]="e13";
	  strain[3]="e22";
	  strain[4]="e23";
	  strain[5]="e33";
	  }
	  else if (ndof==2) {
	   
	  strain[0]="e11";
	  strain[1]="e12";
	  strain[2]="e22";
	  }
	  else if (ndof==1) {

	  strain[0] = "e11";
	  }
	num_labels+=num_stress;
	num_labels++; //another label for the centrosymmetry
	num_labels+=num_stress; //another for the strain
	num_labels+=ndof; /*and another for the slip vector*/
#endif /* NO_PARTICLE_STRESS_OUTPUT */
	
	labels.Dimension(num_labels);
	int dex = 0;
	for (dex = 0; dex < NumDOF(); dex++)
		labels[dex] = disp[dex];
	labels[dex++] = "PE";
	labels[dex++] = "KE";

#ifndef NO_PARTICLE_STRESS_OUTPUT
	for (int ns =0 ; ns<num_stress; ns++)
	  labels[dex++]=stress[ns];
	for (int ns =0 ; ns<num_stress; ns++)
	  labels[dex++]=strain[ns];
	for (int i=0; i<ndof; i++)
	  labels[dex++]=SV[i];
	labels[dex++]= "CS";
#endif /* NO_PARTICLE_STRESS_OUTPUT */
}

/* form group contribution to the stiffness matrix */
void ParticlePairT::LHSDriver(GlobalT::SystemTypeT sys_type)
{
	/* time integration parameters */
	double constK = 0.0;
	double constM = 0.0;
	int formK = fIntegrator->FormK(constK);
	int formM = fIntegrator->FormM(constM);

	/* assemble particle mass */
	if (formM) {

		/* collect mass per particle */
		dArrayT mass(fNumTypes);
		for (int i = 0; i < fNumTypes; i++)
			mass[i] = fPairProperties[fPropertiesMap(i,i)]->Mass();
		mass *= constM;
	
		AssembleParticleMass(mass);
	}
	
	/* assemble diagonal stiffness */
	if (formK && sys_type == GlobalT::kDiagonal)
	{
		/* assembly information */
		const ElementSupportT& support = ElementSupport();
		int group = Group();
		int ndof = NumDOF();
	
		/* global coordinates */
		const dArray2DT& coords = support.CurrentCoordinates();

		/* pair properties function pointers */
		int current_property = -1;
		PairPropertyT::ForceFunction force_function = NULL;
		PairPropertyT::StiffnessFunction stiffness_function = NULL;

		/* run through neighbor list */
		fForce = 0.0;
		iArrayT neighbors;
		dArrayT x_i, x_j, r_ij(ndof);
		for (int i = 0; i < fNeighbors.MajorDim(); i++)
		{
			/* row of neighbor list */
			fNeighbors.RowAlias(i, neighbors);

			/* type */
			int  tag_i = neighbors[0]; /* self is 1st spot */
			int type_i = fType[tag_i];
			double* k_i = fForce(tag_i);
		
			/* run though neighbors for one atom - first neighbor is self */
			coords.RowAlias(tag_i, x_i);
			for (int j = 1; j < neighbors.Length(); j++)
			{
				/* global tag */
				int  tag_j = neighbors[j];
				int type_j = fType[tag_j];
				double* k_j = fForce(tag_j);
			
				/* set pair property (if not already set) */
				int property = fPropertiesMap(type_i, type_j);
				if (property != current_property)
				{
					force_function = fPairProperties[property]->getForceFunction();
					stiffness_function = fPairProperties[property]->getStiffnessFunction();
					current_property = property;
				}
		
				/* global coordinates */
				coords.RowAlias(tag_j, x_j);
		
				/* connecting vector */
				r_ij.DiffOf(x_j, x_i);
				double r = r_ij.Magnitude();
			
				/* interaction functions */
				double F = force_function(r, NULL, NULL);
				double K = stiffness_function(r, NULL, NULL);
				K = (K < 0.0) ? 0.0 : K;

				double Fbyr = F/r;
				for (int k = 0; k < ndof; k++)
				{
					double r_k = r_ij[k]*r_ij[k]/r/r;
					double K_k = constK*(K*r_k + Fbyr*(1.0 - r_k));
					k_i[k] += K_k;
					k_j[k] += K_k;
				}
			}
		}

		/* assemble */
		support.AssembleLHS(group, fForce, Field().Equations());
	}
	else if (formK)
	{
		/* assembly information */
		const ElementSupportT& support = ElementSupport();
		int group = Group();
		int ndof = NumDOF();
		fLHS.Dimension(2*ndof);
		
		/* global coordinates */
		const dArray2DT& coords = support.CurrentCoordinates();

		/* pair properties function pointers */
		int current_property = -1;
		PairPropertyT::ForceFunction force_function = NULL;
		PairPropertyT::StiffnessFunction stiffness_function = NULL;

		/* work space */
		dArrayT r_ij(NumDOF(), fRHS.Pointer());
		dArrayT r_ji(NumDOF(), fRHS.Pointer() + NumDOF());

		/* run through neighbor list */
		const iArray2DT& field_eqnos = Field().Equations();
		iArray2DT pair_eqnos(2, ndof); 
		iArrayT pair(2);
		iArrayT neighbors;
		dArrayT x_i, x_j;
		for (int i = 0; i < fNeighbors.MajorDim(); i++)
		{
			/* row of neighbor list */
			fNeighbors.RowAlias(i, neighbors);

			/* type */
			int  tag_i = neighbors[0]; /* self is 1st spot */
			int type_i = fType[tag_i];
			pair[0] = tag_i;
		
			/* run though neighbors for one atom - first neighbor is self */
			coords.RowAlias(tag_i, x_i);
			for (int j = 1; j < neighbors.Length(); j++)
			{
				/* global tag */
				int  tag_j = neighbors[j];
				int type_j = fType[tag_j];
				pair[1] = tag_j;
			
				/* set pair property (if not already set) */
				int property = fPropertiesMap(type_i, type_j);
				if (property != current_property)
				{
					force_function = fPairProperties[property]->getForceFunction();
					stiffness_function = fPairProperties[property]->getStiffnessFunction();
					current_property = property;
				}
		
				/* global coordinates */
				coords.RowAlias(tag_j, x_j);
		
				/* connecting vector */
				r_ij.DiffOf(x_j, x_i);
				double r = r_ij.Magnitude();
				r_ji.SetToScaled(-1.0, r_ij);
			
				/* interaction functions */
				double F = constK*force_function(r, NULL, NULL);
				double K = constK*stiffness_function(r, NULL, NULL);
				double Fbyr = F/r;

				/* 1st term */
				fLHS.Outer(fRHS, fRHS, (K - Fbyr)/r/r);
		
				/* 2nd term */
				fLHS.AddScaled(Fbyr, fOneOne);
				
				/* assemble */
				pair_eqnos.RowCollect(pair, field_eqnos);
				support.AssembleLHS(group, fLHS, pair_eqnos);
			}
		}
	}
}

/* form group contribution to the residual */
void ParticlePairT::RHSDriver(void)
{
	int nsd = NumSD();
	if (nsd == 3)
		RHSDriver3D();
	else if (nsd == 2)
		RHSDriver2D();
	else
		ExceptionT::GeneralFail("ParticlePairT::RHSDriver");
		
	ApplyDamping(fNeighbors);
	
	/* assemble */
	ElementSupport().AssembleRHS(Group(), fForce, Field().Equations());
}

void ParticlePairT::RHSDriver2D(void)
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

	//TEMP - interial force not implemented
	if (formMa) ExceptionT::GeneralFail(caller, "inertial force not implemented");

	/* assembly information */
	const ElementSupportT& support = ElementSupport();
	int group = Group();
	int ndof = NumDOF();
	
	/* global coordinates */
	const dArray2DT& coords = support.CurrentCoordinates();

	/* pair properties function pointers */
	int current_property = -1;
	PairPropertyT::ForceFunction force_function = NULL;
	const double* Paradyn_table = NULL;
	double dr = 1.0;
	int row_size = 0, num_rows = 0;

	/* run through neighbor list */
	fForce = 0.0;
	iArrayT neighbors;
	for (int i = 0; i < fNeighbors.MajorDim(); i++)
	{
		/* row of neighbor list */
		fNeighbors.RowAlias(i, neighbors);

		/* type */
		int   tag_i = neighbors[0]; /* self is 1st spot */
		int  type_i = fType[tag_i];
		double* f_i = fForce(tag_i);
		const double* x_i = coords(tag_i);
		
		/* run though neighbors for one atom - first neighbor is self */
		for (int j = 1; j < neighbors.Length(); j++)
		{
			/* global tag */
			int   tag_j = neighbors[j];
			int  type_j = fType[tag_j];
			double* f_j = fForce(tag_j);
			const double* x_j = coords(tag_j);

			/* set pair property (if not already set) */
			int property = fPropertiesMap(type_i, type_j);
			if (property != current_property)
			{
				if (!fPairProperties[property]->getParadynTable(&Paradyn_table, dr, row_size, num_rows))
					force_function = fPairProperties[property]->getForceFunction();
				current_property = property;
			}
		
			/* connecting vector */
			double r_ij_0 = x_j[0] - x_i[0];
			double r_ij_1 = x_j[1] - x_i[1];
			double r = sqrt(r_ij_0*r_ij_0 + r_ij_1*r_ij_1);
			
			/* interaction force */
			double F;
			if (Paradyn_table)
			{
				double pp = r*dr;
				int kk = int(pp);
				int max_row = num_rows-2;
				kk = (kk < max_row) ? kk : max_row;
				pp -= kk;
				pp = (pp < 1.0) ? pp : 1.0;				
				const double* c = Paradyn_table + kk*row_size;
				F = c[4] + pp*(c[5] + pp*c[6]);
			}
			else
				F = force_function(r, NULL, NULL);
			double Fbyr = formKd*F/r;

			r_ij_0 *= Fbyr;
			f_i[0] += r_ij_0;
			f_j[0] +=-r_ij_0;

			r_ij_1 *= Fbyr;
			f_i[1] += r_ij_1;
			f_j[1] +=-r_ij_1;
		}
	}

}

void ParticlePairT::RHSDriver3D(void)
{
	/* function name */
	const char caller[] = "ParticlePairT::RHSDriver3D";

	/* check 3D */
	if (NumDOF() != 3) ExceptionT::GeneralFail(caller, "3D only: %d", NumDOF());

	/* time integration parameters */
	double constMa = 0.0;
	double constKd = 0.0;
	int formMa = fIntegrator->FormMa(constMa);
	int formKd = fIntegrator->FormKd(constKd);

	//TEMP - interial force not implemented
	if (formMa) ExceptionT::GeneralFail(caller, "inertial force not implemented");

	/* assembly information */
	const ElementSupportT& support = ElementSupport();
	int group = Group();
	int ndof = NumDOF();
	
	/* global coordinates */
	const dArray2DT& coords = support.CurrentCoordinates();

	/* pair properties function pointers */
	int current_property = -1;
	PairPropertyT::ForceFunction force_function = NULL;
	const double* Paradyn_table = NULL;
	double dr = 1.0;
	int row_size = 0, num_rows = 0;

	/* run through neighbor list */
	fForce = 0.0;
	iArrayT neighbors;
	for (int i = 0; i < fNeighbors.MajorDim(); i++)
	{
		/* row of neighbor list */
		fNeighbors.RowAlias(i, neighbors);

		/* type */
		int   tag_i = neighbors[0]; /* self is 1st spot */
		int  type_i = fType[tag_i];
		double* f_i = fForce(tag_i);
		const double* x_i = coords(tag_i);
		
		/* run though neighbors for one atom - first neighbor is self */
		for (int j = 1; j < neighbors.Length(); j++)
		{
			/* global tag */
			int   tag_j = neighbors[j];
			int  type_j = fType[tag_j];
			double* f_j = fForce(tag_j);
			const double* x_j = coords(tag_j);

			/* set pair property (if not already set) */
			int property = fPropertiesMap(type_i, type_j);
			if (property != current_property)
			{
				if (!fPairProperties[property]->getParadynTable(&Paradyn_table, dr, row_size, num_rows))
					force_function = fPairProperties[property]->getForceFunction();
				current_property = property;
			}
		
			/* connecting vector */
			double r_ij_0 = x_j[0] - x_i[0];
			double r_ij_1 = x_j[1] - x_i[1];
			double r_ij_2 = x_j[2] - x_i[2];
			double r = sqrt(r_ij_0*r_ij_0 + r_ij_1*r_ij_1 + r_ij_2*r_ij_2);
			
			/* interaction force */
			double F;
			if (Paradyn_table)
			{
				double pp = r*dr;
				int kk = int(pp);
				int max_row = num_rows-2;
				kk = (kk < max_row) ? kk : max_row;
				pp -= kk;
				pp = (pp < 1.0) ? pp : 1.0;				
				const double* c = Paradyn_table + kk*row_size;
				F = c[4] + pp*(c[5] + pp*c[6]);
			}
			else
				F = force_function(r, NULL, NULL);
			double Fbyr = formKd*F/r;

			r_ij_0 *= Fbyr;
			f_i[0] += r_ij_0;
			f_j[0] +=-r_ij_0;

			r_ij_1 *= Fbyr;
			f_i[1] += r_ij_1;
			f_j[1] +=-r_ij_1;

			r_ij_2 *= Fbyr;
			f_i[2] += r_ij_2;
			f_j[2] +=-r_ij_2;
		}
	}

}

/* set neighborlists */
void ParticlePairT::SetConfiguration(void)
{
	/* inherited */
	ParticleT::SetConfiguration();

	/* reset neighbor lists */
	CommManagerT& comm_manager = ElementSupport().CommManager();
	const ArrayT<int>* part_nodes = comm_manager.PartitionNodes();
	if (fActiveParticles) 
		part_nodes = fActiveParticles;
	GenerateNeighborList(part_nodes, NearestNeighborDistance, NearestNeighbors, true, true);
	GenerateNeighborList(part_nodes, fNeighborDistance, fNeighbors, false, true);


	/* output stream */
	ofstreamT& out = ElementSupport().Output();

	/* write the search grid statistics */
	if (fGrid) fGrid->WriteStatistics(out);
	
	out << "\n Neighbor statistics:\n";
	out << " Total number of neighbors . . . . . . . . . . . = " << fNeighbors.Length() << '\n';
	out << " Minimum number of neighbors . . . . . . . . . . = " << fNeighbors.MinMinorDim(0) << '\n';
	out << " Maximum number of neighbors . . . . . . . . . . = " << fNeighbors.MaxMinorDim() << '\n';
	if (fNeighbors.MajorDim() > 0)
	out << " Average number of neighbors . . . . . . . . . . = " << double(fNeighbors.Length())/fNeighbors.MajorDim() << '\n';
	else
	out << " Average number of neighbors . . . . . . . . . . = " << 0 << '\n';

	/* verbose */
	if (ElementSupport().PrintInput())
	{
		out << " Neighbor lists (self as leading neighbor):\n";
		out << setw(kIntWidth) << "row" << "  n..." << '\n';
		iArrayT tmp(fNeighbors.Length(), fNeighbors.Pointer());
		tmp++;
		fNeighbors.WriteNumbered(out);
		tmp--;
		out.flush();
	}
}

/* construct the list of properties from the given input stream */
void ParticlePairT::EchoProperties(ifstreamT& in, ofstreamT& out)
{
	/* read potentials */
	int num_potentials = -1;
	in >> num_potentials;
	fPairProperties.Dimension(num_potentials);
	fPairProperties = NULL;
	for (int i = 0; i < fPairProperties.Length(); i++)
	{
		ParticlePropertyT::TypeT property;
		in >> property;
		switch (property)
		{
			case ParticlePropertyT::kHarmonicPair:
			{
				double mass, R0, K;
				in >> mass >> R0 >> K;
				fPairProperties[i] = new HarmonicPairT(mass, R0, K);
				break;
			}
			case ParticlePropertyT::kLennardJonesPair:
			{
				double mass, eps, sigma, alpha;
				in >> mass >> eps >> sigma >> alpha;
				fPairProperties[i] = new LennardJonesPairT(mass, eps, sigma, alpha);
				break;
			}
			case ParticlePropertyT::kParadynPair:
			{
				StringT file;
				in >> file;
				file.ToNativePathName();

				StringT path;
				path.FilePath(in.filename());				
				file.Prepend(path);
			
				fPairProperties[i] = new ParadynPairT(file);
				break;
			}
			case ParticlePropertyT::kMatsuiPair:
			{
				double mass, sqr_q, two_A, two_B, sqr_C, f, rc;
				in >> mass >> sqr_q >> two_A >> two_B >> sqr_C >> f >> rc;
				fPairProperties[i] = new MatsuiPairT(mass, sqr_q, two_A, two_B, sqr_C, f, rc);
				break;
			}
			default:
				ExceptionT::BadInputValue("ParticlePairT::ReadProperties", 
					"unrecognized property type: %d", property);
		}
		

	}

	/* echo particle properties */
	out << "\n Particle properties:\n\n";
	out << " Number of properties. . . . . . . . . . . . . . = " << fPairProperties.Length() << '\n';
	for (int i = 0; i < fPairProperties.Length(); i++)
	{
		out << " Property: " << i+1 << '\n';
		fPairProperties[i]->Write(out);
	}
	
	/* copy into base class list */
	fParticleProperties.Dimension(fPairProperties.Length());
	for (int i = 0; i < fPairProperties.Length(); i++)
		fParticleProperties[i] = fPairProperties[i];
}


  
