#include "nGear6.h"
#include "iArrayT.h"
#include "dArrayT.h"
#include "iArray2DT.h"
#include "KBC_CardT.h"
#include "BasicFieldT.h"

using namespace Tahoe;

/* constructor */
nGear6::nGear6(void)
{
  /* Gear constants */
  F02 = 3.0/16.0;
  F12 = 251.0/360.0;
  F32 = 11.0/18.0;
  F42 = 1.0/6.0;
  F52 = 1.0/60.0;
}

/* consistent BC's - updates predictors and acceleration only */
void nGear6::ConsistentKBC(BasicFieldT& field, const KBC_CardT& KBC)
{
	const char caller[] = "nGear6::ConsistentKBC";

	/* check */
	if (field.Order() != 6)
		ExceptionT::GeneralFail(caller, "field must be order 6: %d", field.Order());

	/* destinations */
	int node = KBC.Node();
	int dof  = KBC.DOF();
	double& d = (field[0])(node, dof);
	double& v = (field[1])(node, dof);
	double& a = (field[2])(node, dof);

	switch ( KBC.Code() )
	{
		case KBC_CardT::kFix: /* zero displacement */
		{
			d = 0.0;
			v = 0.0; //correct?	
			a = 0.0; //correct?	
			break;
		}
		case KBC_CardT::kDsp: /* prescribed displacement */
		{
			d = KBC.Value();
		   	break;
			/* NOTE:  haven't figured out a correct way to
			   compute velocities and accelerations given a
			   prescribed displacement...*/
		}
		
		case KBC_CardT::kVel: /* prescribed velocity */
		{
			double v_next = KBC.Value();
			// NEED ACTUAL, NOT PREDICTED ACCELERATION TO DEFINE
			// THIS KBC!!! (update array?)
			break;
		}
		
		case KBC_CardT::kAcc: /* prescribed acceleration */
		{
		        double a_next  = KBC.Value();
			v -= F12 * (a - a_next);
			d -= F02 * (a - a_next);
			a = a_next;
			break;
		}

		default:
			ExceptionT::BadInputValue(caller, "unknown BC code: %d", KBC.Code() );
	}
}		

/* predictors - map ALL */
void nGear6::Predictor(BasicFieldT& field)
{
	/* check */
	if (field.Order() != 6)
		ExceptionT::GeneralFail("nGear6::Predictor", "field must be order 6: %d", field.Order());

	/* unknowns */
	const iArray2DT& eqnos = field.Equations();

	/* fetch pointers */
	double* p0 = field[0].Pointer(); 
	double* p1 = field[1].Pointer();
	double* p2 = field[2].Pointer();
	double* p3 = field[3].Pointer();
	double* p4 = field[4].Pointer();
	double* p5 = field[5].Pointer();

	for (int i = 0; i < eqnos.Length(); i++)
	{
		(*p0++) += (*p1) + (*p2) + (*p3) + (*p4) + (*p5);
		(*p1++) += 2.0 * (*p2) + 3.0 * (*p3) + 4.0 * (*p4) + 5.0 * (*p5);
		(*p2++) += 3.0 * (*p3) + 6.0 * (*p4) + 10.0 * (*p5);
		(*p3++) += 4.0 * (*p4) + 10.0 * (*p5);
		(*p4++) += 5.0 * (*p5);
	}
}		

/* correctors - map ACTIVE */
void nGear6::Corrector(BasicFieldT& field, const dArrayT& update, 
	int eq_start, int num_eq)
{
	/* check */
	if (field.Order() != 6)
		ExceptionT::GeneralFail("nGear6::Corrector", "field must be order 6: %d", field.Order());

	const iArray2DT& eqnos = field.Equations();

	/* add update - assumes that fEqnos maps directly into dva */
	int    *peq = eqnos.Pointer();
	
	/* fetch pointers */
	double* p0 = field[0].Pointer(); 
	double* p1 = field[1].Pointer();
	double* p2 = field[2].Pointer();
	double* p3 = field[3].Pointer();
	double* p4 = field[4].Pointer();
	double* p5 = field[5].Pointer();

	for (int i = 0; i < eqnos.Length(); i++)
	{
		int eq = *peq++ - eq_start;
		
		/* active dof */
		if (eq > -1 && eq < num_eq)
		  {
		        double a = update[eq];
			double error = (*p2) - a;
		        (*p0++) -= error * F02;
			(*p1++) -= error * F12;
			(*p2++) -= error;
			(*p3++) -= error * F32; 
			(*p4++) -= error * F42;
			(*p5++) -= error * F52;
		}
	}
}

void nGear6::MappedCorrector(BasicFieldT& field, const iArrayT& map, 
	const iArray2DT& flags, const dArray2DT& update)
{
	/* check */
	if (field.Order() != 6)
		ExceptionT::GeneralFail("nGear6::MappedCorrector", "field must be order 6: %d", field.Order());

	/* run through map */
	int minordim = flags.MinorDim();
	const int* pflag = flags.Pointer();
	const double* pupdate = update.Pointer();

	/* fetch pointers */
	double* p3 = field[3].Pointer();
	double* p4 = field[4].Pointer();
	double* p5 = field[5].Pointer();

	for (int i = 0; i < map.Length(); i++)
	{
		int row = map[i];
		int* pflags = flags(i);
		
		double* p0 = (field[0])(row);
		double* p1 = (field[1])(row);
		double* p2 = (field[2])(row);
		for (int j = 0; j < minordim; j++)
		{
			/* active */
			if (*pflag > 0)
			{
                                double a = *pupdate;
				double error = (*p2) - a;
				(*p0++) -= error * F02;
				(*p1++) -= error * F12;
				(*p2++) -= error;
				(*p3++) -= error * F32;
				(*p4++) -= error * F42;
				(*p5++) -= error * F52;
			}
			
			/* next */
			pflag++; pupdate++; 
		}
	}
}

/* return the field array needed by nControllerT::MappedCorrector. */
const dArray2DT& nGear6::MappedCorrectorField(BasicFieldT& field) const
{
	/* check */
	if (field.Order() != 6)
		ExceptionT::GeneralFail("nGear6::MappedCorrectorField", "field must be order 6: %d", field.Order());

	return field[2];
}

/* pseudo-boundary conditions for external nodes */
KBC_CardT::CodeT nGear6::ExternalNodeCondition(void) const
{
	return KBC_CardT::kAcc;
}

/***********************************************************************
* Protected
***********************************************************************/

/* recalculate time stepping constants */
void nGear6::nComputeParameters(void)
{
  /* nothing implemented here */

}
