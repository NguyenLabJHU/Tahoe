/* $Id: ElementListT.cpp,v 1.62.2.1 2003-09-10 17:56:36 paklein Exp $ */
/* created: paklein (04/20/1998) */
#include "ElementListT.h"
#include "ElementsConfig.h"

#ifdef __DEVELOPMENT__
#include "DevelopmentElementsConfig.h"
#endif

#include <iostream.h>
#include "fstreamT.h"
#include "StringT.h"
#include "ElementT.h"
#include "ElementSupportT.h"
#include "GeometryT.h"

#include "ElementBaseT.h"

#ifdef ADHESION_ELEMENT
#include "AdhesionT.h"
#endif

#ifdef COHESIVE_SURFACE_ELEMENT
#include "CSEIsoT.h"
#include "CSEAnisoT.h"
#include "MeshFreeCSEAnisoT.h"
#include "ThermalSurfaceT.h"
#endif

#ifdef CONTINUUM_ELEMENT
#include "ViscousDragT.h"
#include "SmallStrainT.h"
#include "UpdatedLagrangianT.h"
#include "UpLagAdaptiveT.h"
#include "TotalLagrangianT.h"
#include "LocalizerT.h"
#include "SimoFiniteStrainT.h"
#include "SimoQ1P0.h"
#include "DiffusionElementT.h"
#include "NLDiffusionElementT.h"
#include "MeshFreeSSSolidT.h"
#include "MeshFreeFSSolidT.h"
#include "D2MeshFreeFSSolidT.h"
#include "UpLagr_ExternalFieldT.h"
#endif

#ifdef BRIDGING_ELEMENT
#include "BridgingScaleT.h"
#include "MeshfreeBridgingT.h"
#endif

#ifdef CONTACT_ELEMENT
#include "PenaltyContact2DT.h"
#include "PenaltyContact3DT.h"
#include "AugLagContact2DT.h"
#include "AugLagContact3DT.h"
#include "ACME_Contact3DT.h"
#include "PenaltyContactDrag2DT.h"
#include "PenaltyContactDrag3DT.h"
#endif

#ifdef PARTICLE_ELEMENT
#include "ParticlePairT.h"
#include "EAMT.h"
#endif

#ifdef SPRING_ELEMENT
#include "SWDiamondT.h"
#include "MixedSWDiamondT.h"
#include "VirtualSWDC.h"
#include "RodT.h"
#include "UnConnectedRodT.h"
#include "VirtualRodT.h"
#endif

#ifdef CONTACT_ELEMENT_DEV
#include "MultiplierContact3DT.h"
#include "MultiplierContactElement2DT.h"
#include "PenaltyContactElement2DT.h"
#include "PenaltyContactElement3DT.h"
#endif

#ifdef BEM_ELEMENT_DEV
#include "BEMelement.h"
#endif

#ifdef MULTISCALE_ELEMENT_DEV
#include "StaggeredMultiScaleT.h"
#endif

#ifdef DORGAN_VOYIADJIS_MARIN_DEV
#include "DorganVoyiadjisMarin.h"
#endif

#ifdef SOLID_ELEMENT_DEV
#ifdef MATERIAL_FORCE_ELEMENT_DEV
#include "SmallStrainQ2P1.h"
#include "UpdatedLagrangianMF.h"
#include "SmallStrainMF.h"
#include "SSMF.h"
#include "SSQ2P1MF.h"
#include "SmallStrainQ1P0.h"
#include "SSQ1P0MF.h"
#endif /* MATERIAL_FORCE_ELEMENT_DEV */
#endif

using namespace Tahoe;

/* constructors */
ElementListT::ElementListT(FEManagerT& fe):
	ParameterInterfaceT("element_list")
{
	/* initialize element support */
	fSupport.SetFEManager(&fe);
}

/* destructor */
ElementListT::~ElementListT(void)
{
	/* activate all */
	if (fAllElementGroups.Length() > 0) {
		ArrayT<bool> mask(fAllElementGroups.Length());
		mask = true;
		SetActiveElementGroupMask(mask);
	}
}

