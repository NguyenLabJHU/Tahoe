/* $Id: ElementT.cpp,v 1.41.12.1 2004-06-24 04:55:21 paklein Exp $ */
#include "ElementT.h"
#include "ExceptionT.h"

using namespace Tahoe;

ElementT::TypeT ElementT::int2TypeT(int i)
{
	switch (i)
	{
		case ElementT::kRod:
			return ElementT::kRod;

		case ElementT::kElastic:
			return ElementT::kElastic;

		case ElementT::kElasticAxi:
			return ElementT::kElasticAxi;

		case ElementT::kHyperElastic:
			return ElementT::kHyperElastic;

		case ElementT::kHyperElasticAxi:
			return ElementT::kHyperElasticAxi;

		case ElementT::kLocalizing:
			return ElementT::kLocalizing;

		case ElementT::kSWDiamond:
			return ElementT::kSWDiamond;

		case ElementT::kMixedSWDiamond:
			return ElementT::kMixedSWDiamond;

		case ElementT::kUnConnectedRod:
			return ElementT::kUnConnectedRod;

		case ElementT::kVirtualRod:
			return ElementT::kVirtualRod;

		case ElementT::kVirtualSWDC:
			return ElementT::kVirtualSWDC;

		case ElementT::kCohesiveSurface:
			return ElementT::kCohesiveSurface;

		case ElementT::kThermalSurface:
			return ElementT::kThermalSurface;

		case ElementT::kViscousDrag:
			return ElementT::kViscousDrag;

		case ElementT::kPenaltyContact:
			return ElementT::kPenaltyContact;

		case ElementT::kBEMelement:
			return ElementT::kBEMelement;

		case ElementT::kAugLagContact:
			return ElementT::kAugLagContact;

		case ElementT::kTotLagHyperElastic:
			return ElementT::kTotLagHyperElastic;

		case ElementT::kTotLagHyperElasticAxi:
			return ElementT::kTotLagHyperElasticAxi;

		case ElementT::kMeshFreeElastic:
			return ElementT::kMeshFreeElastic;

		case ElementT::kMeshFreeFDElastic:
			return ElementT::kMeshFreeFDElastic;

		case ElementT::kMeshFreeFDElasticAxi:
			return ElementT::kMeshFreeFDElasticAxi;

		case ElementT::kD2MeshFreeFDElastic:
			return ElementT::kD2MeshFreeFDElastic;

		case ElementT::kLinearDiffusion:
			return ElementT::kLinearDiffusion;

		case ElementT::kNonLinearDiffusion:
			return ElementT::kNonLinearDiffusion;

		case ElementT::kMFCohesiveSurface:
			return ElementT::kMFCohesiveSurface;

		case ElementT::kACME_Contact:
			return ElementT::kACME_Contact;

		case ElementT::kMultiplierContact3D:
			return ElementT::kMultiplierContact3D;

		case ElementT::kPenaltyContactElement2D:
			return ElementT::kPenaltyContactElement2D;

		case ElementT::kPenaltyContactElement3D:
			return ElementT::kPenaltyContactElement3D;

		case ElementT::kTotLagrExternalField:
			return ElementT::kTotLagrExternalField;

		case ElementT::kMultiplierContactElement2D:
			return ElementT::kMultiplierContactElement2D;

		case ElementT::kSimoFiniteStrain:
			return ElementT::kSimoFiniteStrain;

		case ElementT::kStaggeredMultiScale:
			return ElementT::kStaggeredMultiScale;

		case ElementT::kBridgingScale:
			return ElementT::kBridgingScale;

		case ElementT::kMeshfreeBridging:
			return ElementT::kMeshfreeBridging;

		case ElementT::kSimoQ1P0:
			return ElementT::kSimoQ1P0;

		case ElementT::kSimoQ1P0Axi:
			return ElementT::kSimoQ1P0Axi;

		case ElementT::kAdhesion:
			return ElementT::kAdhesion;

		case ElementT::kParticlePair:
			return ElementT::kParticlePair;

		case ElementT::kEAM:
			return ElementT::kEAM;

		case ElementT::kFSMatForce:
		    return ElementT::kFSMatForce;

		case ElementT::kSSMatForceD:
		    return ElementT::kSSMatForceD;

		case ElementT::kSSMatForceS:
		    return ElementT::kSSMatForceS;

		case ElementT::kSmallStrainQ2P1:
		    return ElementT::kSmallStrainQ2P1;

		case ElementT::kSSQ2P1MF:
		    return ElementT::kSSQ2P1MF;

		case ElementT::kSmallStrainQ1P0:
		    return ElementT::kSmallStrainQ1P0;

		case ElementT::kSSQ1P0MF:
		    return ElementT::kSSQ1P0MF;

		case ElementT::kGradSmallStrain:
		    return ElementT::kGradSmallStrain;

		case ElementT::kGradC0SmallStrain:
		    return ElementT::kGradC0SmallStrain;

		case ElementT::kAPSgrad:
		    return ElementT::kAPSgrad;

		case ElementT::kHyperElasticInitCSE:
		    return ElementT::kHyperElasticInitCSE;

		case ElementT::kPenaltyContactDrag:
		    return ElementT::kPenaltyContactDrag;

		case ElementT::kMeshfreePenaltyContact:
		    return ElementT::kMeshfreePenaltyContact;

		case ElementT::kTotLagSplitIntegration:
		    return ElementT::kTotLagSplitIntegration;

		case ElementT::kSS_SCNIMF:
			return ElementT::kSS_SCNIMF;

		case ElementT::kFS_SCNIMF:
			return ElementT::kFS_SCNIMF;

		case ElementT::kAPSVgrad:
		    return ElementT::kAPSVgrad;

		case ElementT::kTotLagFlat:
		    return ElementT::kTotLagFlat;

		default:
			ExceptionT::BadInputValue("ElementT::int2TypeT", "unknown type: %d", i);
	}

	/* dummy */
	return ElementT::kRod;
}
