/* $Id: PenaltyRegionT.cpp,v 1.21.8.2 2005-07-03 05:39:52 paklein Exp $ */
/* created: paklein (04/30/1998) */
#include "PenaltyRegionT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>
#include <ctype.h>

#include "ofstreamT.h"
#include "GlobalT.h"
#include "FieldT.h"
#include "ModelManagerT.h"
#include "CommManagerT.h"

#include "ScheduleT.h"
#include "eIntegratorT.h"
#include "IOBaseT.h"
#include "OutputSetT.h"
#include "CommunicatorT.h"

#include "ParameterContainerT.h"
#include "ParameterUtils.h"
#include "FieldSupportT.h"
#include "SecantMethodT.h"

using namespace Tahoe;

const double Pi = acos(-1.0);

PenaltyRegionT::PenaltyRegionT(void):
	fMass(0.0),
	fLTf(NULL),
	fOutputID(-1),
	fk(-1.0),
	fRollerDirection(-1),
	fSecantSearch(NULL),
	fContactNodes_space(25),
	fContactArea(0.0),
	fContactEqnos_man(10, fContactEqnos),
	fdContactNodesGroup2D(10, false, 0),
	fdContactNodesGroup(10, false)
{
	/* set memory groups */
	fdContactNodesGroup.Register(fGap);
//	fdContactNodesGroup.Register(fTempNumNodes);
	fdContactNodesGroup.Register(fNodalAreas);
	fdContactNodesGroup2D.Register(fContactForce2D);
}

/* destructor */
PenaltyRegionT::~PenaltyRegionT(void) { delete fSecantSearch; }

/* form of tangent matrix */
GlobalT::SystemTypeT PenaltyRegionT::TangentType(void) const
{
	/* no tangent */
	return GlobalT::kDiagonal;
}

void PenaltyRegionT::Equations(AutoArrayT<const iArray2DT*>& eq_1,
	AutoArrayT<const RaggedArray2DT<int>*>& eq_2)
{
	/* inherited */
	FBC_ControllerT::Equations(eq_1, eq_2);

	/* NOTE - just collect equation numbers, don't append
	 * since contact nodes only act on self */

	/* collect contact equation numbers */
	const iArray2DT& eqnos = Field().Equations();
	iArray2DT eqtemp(fContactNodes.Length(), eqnos.MinorDim(), fContactEqnos.Pointer());
	
	/* set current equation number */
	eqtemp.RowCollect(fContactNodes, eqnos);
}

void PenaltyRegionT::InitialCondition(void)
{
	/* initialize position and velocity */
	fx = fx0;
	fv = fv0;
	
	/* initialize history */
	fxlast = fx;
	fvlast = fv;
}

void PenaltyRegionT::ReadRestart(istream& in)
{
	/* inherited */
	FBC_ControllerT::ReadRestart(in);

	in >> fx;     // position
	in >> fv;     // velocity
	in >> fxlast; // last converged position
	in >> fvlast; // last converged velocity
}

void PenaltyRegionT::WriteRestart(ostream& out) const
{
	/* inherited */
	FBC_ControllerT::WriteRestart(out);

	out << fx << '\n';     // position
	out << fv << '\n';     // velocity
	out << fxlast << '\n'; // last converged position
	out << fvlast << '\n'; // last converged velocity
}

