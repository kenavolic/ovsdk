#ifndef __OpenViBEKernel_Kernel_CAlgorithmContext_H__
#define __OpenViBEKernel_Kernel_CAlgorithmContext_H__

#include "../ovkTKernelObject.h"

namespace OpenViBE
{
	namespace Kernel
	{
		class CAlgorithmProxy;

		class CAlgorithmContext : public OpenViBE::Kernel::TKernelObject < OpenViBE::Kernel::IAlgorithmContext >
		{
		public:

			CAlgorithmContext(const OpenViBE::Kernel::IKernelContext& rKernelContext, OpenViBE::Kernel::CAlgorithmProxy& rAlgorithmProxy, const OpenViBE::Plugins::IPluginObjectDesc& rPluginObjectDesc);
			virtual ~CAlgorithmContext(void);

			virtual OpenViBE::Kernel::IConfigurationManager& getConfigurationManager(void) const;
			virtual OpenViBE::Kernel::IAlgorithmManager& getAlgorithmManager(void) const;
			virtual OpenViBE::Kernel::ILogManager& getLogManager(void) const;
			virtual OpenViBE::Kernel::IErrorManager& getErrorManager(void) const;
			virtual OpenViBE::Kernel::ITypeManager& getTypeManager(void) const;

			virtual OpenViBE::CIdentifier getNextInputParameterIdentifier(
				const OpenViBE::CIdentifier& rPreviousInputParameterIdentifier) const;
			virtual OpenViBE::Kernel::IParameter* getInputParameter(
				const OpenViBE::CIdentifier& rInputParameterIdentifier);

			virtual OpenViBE::CIdentifier getNextOutputParameterIdentifier(
				const OpenViBE::CIdentifier& rPreviousOutputParameterIdentifier) const;
			virtual OpenViBE::Kernel::IParameter* getOutputParameter(
				const OpenViBE::CIdentifier& rOutputParameterIdentifier);

			virtual OpenViBE::boolean isInputTriggerActive(
				const OpenViBE::CIdentifier& rInputTriggerIdentifier) const;

			virtual OpenViBE::boolean activateOutputTrigger(
				const OpenViBE::CIdentifier& rOutputTriggerIdentifier,
				const OpenViBE::boolean bTriggerState);

			_IsDerivedFromClass_Final_(OpenViBE::Kernel::TKernelObject < OpenViBE::Kernel::IAlgorithmContext >, OVK_ClassId_Kernel_Algorithm_AlgorithmContext)

		protected:

			OpenViBE::Kernel::ILogManager& m_rLogManager;
			OpenViBE::Kernel::CAlgorithmProxy& m_rAlgorithmProxy;
		};
	};
};

#endif // __OpenViBEKernel_Kernel_CAlgorithmContext_H__
