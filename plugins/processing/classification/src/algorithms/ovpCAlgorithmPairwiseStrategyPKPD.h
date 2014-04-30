#ifndef __OpenViBEPlugins_Algorithm_PairwiseStrategy_PKPD_H__
#define __OpenViBEPlugins_Algorithm_PairwiseStrategy_PKPD_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "ovpCAlgorithmPairwiseDecision.h"

#define OVP_ClassId_Algorithm_PairwiseStrategy_PKPD												OpenViBE::CIdentifier(0x26EF6DDA, 0xF137053C)
#define OVP_ClassId_Algorithm_PairwiseStrategy_PKPDDesc											OpenViBE::CIdentifier(0x191EB02A, 0x6866214A)

namespace OpenViBEPlugins
{
	namespace Classification
	{
		class CAlgorithmPairwiseStrategyPKPD : virtual public OpenViBEPlugins::Classification::CAlgorithmPairwiseDecision
		{

		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean classify(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_Algorithm_PairwiseStrategy_PKPD)
		};

		class CAlgorithmPairwiseStrategyPKPDDesc : virtual public OpenViBEPlugins::Classification::CAlgorithmPairwiseDecisionDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Pairwise decision strategy based on PKPD"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Serrière Guillaume"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("."); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_Algorithm_PairwiseStrategy_PKPD; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Classification::CAlgorithmPairwiseStrategyPKPD; }

			virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmPrototype) const
			{
				CAlgorithmPairwiseDecisionDesc::getAlgorithmPrototype(rAlgorithmPrototype);
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_Algorithm_PairwiseStrategy_PKPDDesc)
		};
	}
}

#endif //__OpenViBEPlugins_Algorithm_PairwiseStrategy_PKPD_H__
