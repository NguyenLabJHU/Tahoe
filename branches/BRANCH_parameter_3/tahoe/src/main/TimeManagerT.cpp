/* $Id: TimeManagerT.cpp,v 1.19.22.1 2004-04-08 07:33:48 paklein Exp $ */
/* created: paklein (05/23/1996) */
#include "TimeManagerT.h"

#include <iostream.h>
#include <iomanip.h>
#include <math.h>

#include "fstreamT.h"

#include "toolboxConstants.h"
#include "FEManagerT.h"
#include "TimeSequence.h"
#include "dArrayT.h"

using namespace Tahoe;

/* constructor */
TimeManagerT::TimeManagerT(FEManagerT& FEM):
	ParameterInterfaceT("time"),
	theBoss(FEM),

	/* runtime data for the current sequence */
	fCurrentSequence(-1),
	fStepNum(0),
	fTime(0.0),
	fNumStepCuts(0),
	fStepCutStatus(kSameStep),
	
	fNumSteps(0), fOutputInc(1), fMaxCuts(0), fTimeStep(1.0),
	fIsTimeShifted(0), fTimeShift(0.0)
{

}

/* initialization */
void TimeManagerT::Initialize(void)
{
	ifstreamT& in  = theBoss.Input();
	ostream&   out = theBoss.Output();

	/* Time sequences - allocate memory and echo */
	int num_sequences = -1;
	in >> num_sequences;	
	if (num_sequences < 1) throw ExceptionT::kBadInputValue;
	fSequences.Dimension(num_sequences);
	EchoTimeSequences(in, out);
	
	/* Loadtime functions - allocate memory and echo */	
	int num_LTf = -1;
	in >> num_LTf;
	if (num_LTf < 0) throw ExceptionT::kBadInputValue;
	fSchedule.Dimension(num_LTf); // add: f(t) = 1.0

//	EchoSchedule(in, out);
	
	/* console variables */
	iSetName("time");
	iAddVariable("num_steps", fNumSteps);
	iAddVariable("output_inc", fOutputInc);
	iAddVariable("max_step_cuts", fMaxCuts);
	iAddVariable("time_step", fTimeStep);
}

/* run through the time sequences.  NextSequence returns 0
* if there are no more time sequences */
void TimeManagerT::Top(void)
{
#pragma message("remove me")
	fCurrentSequence = -1;
}

bool TimeManagerT::NextSequence(void)
{
#pragma message("remove me")
	fCurrentSequence++;
	
	/* initialize next sequence */
	if (fCurrentSequence < fSequences.Length())
	{
		TimeSequence& curr_sequence = fSequences[fCurrentSequence];
		
		/* runtime data for the current sequence */
		fStepNum       = 0;
		fTime          = 0.0;
		fNumStepCuts   = 0;
		fStepCutStatus = kSameStep;
		
		/* copy from current sequence */
		fNumSteps  = curr_sequence.fNumSteps;
		fOutputInc = curr_sequence.fOutputInc;
		fMaxCuts   = curr_sequence.fMaxCuts;
		fTimeStep  = curr_sequence.fTimeStep;
		
		/* broadcast time step */
		theBoss.SetTimeStep(fTimeStep);

		/* print headers */
		theBoss.Output() << "\n T i m e   S e q u e n c e : ";
		theBoss.Output() << fCurrentSequence + 1 << "\n\n";
		cout << "\n T i m e   S e q u e n c e : ";
		cout << fCurrentSequence + 1 << endl;
		
		/* see if all Integrators are explicit */
#pragma message ("delete me")
#if 0
		fImpExp = IntegratorT::kExplicit;
		for (int i = 0; fImpExp == IntegratorT::kExplicit && 
			i < theBoss.NumIntegrators(); i++)
			fImpExp = theBoss.Integrator(i)->ImplicitExplicit();
#endif
		
		return true;
	}
	else
		return false;
}

