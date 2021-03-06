#include "ovpCSpectrumEncoder.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::StreamCodecs;

boolean CSpectrumEncoder::initialize(void)
{
	CStreamedMatrixEncoder::initialize();
	ip_pFrequencyAbscissa.initialize(getInputParameter(OVP_Algorithm_SpectrumStreamEncoder_InputParameterId_FrequencyAbscissa));
	ip_pSamplingRate.initialize(getInputParameter(OVP_Algorithm_SpectrumStreamEncoder_InputParameterId_SamplingRate));
	return true;
}

boolean CSpectrumEncoder::uninitialize(void)
{
	ip_pFrequencyAbscissa.uninitialize();
	ip_pSamplingRate.uninitialize();

	CStreamedMatrixEncoder::uninitialize();

	return true;
}

// ________________________________________________________________________________________________________________
//

boolean CSpectrumEncoder::processHeader(void)
{
	// ip_pFrequencyAbscissa dimension count should be 1
	// ip_pFrequencyAbscissa dimension size 0 should be the same as streamed matrix dimension size 1

	IMatrix* frequencyAbscissa = ip_pFrequencyAbscissa;
	uint64 samplingRate = ip_pSamplingRate;
	CStreamedMatrixEncoder::processHeader();
	m_pEBMLWriterHelper->openChild(OVTK_NodeId_Header_Spectrum);
	m_pEBMLWriterHelper->openChild(OVTK_NodeId_Header_Spectrum_SamplingRate);
	m_pEBMLWriterHelper->setUIntegerAsChildData(samplingRate);
	m_pEBMLWriterHelper->closeChild();
	for(uint32 i=0; i<frequencyAbscissa->getDimensionSize(0); i++)
	{
		m_pEBMLWriterHelper->openChild(OVTK_NodeId_Header_Spectrum_FrequencyAbscissa);
		m_pEBMLWriterHelper->setFloat64AsChildData(frequencyAbscissa->getBuffer()[i]);
		m_pEBMLWriterHelper->closeChild();
	}
	m_pEBMLWriterHelper->closeChild();

	return true;
}