/* (re-)set the configuration */
GlobalT::InitStatusT PenaltyRegionT::UpdateConfiguration(void)
{
	GlobalT::InitStatusT status = FBC_ControllerT::UpdateConfiguration();

	/* get contact nodes */
	ModelManagerT& model = FieldSupport().ModelManager();
	model.ManyNodeSets(fNodeID, fContactNodes_man);
	fContactNodes.Alias(fContactNodes_man);

	/* remove "external" nodes */
	CommManagerT& comm = FieldSupport().CommManager();
	const ArrayT<int>* p_map = comm.ProcessorMap();
	if (fContactNodes.Length() > 0 && p_map)
	{
		/* wrap it */
		iArrayT processor;
		processor.Alias(*p_map);

		/* collect processor nodes */
		fContactNodes_space.Dimension(0);
		int nnd = fContactNodes.Length();
		int rank = comm.Rank();
		for (int i = 0; i < nnd; i++)
			if (processor[fContactNodes[i]] == rank)
				fContactNodes_space.Append(fContactNodes[i]);

		/* reset list */
		if (fContactNodes_space.Length() != nnd) {
			fContactNodes_man = fContactNodes_space;
			fContactNodes.Alias(fContactNodes_man);
		}
	}

	/* no changes */
	if (fContactNodes == fContactNodes_last) 
		return status;
	else
		fContactNodes_last = fContactNodes;
	
	/* dimension workspace */
	int nnd = fContactNodes.Length();
	int ndof = Field().NumDOF();	
	fdContactNodesGroup.Dimension(nnd, false);
	fdContactNodesGroup2D.Dimension(nnd, ndof);
	
	/* write contact nodes */
	ofstreamT& out = FieldSupport().Output();
	out << " Number of contact nodes . . . . . . . . . . . . = "
	    << fContactNodes.Length() << endl;	
	if (FieldSupport().PrintInput()) {
		iArrayT tmp;
		tmp.Alias(fContactNodes);
		tmp++;
		out << tmp.wrap(5) << '\n';
		tmp--;				
	}	

	/* collect volume element block ID's containing the strikers */
	ArrayT<StringT> element_id_all;
	model.ElementGroupIDsWithNodes(fContactNodes, element_id_all);
	iArrayT volume_element(element_id_all.Length());
	for (int i = 0; i < element_id_all.Length(); i++) {
	        GeometryT::CodeT geom = model.ElementGroupGeometry(element_id_all[i]);
	        volume_element[i] = (GeometryT::GeometryToNumSD(geom) == FieldSupport().NumSD()) ? 1 : 0;
	}
	int count = 0;
	ArrayT<StringT> element_id(volume_element.Count(1));
	for (int i = 0; i < element_id_all.Length(); i++)
	        if (volume_element[i])
	                element_id[count++] = element_id_all[i];

	/* compute nodal areas */
	FieldSupport().ModelManager().ComputeNodalArea(fContactNodes,
		element_id, fNodalAreas, fGlobal2Local);

	/* allocate memory for equation numbers */
	fContactEqnos_man.SetLength(nnd*ndof, false);
	
	/* allocate memory for force vector */
	fContactForce.Alias(nnd*ndof, fContactForce2D.Pointer());
	fContactForce2D = 0.0;

	/* initialize gaps */
	fGap = 0.0;

	/* return */
	return GlobalT::MaxPrecedence(status, GlobalT::kNewInteractions);
}

/* compute the nodal contribution to the residual force vector */
void PenaltyRegionT::ApplyRHS(void)
{
	double constKd = 0.0;
	int formKd = fIntegrator->FormKd(constKd);
	if (!formKd) return;

	/* recompute contact forces */
	ComputeContactForce(constKd);

	/* assemble */
	FieldSupport().AssembleRHS(fGroup, fContactForce, fContactEqnos);
}

/* apply kinematic boundary conditions */
void PenaltyRegionT::InitStep(void)
{
	/* the time step */
	double time_step = FieldSupport().TimeStep();

	/* store */
	double x;
	if (fRollerDirection != -1)
		x = fx[fRollerDirection];

	/* compute impulse acting on region */
	if (fSlow == kImpulse)
		for (int i = 0; i < fContactForce2D.MinorDim(); i++)
			fv[i] -= time_step*fContactForce2D.ColumnSum(i)/fMass;
	else if (fSlow == kSchedule)
		fv.SetToScaled(fLTf->Value(), fv0);
	
	/* compute new position */
	fx.AddScaled(time_step, fv);

	/* override motion */
	if (fRollerDirection != -1)
		fx[fRollerDirection] = x;
	
	/* reset search */
	if (fSecantSearch) fSecantSearch->Reset();
}

/* finalize step */
void PenaltyRegionT::CloseStep(void)
{
	/* store position and velocity */
	fxlast = fx;
	fvlast = fv;
}

/* reset to the last known solution */
void PenaltyRegionT::Reset(void)
{
	/* restore position and velocity */
	fx = fxlast;
	fv = fvlast;
}

/* update constrain forces */
GlobalT::RelaxCodeT PenaltyRegionT::RelaxSystem(void)
{
	GlobalT::RelaxCodeT relax = FBC_ControllerT::RelaxSystem();
	
	/* re-center */
	GlobalT::RelaxCodeT my_relax = GlobalT::kNoRelax;
	if (fRollerDirection != -1)
	{
		int num_its = fSecantSearch->Iterations();
		if (num_its == -1) {
			double error = -fContactForce2D.ColumnSum(fRollerDirection);
			if (fabs(error) > kSmall)
			{
				/* init guess 1 */
				fSecantSearch->NextPoint(fx[fRollerDirection], error);
				
				/* next position */
				//fx[fRollerDirection] += (error/fabs(error))*fRadius/100.0;
				fx[fRollerDirection] += error/fk;
				
				/* keep solving */
				my_relax = GlobalT::kRelax;
			}	
		}
		else if (num_its == 0)
		{
			/* init guess 2 */
			fSecantSearch->NextPoint(fx[fRollerDirection], -fContactForce2D.ColumnSum(fRollerDirection));
		
			/* next position */
			fx[fRollerDirection] = fSecantSearch->NextGuess();

			/* keep solving */
			my_relax = GlobalT::kRelax;
		}
		else
		{
			/* check error */
			int status = fSecantSearch->NextPoint(fx[fRollerDirection], -fContactForce2D.ColumnSum(fRollerDirection));			
			if (status == SecantMethodT::kContinue)
			{
				/* next position */
				fx[fRollerDirection] = fSecantSearch->NextGuess();

				/* keep solving */
				my_relax = GlobalT::kRelax;
			}
			else if (status == SecantMethodT::kConverged)
				my_relax = GlobalT::kNoRelax;
			else
				ExceptionT::GeneralFail("PenaltyRegionT::RelaxSystem", "secant search failed");
		}
	}

	/* return */
	return GlobalT::MaxPrecedence(relax, my_relax);
}