/* advance one time step, return 0 when if the sequence
* has ended */
bool TimeManagerT::Step(void)
{
	/* check that time has not been shifted */
	if (fIsTimeShifted) throw ExceptionT::kGeneralFail;

	if (fStepNum < fNumSteps)
	{
		/* adjust time step */
		if (fStepCutStatus == kDecreaseStep)
			DoDecreaseStep();
		else if (fStepCutStatus == kIncreaseStep)
			DoIncreaseStep();
			
		/* advance time */
		fStepNum++;
		IncrementTime(fTimeStep);
		
		/* print less often for explicit */
		GlobalT::AnalysisCodeT analysiscode = theBoss.Analysis();
		//bool is_explicit = fImpExp == IntegratorT::kExplicit;
		bool is_explicit = (fNumSteps > 100);
		
		/* verbose flag */
		bool write_header = !is_explicit     ||
		                    fOutputInc == -1 ||
		                    fOutputInc == 0  ||
		                    fmod(double(fStepNum), fOutputInc) == 0;

		
		/* step header */
		int start_count = 5;
		if (write_header || (is_explicit && fStepNum <= start_count))
	{
			cout << "\n     Time: " << fTime << '\n';
			cout <<   "Step size: " << fTimeStep << '\n';
			cout <<   "     Step: " << fStepNum << " of " << fNumSteps << '\n';
			cout << endl;
		}
		if (is_explicit && fStepNum == start_count && fOutputInc > 1 && fStepNum < fNumSteps)
			cout << " (marker at output increments only)" << endl;
				
		return true;
	}
	else
		/* reached end of time sequence */
		return false;
}

void TimeManagerT::ResetStep(void)
{
	const char caller[] = "TimeManagerT::ResetStep";

	/* check that time has not been shifted */
	if (fIsTimeShifted) ExceptionT::GeneralFail(caller);

	/* too far */
	if (fStepNum == 0)
		 ExceptionT::GeneralFail(caller, "already at the start time");
	/* return to previous time */
	else
	{		
		IncrementTime(-fTimeStep);
		fStepNum--;
	}
}

/* load control functions */
bool TimeManagerT::DecreaseLoadStep(void)
{
	if (fNumStepCuts >= fMaxCuts)
	{
		cout << "\n TimeManagerT::DecreaseLoadStep: load increment cut ";
		cout << fMaxCuts << " times.\n";
		cout << "\n Exiting time sequence\n" << endl;

		/* end current time sequence */
		fStepNum = fNumSteps;		
		return false;
	}
	else
	{
		/* set flag */
		fStepCutStatus = kDecreaseStep;		
		return true;
	}
}

bool TimeManagerT::IncreaseLoadStep(void)
{
	/* set flag */
	if (fNumStepCuts > 0  &&		/* only as big as original */
		IsEven(fStepNum)  && 		/* step number divisible by 2 */		
		IsEven(fNumSteps) && 		/* total steps divisible by 2 */
	    fStepNum < fNumSteps )		/* not the last step */
	{
		fStepCutStatus = kIncreaseStep;
		return true;
	}
	else
		return false;
}

/* return a pointer to the specified ScheduleT function */
ScheduleT* TimeManagerT::Schedule(int num) const
{
	/* range check */
	if (num < 0 || num >= fSchedule.Length())
		ExceptionT::OutOfRange("TimeManagerT::Schedule", "schedule %d is out of range {0,%d}",
			num, fSchedule.Length() - 1);

	return fSchedule[num];
}

double TimeManagerT::ScheduleValue(int num) const
{
	return fSchedule[num]->Value();
}

/* initialize/restart functions
*
* Initialize functions reset all kinematic data to the
* default initial system state.  The restart functions
* should read/write any data that overrides the default
* values */
void TimeManagerT::ReadRestart(istream& restart_in)
{	
	int sequencenumber;
	restart_in >> sequencenumber;
	if (sequencenumber != fCurrentSequence) throw ExceptionT::kBadInputValue;

#if 0 
	/* total desired simulation time */ 
	double tot_time = fTimeStep*fNumSteps; 
         
	restart_in >> fTime >> fNumStepCuts >> fTimeStep; 
	cout << " Restarting from time: " << fTime << '\n';

	/* reset number of steps to preserve the total time */ 
	double float_steps = tot_time/fTimeStep; 
	fNumSteps = int((2.0*float_steps + 1.0)/2.0); 
#endif 
 
	//TEMP - do no use the time step written in the restart file
	//       will become restart option
	double time_step; 
	restart_in >> fTime >> fNumStepCuts >> time_step; 
	cout << " Restarting from time: " << fTime << '\n';  
	cout << "   Previous step cuts: " << fNumStepCuts << '\n';  
	cout << "   Previous step size: " << time_step << '\n';  
	cout << "    Current step size: " << fTimeStep << '\n';
        
	/* set load factors */
	for (int i = 0; i < fSchedule.Length(); i++)
		fSchedule[i]->SetTime(fTime);
}

