/* $Id: nTrapezoid.cpp,v 1.2.4.5 2002-06-05 09:25:31 paklein Exp $ */
/* created: paklein (10/03/1999) */

#include "nTrapezoid.h"
#include "dArrayT.h"
#include "iArrayT.h"
#include "iArray2DT.h"
#include "dArray2DT.h"
#include "KBC_CardT.h"
#include "BasicFieldT.h"

/* constructor */
nTrapezoid::nTrapezoid(void) { }

/* consistent BC's */
void nTrapezoid::ConsistentKBC(BasicFieldT& field, const KBC_CardT& KBC)
{
	/* destinations */
	int node = KBC.Node();
	int dof  = KBC.DOF();
	double& d = (field[0])(node, dof);
	double& v = (field[1])(node, dof);
	
	switch ( KBC.Code() )
	{
		case KBC_CardT::kFix: /* zero displacement */
		case KBC_CardT::kDsp: /* prescribed displacement */
		{
			double d_next = KBC.Value();
			v = (d_next - d)/dcorr_v;
			d = d_next;
			break;
		}
		case KBC_CardT::kVel: /* prescribed velocity */
		{
			v  = KBC.Value();
			d += dcorr_v*v;
			break;
		}
		default:
			cout << "\n nTrapezoid::ConsistentKBC:unknown BC code\n" << endl;
			throw eBadInputValue;
	}
}		

/* predictors - map ALL */
void nTrapezoid::Predictor(BasicFieldT& field)
{
	/* displacement predictor */
	field[0].AddScaled(dpred_v, field[1]);
	
	/* velocity predictor */
	field[1] = 0.0;
}		

/* correctors - map ACTIVE */
void nTrapezoid::Corrector(BasicFieldT& field, const dArrayT& update, 
	int eq_start, int num_eq)
{
	const iArray2DT& eqnos = field.Equations();

	/* add update - assumes that fEqnos maps directly into dva */
	int    *peq = eqnos.Pointer();
	double *pd  = field[0].Pointer();
	double *pv  = field[1].Pointer();
	for (int i = 0; i < eqnos.Length(); i++)
	{
		int eq = *peq++ - eq_start;
		/* active dof */
		if (eq > -1 && eq < num_eq)
		{
			double v = update[eq];
			*pd += dcorr_v*v;
			*pv += v;
		}
		pd++;
		pv++;
	}
}

void nTrapezoid::MappedCorrector(BasicFieldT& field, const iArrayT& map, 
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

		double* pd = (field[0])(row);
		double* pv = (field[1])(row);
		for (int j = 0; j < minordim; j++)
		{
			/* active */
			if (*pflag > 0)
			{
				double dv = *pupdate - *pv; 
/* NOTE: update is the total v_n+1, so we need to recover dv, the velocity
 *       increment. Due to truncation errors, this will not match the update
 *       applied in nTrapezoid::Corrector exactly. Therefore, ghosted nodes
 *       will not have exactly the same trajectory as their images. The solution
 *       would be to expand da to the full field, and send it. Otherwise, we
 *       could develop maps from the update vector to each communicated outgoing
 *       packet. Or {d,v} could be recalculated from t_n here and in Corrector,
 *       though this would require the data from t_n. */

				*pd += dcorr_v*dv;
				*pv  = *pupdate; /* a_n+1 matches exactly */
			}
			
			/* next */
			pflag++; pupdate++; pd++; pv++;
		}
	}
}

/* return the field array needed by nControllerT::MappedCorrector. */
const dArray2DT& nTrapezoid::MappedCorrectorField(BasicFieldT& field) const
{
	return field[1];
}

/* pseudo-boundary conditions for external nodes */
KBC_CardT::CodeT nTrapezoid::ExternalNodeCondition(void) const
{
	return KBC_CardT::kVel;
}

/***********************************************************************
* Protected
***********************************************************************/

/* recalculate time stepping constants */
void nTrapezoid::nComputeParameters(void)
{
	/* predictor */
	dpred_v = 0.5*fdt;

	/* corrector */
	dcorr_v = 0.5*fdt;
}
