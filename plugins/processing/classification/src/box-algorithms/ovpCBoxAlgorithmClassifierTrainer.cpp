#include "ovpCBoxAlgorithmClassifierTrainer.h"

#include <system/ovCMemory.h>
#include <system/ovCMath.h>

#include <sstream>
#include <cmath>
#include <algorithm>

#include <map>

#include <iomanip> // setw

#include <xml/IXMLHandler.h>
#include <xml/IXMLNode.h>

//This needs to reachable from outside
const char* const c_sClassifierRoot = "OpenViBE-Classifier";

const char* const c_sFormatVersionAttributeName = "FormatVersion";

const char* const c_sCreatorAttributeName = "Creator";
const char* const c_sCreatorVersionAttributeName = "CreatorVersion";

const char* const c_sIdentifierAttributeName = "class-id";

const char* const c_sStrategyNodeName = "Strategy-Identifier";
const char* const c_sAlgorithmNodeName = "Algorithm-Identifier";
const char* const c_sStimulationsNodeName = "Stimulations";
const char* const c_sRejectedClassNodeName = "Rejected-Class";
const char* const c_sClassStimulationNodeName = "Class-Stimulation";

const char* const c_sClassificationBoxRoot = "OpenViBE-Classifier-Box";

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Classification;
using namespace std;

boolean CBoxAlgorithmClassifierTrainer::initialize(void)
{
	m_pClassifier = NULL;
	m_pParameter = NULL;

	const IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	//As we add some parameter in the middle of "static" parameters, we cannot rely on settings index.
	m_pParameter = new map<CString , CString> ();
	for(uint32 i = 0 ; i < l_rStaticBoxContext.getSettingCount() ; ++i)
	{
		CString l_pInputName;
		l_rStaticBoxContext.getSettingName(i, l_pInputName);
		CString l_pInputValue = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
		(*m_pParameter)[l_pInputName] = l_pInputValue;
	}

	boolean l_bIsPairing=false;

	CString l_sConfigurationFilename(FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2));

	OV_ERROR_UNLESS_KRF(
		l_sConfigurationFilename != CString(""),
		"Invalid empty configuration filename",
		OpenViBE::Kernel::ErrorType::BadSetting
	);

	CIdentifier l_oStrategyClassIdentifier, l_oClassifierAlgorithmClassIdentifier;

	l_oStrategyClassIdentifier=this->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_ClassificationStrategy, (*m_pParameter)[c_sMulticlassStrategySettingName]);
	l_oClassifierAlgorithmClassIdentifier=this->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_ClassificationAlgorithm, (*m_pParameter)[c_sAlgorithmSettingName]);

	if(l_oStrategyClassIdentifier==OV_UndefinedIdentifier)
	{
		//That means that we want to use a classical algorithm so just let's create it
		const CIdentifier l_oClassifierAlgorithmIdentifier = this->getAlgorithmManager().createAlgorithm(l_oClassifierAlgorithmClassIdentifier);

		OV_ERROR_UNLESS_KRF(
			l_oClassifierAlgorithmIdentifier != OV_UndefinedIdentifier,
			"Unable to instantiate classifier for class [" << l_oClassifierAlgorithmIdentifier.toString() << "]",
			OpenViBE::Kernel::ErrorType::BadConfig
		);

		m_pClassifier=&this->getAlgorithmManager().getAlgorithm(l_oClassifierAlgorithmIdentifier);
		m_pClassifier->initialize();
	}
	else
	{
		l_bIsPairing = true;
		m_pClassifier=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(l_oStrategyClassIdentifier));
		m_pClassifier->initialize();
	}
	m_ui64TrainStimulation = this->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation ,(*m_pParameter)[c_sTrainTriggerSettingName]);

	int64 l_i64PartitionCount = this->getConfigurationManager().expandAsInteger((*m_pParameter)[c_sKFoldSettingName]);

	OV_ERROR_UNLESS_KRF(
		l_i64PartitionCount >= 0,
		"Invalid partition count [" << l_i64PartitionCount << "] (expected value >= 0)",
		OpenViBE::Kernel::ErrorType::BadSetting
	);

	m_ui64PartitionCount=uint64(l_i64PartitionCount);

	m_oStimulationDecoder.initialize(*this,0);
	for(uint32 i=1; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		m_vFeatureVectorDecoder.push_back(new OpenViBEToolkit::TFeatureVectorDecoder<CBoxAlgorithmClassifierTrainer>());
		m_vFeatureVectorDecoder.back()->initialize(*this, i);
	}

	//We link the parameters to the extra parameters input parameter to transmit them
	TParameterHandler < map<CString , CString> * > ip_pExtraParameter(m_pClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter));
	ip_pExtraParameter = m_pParameter;

	m_oStimulationEncoder.initialize(*this, 0);

	m_vFeatureCount.clear();

	OV_ERROR_UNLESS_KRF(
		l_rStaticBoxContext.getInputCount() >= 2,
		"Invalid input count [" << l_rStaticBoxContext.getInputCount() << "] (at least 2 input expected)",
		OpenViBE::Kernel::ErrorType::BadSetting
	);

	// Provide the number of classes to the classifier
	const uint32 l_ui32ClassCount = l_rStaticBoxContext.getInputCount() - 1;
	TParameterHandler< uint64 > ip_NumClasses(m_pClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_NumberOfClasses));
	ip_NumClasses = l_ui32ClassCount;

	//If we have to deal with a pairing strategy we have to pass argument
	if(l_bIsPairing)
	{
		TParameterHandler < CIdentifier* > ip_oClassId(m_pClassifier->getInputParameter(OVTK_Algorithm_PairingStrategy_InputParameterId_SubClassifierAlgorithm));
		ip_oClassId = &l_oClassifierAlgorithmClassIdentifier;

		OV_ERROR_UNLESS_KRF(
			m_pClassifier->process(OVTK_Algorithm_PairingStrategy_InputTriggerId_DesignArchitecture),
			"Failed to design architecture",
			OpenViBE::Kernel::ErrorType::Internal
		);
	}

	return true;
}