void TimeManagerT::WriteRestart(ostream& restart_out) const
{
	restart_out << fCurrentSequence << '\n';
	restart_out << fTime            << '\n';
	restart_out << fNumStepCuts     << '\n';
	restart_out << fTimeStep        << '\n';
}

/* return true if output should be written for the current step */
bool TimeManagerT::WriteOutput(void) const
{
	if (fabs(fTimeStep) > kSmall && /* no output if clock is not running */
		fOutputInc != 0 &&
	   		(fmod(double(fStepNum), fOutputInc) == 0 || /* at increment */
	    	fStepNum == fNumSteps) /* at end */
	)
		return true;
	else
		return false;
}

/* describe the parameters needed by the interface */
void TimeManagerT::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	ParameterInterfaceT::DefineParameters(list);

	/* number of steps */
	ParameterT num_steps(ParameterT::Integer, "num_steps");
	num_steps.AddLimit(0, LimitT::LowerInclusive);
	list.AddParameter(num_steps);

	/* results output increment */
	ParameterT output_inc(ParameterT::Integer, "output_inc");
	output_inc.AddLimit(0, LimitT::LowerInclusive);
	list.AddParameter(output_inc);

	/* maximum number of time increment cuts */
	ParameterT max_step_cuts(ParameterT::Integer, "max_step_cuts");
	max_step_cuts.SetDefault(0);
	max_step_cuts.AddLimit(0, LimitT::LowerInclusive);
	list.AddParameter(max_step_cuts);

	/* (initial) time increment */
	ParameterT time_step(ParameterT::Double, "time_step");
	time_step.AddLimit(0, LimitT::LowerInclusive);
	list.AddParameter(time_step);
}

/* accept parameter list */
void TimeManagerT::TakeParameterList(const ParameterListT& list)
{
	/* inherited */
	ParameterInterfaceT::TakeParameterList(list);

	//TEMP - remove multiple time sequences
	fSequences.Dimension(1);
	TimeSequence& seq = fSequences[0];
	seq.fNumSteps  = list.GetParameter("num_steps");
	seq.fOutputInc = list.GetParameter("output_inc");
	seq.fMaxCuts   = list.GetParameter("max_step_cuts");
	seq.fTimeStep  = list.GetParameter("time_step");	

	/* console variables */
	iSetName("time");
	iAddVariable("num_steps", fNumSteps);
	iAddVariable("output_inc", fOutputInc);
	iAddVariable("max_step_cuts", fMaxCuts);
	iAddVariable("time_step", fTimeStep);

	/* construct schedule functions */
	fSchedule.Dimension(list.NumLists("schedule_function"));
	for (int i = 0; i < fSchedule.Length(); i++)
	{
		fSchedule[i] = new ScheduleT;
		fSchedule[i]->TakeParameterList(*(list.List("schedule_function", i)));
		fSchedule[i]->SetTime(0.0);
	}
}

/* information about subordinate parameter lists */
void TimeManagerT::DefineSubs(SubListT& sub_list) const
{
	/* inherited */
	ParameterInterfaceT::DefineSubs(sub_list);

	/* schedule function */
	sub_list.AddSub("schedule_function", ParameterListT::Any);
}

/* a pointer to the ParameterInterfaceT of the given subordinate */
ParameterInterfaceT* TimeManagerT::NewSub(const StringT& list_name) const
{
	if (list_name == "schedule_function")
		return new ScheduleT;
	else /* inherited */
		return ParameterInterfaceT::NewSub(list_name);
}

/************************************************************************
 * Private
 ************************************************************************/

