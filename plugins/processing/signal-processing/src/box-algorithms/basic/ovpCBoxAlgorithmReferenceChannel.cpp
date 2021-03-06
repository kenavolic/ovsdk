#include "ovpCBoxAlgorithmReferenceChannel.h"

#include <cstdio>
#include <limits>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

namespace
{
	uint32 _find_channel_(const IMatrix& rMatrix, const CString& rChannel, const CIdentifier& rMatchMethodIdentifier, uint32 uiStart=0)
	{
		uint32 i, l_ui32Result=std::numeric_limits<uint32>::max();

		if(rMatchMethodIdentifier==OVP_TypeId_MatchMethod_Name)
		{
			for(i=uiStart; i<rMatrix.getDimensionSize(0); i++)
			{
				if(OpenViBEToolkit::Tools::String::isAlmostEqual(rMatrix.getDimensionLabel(0, i), rChannel, false))
				{
					l_ui32Result=i;
				}
			}
		}
		else if(rMatchMethodIdentifier==OVP_TypeId_MatchMethod_Index)
		{
			try
			{
				unsigned int value = std::stoul(rChannel.toASCIIString());
				value--; // => makes it 0-indexed !

				if(uiStart <= uint32(value) && uint32(value) < rMatrix.getDimensionSize(0))
				{
					l_ui32Result=uint32(value);
				}
			}
			catch(const std::exception&)
			{
				// catch block intentionnaly left blank
			}
		}
		else if(rMatchMethodIdentifier==OVP_TypeId_MatchMethod_Smart)
		{
			if(l_ui32Result==std::numeric_limits<uint32>::max()) l_ui32Result=_find_channel_(rMatrix, rChannel, OVP_TypeId_MatchMethod_Name, uiStart);
			if(l_ui32Result==std::numeric_limits<uint32>::max()) l_ui32Result=_find_channel_(rMatrix, rChannel, OVP_TypeId_MatchMethod_Index, uiStart);
		}

		return l_ui32Result;
	}
};

boolean CBoxAlgorithmReferenceChannel::initialize(void)
{
	m_oDecoder.initialize(*this,0);
	m_oEncoder.initialize(*this,0);
	m_oEncoder.getInputSamplingRate().setReferenceTarget(m_oDecoder.getOutputSamplingRate());
	return true;
}

boolean CBoxAlgorithmReferenceChannel::uninitialize(void)
{
	m_oDecoder.uninitialize();
	m_oEncoder.uninitialize();
	return true;
}

boolean CBoxAlgorithmReferenceChannel::processInput(uint32 ui32InputIndex)
{
	this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

#include <cstdio>

boolean CBoxAlgorithmReferenceChannel::process(void)
{
	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	uint32 i, j, k;

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_oDecoder.decode(i);
		if(m_oDecoder.isHeaderReceived())
		{
			IMatrix& l_rInputMatrix=*m_oDecoder.getOutputMatrix();
			IMatrix& l_rOutputMatrix=*m_oEncoder.getInputMatrix();

			OV_ERROR_UNLESS_KRF(
				l_rInputMatrix.getDimensionSize(0) >= 2,
				"Invalid input matrix with [" << l_rInputMatrix.getDimensionSize(0) << "] channels (expected channels >= 2)",
				OpenViBE::Kernel::ErrorType::BadInput
			);

			CString l_sChannel=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
			uint64 l_ui64MatchMethod=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

			m_ui32ReferenceChannelIndex=::_find_channel_(l_rInputMatrix, l_sChannel, l_ui64MatchMethod, 0);

			OV_ERROR_UNLESS_KRF(
				m_ui32ReferenceChannelIndex != std::numeric_limits<uint32>::max(),
				"Invalid channel [" << l_sChannel << "]: channel not found",
				OpenViBE::Kernel::ErrorType::BadSetting
			);

			if(::_find_channel_(*m_oDecoder.getOutputMatrix(), l_sChannel, l_ui64MatchMethod, m_ui32ReferenceChannelIndex+1)!=std::numeric_limits<uint32>::max())
			{
				OV_WARNING_K("Multiple channels match for setting [" << l_sChannel << "]. Selecting [" << m_ui32ReferenceChannelIndex << "]");
			}

			l_rOutputMatrix.setDimensionCount(2);
			l_rOutputMatrix.setDimensionSize(0, l_rInputMatrix.getDimensionSize(0)-1);
			l_rOutputMatrix.setDimensionSize(1, l_rInputMatrix.getDimensionSize(1));
			for(j=0, k=0; j<l_rInputMatrix.getDimensionSize(0); j++)
			{
				if(j!=m_ui32ReferenceChannelIndex)
				{
					l_rOutputMatrix.setDimensionLabel(0, k++, l_rInputMatrix.getDimensionLabel(0, j));
				}
			}

			m_oEncoder.encodeHeader();
		}
		if(m_oDecoder.isBufferReceived())
		{
			IMatrix& l_rInputMatrix=*m_oDecoder.getOutputMatrix();
			IMatrix& l_rOutputMatrix=*m_oEncoder.getInputMatrix();
			float64* l_pInputBuffer=l_rInputMatrix.getBuffer();
			float64* l_pOutputBuffer=l_rOutputMatrix.getBuffer();
			float64* l_pReferenceBuffer=l_rInputMatrix.getBuffer()+m_ui32ReferenceChannelIndex*l_rInputMatrix.getDimensionSize(1);
			uint32 l_ui32ChannelCount=l_rInputMatrix.getDimensionSize(0);
			uint32 l_ui32SampleCount=l_rInputMatrix.getDimensionSize(1);
			for(j=0; j<l_ui32ChannelCount; j++)
			{
				if(j!=m_ui32ReferenceChannelIndex)
				{
					for(k=0; k<l_ui32SampleCount; k++)
					{
						l_pOutputBuffer[k]=l_pInputBuffer[k]-l_pReferenceBuffer[k];
					}
					l_pOutputBuffer+=l_ui32SampleCount;
				}
				l_pInputBuffer+=l_ui32SampleCount;
			}

			m_oEncoder.encodeBuffer();
		}
		if(m_oDecoder.isEndReceived())
		{
			m_oEncoder.encodeEnd();
		}
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
	}

	return true;
}
