/* $Id: TimeManagerT.h,v 1.5 2002-07-05 22:28:07 paklein Exp $ */
/* created: paklein (05/23/1996) */

#ifndef _TIMEMANAGER_T_H_
#define _TIMEMANAGER_T_H_

/* base class */
#include "iConsoleObjectT.h"

/* direct members */
#include "StringT.h"
#include "pArrayT.h"
#include "ScheduleT.h"
#include "iAutoArrayT.h"
#include "IOBaseT.h"
#include "TimeSequence.h"

namespace Tahoe {

/* forward declarations */
class ifstreamT;
class FEManagerT;
class CoordinatorT;
class nLinearHHTalpha;
class NodeManagerT;
class ControllerT;

class TimeManagerT: public iConsoleObjectT
{
	/* time shifters */
	friend class LinearHHTalpha;
	friend class NLHHTalpha;

public:

	/** enum of integrator types */
	enum CodeT {
		kLinearStatic = 0,
		      kStatic = 1,
           kTrapezoid = 2,
           kLinearHHT = 3,
		kNonlinearHHT = 4,
		  kExplicitCD = 5
	};

	/** stream extraction operator */
	friend istream& operator>>(istream& in, TimeManagerT::CodeT& code);

	/* constructor */
	TimeManagerT(FEManagerT& FEM);

	/* run through the time sequences */
	void Top(void);
	bool NextSequence(void);

	/* time sequence */
	bool Step(void);
	void ResetStep(void);
	const int& StepNumber(void) const;
	const int& NumberOfSteps(void) const;
	const double& Time(void) const;
	const double& TimeStep(void) const;

	/* load control functions (returns true if successful) */
	bool DecreaseLoadStep(void);
	bool IncreaseLoadStep(void);
	
	/* return a pointer to the specified ScheduleT function */

	/** \name schedule information */
	/*@{*/
	int NumSchedule(void) const;
	ScheduleT* Schedule(int num) const;
	double ScheduleValue(int num) const;
	/*@}*/

	/* accessors */
	int SequenceNumber(void) const;
	int NumSequences(void) const;
			
	/* initialize/restart functions
	 *
	 * Initialize functions reset all kinematic data to the
	 * default initial system state.  The restart functions
	 * should read/write any data that overrides the default
	 * values */
	void ReadRestart(istream& in);
	void WriteRestart(ostream& out) const;

	/* finalize step (trigger output) */
	void CloseStep(void); //TEMP? - let FEManager control/monitor output?

	/** return a pointer to a integrator of the specified type */
	ControllerT* New_Controller(CodeT type) const;

private:	

	/** step cut status flags */
	enum StatusT { kDecreaseStep =-1,
                       kSameStep = 0,
                   kIncreaseStep = 1};
	
	/* output functions */
	void EchoTimeSequences(ifstreamT& in, ostream& out);
	void EchoSchedule(ifstreamT& in, ostream& out);

	/* increment the time and reset the load factors */
	void IncrementTime(double dt);

	/* returns 1 if the number is even, otherwise returns 0 */
	int IsEven(int number) const;
	
	/* adjust time stepping parameters */
	void DoDecreaseStep(void);
	void DoIncreaseStep(void);

private:

	FEManagerT& theBoss;
	
	ArrayT<TimeSequence> fSequences;
	pArrayT<ScheduleT*>  fSchedule;

	/* copied from current sequence */
	int	   fNumSteps;
	int	   fOutputInc;
	int	   fMaxCuts;
	double fTimeStep;
	
	/* runtime data for the current sequence */
	int	   fCurrentSequence;
	int	   fStepNum;
	double fTime;
	int    fNumStepCuts;
	int	   fStepCutStatus;
	int    fOutputCount;	
	
	/* time stepper */
	int	   fIsTimeShifted;
	double fTimeShift;

/* functions for time shifters */
private:

	/* to allow LinearHHTalpha to adjust the time.  LinearHHTalpha must
	 * call ResetTime when finished.  MUST call ResetTime before the next call
	 * to Step */
	void ShiftTime(double dt);
	
	/* reset the time back to what it was before the calls to IncrementTime */
	void ResetTime(void);
};

/* inlines */
inline const double& TimeManagerT::Time(void) const { return fTime; }
inline const double& TimeManagerT::TimeStep(void) const { return fTimeStep; }
inline const int& TimeManagerT::StepNumber(void) const { return fStepNum; }
inline const int& TimeManagerT::NumberOfSteps(void) const { return fNumSteps; }

inline int TimeManagerT::NumSchedule(void) const { return fSchedule.Length() ; }
inline int TimeManagerT::SequenceNumber(void) const { return fCurrentSequence; }
inline int TimeManagerT::NumSequences(void) const { return fSequences.Length(); }
} // namespace Tahoe 
#endif /* _TIMEMANAGER_T_H_ */