boolean CBoxAlgorithmClassifierTrainer::uninitialize(void)
{
	m_oStimulationDecoder.uninitialize();
	m_oStimulationEncoder.uninitialize();

	if(m_pClassifier)
	{
		m_pClassifier->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pClassifier);
	}

	for(uint32 i=0; i<m_vFeatureVectorDecoder.size(); i++)
	{
		m_vFeatureVectorDecoder[i]->uninitialize();
		delete m_vFeatureVectorDecoder[i];
	}
	m_vFeatureVectorDecoder.clear();

	m_oStimulationEncoder.uninitialize();

	m_oStimulationDecoder.uninitialize();

	for(uint32 i=0;i<m_vDataset.size();i++) {
		delete m_vDataset[i].m_pFeatureVectorMatrix;
		m_vDataset[i].m_pFeatureVectorMatrix = NULL;
	}
	m_vDataset.clear();

	if(m_pParameter)
	{
		delete m_pParameter;
		m_pParameter = NULL;
	}

	// @fixme who frees this? freeing here -> crash
	/*
	if(m_pExtraParameter != NULL)
	{
		delete m_pExtraParameter;
		m_pExtraParameter = NULL;
	}
	*/

	return true;
}

boolean CBoxAlgorithmClassifierTrainer::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

// Find the most likely class and resample the dataset so that each class is as likely
boolean CBoxAlgorithmClassifierTrainer::balanceDataset()
{
	const IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	const uint32 l_ui32ClassCount = l_rStaticBoxContext.getInputCount() - 1;

	this->getLogManager() << LogLevel_Info << "Balancing dataset...\n";

	// Collect index set of feature vectors per class
	std::vector< std::vector<uint32> > l_vClassIndexes;
	l_vClassIndexes.resize(l_ui32ClassCount);
	for(size_t i=0; i<m_vDataset.size();i++)
	{
		l_vClassIndexes[m_vDataset[i].m_ui32InputIndex].push_back(i);
	}

	// Count how many vectors the largest class has
	uint32 l_ui32MaxCount = 0;
	for(uint32 i=0;i<l_ui32ClassCount;i++) {
		l_ui32MaxCount = std::max<uint32>(l_ui32MaxCount,l_vClassIndexes[i].size());
	}

	m_vBalancedDataset.clear();

	// Pad those classes with resampled examples (sampling with replacement) that have fewer examples than the largest class
	for(uint32 i=0;i<l_ui32ClassCount;i++)
	{
		const uint32 l_ui32ExamplesInClass = l_vClassIndexes[i].size();
		const uint32 l_ui32PaddingNeeded = l_ui32MaxCount - l_ui32ExamplesInClass;
		if(l_ui32ExamplesInClass == 0)
		{
			this->getLogManager() << LogLevel_Debug << "Cannot resample class " << i << ", 0 examples\n";
			continue;
		}
		else if(l_ui32PaddingNeeded>0)
		{
			this->getLogManager() << LogLevel_Debug << "Padding class " << i << " with " << l_ui32PaddingNeeded << " examples\n";
		}

		// Copy all the examples first to a temporary array so we don't mess with the original data.
		// This is not too bad as instead of data, we copy the pointer. m_vDataset owns the data pointer.
		const std::vector<uint32>& l_vThisClassesIndexes = l_vClassIndexes[i];
		for(uint32 j=0;j<l_ui32ExamplesInClass;j++)
		{
			m_vBalancedDataset.push_back(m_vDataset[l_vThisClassesIndexes[j]]);
		}

		for(uint32 j=0;j<l_ui32PaddingNeeded;j++)
		{
			const uint32 l_ui32SampledIndex = System::Math::randomUInteger32WithCeiling(l_ui32ExamplesInClass);
			const SFeatureVector& l_rSourceVector = m_vDataset[l_vThisClassesIndexes[l_ui32SampledIndex]];
			m_vBalancedDataset.push_back(l_rSourceVector);
		}
	}

	return true;
}