/* register data for output */
void PenaltyRegionT::RegisterOutput(void)
{
	/* output labels */
	int ndof = Field().NumDOF();
	int num_output = ndof + /* displacements */
	                 ndof + /* force */
	                 1;     /* penetration depth */
	ArrayT<StringT> n_labels(num_output);
	const char* d_labels[] = {"D_X", "D_Y", "D_Z"};
	const char* f_labels[] = {"F_X", "F_Y", "F_Z"};
	int index = 0;
	for (int i = 0; i < ndof; i++)
		n_labels[index++] = d_labels[i];
	for (int i = 0; i < ndof; i++)
		n_labels[index++] = f_labels[i];
	n_labels[index] = "h";
	
	/* register output */
	OutputSetT output_set(fContactNodes, n_labels, FieldSupport().CommManager().PartitionNodesChanging());
	fOutputID = FieldSupport().RegisterOutput(output_set);
}

/* writing results */
void PenaltyRegionT::WriteOutput(ostream& out) const
{
	int d_width = out.precision() + kDoubleExtra;

	/* mp support */
	const CommunicatorT& comm = FieldSupport().Communicator();

	/* maximum penetration */
	double h_max = 0.0;
	if (fContactNodes.Length() > 0)
		h_max = fGap.Min();

	out << "\n P e n a l t y   R e g i o n   D a t a :\n\n";
	out << " Local maximum penetration. . . . . . . . . =\n"
	    << setw(d_width) << h_max << '\n';
	out << " Global maximum penetration . . . . . . . . =\n"
	    << setw(d_width) << comm.Min(h_max) << '\n';
	out << " Position . . . . . . . . . . . . . . . . . =\n" << fx << '\n';
	out << " Velocity . . . . . . . . . . . . . . . . . =\n" << fv << '\n';
	
	/* compute contact force */
	dArrayT loc_sum(fContactForce2D.MinorDim());
	for (int i = 0; i < fContactForce2D.MinorDim(); i++)
		loc_sum[i] = -fContactForce2D.ColumnSum(i);
	dArrayT global_sum(loc_sum.Length());
	comm.Sum(loc_sum, global_sum);

	/* write output */
	out << " Local contact force. . . . . . . . . . . . =\n";
	for (int i = 0; i < loc_sum.Length(); i++)
		out << setw(kDoubleWidth) << loc_sum[i] << '\n';

	out << " Glocal contact force . . . . . . . . . . . =\n";
	for (int i = 0; i < global_sum.Length(); i++)
		out << setw(kDoubleWidth) << global_sum[i] << '\n';

	out << " Contact Area . . . . . . . . . . . . . . . =\n" << fContactArea << '\n';

	/* collect output data */
	const dArray2DT& disp = Field()[0];
	int ndof = disp.MinorDim();
	int num_output = ndof + /* displacements */
	                 ndof + /* force */
	                 1;     /* penetration depth */
	dArray2DT n_values(fContactNodes.Length(), num_output);
	dArray2DT n_disp(fContactNodes.Length(), ndof);

	/* collect displacements */
	int index = 0;
	n_disp.RowCollect(fContactNodes, disp);
	for (int i = 0; i < ndof; i++)
		n_values.ColumnCopy(index++, n_disp, i);

	/* collect the forces */
	for (int i = 0; i < ndof; i++)
		n_values.ColumnCopy(index++, fContactForce2D, i);	

	/* collect gaps */
	n_values.SetColumn(index, fGap);

	/* send output */
	FieldSupport().WriteOutput(fOutputID, n_values);
}

/* describe the parameters needed by the interface */
void PenaltyRegionT::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	FBC_ControllerT::DefineParameters(list);

	ParameterT k(fk, "stiffness");
	k.AddLimit(0.0, LimitT::LowerInclusive);
	list.AddParameter(k);
}

/* information about subordinate parameter lists */
void PenaltyRegionT::DefineSubs(SubListT& sub_list) const
{
	/* inherited */
	FBC_ControllerT::DefineSubs(sub_list);

	/* position */
	sub_list.AddSub("bc_initial_position");	
	
	/* velocity */
	sub_list.AddSub("bc_velocity");

	/* motion control */
	sub_list.AddSub("motion_control_choice", ParameterListT::Once, true);	

	/* roller conditions */
	sub_list.AddSub("roller_condition", ParameterListT::ZeroOrOnce);	

	/* nodes */
	sub_list.AddSub("node_ID_list");
}

