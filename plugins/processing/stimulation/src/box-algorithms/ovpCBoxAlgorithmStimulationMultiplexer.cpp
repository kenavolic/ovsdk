#include "ovpCBoxAlgorithmStimulationMultiplexer.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Stimulation;

using namespace std;

bool CBoxAlgorithmStimulationMultiplexer::initialize(void)
{
	const IBox& staticBoxContext=this->getStaticBoxContext();

	m_StimulationDecoders.resize(staticBoxContext.getInputCount());
	unsigned int inputIndex = 0;
	for (auto& stimulationDecoder : m_StimulationDecoders)
	{
		stimulationDecoder.initialize(*this, inputIndex);
		inputIndex += 1;
	}

	m_StimulationEncoder.initialize(*this, 0);

	m_StreamDecoderEndTimes = std::vector<uint64>(staticBoxContext.getInputCount(), 0ULL);

	m_LastStartTime = 0;
	m_LastEndTime = 0;
	m_WasHeaderSent = false;

	return true;
}

bool CBoxAlgorithmStimulationMultiplexer::uninitialize(void)
{
	for (auto& stimulationDecoder : m_StimulationDecoders)
	{
		stimulationDecoder.uninitialize();
	}

	m_StimulationEncoder.uninitialize();

	return true;
}

bool CBoxAlgorithmStimulationMultiplexer::processInput(uint32)
{
	this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

bool CBoxAlgorithmStimulationMultiplexer::process(void)
{
	const IBox& staticBoxContext = this->getStaticBoxContext();
	IBoxIO& dynamicBoxContext = this->getDynamicBoxContext();

	if (!m_WasHeaderSent)
	{
		m_StimulationEncoder.encodeHeader();
		dynamicBoxContext.markOutputAsReadyToSend(0, m_LastEndTime, m_LastEndTime);
		m_WasHeaderSent = true;
	}

	uint64 earliestReceivedChunkEndTime = 0xffffffffffffffffULL;

	for (uint32 input = 0; input < staticBoxContext.getInputCount(); ++input)
	{
		for (uint32 chunk = 0; chunk < dynamicBoxContext.getInputChunkCount(input); ++chunk)
		{
			m_StimulationDecoders[input].decode(chunk);

			if (m_StimulationDecoders[input].isBufferReceived())
			{
				for(uint32 stimulation = 0; stimulation < m_StimulationDecoders[input].getOutputStimulationSet()->getStimulationCount(); ++stimulation)
				{
					m_vStimulation.insert(std::make_pair(
											  m_StimulationDecoders[input].getOutputStimulationSet()->getStimulationDate(stimulation),
											  std::make_tuple(
												  m_StimulationDecoders[input].getOutputStimulationSet()->getStimulationIdentifier(stimulation),
												  m_StimulationDecoders[input].getOutputStimulationSet()->getStimulationDate(stimulation),
												  m_StimulationDecoders[input].getOutputStimulationSet()->getStimulationDuration(stimulation)
												  )));

				}

			}

			m_StreamDecoderEndTimes[input] = dynamicBoxContext.getInputChunkEndTime(input, chunk);
		}

		if (earliestReceivedChunkEndTime > m_StreamDecoderEndTimes[input])
		{
			earliestReceivedChunkEndTime = m_StreamDecoderEndTimes[input];
		}
	}

	if (earliestReceivedChunkEndTime >= m_LastEndTime)
	{
		m_StimulationEncoder.getInputStimulationSet()->clear();

		for (auto stimulation = m_vStimulation.begin(); stimulation != m_vStimulation.end();)
		{
			if (stimulation->first < earliestReceivedChunkEndTime)
			{
				m_StimulationEncoder.getInputStimulationSet()->appendStimulation(
							std::get<0>(stimulation->second),
							std::get<1>(stimulation->second),
							std::get<2>(stimulation->second)
							);
				stimulation = m_vStimulation.erase(stimulation);
			}
			else
			{
				++stimulation;
			}
		}

		m_StimulationEncoder.encodeBuffer();

		dynamicBoxContext.markOutputAsReadyToSend(0, m_LastEndTime, earliestReceivedChunkEndTime);

		m_LastStartTime = m_LastEndTime;
		m_LastEndTime = earliestReceivedChunkEndTime;
	}

	return true;
}
