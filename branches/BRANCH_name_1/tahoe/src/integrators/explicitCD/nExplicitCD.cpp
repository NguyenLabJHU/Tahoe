/* $Id: nExplicitCD.cpp,v 1.3.2.1 2002-06-27 18:02:30 cjkimme Exp $ */
/* created: paklein (03/23/1997) */

#include "nExplicitCD.h"
#include "iArrayT.h"
#include "dArrayT.h"
#include "iArray2DT.h"
#include "dArray2DT.h"
#include "KBC_CardT.h"
#include "BasicFieldT.h"

/* constructor */

using namespace Tahoe;

nExplicitCD::nExplicitCD(void) { }

/* consistent BC's - updates predictors and acceleration only */
void nExplicitCD::ConsistentKBC(BasicFieldT& field, const KBC_CardT& KBC)
{
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
			a = 0.0; //correct?	
			break;
		}
		
		case KBC_CardT::kVel: /* prescribed velocity */
		{
			double v_next = KBC.Value();
	
			a = (v_next - v)/vcorr_a;
			v = v_next;
			break;
		}
		
		case KBC_CardT::kAcc: /* prescribed acceleration */
		{
			a  = KBC.Value();
			v += vcorr_a*a;
			break;
		}

		default:
		
			cout << "\n nExplicitCD::ConsistentKBC:unknown BC code\n" << endl;
			throw eBadInputValue;
	}
}		

/* predictors - map ALL */
void nExplicitCD::Predictor(BasicFieldT& field)
{
	/* displacement predictor */
	field[0].AddCombination(dpred_v, field[1], dpred_a, field[2]);	

	/* velocity predictor */
	field[1].AddScaled(vpred_a, field[2]);
}		

/* correctors - map ACTIVE */
void nExplicitCD::Corrector(BasicFieldT& field, const dArrayT& update, 
	int eq_start, int num_eq)
{
	const iArray2DT& eqnos = field.Equations();

	/* add update - assumes that fEqnos maps directly into dva */
	int    *peq = eqnos.Pointer();
	double *pv  = field[1].Pointer();
	double *pa  = field[2].Pointer();
	for (int i = 0; i < eqnos.Length(); i++)
	{
		int eq = *peq++ - eq_start;
		
		/* active dof */
		if (eq > -1 && eq < num_eq)
		{
			double a = update[eq];
			*pv += vcorr_a*a;
			*pa = a;
		}
		pv++;
		pa++;
	}
}

void nExplicitCD::MappedCorrector(BasicFieldT& field, const iArrayT& map, 
	const iArray2DT& flags, const dArray2DT& update)
{
	/* run through map */
	int minordim = flags.MinorDim();
	const int* pflag = flags.Pointer();
	const double* pupdate = update.Pointer();
	for (int i = 0; i < map.Length(); i++)
	{
		int row = map[i];
		int* pflags = flags(i);

		double* pv = (field[1])(row);
		double* pa = (field[2])(row);
		for (int j = 0; j < minordim; j++)
		{
			/* active */
			if (*pflag > 0)
			{
				double a = *pupdate;
				*pv += vcorr_a*a;
				*pa = a;
			}
			
			/* next */
			pflag++; pupdate++; pv++; pa++;
		}
	}
}

/* return the field array needed by nControllerT::MappedCorrector. */
const dArray2DT& nExplicitCD::MappedCorrectorField(BasicFieldT& field) const
{
	return field[2];
}

/* pseudo-boundary conditions for external nodes */
KBC_CardT::CodeT nExplicitCD::ExternalNodeCondition(void) const
{
	return KBC_CardT::kAcc;
}

/***********************************************************************
* Protected
***********************************************************************/

/* recalculate time stepping constants */
void nExplicitCD::nComputeParameters(void)
{
	/* predictor */
	dpred_v		= fdt;
	dpred_a		= 0.5*fdt*fdt;
	vpred_a		= 0.5*fdt;
	
	/* corrector */
	vcorr_a		= 0.5*fdt;
}
