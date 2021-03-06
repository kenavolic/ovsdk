#include "ovpCBoxAlgorithmStimulationListener.h"
#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Tools;

boolean CBoxAlgorithmStimulationListener::initialize(void)
{
	const IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		m_vStimulationDecoder.push_back(new OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmStimulationListener >(*this,i));
	}

	m_eLogLevel = static_cast<ELogLevel>((uint64)FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0));

	return true;
}

boolean CBoxAlgorithmStimulationListener::uninitialize(void)
{
	const IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		m_vStimulationDecoder[i]->uninitialize();
		delete m_vStimulationDecoder[i];
	}
	m_vStimulationDecoder.clear();

	return true;
}

boolean CBoxAlgorithmStimulationListener::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxAlgorithmStimulationListener::process(void)
{
	const IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(i); j++)
		{
			m_vStimulationDecoder[i]->decode(j);
			if(m_vStimulationDecoder[i]->isHeaderReceived())
			{
			}
			if(m_vStimulationDecoder[i]->isBufferReceived())
			{
				const IStimulationSet* op_pStimulationSet = m_vStimulationDecoder[i]->getOutputStimulationSet();

				CString l_sInputName;
				l_rStaticBoxContext.getInputName(i, l_sInputName);
				for(uint64 k=0; k<op_pStimulationSet->getStimulationCount(); k++)
				{
					this->getLogManager() << m_eLogLevel
						<< "For input " << i << " with name " << l_sInputName
						<< " got stimulation " << op_pStimulationSet->getStimulationIdentifier(k)
						<< "[" << this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation, op_pStimulationSet->getStimulationIdentifier(k)) << "]"
						<< " at date " << time64(op_pStimulationSet->getStimulationDate(k))
						<< " and duration " << time64(op_pStimulationSet->getStimulationDuration(k))
						<< "\n";

					OV_WARNING_UNLESS_K(
						op_pStimulationSet->getStimulationDate(k) >= l_rDynamicBoxContext.getInputChunkStartTime(i, j) && op_pStimulationSet->getStimulationDate(k) <= l_rDynamicBoxContext.getInputChunkEndTime(i, j),
						"Invalid out of range date [" << time64(op_pStimulationSet->getStimulationDate(k)) << "] (expected value between [" << time64(l_rDynamicBoxContext.getInputChunkStartTime(i, j)) << "] and [" << time64(l_rDynamicBoxContext.getInputChunkEndTime(i, j)) << "])"
					);
				}
				/*
				if(ITimeArithmetics::timeToSeconds(l_rDynamicBoxContext.getInputChunkStartTime(i, j)) > 234 && op_pStimulationSet->getStimulationCount()==0)
				{
								this->getLogManager() << LogLevel_Info
							<< "Chunk is empty at ["
							<< time64(l_rDynamicBoxContext.getInputChunkStartTime(i, j)) << ", " << time64(l_rDynamicBoxContext.getInputChunkEndTime(i, j)) << "]\n";
				}
				*/
			}
			if(m_vStimulationDecoder[i]->isEndReceived())
			{
			}
			l_rDynamicBoxContext.markInputAsDeprecated(i, j);
		}
	}

	return true;
}