/* a pointer to the ParameterInterfaceT of the given subordinate */
ParameterInterfaceT* PenaltyRegionT::NewSub(const StringT& name) const
{
	if (name == "bc_initial_position")
	{
		ParameterContainerT* x_choice = new ParameterContainerT(name);
		
		/* by dimension */
		x_choice->SetListOrder(ParameterListT::Choice);
		x_choice->AddSub("Vector_2");
		x_choice->AddSub("Vector_3");
	
		return x_choice;
	}
	else if (name == "bc_velocity")
	{
		ParameterContainerT* v_choice = new ParameterContainerT(name);
		
		/* by dimension */
		v_choice->SetListOrder(ParameterListT::Choice);
		v_choice->AddSub("Vector_2");
		v_choice->AddSub("Vector_3");
	
		return v_choice;
	}
	else if (name == "motion_control_choice") {

		ParameterContainerT* motion = new ParameterContainerT(name);

		/* options */	
		motion->SetListOrder(ParameterListT::Choice);
		motion->AddSub(ParameterContainerT("velocity_constant"));
		ParameterContainerT v_sched("velocity_schedule");
		v_sched.AddParameter(ParameterT::Integer, "schedule");
		motion->AddSub(v_sched);
		ParameterContainerT v_slow("integrate_impulse");
		v_slow.AddParameter(ParameterT::Double, "mass");
		motion->AddSub(v_slow);
		
		return motion;
	}
	else if (name == "roller_condition")
	{
		ParameterContainerT* roller = new ParameterContainerT(name);
	
		roller->AddParameter(ParameterT::Integer, "direction");
		roller->AddParameter(ParameterT::Double, "force_tolerance");
	
		return roller;
	}
	else /* inherited */
		return FBC_ControllerT::NewSub(name);
}

/* accept parameter list */
void PenaltyRegionT::TakeParameterList(const ParameterListT& list)
{
	const char caller[] = "PenaltyRegionT::TakeParameterList";

	/* inherited */
	FBC_ControllerT::TakeParameterList(list);

	/* stiffness */
	fk = list.GetParameter("stiffness");

	/* dimension */
	int nsd = FieldSupport().NumSD();

	/* get parameters */
	const ParameterListT* roller = list.List("roller_condition");
	if (roller) {
		fRollerDirection = roller->GetParameter("direction");
		fRollerDirection--;
		if (fRollerDirection < 0 || fRollerDirection >= nsd)
			ExceptionT::GeneralFail(caller, "\"zero_force_direction\" %d is out of range {1,%d}",
				fRollerDirection, nsd+1);

		/* initialize secont search object */
		if (fSecantSearch) delete fSecantSearch;
		double tolerance = roller->GetParameter("force_tolerance");
		fSecantSearch = new SecantMethodT(100, tolerance);
	}

	/* initial position */
	const ParameterListT& x_vec = list.GetListChoice(*this, "bc_initial_position");
	VectorParameterT::Extract(x_vec, fx0);
	if (fx0.Length() != nsd) 
		ExceptionT::GeneralFail(caller, "\"bc_initial_position\" should be length %d not %d", nsd, fx0.Length());

	/* velocity */
	const ParameterListT& v_vec = list.GetListChoice(*this, "bc_velocity");
	VectorParameterT::Extract(v_vec, fv0);
	if (fv0.Length() != nsd) 
		ExceptionT::GeneralFail(caller, "\"bc_velocity\" should be length %d not %d", nsd, fv0.Length());
	
	/* motion control */
	fSlow = kConstantVelocity;
	const ParameterListT& motion = list.GetListChoice(*this, "motion_control_choice");
	if (motion.Name() == "velocity_schedule") {
		fSlow = kSchedule;
		int schedule = motion.GetParameter("schedule");
		schedule--;
		fLTf = FieldSupport().Schedule(schedule);
		if (!fLTf) ExceptionT::BadInputValue(caller, "could not resolve schedule %d", schedule+1);
	}
	else if (motion.Name() == "integrate_impulse") {
		fSlow = kImpulse;
		fMass = motion.GetParameter("mass");
	}

	/* affected nodes */
	StringListT::Extract(list.GetList("node_ID_list"), fNodeID);

	/* state variables */
	fx.Dimension(nsd);
	fv.Dimension(nsd);
	fxlast.Dimension(nsd);
	fvlast.Dimension(nsd);

	/* contact nodes need to be set ASAP is geometry is not changing */
	if (!FieldSupport().CommManager().PartitionNodesChanging())
		UpdateConfiguration();
}