boolean CBoxAlgorithmClassifierTrainer::process(void)
{
	const IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	uint32 i, j;
	boolean l_bTrainStimulationReceived=false;

	// Parses stimulations
	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_oStimulationDecoder.decode(i);

		if(m_oStimulationDecoder.isHeaderReceived())
		{
			m_oStimulationEncoder.encodeHeader();
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, 0, 0);
		}
		if(m_oStimulationDecoder.isBufferReceived())
		{
			const IStimulationSet* l_oInputStimulationSet = m_oStimulationDecoder.getOutputStimulationSet();
			IStimulationSet* l_oOutputStimulationSet = m_oStimulationEncoder.getInputStimulationSet();
			l_oOutputStimulationSet->clear();

			for(uint64 j=0; j<l_oInputStimulationSet->getStimulationCount(); j++)
			{
				if(l_oInputStimulationSet->getStimulationIdentifier(j)==m_ui64TrainStimulation)
				{
					l_bTrainStimulationReceived=true;

					uint64 l_ui32TrainCompletedStimulation = this->getTypeManager().getEnumerationEntryValueFromName(OV_TypeId_Stimulation,"OVTK_StimulationId_TrainCompleted");
					l_oOutputStimulationSet->appendStimulation(l_ui32TrainCompletedStimulation, l_oInputStimulationSet->getStimulationDate(j), 0);
				}
			}
			m_oStimulationEncoder.encodeBuffer();

			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
		if(m_oStimulationDecoder.isEndReceived())
		{
			m_oStimulationEncoder.encodeEnd();

			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
	}

	// Parses feature vectors
	for(i=1; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		for(j=0; j<l_rDynamicBoxContext.getInputChunkCount(i); j++)
		{
			m_vFeatureVectorDecoder[i-1]->decode(j);

			if(m_vFeatureVectorDecoder[i-1]->isHeaderReceived())
			{
			}
			if(m_vFeatureVectorDecoder[i-1]->isBufferReceived())
			{
				const IMatrix* pFeatureVectorMatrix = m_vFeatureVectorDecoder[i-1]->getOutputMatrix();

				CBoxAlgorithmClassifierTrainer::SFeatureVector l_oFeatureVector;
				l_oFeatureVector.m_pFeatureVectorMatrix=new CMatrix();
				l_oFeatureVector.m_ui64StartTime=l_rDynamicBoxContext.getInputChunkStartTime(i, j);
				l_oFeatureVector.m_ui64EndTime=l_rDynamicBoxContext.getInputChunkEndTime(i, j);
				l_oFeatureVector.m_ui32InputIndex=i-1;

				OpenViBEToolkit::Tools::Matrix::copy(*l_oFeatureVector.m_pFeatureVectorMatrix, *pFeatureVectorMatrix);
				m_vDataset.push_back(l_oFeatureVector);
				m_vFeatureCount[i]++;
			}
			if(m_vFeatureVectorDecoder[i-1]->isEndReceived())
			{
			}
		}
	}

	// On train stimulation reception, build up the labelled feature vector set matrix and go on training
	if(l_bTrainStimulationReceived)
	{
		OV_ERROR_UNLESS_KRF(
			m_vDataset.size() >= m_ui64PartitionCount,
			"Received fewer examples (" << static_cast<uint32>(m_vDataset.size()) << ") than specified partition count (" << m_ui64PartitionCount << ")",
			OpenViBE::Kernel::ErrorType::BadInput
		);

		OV_ERROR_UNLESS_KRF(
			!m_vDataset.empty(),
			"No training example received",
			OpenViBE::Kernel::ErrorType::BadInput
		);

		this->getLogManager() << LogLevel_Info << "Received train stimulation. Data dim is [" << (uint32) m_vDataset.size() << "x"
			<< m_vDataset[0].m_pFeatureVectorMatrix->getBufferElementCount() << "]\n";
		for(i=1; i<l_rStaticBoxContext.getInputCount(); i++)
		{
			this->getLogManager() << LogLevel_Info << "For information, we have " << m_vFeatureCount[i] << " feature vector(s) for input " << i << "\n";
		}

		const boolean l_bBalanceDataset = this->getConfigurationManager().expandAsBoolean((*m_pParameter)[c_sBalanceSettingName]);
		if(l_bBalanceDataset)
		{
			balanceDataset();
		}

		const std::vector < SFeatureVector >& l_rActualDataset = (l_bBalanceDataset ? m_vBalancedDataset : m_vDataset);

		vector<float64> l_vPartitionAccuracies((unsigned int)m_ui64PartitionCount);

		const bool l_bRandomizeVectorOrder = this->getConfigurationManager().expandAsBoolean("${Plugin_Classification_RandomizeKFoldTestData}", false);

		// create a vector used for mapping feature vectors (initialize it as v[i] = i)
		std::vector < size_t > l_vFeaturePermutation;
		for (size_t i = 0; i < l_rActualDataset.size(); i++)
		{
			l_vFeaturePermutation.push_back(i);
		}

		// randomize the vector if necessary
		if (l_bRandomizeVectorOrder)
		{
			this->getLogManager() << LogLevel_Info << "Randomizing the feature vector set\n";
			random_shuffle(l_vFeaturePermutation.begin(), l_vFeaturePermutation.end(), System::Math::randomUInteger32WithCeiling);
		}

		const uint32 l_ui32ClassCount = l_rStaticBoxContext.getInputCount() - 1;
		CMatrix l_oConfusion;
		l_oConfusion.setDimensionCount(2);
		l_oConfusion.setDimensionSize(0, l_ui32ClassCount);
		l_oConfusion.setDimensionSize(1, l_ui32ClassCount);

		if(m_ui64PartitionCount>=2)
		{
			float64 l_f64PartitionAccuracy=0;
			float64 l_f64FinalAccuracy=0;

			OpenViBEToolkit::Tools::Matrix::clearContent(l_oConfusion);

			this->getLogManager() << LogLevel_Info << "k-fold test could take quite a long time, be patient\n";
			for(uint64 i=0; i<m_ui64PartitionCount; i++)
			{
				const size_t l_uiStartIndex=(size_t)(((i  )*l_rActualDataset.size())/m_ui64PartitionCount);
				const size_t l_uiStopIndex =(size_t)(((i+1)*l_rActualDataset.size())/m_ui64PartitionCount);

				this->getLogManager() << LogLevel_Trace << "Training on partition " << i << " (feature vectors " << (uint32)l_uiStartIndex << " to " << (uint32)l_uiStopIndex-1 << ")...\n";

				OV_ERROR_UNLESS_KRF(
					this->train(l_rActualDataset, l_vFeaturePermutation, l_uiStartIndex, l_uiStopIndex),
					"Training failed: bailing out (from xval)",
					OpenViBE::Kernel::ErrorType::Internal
				);

				l_f64PartitionAccuracy = this->getAccuracy(l_rActualDataset, l_vFeaturePermutation, l_uiStartIndex, l_uiStopIndex, l_oConfusion);
				l_vPartitionAccuracies[(unsigned int)i]=l_f64PartitionAccuracy;
				l_f64FinalAccuracy+=l_f64PartitionAccuracy;

				this->getLogManager() << LogLevel_Info << "Finished with partition " << i+1 << " / " << m_ui64PartitionCount << " (performance : " << l_f64PartitionAccuracy << "%)\n";
			}

			const float64 l_fMean = l_f64FinalAccuracy/m_ui64PartitionCount;
			float64 l_fDeviation = 0;

			for (uint64 i = 0; i < m_ui64PartitionCount; i++)
			{
				const float64 l_fDiff = l_vPartitionAccuracies[(unsigned int)i] - l_fMean;
				l_fDeviation += l_fDiff * l_fDiff;
			}
			l_fDeviation = sqrt( l_fDeviation / m_ui64PartitionCount );

			this->getLogManager() << LogLevel_Info << "Cross-validation test accuracy is " << l_fMean << "% (sigma = " << l_fDeviation << "%)\n";

			printConfusionMatrix(l_oConfusion);
		}
		else
		{
			this->getLogManager() << LogLevel_Info << "Training without cross-validation.\n";
			this->getLogManager() << LogLevel_Info << "*** Reported training set accuracy will be optimistic ***\n";
		}


		this->getLogManager() << LogLevel_Trace << "Training final classifier on the whole set...\n";

		OV_ERROR_UNLESS_KRF(
			this->train(l_rActualDataset, l_vFeaturePermutation, 0, 0),
			"Training failed: bailing out (from whole set training)",
			OpenViBE::Kernel::ErrorType::Internal
		);

		OpenViBEToolkit::Tools::Matrix::clearContent(l_oConfusion);
		const float64 l_f64TrainAccuracy = this->getAccuracy(l_rActualDataset, l_vFeaturePermutation, 0, l_rActualDataset.size(), l_oConfusion);

		this->getLogManager() << LogLevel_Info << "Training set accuracy is " << l_f64TrainAccuracy << "% (optimistic)\n";

		printConfusionMatrix(l_oConfusion);

		OV_ERROR_UNLESS_KRF(
			this->saveConfiguration(),
			"Failed to save configuration",
			OpenViBE::Kernel::ErrorType::Internal
		);
	}

	return true;
}

