#ifndef __OpenViBEKernel_Kernel_TAttributable_H__
#define __OpenViBEKernel_Kernel_TAttributable_H__

#include "../ovk_base.h"

#include <openvibe/ov_all.h>

#include <map>
#include <iostream>

namespace OpenViBE
{
	namespace Kernel
	{
		template <class T> class TAttributable : virtual public T
		{
		public:

			virtual OpenViBE::boolean addAttribute(
				const OpenViBE::CIdentifier& rAttributeIdentifier,
				const OpenViBE::CString& sAttributeValue)
			{
				std::map<OpenViBE::CIdentifier, OpenViBE::CString>::iterator itAttribute=m_vAttribute.find(rAttributeIdentifier);
				if(itAttribute!=m_vAttribute.end())
				{
					return false;
				}
				m_vAttribute[rAttributeIdentifier]=sAttributeValue;
				return true;
			}

			virtual OpenViBE::boolean removeAttribute(
				const OpenViBE::CIdentifier& rAttributeIdentifier)
			{
				std::map<OpenViBE::CIdentifier, OpenViBE::CString>::iterator itAttribute=m_vAttribute.find(rAttributeIdentifier);
				if(itAttribute==m_vAttribute.end())
				{
					return false;
				}
				m_vAttribute.erase(itAttribute);
				return true;
			}

			virtual OpenViBE::boolean removeAllAttributes(void)
			{
				m_vAttribute.clear();
				return true;
			}

			virtual OpenViBE::CString getAttributeValue(
				const OpenViBE::CIdentifier& rAttributeIdentifier) const
			{
				std::map<OpenViBE::CIdentifier, OpenViBE::CString>::const_iterator itAttribute=m_vAttribute.find(rAttributeIdentifier);
				if(itAttribute==m_vAttribute.end())
				{
					return OpenViBE::CString("");
				}
				return itAttribute->second;
			}

			virtual OpenViBE::boolean setAttributeValue(
				const OpenViBE::CIdentifier& rAttributeIdentifier,
				const OpenViBE::CString& sAttributeValue)
			{
				std::map<OpenViBE::CIdentifier, OpenViBE::CString>::iterator itAttribute=m_vAttribute.find(rAttributeIdentifier);
				if(itAttribute==m_vAttribute.end())
				{
					return false;
				}
				itAttribute->second=sAttributeValue;
				return true;
			}

			virtual OpenViBE::boolean hasAttribute(
				const OpenViBE::CIdentifier& rAttributeIdentifier) const
			{
				std::map<OpenViBE::CIdentifier, OpenViBE::CString>::const_iterator itAttribute=m_vAttribute.find(rAttributeIdentifier);
				if(itAttribute==m_vAttribute.end())
				{
					return false;
				}
				return true;
			}

			virtual OpenViBE::boolean hasAttributes(void) const
			{
				return m_vAttribute.size()!=0;
			}

#if 0
			virtual OpenViBE::boolean enumerateAttributes(
				OpenViBE::Kernel::IAttributable::IAttributeEnum& rCallback) const
			{
				rCallback.preCallback();
				std::map<OpenViBE::CIdentifier, OpenViBE::CString>::const_iterator itAttribute=m_vAttribute.begin();
				while(itAttribute!=m_vAttribute.end())
				{
					if(!rCallback.callback(*this, itAttribute->first, itAttribute->second))
					{
						rCallback.postCallback();
						return true;
					}
					itAttribute++;
				}
				rCallback.postCallback();
				return true;
			}
#endif

			virtual OpenViBE::CIdentifier getNextAttributeIdentifier(
				const OpenViBE::CIdentifier& rPreviousIdentifier) const
			{
				std::map<OpenViBE::CIdentifier, OpenViBE::CString>::const_iterator itAttribute;

				if(rPreviousIdentifier==OV_UndefinedIdentifier)
				{
					itAttribute=m_vAttribute.begin();
				}
				else
				{
					itAttribute=m_vAttribute.find(rPreviousIdentifier);
					if(itAttribute==m_vAttribute.end())
					{
						return OV_UndefinedIdentifier;
					}
					itAttribute++;
				}

				return itAttribute!=m_vAttribute.end()?itAttribute->first:OV_UndefinedIdentifier;

			}

			_IsDerivedFromClass_(T, OVK_ClassId_Kernel_Attributable);

		protected:

			std::map<OpenViBE::CIdentifier, OpenViBE::CString> m_vAttribute;
		};
	};
};

#endif // __OpenViBEKernel_Kernel_TAttributable_H__