/* initialization functions */
void ElementListT::EchoElementData(ifstreamT& in, ostream& out)
{
	const char caller[] = "ElementListT::EchoElementData";

	/* print header */
	out << "\n E l e m e n t   G r o u p   D a t a :\n\n";
	out << " Number of element groups. . . . . . . . . . . . = ";
	out << Length() << "\n";

	/* construct element groups */
	for (int i = 0; i < Length(); i++)
	{
		int	group;
		in >> group;
		group--;

		/* read code */
		ElementT::TypeT code;
		in >> code;

		/* check */
		if (group < 0 || group >= Length())
			ExceptionT::BadInputValue(caller, "element group %d is out of range", group+1);

		/* no over-writing existing groups */
		if (fArray[group])
			ExceptionT::BadInputValue(caller, "group %d already exists", group+1);

		/* no predefined field names */
		const FieldT* field = NULL;
		if (fSupport.Analysis() == GlobalT::kMultiField)
		{
			StringT name;
			in >> name;
			field = fSupport.Field(name);
		}
		else /* legacy - field set by analysis code */
		{
			switch (fSupport.Analysis())
			{
				case GlobalT::kPML:
				{
					ExceptionT::BadInputValue(caller, "PML not fully implemented");
				}
				case GlobalT::kLinStaticHeat:
				case GlobalT::kLinTransHeat:
				case GlobalT::kNLStaticHeat:
				case GlobalT::kNLTransHeat:
				{
					field = fSupport.Field("temperature");
					break;
				}
				case GlobalT::kLinStatic:
				case GlobalT::kLinDynamic:
				case GlobalT::kLinExpDynamic:
				case GlobalT::kNLExpDynamic:
				case GlobalT::kNLExpDynKfield:
				case GlobalT::kDR:
				case GlobalT::kNLStatic:
				case GlobalT::kNLDynamic:
				case GlobalT::kNLStaticKfield:
				{
					field = fSupport.Field("displacement");
					break;
				}
				default:
					ExceptionT::BadInputValue(caller, "unrecognized analysis type: %d", fSupport.Analysis());
			}
		}
		
		/* check field */
		if (!field)
			ExceptionT::BadInputValue(caller, "could not resolve field for group %d", group+1);

		/* write parameters */
		out << "\n Group number. . . . . . . . . . . . . . . . . . = " << group + 1 << '\n';
		out <<   " Element type code . . . . . . . . . . . . . . . = " <<      code << '\n';
		out << "    eq. " << ElementT::kRod                << ", rod\n";
		out << "    eq. " << ElementT::kElastic            << ", elastic\n";
		out << "    eq. " << ElementT::kHyperElastic       << ", hyperelastic\n";
		out << "    eq. " << ElementT::kLocalizing         << ", hyperelastic with localization\n";
		out << "    eq. " << ElementT::kSWDiamond          << ", diamond cubic lattice\n";   	
		out << "    eq. " << ElementT::kMixedSWDiamond     << ", diamond cubic lattice with evolving params\n";   	
		out << "    eq. " << ElementT::kUnConnectedRod     << ", self-connecting rods\n";   	
		out << "    eq. " << ElementT::kVirtualRod         << ", self-connecting rods with periodic BC's\n";   	
		out << "    eq. " << ElementT::kVirtualSWDC        << ", diamond cubic lattice with periodic BC's\n";   	
		out << "    eq. " << ElementT::kCohesiveSurface    << ", cohesive surface element\n";   	
		out << "    eq. " << ElementT::kThermalSurface     << ", thermal surface element\n";   	
		out << "    eq. " << ElementT::kPenaltyContact     << ", penalty contact\n";
		out << "    eq. " << ElementT::kAugLagContact      << ", augmented Lagrangian contact\n";
		out << "    eq. " << ElementT::kTotLagHyperElastic << ", hyperelastic (total Lagrangian)\n";
		out << "    eq. " << ElementT::kMeshFreeElastic    << ", elastic with MLS displacements\n";
		out << "    eq. " << ElementT::kMeshFreeFDElastic  << ", hyperelastic MLS (total Lagrangian)\n";
		out << "    eq. " << ElementT::kD2MeshFreeFDElastic << ", hyperelastic MLS (total Lagrangian)\n";
		out << "    eq. " << ElementT::kLinearDiffusion    << ", linear diffusion element\n";
		out << "    eq. " << ElementT::kMFCohesiveSurface  << ", meshfree cohesive surface element\n";
		out << "    eq. " << ElementT::kStaggeredMultiScale << ", Staggered MultiScale Element (for VMS) \n";
		out << "    eq. " << ElementT::kACME_Contact       << ", 3D contact using ACME\n";
		out << "    eq. " << ElementT::kMultiplierContact3D       << ", 3D contact using Lagrange multipliers\n";
		out << "    eq. " << ElementT::kMultiplierContactElement2D       << ", 2D Lagrange multiplier contact elements\n";
		out << "    eq. " << ElementT::kPenaltyContactElement2D       << ", 2D penalty contact elements\n";
		out << "    eq. " << ElementT::kPenaltyContactElement3D       << ", 3D penalty contact elements\n";
		out << "    eq. " << ElementT::kBridgingScale      << ", Bridging Scale\n";
		out << "    eq. " << ElementT::kSimoQ1P0           << ", Q1P0 mixed element\n";
		out << "    eq. " << ElementT::kAdhesion           << ", surface adhesion\n";
		out << "    eq. " << ElementT::kParticlePair       << ", Pair Potential\n";
		out << "    eq. " << ElementT::kEAM                << ", EAM Potential\n";


		/* create new element group - read control parameters
		   and allocate space in the contructors */
		switch (code)
		{
			case ElementT::kRod:
			{
#ifdef SPRING_ELEMENT
				fArray[group] = new RodT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "SPRING_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kViscousDrag:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new ViscousDragT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kElastic:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new SmallStrainT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kMeshFreeElastic:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new MeshFreeSSSolidT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kHyperElastic:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new UpdatedLagrangianT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kHyperElasticInitCSE:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new UpLagAdaptiveT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kTotLagHyperElastic:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new TotalLagrangianT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kSimoFiniteStrain:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new SimoFiniteStrainT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kSimoQ1P0:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new SimoQ1P0(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kStaggeredMultiScale:
			{
#ifdef MULTISCALE_ELEMENT_DEV
				/* must be using multi-field solver */
				if (fSupport.Analysis() != GlobalT::kMultiField)				
					ExceptionT::BadInputValue(caller, "multi field required");
			
				/* coarse scale field read above */
				const FieldT* coarse_scale = field;

				/* fine scale field */				
				StringT fine_field_name;
				in >> fine_field_name;
				const FieldT* fine_scale = fSupport.Field(fine_field_name);
				if (!coarse_scale || !fine_scale)
					ExceptionT::BadInputValue(caller, "error resolving field names");
			
				fArray[group] = new StaggeredMultiScaleT(fSupport, *coarse_scale, *fine_scale);
				break;
#else
				ExceptionT::BadInputValue(caller, "MULTISCALE_ELEMENT_DEV not enabled: %d", code);
#endif
			}
			case ElementT::kMeshFreeFDElastic:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new MeshFreeFSSolidT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kD2MeshFreeFDElastic:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new D2MeshFreeFSSolidT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kLocalizing:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new LocalizerT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kSWDiamond:
			{
#ifdef SPRING_ELEMENT
				fArray[group] = new SWDiamondT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "SPRING_ELEMENT not enabled: %d", code);
#endif
			}	
			case ElementT::kMixedSWDiamond:
			{
#ifdef SPRING_ELEMENT
				fArray[group] = new MixedSWDiamondT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "SPRING_ELEMENT not enabled: %d", code);
#endif
			}	
			case ElementT::kUnConnectedRod:
			{
#ifdef SPRING_ELEMENT
				fArray[group] = new UnConnectedRodT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "SPRING_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kVirtualRod:
			{
#ifdef SPRING_ELEMENT
				fArray[group] = new VirtualRodT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "SPRING_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kVirtualSWDC:
			{
#ifdef SPRING_ELEMENT
				fArray[group] = new VirtualSWDC(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "SPRING_ELEMENT not enabled: %d", code);
#endif
			}	
			case ElementT::kCohesiveSurface:
			{
#ifdef COHESIVE_SURFACE_ELEMENT
				int CSEcode;
				in >> CSEcode;
				out << " Cohesive surface formulation code . . . . . . . = " << CSEcode << '\n';
				out << "    eq. " << CSEBaseT::Isotropic   << ", isotropic\n";
				out << "    eq. " << CSEBaseT::Anisotropic << ", anisotropic\n";
				out << "    eq. " << CSEBaseT::NoRotateAnisotropic << ", fixed-frame anisotropic\n";

				if (CSEcode == CSEBaseT::Isotropic)
					fArray[group] = new CSEIsoT(fSupport, *field);	
				else if (CSEcode == CSEBaseT::Anisotropic)
					fArray[group] = new CSEAnisoT(fSupport, *field, true);
				else if (CSEcode == CSEBaseT::NoRotateAnisotropic)
					fArray[group] = new CSEAnisoT(fSupport, *field, false);
				else
				{
					ExceptionT::BadInputValue(caller, "unknown CSE formulation: %d", CSEcode);
				}
				break;
#else
				ExceptionT::BadInputValue(caller, "COHESIVE_SURFACE_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kThermalSurface:
			{
#ifdef COHESIVE_SURFACE_ELEMENT
				fArray[group] = new ThermalSurfaceT(fSupport, *field);	
				break;
#else
				ExceptionT::BadInputValue(caller, "COHESIVE_SURFACE_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kPenaltyContact:
			{
#ifdef CONTACT_ELEMENT
				int nsd = fSupport.NumSD();
				if (nsd == 2)
					fArray[group] = new PenaltyContact2DT(fSupport, *field);
				else
					fArray[group] = new PenaltyContact3DT(fSupport, *field);
					
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTACT_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kPenaltyContactDrag:
			{
#ifdef CONTACT_ELEMENT
				int nsd = fSupport.NumSD();
				if (nsd == 2)
					fArray[group] = new PenaltyContactDrag2DT(fSupport, *field);
				else if (nsd == 3)				
					fArray[group] = new PenaltyContactDrag3DT(fSupport, *field);
				else
					ExceptionT::GeneralFail(caller, "kPenaltyContactDrag only 2D and 3D");
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTACT_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kAugLagContact:
			{
#ifdef CONTACT_ELEMENT
				int nsd = fSupport.NumSD();
				if (nsd == 2)
					fArray[group] = new AugLagContact2DT(fSupport, *field);
				else
					fArray[group] = new AugLagContact3DT(fSupport, *field);

				break;
#else
				ExceptionT::BadInputValue(caller, "CONTACT_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kBEMelement:
			{
#ifdef BEM_ELEMENT_DEV
				StringT BEMfilename;
				in >> BEMfilename;
			
				fArray[group] = new BEMelement(fSupport, *field, BEMfilename);	
				break;
#else
				ExceptionT::BadInputValue(caller, "BEM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kLinearDiffusion:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new DiffusionElementT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kNonLinearDiffusion:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new NLDiffusionElementT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kMFCohesiveSurface:
			{
#ifdef COHESIVE_SURFACE_ELEMENT
				fArray[group] = new MeshFreeCSEAnisoT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "COHESIVE_SURFACE_ELEMENT not enabled: %d", code);
#endif
			}
			case ElementT::kTotLagrExternalField:
			{
#ifdef CONTINUUM_ELEMENT
				fArray[group] = new UpLagr_ExternalFieldT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTINUUM_ELEMENT not enabled: %d", code);
#endif
			}				
			case ElementT::kACME_Contact:
			{
#ifdef CONTACT_ELEMENT
#ifdef __ACME__
				fArray[group] = new ACME_Contact3DT(fSupport, *field);
#else
				ExceptionT::GeneralFail(caller, "ACME not installed");
#endif /* __ACME__ */			
				break;
#else /* CONTACT_ELEMENT */
				ExceptionT::BadInputValue(caller, "CONTACT_ELEMENT not enabled: %d", code);
#endif				
			}
			case ElementT::kMultiplierContact3D:
			{
#ifdef CONTACT_ELEMENT_DEV
				fArray[group] = new MultiplierContact3DT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTACT_ELEMENT_DEV not enabled: %d", code);
#endif				
			}
			case ElementT::kMultiplierContactElement2D:
			{
#ifdef CONTACT_ELEMENT_DEV
				fArray[group] = new MultiplierContactElement2DT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTACT_ELEMENT_DEV not enabled: %d", code);
#endif				
			}
			case ElementT::kPenaltyContactElement2D:
			{
#ifdef CONTACT_ELEMENT_DEV
				fArray[group] = new PenaltyContactElement2DT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTACT_ELEMENT_DEV not enabled: %d", code);
#endif				
			}
			case ElementT::kPenaltyContactElement3D:
			{
#ifdef CONTACT_ELEMENT_DEV
				fArray[group] = new PenaltyContactElement3DT(fSupport, *field);
				break;
#else
				ExceptionT::BadInputValue(caller, "CONTACT_ELEMENT_DEV not enabled: %d", code);
#endif				
			}
		case ElementT::kBridgingScale:
		{
#if defined (BRIDGING_ELEMENT) && defined (CONTINUUM_ELEMENT)
			/* associated group numbers */
			int solid_group = -99;
			in >> solid_group;

			const SolidElementT* solid = dynamic_cast<const SolidElementT*>(&(fSupport.ElementGroup(--solid_group)));
			if (!solid)
				ExceptionT::BadInputValue(caller, "unable to cast pointer to group %d to type SolidElementT", solid_group+1);

			fArray[group] = new BridgingScaleT(fSupport, *field, *solid);
		    break;
#else
				ExceptionT::BadInputValue(caller, "BRIDGING_ELEMENT or CONTINUUM_ELEMENT not enabled: %d", code);
#endif				
		}
		case ElementT::kMeshfreeBridging:
		{
#if defined (BRIDGING_ELEMENT) && defined (CONTINUUM_ELEMENT)
			/* associated group numbers */
			int solid_group = -99;
			in >> solid_group;

			const SolidElementT* solid = dynamic_cast<const SolidElementT*>(&(fSupport.ElementGroup(--solid_group)));
			if (!solid)
				ExceptionT::BadInputValue(caller, "unable to cast pointer to group %d to type SolidElementT", solid_group+1);

			fArray[group] = new MeshfreeBridgingT(fSupport, *field, *solid);
		    break;
#else
				ExceptionT::BadInputValue(caller, "BRIDGING_ELEMENT or CONTINUUM_ELEMENT not enabled: %d", code);
#endif				
		}
		case ElementT::kAdhesion:
		{
#ifdef ADHESION_ELEMENT
			fArray[group] = new AdhesionT(fSupport, *field);
			break;
#else
			ExceptionT::BadInputValue(caller, "ADHESION_ELEMENT not enabled: %d", code);
#endif
		}
		case ElementT::kParticlePair:
		{
#ifdef PARTICLE_ELEMENT
			fArray[group] = new ParticlePairT(fSupport, *field);
			break;
#else
			ExceptionT::BadInputValue(caller, "PARTICLE_ELEMENT not enabled: %d", code);
#endif				
		}
		// SA
		case ElementT::kEAM:
		{
#ifdef PARTICLE_ELEMENT
			fArray[group] = new EAMT(fSupport, *field);
			break;
#else
			ExceptionT::BadInputValue(caller, "PARTICLE_ELEMENT not enabled: %d", code);
#endif				
		}
		case ElementT::kFSMatForce:
	        {
#if defined (SOLID_ELEMENT_DEV) && defined (MATERIAL_FORCE_ELEMENT_DEV)
		        fArray[group] = new UpdatedLagrangianMF(fSupport, *field);
			break;
#else
			ExceptionT::BadInputValue(caller, "SOLID_ELEMENT_DEV or MATERIAL_FORCE_ELEMENT_DEV not enabled: %d", code);
#endif
		}
		case ElementT::kSSMatForceD:
		{
#if defined (SOLID_ELEMENT_DEV) && defined (MATERIAL_FORCE_ELEMENT_DEV)
		  fArray[group] = new SmallStrainMF(fSupport, *field);
		  break;
#else
			ExceptionT::BadInputValue(caller, "SOLID_ELEMENT_DEV or MATERIAL_FORCE_ELEMENT_DEV not enabled: %d", code);
#endif
		}
		case ElementT::kSSMatForceS:
		{
#if defined (SOLID_ELEMENT_DEV) && defined (MATERIAL_FORCE_ELEMENT_DEV)
		  fArray[group] = new SSMF(fSupport, *field);
		  break;
#else
			ExceptionT::BadInputValue(caller, "SOLID_ELEMENT_DEV or MATERIAL_FORCE_ELEMENT_DEV not enabled: %d", code);
#endif
		}
		case ElementT::kSmallStrainQ2P1:
		{
#if defined (SOLID_ELEMENT_DEV) && defined (MATERIAL_FORCE_ELEMENT_DEV)
		  fArray[group] = new SmallStrainQ2P1(fSupport, *field);
		  break;
#else
			ExceptionT::BadInputValue(caller, "SOLID_ELEMENT_DEV or MATERIAL_FORCE_ELEMENT_DEV not enabled: %d", code);
#endif
		}
		case ElementT::kSmallStrainQ1P0:
		{
#if defined (SOLID_ELEMENT_DEV) && defined (MATERIAL_FORCE_ELEMENT_DEV)
		  fArray[group] = new SmallStrainQ1P0(fSupport, *field);
		  break;
#else
			ExceptionT::BadInputValue(caller, "SOLID_ELEMENT_DEV or MATERIAL_FORCE_ELEMENT_DEV not enabled: %d", code);
#endif
		}
		case ElementT::kSSQ2P1MF:
		{
#if defined (SOLID_ELEMENT_DEV) && defined (MATERIAL_FORCE_ELEMENT_DEV)
		  fArray[group] = new SSQ2P1MF(fSupport, *field);
		  break;
#else
			ExceptionT::BadInputValue(caller, "SOLID_ELEMENT_DEV or MATERIAL_FORCE_ELEMENT_DEV not enabled: %d", code);
#endif
		}
		case ElementT::kSSQ1P0MF:
		{
#if defined (SOLID_ELEMENT_DEV) && defined (MATERIAL_FORCE_ELEMENT_DEV)
		  fArray[group] = new SSQ1P0MF(fSupport, *field);
		  break;
#else
			ExceptionT::BadInputValue(caller, "SOLID_ELEMENT_DEV or MATERIAL_FORCE_ELEMENT_DEV not enabled: %d", code);
#endif
		}
		case ElementT::kDorganVoyiadjisMarin:
		{
#ifdef DORGAN_VOYIADJIS_MARIN_DEV
		  /* displacement field read above */
		  const FieldT* disp = field;
		  
		  /* hardness field */				
		  StringT hardness_field_name;
		  in >> hardness_field_name;
		  const FieldT* hardness = fSupport.Field(hardness_field_name);
		  if (!disp || !hardness)
		    ExceptionT::BadInputValue(caller, "error resolving field names");
		  
		  fArray[group] = new DorganVoyiadjisMarin(fSupport, *disp, *hardness);
		  break;
#else
		  ExceptionT::BadInputValue(caller, "DORGAN_VOYIADJIS_MARIN_DEV not enabled: %d", code);
#endif			
		}

		default:
		  ExceptionT::BadInputValue(caller, "unknown element type: %d", code);
		}
		
		if (!fArray[group]) ExceptionT::OutOfMemory();
		fArray[group]->Initialize();
	}

#ifdef PARTICLE_ELEMENT
	/* can only have one particle group */
	int count = 0;
	for (int i = 0; i < Length(); i++)
	{
		ElementBaseT* e_group = fArray[i];
		ParticleT* particle = dynamic_cast<ParticleT*>(e_group);
		if (particle) count++;
	}
//	if (count > 1) ExceptionT::BadInputValue(caller, "only one particle group allowed: %d", count);
	if (count > 1) cout << "\n " << caller << ": WARNING found more than 1 particle group" << endl;
#endif
}

/* returns true of ALL element groups have interpolant DOF's */
bool ElementListT::InterpolantDOFs(void) const
{
	bool are_interp = true;
	for (int i = 0; i < Length(); i++)
		if (!fArray[i]->InterpolantDOFs()) are_interp = false;

	return are_interp;
}

/* returns true if contact group present */
bool ElementListT::HasContact(void) const
{
#ifdef __NO_RTTI__
	cout << "\n ElementListT::HasContact: needs RTTI: returning false" << endl;
	return false; // safe default??
#else /* __NO_RTTI__ */
#ifdef CONTACT_ELEMENT
	for (int i = 0; i < Length(); i++)
	{
		ContactT* contact_elem = dynamic_cast<ContactT*>(fArray[i]);
		if (contact_elem) return true;
	}
	return false;
#else  /* CONTACT_ELEMENT */
	return false;
#endif /* CONTACT_ELEMENT */
#endif /* __NO_RTTI__ */
}

/* change the number of active element groups */
void ElementListT::SetActiveElementGroupMask(const ArrayT<bool>& mask)
{
	/* first time */
	if (fAllElementGroups.Length() == 0) 
	{
		/* cache all pointers */
		fAllElementGroups.Dimension(Length());
		for (int i = 0; i < fAllElementGroups.Length(); i++)
		{
			ElementBaseT* element = (*this)[i];
			fAllElementGroups[i] = element;
		}
	}

	/* check */
	if (mask.Length() != fAllElementGroups.Length())
		ExceptionT::SizeMismatch("ElementListT::SetActiveElementGroupMask",
			"expecting mask length %d not %d", fAllElementGroups.Length(), mask.Length());

	/* reset active element groups */
	int num_active = 0;
	for (int i = 0; i < mask.Length(); i++)
		if (mask[i])
			num_active++;
			
	/* cast this to an ArrayT */
	ArrayT<ElementBaseT*>& element_list = *this;
	element_list.Dimension(num_active);
	num_active = 0;
	for (int i = 0; i < mask.Length(); i++)
		if (mask[i])
			element_list[num_active++] = fAllElementGroups[i];
}

/* information about subordinate parameter lists */
void ElementListT::DefineSubs(SubListT& sub_list) const
{
#ifdef COHESIVE_SURFACE_ELEMENT
	sub_list.AddSub("isotropic_CSE", ParameterListT::Any);
	sub_list.AddSub("anisotropic_CSE", ParameterListT::Any);
#endif

#ifdef ADHESION_ELEMENT
	sub_list.AddSub("adhesion", ParameterListT::Any);
#endif
}

/* a pointer to the ParameterInterfaceT of the given subordinate */
ParameterInterfaceT* ElementListT::NewSub(const StringT& list_name) const
{
	if (false) /* dummy */
		return NULL;

#ifdef COHESIVE_SURFACE_ELEMENT	
	else if (list_name == "isotropic_CSE")
		return new CSEIsoT(fSupport);
		
	else if (list_name == "anisotropic_CSE")
		return new CSEAnisoT(fSupport);
#endif

#ifdef ADHESION_ELEMENT
	else if (list_name == "adhesion")
		return new AdhesionT(fSupport);
#endif

	/* inherited */	
	else
		return ParameterInterfaceT::NewSub(list_name);
}