void TimeManagerT::EchoTimeSequences(ifstreamT& in, ostream& out)
{
	int num_seq = fSequences.Length();
	out << "\n T i m e   S e q u e n c e   D a t a :\n\n";
	out << " Number of time sequences  . . . . . . . . . . . = " << num_seq;
	out << "\n\n";
	
	for (int i = 0; i < num_seq; i++)
	{
		int seqnum;
		in >> seqnum;	
		if (seqnum < 1 ||
		    seqnum > num_seq) throw ExceptionT::kBadInputValue;

		out << " Sequence number . . . . . . . . . . . . . . . . = ";
		out << seqnum << '\n';
		
		/* echo data */
		seqnum--;
		fSequences[seqnum].Read(in);
		fSequences[seqnum].Write(out);
		out << '\n';
	}
}

#if 0
void TimeManagerT::EchoSchedule(ifstreamT& in, ostream& out)
{
	int num_LTf = fSchedule.Length();
	out << "\n L o a d - T i m e   F u n c t i o n   D a t a :\n\n";
	out << " Number of load-time functions . . . . . . . . . = " << num_LTf << '\n';

	for (int i = 0; i < num_LTf; i++)
	{
		int LTfnum, numpts;
		in >> LTfnum >> numpts;

		/* checks */
		if (LTfnum < 1 || LTfnum > num_LTf) throw ExceptionT::kBadInputValue;
		if (numpts < 1) throw ExceptionT::kBadInputValue;

		out << " Loadtime function number. . . . . . . . . . . . = ";
		out << LTfnum << "\n\n";
		
		/* echo data */
		LTfnum--;
		fSchedule[LTfnum] = new ScheduleT(numpts);
		if (!fSchedule[LTfnum]) throw ExceptionT::kOutOfMemory;

		fSchedule[LTfnum]->Read(in);
		fSchedule[LTfnum]->Write(out);
		out << '\n';
		
		/* initialize time */
		fSchedule[LTfnum]->SetTime(0.0);
	}
}
#endif

/* increment the time and reset the load factors */
void TimeManagerT::IncrementTime(double dt)
{
	/* Increment the time */
	fTime += dt;

	/* set load factors */
	for (int i = 0; i < fSchedule.Length(); i++)
		fSchedule[i]->SetTime(fTime);
}

/* returns 1 if the number is even, otherwise returns 0	*/
int TimeManagerT::IsEven(int number) const
{
	return (fmod(double(number),2) < 0.5) ? 1 : 0;
}

/* adjust time stepping parameters */
void TimeManagerT::DoDecreaseStep(void)
{
	fNumStepCuts++;
	fStepCutStatus = kSameStep;

	/* set new time step parameters */
	fTimeStep    /= 2;
	fOutputInc   *= (fOutputInc < 0) ? 1 : 2;
	fNumSteps    *= 2;
	fStepNum     *= 2;
	
	/* broadcast time step change */
	theBoss.SetTimeStep(fTimeStep);
	cout << "\n TimeManagerT: step reduced to: " << fTimeStep << endl;
}

void TimeManagerT::DoIncreaseStep(void)
{
	fNumStepCuts--;
	fStepCutStatus = kSameStep;
	
	/* set new time step parameters */
	fTimeStep    *= 2;
	fOutputInc   /= (fOutputInc < 0) ? 1 : 2;
	fNumSteps    /= 2;	
	fStepNum     /= 2;

	/* broadcast time step change */
	theBoss.SetTimeStep(fTimeStep);

	cout << "\n TimeManagerT: step increased to: " << fTimeStep << endl;
}

/* to allow LinearHHTalpha to adjust the time.  LinearHHTalpha must
* also restore then time when finished */
void TimeManagerT::ShiftTime(double dt)
{
	/* set flag */
	fIsTimeShifted = 1;
	fTimeShift   += dt;

	IncrementTime(dt);
}

/* reset the time back to what it was before the calls to IncrementTime */
void TimeManagerT::ResetTime(void)
{
	/* unset flag */
	fIsTimeShifted = 0;

	IncrementTime(-fTimeShift);
	fTimeShift = 0.0;
}