boolean CBoxAlgorithmClassifierTrainer::train(const std::vector < SFeatureVector >& rDataset,
	const std::vector< size_t >& rPermutation, const size_t uiStartIndex, const size_t uiStopIndex)
{
	OV_ERROR_UNLESS_KRF(
		uiStopIndex - uiStartIndex != 1,
		"Invalid indexes: stopIndex - trainIndex = 1",
		OpenViBE::Kernel::ErrorType::BadArgument
	);

	const uint32 l_ui32FeatureVectorCount=rDataset.size()-(uiStopIndex-uiStartIndex);
	const uint32 l_ui32FeatureVectorSize=rDataset[0].m_pFeatureVectorMatrix->getBufferElementCount();

	TParameterHandler < IMatrix* > ip_pFeatureVectorSet(m_pClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_FeatureVectorSet));

	ip_pFeatureVectorSet->setDimensionCount(2);
	ip_pFeatureVectorSet->setDimensionSize(0, l_ui32FeatureVectorCount);
	ip_pFeatureVectorSet->setDimensionSize(1, l_ui32FeatureVectorSize+1);

	float64* l_pFeatureVectorSetBuffer=ip_pFeatureVectorSet->getBuffer();
	for(size_t j=0; j<rDataset.size()-(uiStopIndex-uiStartIndex); j++)
	{
		size_t k=rPermutation[(j<uiStartIndex?j:j+(uiStopIndex-uiStartIndex))];
		float64 l_f64Class=(float64)rDataset[k].m_ui32InputIndex;
		System::Memory::copy(
			l_pFeatureVectorSetBuffer,
			rDataset[k].m_pFeatureVectorMatrix->getBuffer(),
			l_ui32FeatureVectorSize*sizeof(float64));

		l_pFeatureVectorSetBuffer[l_ui32FeatureVectorSize]=l_f64Class;
		l_pFeatureVectorSetBuffer+=(l_ui32FeatureVectorSize+1);
	}

	OV_ERROR_UNLESS_KRF(
		m_pClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_Train),
		"Training failed",
		OpenViBE::Kernel::ErrorType::Internal
	);

	TParameterHandler < XML::IXMLNode* > op_pConfiguration(m_pClassifier->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration));
	XML::IXMLNode *l_pTempNode = (XML::IXMLNode*)op_pConfiguration;

	if(l_pTempNode != NULL){
		l_pTempNode->release();
	}
	op_pConfiguration = NULL;

	bool l_bReturnValue = m_pClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_SaveConfiguration);

	return l_bReturnValue;
}

