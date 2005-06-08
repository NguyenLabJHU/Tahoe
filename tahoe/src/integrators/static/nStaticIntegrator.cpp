/* $Id: nStaticIntegrator.cpp,v 1.10.12.1 2005-06-08 17:22:54 paklein Exp $ */
/* created: paklein (10/14/1996) */
#include "nStaticIntegrator.h"
#include "BasicFieldT.h"
#include "dArrayT.h"
#include "iArrayT.h"
#include "dArray2DT.h"
#include "iArray2DT.h"

using namespace Tahoe;

/* constructor */
nStaticIntegrator::nStaticIntegrator(void) { }

/* consistent BC's */
void nStaticIntegrator::ConsistentKBC(BasicFieldT& field, const KBC_CardT& KBC, const iArrayT* nodes)
{
	const char caller[] = "nStaticIntegrator::ConsistentKBC";
	if (KBC.Mode() == KBC_CardT::kSet && !nodes)
		ExceptionT::GeneralFail(caller, "expecting non-NULL nodes");

	/* destinations */
	int node = KBC.Node();
	int dof  = KBC.DOF();
	int nnd = (KBC.Mode() == KBC_CardT::kSet) ? nodes->Length() : 1;
	const int* pnd = (KBC.Mode() == KBC_CardT::kSet) ? nodes->Pointer() : &node;

	/* apply to nodes */
	for (int i = 0; i < nnd; i++)
	{
		double& d = (field[0])(*pnd, dof);
		pnd++; /* next */
	
		switch (KBC.Code())
		{
			case KBC_CardT::kFix: /* zero displacement */
			{
				d = 0.0;
				break;
			}
			case KBC_CardT::kDsp: /* prescribed displacement */
			{
				d = KBC.Value();
				break;
			}
			case KBC_CardT::kNull: /* do nothing and like it */
			{
				break;
			}
			default:
				ExceptionT::BadInputValue(caller, "unknown BC code: %d", KBC.Code());
		}
	}
}		

// predictors - map ALL, unless limit arguments are specified
void nStaticIntegrator::Predictor(BasicFieldT& field, int fieldstart /*= 0*/, int fieldend /*= -1*/)
{
#pragma unused(field)
#pragma unused(fieldstart)
#pragma unused(fieldend)
	//nothing to do
}

/* corrector. Maps ALL degrees of freedom forward. */
void nStaticIntegrator::Corrector(BasicFieldT& field, const dArray2DT& update, int fieldstart /*= 0*/, int fieldend /*= -1*/, int dummy /*= 0*/)
{
#pragma unused(dummy)

	if (fieldend == -1) // operate on full arrays
	{
		/* update displacements */
		field[0] += update;
	}
	else
	{
		/* update displacements */
		field[0].AddScaled(1.0, update, fieldstart, fieldend);
	}
}

/* correctors - map ACTIVE */
void nStaticIntegrator::Corrector(BasicFieldT& field, const dArrayT& update, 
		int eq_start, int num_eq)
{
	const iArray2DT& eqnos = field.Equations();

	/* add update - assumes that fEqnos maps directly into dva */
	const int* peq = eqnos.Pointer();
	double* pd  = field[0].Pointer();
	for (int i = 0; i < eqnos.Length(); i++)
	{
		int eq = *peq++ - eq_start;
	
		/* active dof */
		if (eq > -1 && eq < num_eq) *pd += update[eq];

		/* next */
		pd++;
	}
}

void nStaticIntegrator::MappedCorrector(BasicFieldT& field, const iArrayT& map, 
		const iArray2DT& flags, const dArray2DT& update)
{
	/* check dimensions */
	if (flags.MajorDim() != update.MajorDim() ||
	    flags.MinorDim() != update.MinorDim()) throw ExceptionT::kSizeMismatch;

	/* run through map */
	int minordim = flags.MinorDim();
	const int* pflag = flags.Pointer();
	const double* pupdate = update.Pointer();
	for (int i = 0; i < map.Length(); i++)
	{
		int row = map[i];
		const int* pflags = flags(i);

		double* pd = (field[0])(row);
		for (int j = 0; j < minordim; j++)
		{
			/* active */
			if (*pflag > 0) *pd = *pupdate;
			
			/* next */
			pflag++; pupdate++; pd++;
		}
	}
}

/* return the field array needed by nIntegrator::MappedCorrector. */
const dArray2DT& nStaticIntegrator::MappedCorrectorField(BasicFieldT& field) const
{
	return field[0];
}

/* pseudo-boundary conditions for external nodes */
KBC_CardT::CodeT nStaticIntegrator::ExternalNodeCondition(void) const
{
	return KBC_CardT::kDsp;
}

/*************************************************************************
* Protected
*************************************************************************/

/* recalculate time stepping constants */
void nStaticIntegrator::nComputeParameters(void) { }
