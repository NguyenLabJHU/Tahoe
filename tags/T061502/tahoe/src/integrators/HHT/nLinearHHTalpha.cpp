/* $Id: nLinearHHTalpha.cpp,v 1.4 2002-06-08 20:20:10 paklein Exp $ */
/* created: paklein (10/14/1996) */

#include "nLinearHHTalpha.h"
#include "dArrayT.h"
#include "iArrayT.h"
#include "iArray2DT.h"
#include "dArray2DT.h"
#include "KBC_CardT.h"
#include "BasicFieldT.h"

/* constructor */
nLinearHHTalpha::nLinearHHTalpha(ifstreamT& in, ostream& out, bool auto2ndorder):
	HHTalpha(in, out, auto2ndorder)
{

}

/* consistent BC's - updates predictors and acceleration only */
void nLinearHHTalpha::ConsistentKBC(BasicFieldT& field, const KBC_CardT& KBC)
{
	/* destinations */
	int node = KBC.Node();
	int dof  = KBC.DOF();
	double& d = (field[0])(node, dof);
	double& v = (field[1])(node, dof);
	double& a = (field[2])(node, dof);

	switch (KBC.Code())
	{
		case KBC_CardT::kFix: /* zero displacement */
		case KBC_CardT::kDsp: /* prescribed displacement */
		{
			double d_alpha = (1.0 + falpha)*KBC.Value() - falpha*dn(node,dof);

			a  = (d_alpha - d)/dalpha_a;
			d  = d_alpha;
			v += valpha_a*a;
		
			break;
		}
		
		case KBC_CardT::kVel: /* prescribed velocity */
		{
			double v_alpha = (1.0 + falpha)*KBC.Value() - falpha*vn(node,dof);
	
			a  = (v_alpha - v)/valpha_a;
			v  = v_alpha;
			d += dalpha_a*a;
	
			break;
		}
		
		case KBC_CardT::kAcc: /* prescribed acceleration */
		{
			a  = KBC.Value();
			d += dalpha_a*a;
			v += valpha_a*a;

			break;
		}

		default:
		
			cout << "\nnLinearHHTalpha::ConsistentKBC:unknown BC code\n" << endl;
			throw eBadInputValue;
	}
}		

/* predictors - map ALL */
void nLinearHHTalpha::Predictor(BasicFieldT& field)
{
	/* save values from t_n (need by HHT-alpha) */
	dn = field[0];
	vn = field[1];

	/* displacement predictor */
	field[0].AddCombination(dpred_v, field[1], dpred_a, field[2]);

	/* velocity predictor */
	field[1].AddScaled(vpred_a, field[2]);
	
	/* acceleration predictor */
	field[2] = 0.0;	
}		

/* correctors - map ACTIVE */
void nLinearHHTalpha::Corrector(BasicFieldT& field, const dArrayT& update, 
	int eq_start, int num_eq)
{
	/* displacement */
	field[0] *= dcorr_dpred;
	field[0].AddScaled(dcorr_d, dn);

	/* velocity */
	field[1] *= vcorr_vpred;
	field[1].AddScaled(vcorr_v, vn);

	/* add update - assumes that fEqnos maps directly into dva */
	const iArray2DT& eqnos = field.Equations();
	
	int   *peq = eqnos.Pointer();
	double *pd = field[0].Pointer();
	double *pv = field[1].Pointer();
	double *pa = field[2].Pointer();
	for (int i = 0; i < eqnos.Length(); i++)
	{
		int eq = *peq++ - eq_start;
		
		/* active dof */
		if (eq > -1 && eq < num_eq)
		{
			double a = update[eq];
		
			*pd += dcorr_a*a;
			*pv += vcorr_a*a;
			*pa = a;
		}		
		pd++;
		pv++;
		pa++;
	}
}

void nLinearHHTalpha::MappedCorrector(BasicFieldT& field, const iArrayT& map,
		const iArray2DT& flags, const dArray2DT& update)
{
	/* check dimensions */
	if (flags.MajorDim() != update.MajorDim() ||
	    flags.MinorDim() != update.MinorDim()) throw eSizeMismatch;

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
		double* pa = (field[2])(row);
		for (int j = 0; j < minordim; j++)
		{
			/* active */
			if (*pflag > 0)
			{
				double a = *pupdate;
			
				*pd += dcorr_a*a;
				*pv += vcorr_a*a;
				*pa = a;
			}
			
			/* next */
			pflag++; pupdate++; pd++; pv++; pa++;
		}
	}
}

/* return the field array needed by nControllerT::MappedCorrector. */
const dArray2DT& nLinearHHTalpha::MappedCorrectorField(BasicFieldT& field) const
{
	return field[2];
}

/* pseudo-boundary conditions for external nodes */
KBC_CardT::CodeT nLinearHHTalpha::ExternalNodeCondition(void) const
{
	return KBC_CardT::kAcc;
}

/***********************************************************************
* Protected
***********************************************************************/

/* recalculate time stepping constants */
void nLinearHHTalpha::nComputeParameters(void)
{
	/* predictor */
	dpred_v		= (1.0 + falpha)*fdt;
	dpred_a		= (1.0 + falpha)*(1.0 - 2.0*fbeta)*0.5*fdt*fdt;
	vpred_a		= (1.0 + falpha)*(1.0 - fgamma)*fdt;
	
	/* corrector */
	dcorr_d		= falpha/(1.0 + falpha);
	dcorr_dpred = 1.0/(1.0 + falpha);
	dcorr_a		= fbeta*fdt*fdt;
	
	vcorr_v		= dcorr_d;
	vcorr_vpred = dcorr_dpred;
	vcorr_a		= fgamma*fdt;

	/* consistent BC */
	dalpha_a    = (1.0 + falpha)*fbeta*fdt*fdt;
	valpha_a    = (1.0 + falpha)*fgamma*fdt;
}