// Note that this function is incremental for oConfusionMatrix and can be called many times; so we don't clear the matrix
float64 CBoxAlgorithmClassifierTrainer::getAccuracy(const std::vector < CBoxAlgorithmClassifierTrainer::SFeatureVector >& rDataset,
	const std::vector< size_t >& rPermutation, const size_t uiStartIndex, const size_t uiStopIndex, CMatrix& oConfusionMatrix)
{
	OV_ERROR_UNLESS_KRF(
		uiStopIndex != uiStartIndex,
		"Invalid indexes: start index equals stop index",
		OpenViBE::Kernel::ErrorType::BadArgument
	);

	const uint32 l_ui32FeatureVectorSize=rDataset[0].m_pFeatureVectorMatrix->getBufferElementCount();

	TParameterHandler < XML::IXMLNode* > op_pConfiguration(m_pClassifier->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration));
	XML::IXMLNode * l_pNode = op_pConfiguration;//Requested for affectation
	TParameterHandler < XML::IXMLNode* > ip_pConfiguration(m_pClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_Configuration));
	ip_pConfiguration = l_pNode;

	m_pClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_LoadConfiguration);

	TParameterHandler < IMatrix* > ip_pFeatureVector(m_pClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_FeatureVector));
	TParameterHandler < float64 > op_f64ClassificationStateClass(m_pClassifier->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Class));
	ip_pFeatureVector->setDimensionCount(1);
	ip_pFeatureVector->setDimensionSize(0, l_ui32FeatureVectorSize);

	size_t l_iSuccessCount=0;

	for(size_t j=uiStartIndex; j<uiStopIndex; j++)
	{
		const size_t k = rPermutation[j];

		float64* l_pFeatureVectorBuffer=ip_pFeatureVector->getBuffer();
		const float64 l_f64CorrectValue=(float64)rDataset[k].m_ui32InputIndex;

		this->getLogManager() << LogLevel_Debug << "Try to recognize " << l_f64CorrectValue << "\n";

		System::Memory::copy(
			l_pFeatureVectorBuffer,
			rDataset[k].m_pFeatureVectorMatrix->getBuffer(),
			l_ui32FeatureVectorSize*sizeof(float64));

		m_pClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_Classify);

		const float64 l_f64PredictedValue = op_f64ClassificationStateClass;

		this->getLogManager() << LogLevel_Debug << "Recognize " << l_f64PredictedValue << "\n";

		if(l_f64PredictedValue==l_f64CorrectValue)
		{
			l_iSuccessCount++;
		}

		if(l_f64PredictedValue<oConfusionMatrix.getDimensionSize(0) && l_f64CorrectValue<oConfusionMatrix.getDimensionSize(0))
		{
			float64* buf = oConfusionMatrix.getBuffer();
			buf[static_cast<uint32>(l_f64CorrectValue)*oConfusionMatrix.getDimensionSize(1) +
				static_cast<uint32>(l_f64PredictedValue)] += 1.0;
		}
		else
		{
			std::cout << "errorn\n";
		}
	}

	return static_cast<float64>((l_iSuccessCount*100.0)/(uiStopIndex-uiStartIndex));
}

