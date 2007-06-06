#ifndef __OpenViBEKernel_Kernel_CProcessingUnit_H__
#define __OpenViBEKernel_Kernel_CProcessingUnit_H__

#include "ovkTAttributable.h"
#include "ovkTKernelObject.h"

#include <vector>
#include <map>

namespace OpenViBE
{
	namespace Kernel
	{
		class CProcessingUnit : virtual public OpenViBE::Kernel::TKernelObject<OpenViBE::Kernel::TAttributable<OpenViBE::Kernel::IProcessingUnit> >
		{
		public:

			CProcessingUnit(const OpenViBE::Kernel::IKernelContext& rKernelContext);

			virtual OpenViBE::boolean setIdentifier(
				const OpenViBE::CIdentifier& rIdentifier);
			virtual OpenViBE::CIdentifier getIdentifier(void) const;

			virtual OpenViBE::boolean setName(
				const OpenViBE::CString& sName);
			virtual OpenViBE::CString getName(void) const;

			_IsDerivedFromClass_Final_(OpenViBE::Kernel::TKernelObject<OpenViBE::Kernel::TAttributable<OpenViBE::Kernel::IProcessingUnit> >, OVK_ClassId_Kernel_ProcessingUnit)

		protected:

			OpenViBE::CIdentifier m_oIdentifier;
			OpenViBE::CString m_sName;
		};
	};
};

#endif // __OpenViBEKernel_Kernel_CProcessingUnit_H__
