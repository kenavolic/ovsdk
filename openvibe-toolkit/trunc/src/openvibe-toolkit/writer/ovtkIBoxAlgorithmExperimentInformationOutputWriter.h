#ifndef __OpenViBEToolkit_Writer_IBoxAlgorithmExperimentInformationOutputWriter_H__
#define __OpenViBEToolkit_Writer_IBoxAlgorithmExperimentInformationOutputWriter_H__

#include "ovtkIBoxAlgorithmOutputWriter.h"

namespace OpenViBEToolkit
{
	class OVTK_API IBoxAlgorithmExperimentInformationOutputWriter : virtual public OpenViBEToolkit::IBoxAlgorithmOutputWriter
	{
	public:

		enum
		{
			Value_ExperimentIdentifier, // unsigned integer
			Value_SubjectIdentifier,    // unsigned integer
			Value_SubjectAge,           // unsigned integer
			Value_SubjectSex,           // unsigned integer
			Value_LaboratoryIdentifier, // unsigned integer
			Value_TechnicianIdentifier, // unsigned integer

			Value_ExperimentDate,       // string
			Value_SubjectName,          // string
			Value_LaboratoryName,       // string
			Value_TechnicianName,       // string
		};

		virtual OpenViBE::boolean setValue(const OpenViBE::uint32 ui32ValueIdentifier, const OpenViBE::uint32 ui32Value)=0;
		virtual OpenViBE::boolean setValue(const OpenViBE::uint32 ui32ValueIdentifier, const char* sValue)=0;

		virtual OpenViBE::boolean writeHeader(EBML::IWriter& rWriter)=0;
		// virtual OpenViBE::boolean writeBuffer(EBML::IWriter& rWriter)=0;

		_IsDerivedFromClass_(OpenViBEToolkit::IBoxAlgorithmOutputWriter, OVTK_ClassId_);
	};

	extern OVTK_API OpenViBEToolkit::IBoxAlgorithmExperimentInformationOutputWriter* createBoxAlgorithmExperimentInformationOutputWriter(void);
	extern OVTK_API void releaseBoxAlgorithmExperimentInformationOutputWriter(OpenViBEToolkit::IBoxAlgorithmExperimentInformationOutputWriter* pOutputWriter);
};

#endif // __OpenViBEToolkit_Writer_IBoxAlgorithmExperimentInformationOutputWriter_H__