boolean CBoxAlgorithmClassifierTrainer::printConfusionMatrix(const CMatrix& oMatrix)
{
	OV_ERROR_UNLESS_KRF(
		oMatrix.getDimensionCount() == 2 && oMatrix.getDimensionSize(0) == oMatrix.getDimensionSize(1),
		"Invalid confution matrix [dim count = " << oMatrix.getDimensionCount() << ", dim size 0 = "
		<< oMatrix.getDimensionSize(0) << ", dim size 1 = "<< oMatrix.getDimensionSize(1) << "] (expected 2 dimensions with same size)",
		OpenViBE::Kernel::ErrorType::BadArgument
	);

	const uint32 l_ui32Rows = oMatrix.getDimensionSize(0);

	if(l_ui32Rows>10 && !this->getConfigurationManager().expandAsBoolean("${Plugin_Classification_ForceConfusionMatrixPrint}"))
	{
		this->getLogManager() << LogLevel_Info << "Over 10 classes, not printing the confusion matrix. If needed, override with setting Plugin_Classification_ForceConfusionMatrixPrint token to true.\n";
		return true;
	}

	// Normalize
	CMatrix l_oTmp,l_oRowSum;
	OpenViBEToolkit::Tools::Matrix::copy(l_oTmp, oMatrix);
	l_oRowSum.setDimensionCount(1);
	l_oRowSum.setDimensionSize(0,l_ui32Rows);
	OpenViBEToolkit::Tools::Matrix::clearContent(l_oRowSum);

	for(uint32 i=0;i<l_ui32Rows;i++) {
		for(uint32 j=0;j<l_ui32Rows;j++) {
			l_oRowSum[i] += l_oTmp[i*l_ui32Rows+j];
		}
		for(uint32 j=0;j<l_ui32Rows;j++) {
			l_oTmp[i*l_ui32Rows+j] /= l_oRowSum[i];
		}
	}

	std::stringstream ss;
	ss << std::fixed;

	ss << "  Cls vs cls ";
	for(uint32 i=0;i<l_ui32Rows;i++) {
		ss << setw(6) << (i+1);
	}
	this->getLogManager() << LogLevel_Info << ss.str().c_str()<< "\n";

	ss.precision(1);
	for(uint32 i=0;i<l_ui32Rows;i++) {
		ss.str("") ;
		ss << "  Target " << setw(2) << (i+1) << ": ";
		for(uint32 j=0;j<l_ui32Rows;j++) {
			ss << setw(6) << l_oTmp[i*l_ui32Rows+j]*100;
		}
		this->getLogManager() << LogLevel_Info << ss.str().c_str() << " %, " << static_cast<uint32>(l_oRowSum[i]) << " examples\n";
	}

	return true;
}

boolean CBoxAlgorithmClassifierTrainer::saveConfiguration(void)
{
	CIdentifier l_oStrategyClassIdentifier, l_oClassifierAlgorithmClassIdentifier;
	const IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	TParameterHandler < XML::IXMLNode* > op_pConfiguration(m_pClassifier->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration));
	XML::IXMLNode* l_pAlgorithmConfigurationNode = XML::createNode(c_sClassifierRoot);
	l_pAlgorithmConfigurationNode->addChild((XML::IXMLNode*)op_pConfiguration);

	XML::IXMLHandler *l_pHandler = XML::createXMLHandler();
	CString l_sConfigurationFilename(this->getConfigurationManager().expand((*m_pParameter)[c_sFilenameSettingName]));

	CString l_sStrategyClassIdentifier = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	CString l_sClassifierAlgorithmClassIdentifier = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);

	XML::IXMLNode *l_sRoot = XML::createNode(c_sClassificationBoxRoot);
	std::stringstream l_sVersion;
	l_sVersion << OVP_Classification_BoxTrainerFormatVersion;
	l_sRoot->addAttribute(c_sFormatVersionAttributeName, l_sVersion.str().c_str());

	auto cleanup = [&](){
		l_pHandler->release();
		l_sRoot->release();
		op_pConfiguration=NULL;
	};
	l_sRoot->addAttribute(c_sCreatorAttributeName, this->getConfigurationManager().expand("${Application_Name}"));
	l_sRoot->addAttribute(c_sCreatorVersionAttributeName, this->getConfigurationManager().expand("${Application_Version}"));

	XML::IXMLNode *l_pTempNode = XML::createNode(c_sStrategyNodeName);
	l_oStrategyClassIdentifier = this->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_ClassificationStrategy, (*m_pParameter)[c_sMulticlassStrategySettingName]);
	l_pTempNode->addAttribute(c_sIdentifierAttributeName, l_oStrategyClassIdentifier.toString());
	l_pTempNode->setPCData((*m_pParameter)[c_sMulticlassStrategySettingName].toASCIIString());
	l_sRoot->addChild(l_pTempNode);

	l_pTempNode = XML::createNode(c_sAlgorithmNodeName);
	l_oClassifierAlgorithmClassIdentifier = this->getTypeManager().getEnumerationEntryValueFromName(OVTK_TypeId_ClassificationAlgorithm, (*m_pParameter)[c_sAlgorithmSettingName]);
	l_pTempNode->addAttribute(c_sIdentifierAttributeName, l_oClassifierAlgorithmClassIdentifier.toString());
	l_pTempNode->setPCData((*m_pParameter)[c_sAlgorithmSettingName].toASCIIString());
	l_sRoot->addChild(l_pTempNode);


	XML::IXMLNode *l_pStimulationsNode = XML::createNode(c_sStimulationsNodeName);

	for(OpenViBE::uint32 i =1 ; i < l_rStaticBoxContext.getInputCount() ; ++i)
	{
		char l_sBufferSettingName[64];
		sprintf(l_sBufferSettingName, "Class %d label", i);

		l_pTempNode = XML::createNode(c_sClassStimulationNodeName);
		std::stringstream l_sBuffer;
		l_sBuffer << (i-1);
		l_pTempNode->addAttribute(c_sIdentifierAttributeName, l_sBuffer.str().c_str());
		l_pTempNode->setPCData((*m_pParameter)[l_sBufferSettingName].toASCIIString());
		l_pStimulationsNode->addChild(l_pTempNode);
	}
	l_sRoot->addChild(l_pStimulationsNode);

	l_sRoot->addChild(l_pAlgorithmConfigurationNode);

	if(!l_pHandler->writeXMLInFile(*l_sRoot, l_sConfigurationFilename.toASCIIString()))
	{
		cleanup();
		OV_ERROR_KRF("Failed saving configuration to file [" << l_sConfigurationFilename << "]", OpenViBE::Kernel::ErrorType::BadFileWrite);
	}

	cleanup();
	return true;
}
