
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "stdafx.h"
#include "XmlEvaluationReader.h"

using namespace PRImA;
using namespace std;

/*
 * Class CXmlEvaluationReader
 *
 * Class to read layout evaluation results and profiles from XML files.
 *
 * CC 15.01.2010 - created
 * CC 10.06.2010 - switched to MS XML
 * CC 22.07.2013 - new schema
 */

const wchar_t* CXmlEvaluationReader::ATTR_evalId			= _T("evalId");
const wchar_t* CXmlEvaluationReader::ATTR_imageFilename		= _T("imageFilename");
const wchar_t* CXmlEvaluationReader::ATTR_imageWidth		= _T("imageWidth");
const wchar_t* CXmlEvaluationReader::ATTR_imageHeight		= _T("imageHeight");
const wchar_t* CXmlEvaluationReader::ATTR_id				= _T("id");
const wchar_t* CXmlEvaluationReader::ATTR_groundTruthFilename			= _T("groundTruthFilename");		
const wchar_t* CXmlEvaluationReader::ATTR_segmentationResultFilename	= _T("segmentationResultFilename");
const wchar_t* CXmlEvaluationReader::ATTR_version			= _T("version");
const wchar_t* CXmlEvaluationReader::ATTR_producer			= _T("producer");

const wchar_t* CXmlEvaluationReader::ATTR_name				= _T("name");
const wchar_t* CXmlEvaluationReader::ATTR_weight			= _T("weight");
const wchar_t* CXmlEvaluationReader::ATTR_allowableWeight	= _T("allowableWeight");
const wchar_t* CXmlEvaluationReader::ATTR_enableAllowable	= _T("enableAllowable");
const wchar_t* CXmlEvaluationReader::ATTR_useAllowable		= _T("useAllowable");
const wchar_t* CXmlEvaluationReader::ATTR_type				= _T("type");
const wchar_t* CXmlEvaluationReader::ATTR_subtype			= _T("subtype");
const wchar_t* CXmlEvaluationReader::ATTR_value				= _T("value");

const wchar_t* CXmlEvaluationReader::ATTR_regionId			= _T("regionId");
const wchar_t* CXmlEvaluationReader::ATTR_regionId1			= _T("regionId1");
const wchar_t* CXmlEvaluationReader::ATTR_regionId2			= _T("regionId2");

const wchar_t* CXmlEvaluationReader::ATTR_weightedAreaError		= _T("weightedAreaError");
const wchar_t* CXmlEvaluationReader::ATTR_weightedCountError	= _T("weightedCountError");
const wchar_t* CXmlEvaluationReader::ATTR_area					= _T("area");
const wchar_t* CXmlEvaluationReader::ATTR_foregroundPixelCount	= _T("foregroundPixelCount");
const wchar_t* CXmlEvaluationReader::ATTR_count					= _T("count	");
const wchar_t* CXmlEvaluationReader::ATTR_falseAlarm			= _T("falseAlarm");
const wchar_t* CXmlEvaluationReader::ATTR_allowable				= _T("allowable");

const wchar_t* CXmlEvaluationReader::ATTR_penalty				= _T("penalty");
const wchar_t* CXmlEvaluationReader::ATTR_groundTruthRelation	= _T("groundTruthRelation");
const wchar_t* CXmlEvaluationReader::ATTR_segResultRelation		= _T("segResultRelation");

const wchar_t* CXmlEvaluationReader::ATTR_rects		= _T("rects");
const wchar_t* CXmlEvaluationReader::ATTR_left		= _T("left");
const wchar_t* CXmlEvaluationReader::ATTR_top		= _T("top");
const wchar_t* CXmlEvaluationReader::ATTR_right		= _T("right");
const wchar_t* CXmlEvaluationReader::ATTR_bottom	= _T("bottom");

const wchar_t* CXmlEvaluationReader::ATTR_numberOfGroundTruthRegions		= _T("numberOfGroundTruthRegions");
const wchar_t* CXmlEvaluationReader::ATTR_numberOfSegResultRegions			= _T("numberOfSegResultRegions");
const wchar_t* CXmlEvaluationReader::ATTR_imageArea							= _T("imageArea");
const wchar_t* CXmlEvaluationReader::ATTR_overallGroundTruthRegionArea		= _T("overallGroundTruthRegionArea");
const wchar_t* CXmlEvaluationReader::ATTR_overallGroundTruthRegionPixelCount= _T("overallGroundTruthRegionPixelCount");
const wchar_t* CXmlEvaluationReader::ATTR_overallSegResultRegionArea		= _T("overallSegResultRegionArea");
const wchar_t* CXmlEvaluationReader::ATTR_overallSegResultRegionPixelCount	= _T("overallSegResultRegionPixelCount");
const wchar_t* CXmlEvaluationReader::ATTR_overallWeightedAreaError			= _T("overallWeightedAreaError");
const wchar_t* CXmlEvaluationReader::ATTR_overallWeightedCountError			= _T("overallWeightedCountError");
const wchar_t* CXmlEvaluationReader::ATTR_readingOrderError					= _T("readingOrderError");
const wchar_t* CXmlEvaluationReader::ATTR_readingOrderSuccessRate			= _T("readingOrderSuccessRate");
const wchar_t* CXmlEvaluationReader::ATTR_overallWeightedAreaSuccessRate	= _T("overallWeightedAreaSuccessRate");
const wchar_t* CXmlEvaluationReader::ATTR_overallWeightedCountSuccessRate	= _T("overallWeightedCountSuccessRate");
const wchar_t* CXmlEvaluationReader::ATTR_harmonicWeightedAreaSuccessRate	= _T("harmonicWeightedAreaSuccessRate");
const wchar_t* CXmlEvaluationReader::ATTR_harmonicWeightedCountSuccessRate	= _T("harmonicWeightedCountSuccessRate");
const wchar_t* CXmlEvaluationReader::ATTR_errorRateBasedOnSimpleCount		= _T("errorRateBasedOnSimpleCount");
const wchar_t* CXmlEvaluationReader::ATTR_recallNonStrict					= _T("recallNonStrict");
const wchar_t* CXmlEvaluationReader::ATTR_recallStrict						= _T("recallStrict");
const wchar_t* CXmlEvaluationReader::ATTR_precisionNonStrict				= _T("precisionNonStrict");
const wchar_t* CXmlEvaluationReader::ATTR_precisionStrict					= _T("precisionStrict");
const wchar_t* CXmlEvaluationReader::ATTR_fMeasureStrict					= _T("fMeasureStrict");
const wchar_t* CXmlEvaluationReader::ATTR_fMeasureNonStrict					= _T("fMeasureNonStrict");
const wchar_t* CXmlEvaluationReader::ATTR_regionCountDeviation				= _T("regionCountDeviation");
const wchar_t* CXmlEvaluationReader::ATTR_relativeRegionCountDeviation		= _T("relativeRegionCountDeviation");
const wchar_t* CXmlEvaluationReader::ATTR_OCRSuccessRate					= _T("ocrSuccessRate");
const wchar_t* CXmlEvaluationReader::ATTR_OCRSuccessRateExclReplacementChar = _T("ocrSuccessRateExclReplacementChar");
const wchar_t* CXmlEvaluationReader::ATTR_OCRSuccessRateForDigits			= _T("ocrSuccessRateForDigits");
const wchar_t* CXmlEvaluationReader::ATTR_OCRSuccessRateForNumericalChars	= _T("ocrSuccessRateForNumericalChars");

const wchar_t* CXmlEvaluationReader::ATTR_row		= _T("row");
const wchar_t* CXmlEvaluationReader::ATTR_col		= _T("col");

const wchar_t* CXmlEvaluationReader::ATTR_includedBackgroundSuccessRate	= _T("includedBackgroundSuccessRate");
const wchar_t* CXmlEvaluationReader::ATTR_excludedForegroundSuccessRate	= _T("excludedForegroundSuccessRate");
const wchar_t* CXmlEvaluationReader::ATTR_missingRegionAreaSuccessRate	= _T("missingRegionAreaSuccessRate");
const wchar_t* CXmlEvaluationReader::ATTR_overallSuccessRate			= _T("overallSuccessRate");


const wchar_t* CXmlEvaluationReader::ELEMENT_Eval			= _T("Eval");
const wchar_t* CXmlEvaluationReader::ELEMENT_EvalData		= _T("EvalData");
const wchar_t* CXmlEvaluationReader::ELEMENT_Profile		= _T("Profile");

const wchar_t* CXmlEvaluationReader::ELEMENT_Metadata		= _T("Metadata");
const wchar_t* CXmlEvaluationReader::ELEMENT_Creator		= _T("Creator");
const wchar_t* CXmlEvaluationReader::ELEMENT_Created		= _T("Created");
const wchar_t* CXmlEvaluationReader::ELEMENT_LastChange		= _T("LastChange");
const wchar_t* CXmlEvaluationReader::ELEMENT_Comments		= _T("Comments");
const wchar_t* CXmlEvaluationReader::ELEMENT_Software		= _T("Software");

const wchar_t* CXmlEvaluationReader::ELEMENT_GeneralSettings		= _T("GeneralSettings");
const wchar_t* CXmlEvaluationReader::ELEMENT_ErrorTypeWeights		= _T("ErrorTypeWeights");
const wchar_t* CXmlEvaluationReader::ELEMENT_RegionTypeWeights		= _T("RegionTypeWeights");
const wchar_t* CXmlEvaluationReader::ELEMENT_TextLineWeights		= _T("TextLineWeights");
const wchar_t* CXmlEvaluationReader::ELEMENT_WordWeights			= _T("WordWeights");
const wchar_t* CXmlEvaluationReader::ELEMENT_GlyphWeights			= _T("GlyphWeights");
const wchar_t* CXmlEvaluationReader::ELEMENT_ReadingOrderGroupWeights = _T("ReadingOrderGroupWeights");
const wchar_t* CXmlEvaluationReader::ELEMENT_BorderWeights			= _T("BorderWeights");
const wchar_t* CXmlEvaluationReader::ELEMENT_ErrorTypeWeight		= _T("ErrorTypeWeight");
const wchar_t* CXmlEvaluationReader::ELEMENT_RegionTypeWeight		= _T("RegionTypeWeight");
const wchar_t* CXmlEvaluationReader::ELEMENT_SubTypeWeight			= _T("SubTypeWeight");
const wchar_t* CXmlEvaluationReader::ELEMENT_ReadingOrderWeight		= _T("ReadingOrderWeight");
const wchar_t* CXmlEvaluationReader::ELEMENT_Description			= _T("Description");
const wchar_t* CXmlEvaluationReader::ELEMENT_IncludedBackgroundWeight	= _T("IncludedBackgroundWeight");
const wchar_t* CXmlEvaluationReader::ELEMENT_ExcludedForegroundWeight	= _T("ExcludedForegroundWeight");
const wchar_t* CXmlEvaluationReader::ELEMENT_MissingRegionAreaWeights	= _T("MissingRegionAreaWeights");

const wchar_t* CXmlEvaluationReader::ELEMENT_ReadingOrderPenalties	= _T("ReadingOrderPenalties");
const wchar_t* CXmlEvaluationReader::ELEMENT_Penalty				= _T("Penalty");

const wchar_t* CXmlEvaluationReader::ELEMENT_Results			= _T("Results");
const wchar_t* CXmlEvaluationReader::ELEMENT_PageObjectResults	= _T("PageObjectResults");
const wchar_t* CXmlEvaluationReader::ELEMENT_BorderResults		= _T("BorderResults");
const wchar_t* CXmlEvaluationReader::ELEMENT_RawData			= _T("RawData");
const wchar_t* CXmlEvaluationReader::ELEMENT_GroundTruthOverlap = _T("GroundTruthOverlap");
const wchar_t* CXmlEvaluationReader::ELEMENT_SegResultOverlap	= _T("SegResultOverlap");
const wchar_t* CXmlEvaluationReader::ELEMENT_OverlapsRegion		= _T("OverlapsRegion");
const wchar_t* CXmlEvaluationReader::ELEMENT_RegionResults		= _T("RegionResults");
const wchar_t* CXmlEvaluationReader::ELEMENT_ReadingOrderResults= _T("ReadingOrderResults");
const wchar_t* CXmlEvaluationReader::ELEMENT_ReadingOrderError	= _T("ReadingOrderError");

const wchar_t* CXmlEvaluationReader::ELEMENT_Metrics = _T("Metrics");

const wchar_t* CXmlEvaluationReader::ELEMENT_RegionError		= _T("RegionError");
const wchar_t* CXmlEvaluationReader::ELEMENT_RegionErrorMerge	= _T("RegionErrorMerge");
const wchar_t* CXmlEvaluationReader::ELEMENT_RegionErrorSplit	= _T("RegionErrorSplit");
const wchar_t* CXmlEvaluationReader::ELEMENT_RegionErrorMisclass= _T("RegionErrorMisclass");
const wchar_t* CXmlEvaluationReader::ELEMENT_ErrorRects			= _T("ErrorRects");
const wchar_t* CXmlEvaluationReader::ELEMENT_FalseAlarmRects	= _T("FalseAlarmRects");
const wchar_t* CXmlEvaluationReader::ELEMENT_Rect				= _T("Rect");
const wchar_t* CXmlEvaluationReader::ELEMENT_Merge				= _T("Merge");
const wchar_t* CXmlEvaluationReader::ELEMENT_Overlap			= _T("Overlap");
const wchar_t* CXmlEvaluationReader::ELEMENT_AllowableEntry		= _T("AllowableEntry");

const wchar_t* CXmlEvaluationReader::ELEMENT_SegResultRelation		= _T("SegResultRelation");
const wchar_t* CXmlEvaluationReader::ELEMENT_GroundTruthRelation	= _T("GroundTruthRelation");
const wchar_t* CXmlEvaluationReader::ELEMENT_Relation				= _T("Relation");

const wchar_t* CXmlEvaluationReader::ELEMENT_NumberOfGroundTruthRegions				= _T("NumberOfGroundTruthRegions");
const wchar_t* CXmlEvaluationReader::ELEMENT_NumberOfSegResultRegions				= _T("NumberOfSegResultRegions");
const wchar_t* CXmlEvaluationReader::ELEMENT_GroundTruthRegionArea					= _T("GroundTruthRegionArea");
const wchar_t* CXmlEvaluationReader::ELEMENT_GroundTruthRegionPixelCount			= _T("GroundTruthRegionPixelCount");
const wchar_t* CXmlEvaluationReader::ELEMENT_SegResultRegionArea					= _T("SegResultRegionArea");
const wchar_t* CXmlEvaluationReader::ELEMENT_SegResultRegionPixelCount				= _T("SegResultRegionPixelCount");
const wchar_t* CXmlEvaluationReader::ELEMENT_RecallArea								= _T("RecallArea");
const wchar_t* CXmlEvaluationReader::ELEMENT_RecallPixelCount						= _T("RecallPixelCount");
const wchar_t* CXmlEvaluationReader::ELEMENT_OverallWeightedAreaErrorPerErrorType	= _T("OverallWeightedAreaErrorPerErrorType");
const wchar_t* CXmlEvaluationReader::ELEMENT_InfluenceWeightedAreaErrorPerErrorType	= _T("InfluenceWeightedAreaErrorPerErrorType");
const wchar_t* CXmlEvaluationReader::ELEMENT_OverallWeightedAreaErrorPerRegionType	= _T("OverallWeightedAreaErrorPerRegionType");
const wchar_t* CXmlEvaluationReader::ELEMENT_WeightedAreaSuccessRate				= _T("WeightedAreaSuccessRate");
const wchar_t* CXmlEvaluationReader::ELEMENT_OverallWeightedCountErrorPerErrorType	= _T("OverallWeightedCountErrorPerErrorType");
const wchar_t* CXmlEvaluationReader::ELEMENT_InfluenceWeightedCountErrorPerErrorType= _T("InfluenceWeightedCountErrorPerErrorType");
const wchar_t* CXmlEvaluationReader::ELEMENT_OverallWeightedCountErrorPerRegionType	= _T("OverallWeightedCountErrorPerRegionType");
const wchar_t* CXmlEvaluationReader::ELEMENT_WeightedCountSuccessRate				= _T("WeightedCountSuccessRate");
const wchar_t* CXmlEvaluationReader::ELEMENT_ErrorRateBasedOnSimpleCount			= _T("ErrorRateBasedOnSimpleCount");
const wchar_t* CXmlEvaluationReader::ELEMENT_Recall									= _T("Recall");
const wchar_t* CXmlEvaluationReader::ELEMENT_Precision								= _T("Precision");

/*
 * Constructor without XML validator (no validation on read)
 */
CXmlEvaluationReader::CXmlEvaluationReader(void)
{
	m_XmlValidatorProvider = NULL;
	m_Validator = NULL;
	m_Warning = NULL;
	m_SchemaVersion = 0;
	m_Listener = NULL;
}

/*
 * Constructor with XML validator (validation on read)
 */
CXmlEvaluationReader::CXmlEvaluationReader(CXmlValidatorProvider * xmlValidatorProvider)
{
	m_XmlValidatorProvider = xmlValidatorProvider;
	m_Validator = NULL;
	m_Warning = NULL;
	m_SchemaVersion = 0;
	m_Listener = NULL;
}

/*
 * Destructor
 */
CXmlEvaluationReader::~CXmlEvaluationReader(void)
{
	delete m_Warning;
}

/*
 * Reads and validates the given XML file.
 */
IXMLDOMDocument2 * CXmlEvaluationReader::validate(CUniString fileName, CMsXmlReader * msXmlReader)
{
	//CMsXmlReader * msXmlReader = new CMsXmlReader(CUniString(fileName));

	IXMLDOMDocument2 * xmlDoc = msXmlReader->GetXmlDoc();
	if (xmlDoc != NULL)
	{
		m_SchemaName = ExtractSchemaVersion(xmlDoc);
		
		//Validation
		if (m_XmlValidatorProvider != NULL)
		{
			//Known schema versions
			SCHEMA_2010_01_15 = m_XmlValidatorProvider->GetVersionNumber(CUniString(_T("2010-01-15")));
			SCHEMA_2011_08_22 = m_XmlValidatorProvider->GetVersionNumber(CUniString(_T("2011-08-22")));
			SCHEMA_2013_07_15 = m_XmlValidatorProvider->GetVersionNumber(CUniString(_T("2013-07-15")));
			SCHEMA_2018_07_15 = m_XmlValidatorProvider->GetVersionNumber(CUniString(_T("2018-07-15")));
			SCHEMA_2019_07_15 = m_XmlValidatorProvider->GetVersionNumber(CUniString(_T("2019-07-15")));
			//The current version
			m_SchemaVersion = m_XmlValidatorProvider->GetVersionNumber(m_SchemaName);

			//Get validator
			m_Validator = m_XmlValidatorProvider->GetValidator(m_SchemaName);
			
			//Validate
			if (m_Validator != NULL)
			{
				int valRes = m_Validator->validate(xmlDoc);

				//Check result
				if (valRes != 0)
				{
					CUniString errMsg = m_Validator->GetErrorMsg();
					fprintf(stderr, "XML schema not valid!\n");
					fprintf(stderr, errMsg.ToC_Str());
					//delete msXmlReader;
					return NULL;
				}

				//Generate a message if not the most recent schema version
				if (m_XmlValidatorProvider->GetLatestVersion() != m_SchemaName)
				{
					if (m_Warning == NULL)
						m_Warning = new CUniString();
					if (!m_Warning->IsEmpty())
						m_Warning->Append(_T("\n\n"));
					m_Warning->Append(_T("The format of the document has been converted to the latest version.\n\n"));
					m_Warning->Append(_T("  Document: "));
					m_Warning->Append(fileName);
					m_Warning->Append(_T("\n  Old version: "));
					m_Warning->Append(m_SchemaName);
					m_Warning->Append(_T("\n  New version: "));
					m_Warning->Append(m_XmlValidatorProvider->GetLatestVersion());
					m_Warning->Append(_T("\n\nSaving the document will produce an XML file with the new format."));
				}
			}
		}
	}
	//delete msXmlReader;
	return xmlDoc;
}

/*
 * Parses the given XML file and creates an object of CLayoutEvaluation.
 */
bool CXmlEvaluationReader::ReadLayoutEvaluation(CUniString fileName, 
												CLayoutEvaluation * layoutEval,
												CMetaData * metaData)
{
	long res = S_OK;
	bool success = false;

	CMsXmlReader msXmlReader(fileName);

	IXMLDOMDocument2 * xmlDoc = validate(fileName, &msXmlReader);
	if (xmlDoc != NULL)
	{
		//Read the content
		CMsXmlNode baseElement;
		CUniString value;

		IXMLDOMElement * element;
		if (xmlDoc->get_documentElement(&element) != S_OK)
			return false;

		baseElement.SetNodeObject(element, xmlDoc);

		if (baseElement.GetName() != CUniString(ELEMENT_Eval)) //Not a Evaluation XML file
			return false;

		success = ReadMetaData(xmlDoc, metaData);

		if (success)
			success = ReadEvaluationProfile(xmlDoc, layoutEval->GetProfile());

		if (success)
			success = ReadLayoutEvaluation(xmlDoc, layoutEval);

	}
	else
	{
		// Failed to load xml, get last parsing error
		CUniString errMsg(_T("Could not read the XML file.\n\n"));
		errMsg.Append(msXmlReader.GetParseError());
	}

	return success;
}

/*
 * Parses the given XML file and creates an object of CEvaluationProfile.
 * 'crypto' - Optional decryption
 */
bool CXmlEvaluationReader::ReadEvaluationProfile(CUniString fileName, CEvaluationProfile * profile,
												 CMetaData * metaData, CCrypto * crypto /*= NULL*/)
{
	CMsXmlReader * msXmlReader = NULL;
	
	if (crypto != NULL)
		msXmlReader = new CMsXmlReader(fileName, crypto);
	else
		msXmlReader = new CMsXmlReader(fileName);

	IXMLDOMDocument2 * doc = validate(fileName, msXmlReader);

	if (doc == NULL)
	{
		delete msXmlReader;
		return false;
	}

	bool success = false;
	success = ReadMetaData(doc, metaData);

	if (success)
		success = ReadEvaluationProfile(doc, profile);

	delete msXmlReader;
	return success;
}

/*
 * Parses the given XML document and fills the CLayoutEvaluation object.
 * Returns false if an error occurs.
 */
bool CXmlEvaluationReader::ReadLayoutEvaluation(IXMLDOMDocument2 * doc, CLayoutEvaluation * layoutEvaluation)
{
	CMsXmlNode baseElement;
	IXMLDOMElement * element;
	if (doc->get_documentElement(&element) != S_OK)
		return false;
	baseElement.SetNodeObject(element, doc);

	//Traverse children of base element
	CMsXmlNode * tempNode = baseElement.GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_EvalData))
			parseEvalDataNode(layoutEvaluation, tempNode);
		tempNode = tempNode->GetNextSibling();
	}

	return true;
}

/*
 * Parses the given XML document and fills the CEvaluationProfile object.
 * Returns false if an error occurs.
 */
bool CXmlEvaluationReader::ReadEvaluationProfile(IXMLDOMDocument2 * doc, CEvaluationProfile * evaluationProfile)
{
	CMsXmlNode baseElement;
	IXMLDOMElement * element;
	if (doc->get_documentElement(&element) != S_OK)
		return false;
	baseElement.SetNodeObject(element, doc);

	//Traverse children of base element
	CMsXmlNode * tempNode = baseElement.GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_Profile))
			parseProfileNode(evaluationProfile, tempNode);
		tempNode = tempNode->GetNextSibling();
	}

	return true;
}

/*
 * Parses the given XML file and fills the CLayoutEvaluation and
 * CEvaluationProfile objects.
 * Returns false if an error occurs.
 */
bool CXmlEvaluationReader::Read(CUniString fileName,
								CLayoutEvaluation * layoutEvaluation,
								CEvaluationProfile * evaluationProfile,
								CMetaData * metaData)
{
	CMsXmlReader msXmlReader(fileName);

	IXMLDOMDocument2 * doc = validate(fileName, &msXmlReader);

	if (doc == NULL)
		return false;

	bool success = false;
	success = ReadMetaData(doc, metaData);
	if (success)
		success = ReadEvaluationProfile(doc, evaluationProfile);
	if (success)
		success = ReadLayoutEvaluation(doc, layoutEvaluation);

	return success;
}

/*
 * Reads the document meta data.
 */
bool CXmlEvaluationReader::ReadMetaData(IXMLDOMDocument2 * doc, CMetaData * metaData)
{
	CMsXmlNode baseElement;
	IXMLDOMElement * element;
	if (doc->get_documentElement(&element) != S_OK)
		return false;
	baseElement.SetNodeObject(element, doc);

	//Eval ID
	if (baseElement.HasAttribute(ATTR_evalId))
		metaData->SetEvalId(baseElement.GetAttribute(ATTR_evalId));

	//Traverse children of base element
	CMsXmlNode * tempNode = baseElement.GetFirstChild();
	try
	{
		while (tempNode != NULL)
		{
			if(tempNode->GetName() == CUniString(ELEMENT_Metadata))
				parseMetadataNode(metaData, tempNode);
			tempNode = tempNode->GetNextSibling();
		}
	}
	catch(CXmlParseException & )
	{
		return false;
	}

	return true;
}

/*
 * Parses the metadata XML element.
 */
void CXmlEvaluationReader::parseMetadataNode(CMetaData * metaData, CMsXmlNode * metadataNode)
{
	if(metadataNode != NULL)
	{
		CMsXmlNode * tempNode = metadataNode->GetFirstChild();
		while (tempNode != NULL)
		{
			//Creator
			if(tempNode->GetName() == CUniString(ELEMENT_Creator))
			{
				if(tempNode->HasChildren())
					metaData->SetCreator((tempNode->GetTextContent()));
			}
			//Created
			else if(tempNode->GetName() == CUniString(ELEMENT_Created))
			{
				if(!tempNode->HasChildren())
					throw CXmlParseException("Bad XML element 'Created'");
				
				tm timeC = {0, 0, 0, 0, 0, 0, 0, 0, 0};

				CUniString content = tempNode->GetTextContent();
				sscanf ((char *) content.ToC_Str(), "%d-%d-%dT%d:%d:%d", &timeC.tm_year, 
					&timeC.tm_mon, &timeC.tm_mday, &timeC.tm_hour, &timeC.tm_min, &timeC.tm_sec);
				timeC.tm_mon--;
				timeC.tm_year -= 1900;
				metaData->SetCreated(mktime(&timeC));
			}
			//LastChange
			else if(tempNode->GetName() == CUniString(ELEMENT_LastChange))
			{
				if(!tempNode->HasChildren())
					throw CXmlParseException("Bad XML element 'LastChange'");
				
				tm TimeM = {0, 0, 0, 0, 0, 0, 0, 0, 0};

				CUniString content = tempNode->GetTextContent();
				sscanf((char *) content.ToC_Str(), "%d-%d-%dT%d:%d:%d", &TimeM.tm_year, &TimeM.tm_mon, &TimeM.tm_mday, &TimeM.tm_hour, &TimeM.tm_min, &TimeM.tm_sec);
				TimeM.tm_mon--;
				TimeM.tm_year -= 1900;
				metaData->SetLastChange(mktime(&TimeM));
			}
			//Comments
			else if(tempNode->GetName() == CUniString(ELEMENT_Comments))
			{
				if(tempNode->HasChildren())
					metaData->SetComments(tempNode->GetTextContent());
			}
			//Software
			else if(tempNode->GetName() == CUniString(ELEMENT_Software))
			{
				if (tempNode->HasAttribute(ATTR_producer))
				{
					metaData->SetSoftware(	tempNode->GetAttribute(ATTR_name), 
											tempNode->GetAttribute(ATTR_version),
											tempNode->GetAttribute(ATTR_producer));
				}
				else
				{
					metaData->SetSoftware(	tempNode->GetAttribute(ATTR_name), 
											tempNode->GetAttribute(ATTR_version));
				}
			}
			tempNode = tempNode->GetNextSibling();
		}
	}
}

/*
 * Parses the evaluation result XML element.
 */
void CXmlEvaluationReader::parseEvalDataNode(CLayoutEvaluation * layoutEvaluation, CMsXmlNode * evalDataNode)
{
	if(evalDataNode != NULL)
	{
		//Image file
		if(evalDataNode->HasAttribute(ATTR_imageFilename))
			layoutEvaluation->SetBilevelImageLocation(evalDataNode->GetAttribute(ATTR_imageFilename));

		if (m_Listener != NULL)
			m_Listener->ImageFileNameLoaded(layoutEvaluation, CUniString(layoutEvaluation->GetBilevelImageLocation()));

		//Ground truth file
		if(evalDataNode->HasAttribute(ATTR_groundTruthFilename))
			layoutEvaluation->SetGroundTruthLocation(evalDataNode->GetAttribute(ATTR_groundTruthFilename));

		if (m_Listener != NULL)
			m_Listener->GroundTruthFileNameLoaded(layoutEvaluation, CUniString(layoutEvaluation->GetGroundTruthLocation()));

		//Segmentation result file
		if(evalDataNode->HasAttribute(ATTR_segmentationResultFilename))
			layoutEvaluation->SetSegResultLocation(evalDataNode->GetAttribute(ATTR_segmentationResultFilename));

		if (m_Listener != NULL)
			m_Listener->SegmentationResultFileNameLoaded(layoutEvaluation, CUniString(layoutEvaluation->GetSegResultLocation()));

		//Width
		if(evalDataNode->HasAttribute(ATTR_imageWidth))
			layoutEvaluation->SetWidth(evalDataNode->GetIntAttribute(ATTR_imageWidth));

		//Height
		if(evalDataNode->HasAttribute(ATTR_imageHeight))
			layoutEvaluation->SetHeight(evalDataNode->GetIntAttribute(ATTR_imageHeight));

		//Results for page objects (region, text line, word, glyph, group) and border
		CMsXmlNode * tempNode = evalDataNode->GetFirstChild();
		while (tempNode != NULL)
		{
			//Regions / text lines / words / glyphs / reading order groups
			if(tempNode->GetName() == CUniString(ELEMENT_PageObjectResults)
				|| tempNode->GetName() == CUniString(ELEMENT_Results)) //Old; for compatibility only
			{
				int pageObjectLevel = 0;
				if (m_SchemaVersion >= SCHEMA_2013_07_15)
					pageObjectLevel = PageObjectLevelStringToInt(tempNode->GetAttribute(ATTR_type));
				else
					pageObjectLevel = tempNode->GetIntAttribute(ATTR_type);

				CEvaluationResults * results = layoutEvaluation->GetResults(pageObjectLevel, true);
				
				ParsePageObjectResultsNode(tempNode, results, layoutEvaluation->GetProfile());
			}
			//Border
			else if(tempNode->GetName() == CUniString(ELEMENT_BorderResults)) 
			{
				CEvaluationResults * results = layoutEvaluation->GetResults(CLayoutObject::TYPE_BORDER, true);
				
				ParseBorderResultsNode(tempNode, results, layoutEvaluation->GetProfile());
			}
			tempNode = tempNode->GetNextSibling();
		}
	}
}

/*
 * Converts a page object level string (e.g. 'text-line') to the internal corresponing number (CLayoutObject::TYPE_...)
 */
int CXmlEvaluationReader::PageObjectLevelStringToInt(CUniString level)
{
	if (CUniString(L"region") == level)
		return CLayoutObject::TYPE_LAYOUT_REGION;
	if (CUniString(L"text-line") == level)
		return CLayoutObject::TYPE_TEXT_LINE;
	if (CUniString(L"word") == level)
		return CLayoutObject::TYPE_WORD;
	if (CUniString(L"glyph") == level)
		return CLayoutObject::TYPE_GLYPH;
	if (CUniString(L"group") == level)
		return CLayoutObject::TYPE_READING_ORDER_GROUP;
	return CLayoutObject::TYPE_UNKNOWN;
}

/*
 * Converts a region type string (e.g. 'text') to the internal corresponing type number (CLayoutRegion::TYPE_...)
 */
int CXmlEvaluationReader::RegionTypeStringToInt(CUniString typeName)
{
	if (CUniString(L"all") == typeName)
		return CLayoutRegion::TYPE_ALL;
	if (CUniString(L"text") == typeName)
		return CLayoutRegion::TYPE_TEXT;
	if (CUniString(L"image") == typeName)
		return CLayoutRegion::TYPE_IMAGE;
	if (CUniString(L"graphic") == typeName)
		return CLayoutRegion::TYPE_GRAPHIC;
	if (CUniString(L"line-drawing") == typeName)
		return CLayoutRegion::TYPE_LINEART;
	if (CUniString(L"chart") == typeName)
		return CLayoutRegion::TYPE_CHART;
	if (CUniString(L"separator") == typeName)
		return CLayoutRegion::TYPE_SEPARATOR;
	if (CUniString(L"maths") == typeName)
		return CLayoutRegion::TYPE_MATHS;
	if (CUniString(L"advert") == typeName)
		return CLayoutRegion::TYPE_ADVERT;
	if (CUniString(L"chem") == typeName)
		return CLayoutRegion::TYPE_CHEM;
	if (CUniString(L"music") == typeName)
		return CLayoutRegion::TYPE_MUSIC;
	if (CUniString(L"map") == typeName)
		return CLayoutRegion::TYPE_MAP;
	if (CUniString(L"noise") == typeName)
		return CLayoutRegion::TYPE_NOISE;
	if (CUniString(L"table") == typeName)
		return CLayoutRegion::TYPE_TABLE;
	if (CUniString(L"unknown") == typeName)
		return CLayoutRegion::TYPE_UNKNOWN;
	if (CUniString(L"custom") == typeName)
		return CLayoutRegion::TYPE_CUSTOM;
	return CLayoutRegion::TYPE_INVALID;
}

/*
 * Converts a error type string (e.g. 'merge') to the internal corresponing type number (CEvaluationError::TYPE_...)
 */
int CXmlEvaluationReader::ErrorTypeStringToInt(CUniString typeName)
{
	if (CUniString(L"merge") == typeName)
		return CLayoutObjectEvaluationError::TYPE_MERGE;
	if (CUniString(L"split") == typeName)
		return CLayoutObjectEvaluationError::TYPE_SPLIT;
	if (CUniString(L"miss") == typeName)
		return CLayoutObjectEvaluationError::TYPE_MISS;
	if (CUniString(L"partial-miss") == typeName)
		return CLayoutObjectEvaluationError::TYPE_PART_MISS;
	if (CUniString(L"misclassification") == typeName)
		return CLayoutObjectEvaluationError::TYPE_MISCLASS;
	if (CUniString(L"false-detection") == typeName)
		return CLayoutObjectEvaluationError::TYPE_INVENT;
	return CLayoutObjectEvaluationError::TYPE_NONE;
}

/*
 * Parses the evaluation results for a page object level (region, text line, ...)
 */
void CXmlEvaluationReader::ParsePageObjectResultsNode(CMsXmlNode * resultsNode, CEvaluationResults * results, 
											CEvaluationProfile * profile)
{
	CMsXmlNode * tempNode = resultsNode->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_RawData))
			ParseRawData(tempNode, results, profile);
		else if(tempNode->GetName() == CUniString(ELEMENT_Metrics))
			ParseMetricResults(tempNode, results);
		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Parses border evaluation results
 */
void CXmlEvaluationReader::ParseBorderResultsNode(CMsXmlNode * resultsNode, CEvaluationResults * results, 
												  CEvaluationProfile * profile)
{
	if (results == NULL)
		return;

	CBorderEvaluationMetrics * metrics = new CBorderEvaluationMetrics(results, results->GetProfile(), false);
	results->SetMetrics(metrics);

	if (resultsNode->HasAttribute(ATTR_includedBackgroundSuccessRate))
		metrics->SetIncludedBackgroundSuccessRate(resultsNode->GetDoubleAttribute(ATTR_includedBackgroundSuccessRate));
	if (resultsNode->HasAttribute(ATTR_excludedForegroundSuccessRate))
		metrics->SetExcludedForegroundSuccessRate(resultsNode->GetDoubleAttribute(ATTR_excludedForegroundSuccessRate));
	if (resultsNode->HasAttribute(ATTR_missingRegionAreaSuccessRate))
		metrics->SetMissingRegionAreaSuccessRate(resultsNode->GetDoubleAttribute(ATTR_missingRegionAreaSuccessRate));
	if (resultsNode->HasAttribute(ATTR_overallSuccessRate))
		metrics->SetOverallSuccessRate(resultsNode->GetDoubleAttribute(ATTR_overallSuccessRate));
}

/*
 * Parses the metric results (success rates, ...)
 */
void CXmlEvaluationReader::ParseMetricResults(CMsXmlNode * metricsNode, CEvaluationResults * results)
{
	//Type
	int type = CLayoutRegion::TYPE_ALL;
	if (metricsNode->HasAttribute(ATTR_type))
	{
		if (m_SchemaVersion >= SCHEMA_2013_07_15)
			type = RegionTypeStringToInt(metricsNode->GetAttribute(ATTR_type));
		else
			type = metricsNode->GetIntAttribute(ATTR_type);
		if (type == CLayoutRegion::DEPRECATED_TYPE_FRAME)
			return;
	}

	if (type == 0) //CC 22.08.2011 - changed schema: type 0 means overall metrics
		type = CLayoutRegion::TYPE_ALL;
	else if (type == CLayoutRegion::DEPRECATED_TYPE_ALL) //CC 19.07.2013 - added new types
		type = CLayoutRegion::TYPE_ALL;
	else if (type == CLayoutRegion::DEPRECATED_TYPE_FRAME) //CC 19.07.2013 - type removed from schema
		return;

	CLayoutObjectEvaluationMetrics * metrics = new CLayoutObjectEvaluationMetrics(results, results->GetProfile(), false, type);

	ParseMetricsNode(metricsNode, metrics);

	//Add
	if (type == CLayoutRegion::TYPE_ALL)
		results->SetMetrics(metrics);
	else
	{
		map<int,CLayoutObjectEvaluationMetrics*> * metricsPerType = results->GetMetricsPerType();
		map<int,CLayoutObjectEvaluationMetrics*>::iterator it = metricsPerType->find(type);
		if (it != metricsPerType->end()) //already there
		{
			delete (*it).second;
			metricsPerType->erase(it);
		}
		metricsPerType->insert(pair<int,CLayoutObjectEvaluationMetrics*>(type, metrics));
	}
}

/*
 * Parses the metrics node (success rates, ...)
 */
void CXmlEvaluationReader::ParseMetricsNode(CMsXmlNode * metricsNode, CLayoutObjectEvaluationMetrics * metrics)
{
	//Attributes
	if (metricsNode->HasAttribute(ATTR_numberOfGroundTruthRegions))
		metrics->SetNumberOfGroundTruthRegions(metricsNode->GetIntAttribute(ATTR_numberOfGroundTruthRegions));
	if (metricsNode->HasAttribute(ATTR_numberOfSegResultRegions))
		metrics->SetNumberOfSegResultRegions(metricsNode->GetIntAttribute(ATTR_numberOfSegResultRegions));
	if (metricsNode->HasAttribute(ATTR_imageArea))
		metrics->SetImageArea(metricsNode->GetIntAttribute(ATTR_imageArea));
	if (metricsNode->HasAttribute(ATTR_foregroundPixelCount))
		metrics->SetImageForegroundPixelCount(metricsNode->GetIntAttribute(ATTR_foregroundPixelCount));
	if (metricsNode->HasAttribute(ATTR_overallGroundTruthRegionArea))
		metrics->SetOverallGroundTruthRegionArea(metricsNode->GetIntAttribute(ATTR_overallGroundTruthRegionArea));
	if (metricsNode->HasAttribute(ATTR_overallGroundTruthRegionPixelCount))
		metrics->SetOverallGroundTruthRegionPixelCount(metricsNode->GetIntAttribute(ATTR_overallGroundTruthRegionPixelCount));
	if (metricsNode->HasAttribute(ATTR_overallSegResultRegionArea))
		metrics->SetOverallSegResultRegionArea(metricsNode->GetIntAttribute(ATTR_overallSegResultRegionArea));
	if (metricsNode->HasAttribute(ATTR_overallSegResultRegionPixelCount))
		metrics->SetOverallSegResultRegionPixelCount(metricsNode->GetIntAttribute(ATTR_overallSegResultRegionPixelCount));
	if (metricsNode->HasAttribute(ATTR_overallWeightedAreaError))
		metrics->SetOverallWeightedAreaError(metricsNode->GetDoubleAttribute(ATTR_overallWeightedAreaError));
	if (metricsNode->HasAttribute(ATTR_overallWeightedCountError))
		metrics->SetOverallWeightedCountError(metricsNode->GetDoubleAttribute(ATTR_overallWeightedCountError));
	if (metricsNode->HasAttribute(ATTR_readingOrderError))
		metrics->SetReadingOrderError(metricsNode->GetDoubleAttribute(ATTR_readingOrderError));
	if (metricsNode->HasAttribute(ATTR_readingOrderSuccessRate))
		metrics->SetReadingOrderSuccessRate(metricsNode->GetDoubleAttribute(ATTR_readingOrderSuccessRate));
	if (metricsNode->HasAttribute(ATTR_overallWeightedAreaSuccessRate))
		metrics->SetOverallWeightedAreaSuccessRate(metricsNode->GetDoubleAttribute(ATTR_overallWeightedAreaSuccessRate));
	if (metricsNode->HasAttribute(ATTR_overallWeightedCountSuccessRate))
		metrics->SetOverallWeightedCountSuccessRate(metricsNode->GetDoubleAttribute(ATTR_overallWeightedCountSuccessRate));
	if (metricsNode->HasAttribute(ATTR_harmonicWeightedAreaSuccessRate))
		metrics->SetHarmonicWeightedAreaSuccessRate(metricsNode->GetDoubleAttribute(ATTR_harmonicWeightedAreaSuccessRate));
	if (metricsNode->HasAttribute(ATTR_harmonicWeightedCountSuccessRate))
		metrics->SetHarmonicWeightedCountSuccessRate(metricsNode->GetDoubleAttribute(ATTR_harmonicWeightedCountSuccessRate));
	if (metricsNode->HasAttribute(ATTR_errorRateBasedOnSimpleCount))
		metrics->SetErrorRateBasedOnCount(metricsNode->GetDoubleAttribute(ATTR_errorRateBasedOnSimpleCount));
	if (metricsNode->HasAttribute(ATTR_recallNonStrict))
		metrics->SetRecall(false, metricsNode->GetDoubleAttribute(ATTR_recallNonStrict));
	if (metricsNode->HasAttribute(ATTR_recallStrict))
		metrics->SetRecall(true, metricsNode->GetDoubleAttribute(ATTR_recallStrict));
	if (metricsNode->HasAttribute(ATTR_precisionNonStrict))
		metrics->SetPrecision(false, metricsNode->GetDoubleAttribute(ATTR_precisionNonStrict));
	if (metricsNode->HasAttribute(ATTR_precisionStrict))
		metrics->SetPrecision(true, metricsNode->GetDoubleAttribute(ATTR_precisionStrict));
	if (metricsNode->HasAttribute(ATTR_fMeasureNonStrict))
		metrics->SetFMeasure(false, metricsNode->GetDoubleAttribute(ATTR_fMeasureNonStrict));
	if (metricsNode->HasAttribute(ATTR_fMeasureStrict))
		metrics->SetFMeasure(true, metricsNode->GetDoubleAttribute(ATTR_fMeasureStrict));
	if (metricsNode->HasAttribute(ATTR_OCRSuccessRate))
		metrics->SetOCRSuccessRate(metricsNode->GetDoubleAttribute(ATTR_OCRSuccessRate));

	//Elements
	CMsXmlNode * tempNode = metricsNode->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_NumberOfGroundTruthRegions))
			ParseIntPerRegionType(tempNode, metrics->GetNumberOfGroundTruthRegionsPerType());
		if(tempNode->GetName() == CUniString(ELEMENT_NumberOfSegResultRegions))
			ParseIntPerRegionType(tempNode, metrics->GetNumberOfSegResultRegionsPerType());
		if(tempNode->GetName() == CUniString(ELEMENT_GroundTruthRegionArea))
			ParseIntPerRegionType(tempNode, metrics->GetOverallGroundTruthRegionAreaPerType());
		if(tempNode->GetName() == CUniString(ELEMENT_GroundTruthRegionPixelCount))
			ParseIntPerRegionType(tempNode, metrics->GetOverallGroundTruthRegionPixelCountPerType());
		if(tempNode->GetName() == CUniString(ELEMENT_SegResultRegionArea))
			ParseIntPerRegionType(tempNode, metrics->GetOverallSegResultRegionAreaPerType());
		if(tempNode->GetName() == CUniString(ELEMENT_SegResultRegionPixelCount))
			ParseIntPerRegionType(tempNode, metrics->GetOverallSegResultRegionPixelCountPerType());
		if(tempNode->GetName() == CUniString(ELEMENT_RecallArea))
			ParseIntPerRegionType(tempNode, metrics->GetRecallAreaPerType());
		if(tempNode->GetName() == CUniString(ELEMENT_RecallPixelCount))
			ParseIntPerRegionType(tempNode, metrics->GetRecallPixelCountPerType());
		if(tempNode->GetName() == CUniString(ELEMENT_OverallWeightedAreaErrorPerErrorType))
			ParseDoublePerErrorType(tempNode, metrics->GetOverallWeightedAreaErrorPerErrorType());
		if(tempNode->GetName() == CUniString(ELEMENT_InfluenceWeightedAreaErrorPerErrorType))
			ParseDoublePerErrorType(tempNode, metrics->GetInfluenceOverallWeightedAreaErrorPerErrorType());
		if(tempNode->GetName() == CUniString(ELEMENT_OverallWeightedAreaErrorPerRegionType))
			ParseDoublePerRegionType(tempNode, metrics->GetOverallWeightedAreaErrorPerRegionType());
		if(tempNode->GetName() == CUniString(ELEMENT_WeightedAreaSuccessRate))
			ParseDoublePerErrorType(tempNode, metrics->GetWeightedAreaSuccessRatePerType());
		if(tempNode->GetName() == CUniString(ELEMENT_OverallWeightedCountErrorPerErrorType))
			ParseDoublePerErrorType(tempNode, metrics->GetOverallWeightedCountErrorPerErrorType());
		if(tempNode->GetName() == CUniString(ELEMENT_InfluenceWeightedCountErrorPerErrorType))
			ParseDoublePerErrorType(tempNode, metrics->GetInfluenceOverallWeightedCountErrorPerErrorType());
		if(tempNode->GetName() == CUniString(ELEMENT_OverallWeightedCountErrorPerRegionType))
			ParseDoublePerRegionType(tempNode, metrics->GetOverallWeightedCountErrorPerRegionType());
		if(tempNode->GetName() == CUniString(ELEMENT_WeightedCountSuccessRate))
			ParseDoublePerErrorType(tempNode, metrics->GetWeightedCountSuccessRatePerType());
		if(tempNode->GetName() == CUniString(ELEMENT_ErrorRateBasedOnSimpleCount))
			ParseDoublePerErrorType(tempNode, metrics->GetErrorRatePerTypeBasedOnCount());
		if(tempNode->GetName() == CUniString(ELEMENT_Recall))
			ParseDoublePerRegionType(tempNode, metrics->GetRecallPerType());
		if(tempNode->GetName() == CUniString(ELEMENT_Precision))
			ParseDoublePerRegionType(tempNode, metrics->GetPrecisionPerType());

		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Parses an integer value per region type.
 */
void CXmlEvaluationReader::ParseIntPerRegionType(CMsXmlNode * node, map<int,int> * valueMap)
{
	//Type
	int type = 0;
	if (m_SchemaVersion >= SCHEMA_2013_07_15)
		type = RegionTypeStringToInt(node->GetAttribute(ATTR_type));
	else
		type = node->GetIntAttribute(ATTR_type);
	if (type == CLayoutRegion::DEPRECATED_TYPE_FRAME)
		return;

	//Value
	int value = node->GetIntAttribute(ATTR_value);
	//Put into map
	map<int,int>::iterator it = valueMap->find(type);
	if (it != valueMap->end()) //Already there
		(*it).second = value;
	else
		valueMap->insert(pair<int,int>(type, value));
}

/*
 * Parses a double value per region type.
 */
void CXmlEvaluationReader::ParseDoublePerRegionType(CMsXmlNode * node, map<int,double> * valueMap)
{
	//Type
	int type = 0;
	if (m_SchemaVersion >= SCHEMA_2013_07_15)
		type = RegionTypeStringToInt(node->GetAttribute(ATTR_type));
	else
		type = node->GetIntAttribute(ATTR_type);
	if (type == CLayoutRegion::DEPRECATED_TYPE_FRAME)
		return;

	//Value
	double value = node->GetDoubleAttribute(ATTR_value);
	//Put into map
	map<int,double>::iterator it = valueMap->find(type);
	if (it != valueMap->end()) //Already there
		(*it).second = value;
	else
		valueMap->insert(pair<int,double>(type, value));
}

/*
 * Parses an integer value per region type.
 */
void CXmlEvaluationReader::ParseIntPerErrorType(CMsXmlNode * node, map<int,int> * valueMap)
{
	//Type
	int type = 0;
	if (m_SchemaVersion >= SCHEMA_2013_07_15)
		type = ErrorTypeStringToInt(node->GetAttribute(ATTR_type));
	else
		type = node->GetIntAttribute(ATTR_type);

	//Value
	int value = node->GetIntAttribute(ATTR_value);
	//Put into map
	map<int,int>::iterator it = valueMap->find(type);
	if (it != valueMap->end()) //Already there
		(*it).second = value;
	else
		valueMap->insert(pair<int,int>(type, value));
}

/*
 * Parses a double value per region type.
 */
void CXmlEvaluationReader::ParseDoublePerErrorType(CMsXmlNode * node, map<int,double> * valueMap)
{
	//Type
	int type = 0;
	if (m_SchemaVersion >= SCHEMA_2013_07_15)
		type = ErrorTypeStringToInt(node->GetAttribute(ATTR_type));
	else
		type = node->GetIntAttribute(ATTR_type);
	//Value
	double value = node->GetDoubleAttribute(ATTR_value);
	//Put into map
	map<int,double>::iterator it = valueMap->find(type);
	if (it != valueMap->end()) //Already there
		(*it).second = value;
	else
		valueMap->insert(pair<int,double>(type, value));
}

/*
 * Parses the evaluation results for a page level type (region, text line, ...)
 */
void CXmlEvaluationReader::ParseRawData(CMsXmlNode * rawDataNode, CEvaluationResults * results, 
										CEvaluationProfile * profile)
{
	CMsXmlNode * tempNode = rawDataNode->GetFirstChild();
	while (tempNode != NULL)
	{
		//Ground-truth overlaps
		if(tempNode->GetName() == CUniString(ELEMENT_GroundTruthOverlap))
		{
			CUniString groundTruthRegionId = tempNode->GetAttribute(ATTR_regionId);
			//CLayoutObject * groundTruthReg = groundTruth->GetRegionById(groundTruthRegionId);
			if (!groundTruthRegionId.IsEmpty())
			{
				vector<CUniString> segResultRegions = ParseOverlapRegions(tempNode);
				for (unsigned int i=0; i<segResultRegions.size(); i++)
				{
					//CLayoutObject * segResultReg = segResult->GetRegionById(segResultRegions[i]);
					if (!segResultRegions[i].IsEmpty())
						results->AddLayoutObjectOverlap(groundTruthRegionId, segResultRegions[i], NULL);
				}
			}
		}
		//Seg result overlaps
		else if(tempNode->GetName() == CUniString(ELEMENT_SegResultOverlap))
		{
			//Not needed, because the overlap map is already filled using the
			//ground-truth overlaps.
		}
		//Region results
		else if(tempNode->GetName() == CUniString(ELEMENT_RegionResults))
		{
			ParseRegionResults(tempNode, results);
		}
		//Reading order results
		else if(tempNode->GetName() == CUniString(ELEMENT_ReadingOrderResults))
		{
			ParseReadingOrderResults(tempNode, results, profile);
		}
		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Parses reading order results
 */
void CXmlEvaluationReader::ParseReadingOrderResults(CMsXmlNode * resultsNode, 
													CEvaluationResults * results,
													CEvaluationProfile * profile)
{
	CMsXmlNode * tempNode = resultsNode->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_ReadingOrderError))
		{
			CReadingOrderError * error = ParseReadingOrderError(tempNode, results, profile);
			if (error != NULL)
				results->GetReadingOrderResults()->AddError(error);
		}
		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Parses a single reading order error (between two regions)
 */
CReadingOrderError * CXmlEvaluationReader::ParseReadingOrderError(CMsXmlNode * errorNode, 
																  CEvaluationResults * results,
																  CEvaluationProfile * profile)
{
	//CPageLayout * groundTruth = results->GetLayoutEvaluation()->GetGroundTruth();
	//CPageLayout * segResult = results->GetLayoutEvaluation()->GetSegResult();

	//Region 1
	CUniString segResultRegionId1 = errorNode->GetAttribute(ATTR_regionId1);
	//CLayoutObject * segResultRegion1 = segResult->GetRegionById(regionId);
	//Region 2
	CUniString segResultRegionId2 = errorNode->GetAttribute(ATTR_regionId2);
	//CLayoutObject * segResultRegion2 = segResult->GetRegionById(regionId);

	if (segResultRegionId1.IsEmpty() || segResultRegionId2.IsEmpty())
		return NULL;

	//Penalty
	double penalty = 0.0;
	if (errorNode->HasAttribute(ATTR_penalty))
		penalty = errorNode->GetDoubleAttribute(ATTR_penalty);

	//Relations
	set<int> segResultRel;
	CMsXmlNode * tempNode = errorNode->GetFirstChild();
	vector<CFuzzyReadingOrderRelation> groundTruthRel;
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_SegResultRelation))
		{
			segResultRel = ParseRelationSet(tempNode);
		}
		else if(tempNode->GetName() == CUniString(ELEMENT_GroundTruthRelation))
		{
			//Relation set
			set<int> rel = ParseRelationSet(tempNode);
			//Weight
			double weight = 0.0;
			if (tempNode->HasAttribute(ATTR_weight))
				weight = tempNode->GetDoubleAttribute(ATTR_weight);
			//Region 1
			CUniString groundTruthRegionId1 = tempNode->GetAttribute(ATTR_regionId1);
			//CLayoutObject * groundTruthRegion1 = groundTruth->GetRegionById(regionId);
			//Region 2
			CUniString groundTruthRegionId2 = tempNode->GetAttribute(ATTR_regionId2);
			//CLayoutObject * groundTruthRegion2 = groundTruth->GetRegionById(regionId);

			if (!groundTruthRegionId1.IsEmpty() && !groundTruthRegionId2.IsEmpty())
			{
				CFuzzyReadingOrderRelation fuzzyRel(weight, groundTruthRegionId1, groundTruthRegionId2, rel);
				//Used base relations
				fuzzyRel.SetCausingGroundTruthRelation(tempNode->GetIntAttribute(ATTR_groundTruthRelation));
				fuzzyRel.SetCausingSegResultRelation(tempNode->GetIntAttribute(ATTR_segResultRelation));

				groundTruthRel.push_back(fuzzyRel);
			}
		}
		tempNode = tempNode->GetNextSibling();
	}

	CReadingOrderError * error = new CReadingOrderError(segResultRegionId1, segResultRegionId2,
														segResultRel, groundTruthRel, profile);
	return error;
}

/*
 * Parses reading order realtion set (set of base relations)
 */
set<int> CXmlEvaluationReader::ParseRelationSet(CMsXmlNode * node)
{
	set<int> ret;

	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_Relation))
		{
			if (m_SchemaVersion >= SCHEMA_2013_07_15)
				ret.insert(ReadingOrderRelationTypeStringToInt(tempNode->GetAttribute(ATTR_type)));
			else
				ret.insert(tempNode->GetIntAttribute(ATTR_type));
		}
		tempNode = tempNode->GetNextSibling();
	}
	return ret;
}

/*
 * Converts a reading order relation type string to the corresponing type number (CReadingOrder::RELATION_...)
 */
int CXmlEvaluationReader::ReadingOrderRelationTypeStringToInt(CUniString type)
{
	if (CUniString(L"before-directly") == type)
		return CReadingOrder::RELATION_PREDECESSOR;
	if (CUniString(L"after-directly") == type)
		return CReadingOrder::RELATION_SUCCESSOR;
	if (CUniString(L"before-unordered") == type)
		return CReadingOrder::RELATION_BEFORE;
	if (CUniString(L"after-unordered") == type)
		return CReadingOrder::RELATION_AFTER;
	if (CUniString(L"unordered") == type)
		return CReadingOrder::RELATION_UNORDERED;
	if (CUniString(L"unrelated") == type)
		return CReadingOrder::RELATION_UNRELATED;
	if (CUniString(L"unknown") == type)
		return CReadingOrder::RELATION_UNKNOWN;
	return CReadingOrder::RELATION_NOT_DEFINED;
}

/*
 * Parses region results (merge, split, ...)
 */
void CXmlEvaluationReader::ParseRegionResults(CMsXmlNode * resultsNode, 
											  CEvaluationResults * results)
{
	//Region ID
	CUniString regionId = resultsNode->GetAttribute(ATTR_regionId);

	//Check if has 'False Detection' error (then it is a seg result region)
	bool hasFalseDetectionError = false;
	CMsXmlNode * tempNode = resultsNode->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_RegionError))
		{
			int errorType = 0;
			if (m_SchemaVersion >= SCHEMA_2013_07_15)
				errorType = ErrorTypeStringToInt(tempNode->GetAttribute(ATTR_type));
			else 
				errorType = tempNode->GetIntAttribute(ATTR_type);

			if (errorType == CLayoutObjectEvaluationError::TYPE_INVENT)
			{
				hasFalseDetectionError = true;
				break;
			}
		}
		tempNode = tempNode->GetNextSibling();
	}

	//CLayoutObject * region = NULL;
	//if (hasFalseDetectionError)
	//{
	//	CPageLayout * segResult = results->GetLayoutEvaluation()->GetSegResult();
	//	region = segResult->FindRegion(results->GetRegionType(), regionId);
	//}
	//else
	//{
	//	CPageLayout * groundTruth = results->GetLayoutEvaluation()->GetGroundTruth();
	//	region = groundTruth->FindRegion(results->GetRegionType(), regionId);
	//}

	if (!regionId.IsEmpty())
	{
		CLayoutObjectEvaluationResult * result = NULL;
		if (hasFalseDetectionError)
			result = results->GetSegResultObjectResult(regionId, true);
		else
			result = results->GetGroundTruthObjectResult(regionId, true);
		//Parse Errors
		CMsXmlNode * tempNode = resultsNode->GetFirstChild();
		while (tempNode != NULL)
		{
			CLayoutObjectEvaluationError * error = NULL;
			if(tempNode->GetName() == CUniString(ELEMENT_RegionError))
			{
				//Type
				int type = 0;
				if (m_SchemaVersion >= SCHEMA_2013_07_15)
					type = ErrorTypeStringToInt(tempNode->GetAttribute(ATTR_type));
				else
					type = tempNode->GetIntAttribute(ATTR_type);
				error = new CLayoutObjectEvaluationError(type, regionId);
			}
			else if(tempNode->GetName() == CUniString(ELEMENT_RegionErrorMerge))
				error = new CEvaluationErrorMerge(regionId);
			else if(tempNode->GetName() == CUniString(ELEMENT_RegionErrorSplit))
				error = new CEvaluationErrorSplit(regionId);
			else if(tempNode->GetName() == CUniString(ELEMENT_RegionErrorMisclass))
				error = new CEvaluationErrorMisclass(regionId);
			//Parse error
			if (error != NULL)
			{
				ParseRegionError(tempNode, error, results);
				result->AddError(error);
			}
			tempNode = tempNode->GetNextSibling();
		}
	}
}

/*
 * Parses a single error (merge, split, ...)
 */
void CXmlEvaluationReader::ParseRegionError(CMsXmlNode * errorNode, CLayoutObjectEvaluationError * error,
											CEvaluationResults * results)
{
	//Weighted area error
	if (errorNode->HasAttribute(ATTR_weightedAreaError))
		error->SetWeightedAreaError(errorNode->GetDoubleAttribute(ATTR_weightedAreaError));
	//Weighted count error
	if (errorNode->HasAttribute(ATTR_weightedCountError))
		error->SetWeightedCountError(errorNode->GetDoubleAttribute(ATTR_weightedCountError));
	//Area
	if (errorNode->HasAttribute(ATTR_area))
		error->SetArea(errorNode->GetIntAttribute(ATTR_area));
	//Pixel count
	if (errorNode->HasAttribute(ATTR_foregroundPixelCount))
		error->SetPixelCount(errorNode->GetIntAttribute(ATTR_foregroundPixelCount));
	//Count
	if (errorNode->HasAttribute(ATTR_count))
		error->SetCount(errorNode->GetIntAttribute(ATTR_count));
	//False alarm
	if (errorNode->HasAttribute(ATTR_falseAlarm))
		error->SetFalseAlarm(errorNode->GetBoolAttribute(ATTR_falseAlarm));

	//Rects and False Alarm Rects
	CMsXmlNode * tempNode = errorNode->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_ErrorRects))
		{
			list<CRect*> * rects = error->GetRects();
			ParseRects(tempNode, rects);
		}
		else if(tempNode->GetName() == CUniString(ELEMENT_FalseAlarmRects))
		{
			list<CRect*> * rects = error->GetFalseAlarmRects();
			ParseRects(tempNode, rects);
		}
		tempNode = tempNode->GetNextSibling();
	}


	//Merge specific things
	if (error->GetType() == CLayoutObjectEvaluationError::TYPE_MERGE)
	{
		CEvaluationErrorMerge * merge = (CEvaluationErrorMerge*)error;
		//CPageLayout * groundTruth = results->GetLayoutEvaluation()->GetGroundTruth();
		//CPageLayout * segResult = results->GetLayoutEvaluation()->GetSegResult();
		tempNode = errorNode->GetFirstChild();
		while (tempNode != NULL)
		{
			//Overlaps
			if(tempNode->GetName() == CUniString(ELEMENT_Merge))
			{
				//Seg result region
				CUniString segResultRegionId = tempNode->GetAttribute(ATTR_regionId);
				//CLayoutObject * segResultRegion = segResult->GetRegionById(regionId);

				if (!segResultRegionId.IsEmpty())
				{
					COverlapRects * overlapRects = ParseOverlap(tempNode, NULL);
					merge->AddErrorRects(segResultRegionId, overlapRects, false);
				}
			}
			//Allowable
			else if(tempNode->GetName() == CUniString(ELEMENT_AllowableEntry))
			{
				CUniString groundTruthRegionId = tempNode->GetAttribute(ATTR_regionId);
				//CLayoutObject * groundTruthReg = groundTruth->GetRegionById(regionId);

				if (!groundTruthRegionId.IsEmpty())
					merge->SetAllowable(groundTruthRegionId, tempNode->GetBoolAttribute(ATTR_allowable));
			}
			tempNode = tempNode->GetNextSibling();
		}
	}
	//Split specific things
	else if (error->GetType() == CLayoutObjectEvaluationError::TYPE_SPLIT)
	{
		CEvaluationErrorSplit * split = (CEvaluationErrorSplit*)error;
	
		//Allowable
		split->SetAllowable(errorNode->GetBoolAttribute(ATTR_allowable));
		//Overlaps
		//CPageLayout * segResult = results->GetLayoutEvaluation()->GetSegResult();
		COverlapRects * overlapRects = ParseOverlap(errorNode, NULL);
		split->SetSplittingRegions(overlapRects);
		delete overlapRects;
	}
	//Misclassification specific things
	else if (error->GetType() == CLayoutObjectEvaluationError::TYPE_MISCLASS)
	{
		CEvaluationErrorMisclass * misclass = (CEvaluationErrorMisclass*)error;
	
		//Overlaps
		//CPageLayout * segResult = results->GetLayoutEvaluation()->GetSegResult();
		COverlapRects * overlapRects = ParseOverlap(errorNode, NULL);
		misclass->SetMisclassRegions(overlapRects);
		delete overlapRects;
	}
}

/*
 * Parses the given node for rectangles and puts them into the specified vector.
 */
void CXmlEvaluationReader::ParseRects(CMsXmlNode * node, list<CRect*> * rects)
{
	//Rects attribute (list of rectangles)
	if (m_SchemaVersion >= SCHEMA_2013_07_15)
	{
		if (node->HasAttribute(ATTR_rects))
		{
			CUniString rectsString = node->GetAttribute(ATTR_rects);
			vector<CUniString> rectStringList;
			rectsString.Split(L" ", rectStringList);
			for (unsigned int i=0; i<rectStringList.size(); i++)
			{
				CUniString rectString = rectStringList[i];
				vector<CUniString> rectCoords;
				rectString.Split(L",", rectCoords);
				if (rectCoords.size() == 4)
				{
					int left	= rectCoords[0].ToInt();
					int top		= rectCoords[1].ToInt();
					int right	= rectCoords[2].ToInt();
					int bottom	= rectCoords[3].ToInt();
					rects->push_back(new CRect(left, top, right, bottom));
				}
			}
		}
	}
	else //Rect elements (for compatibility)
	{
		CMsXmlNode * tempNode = node->GetFirstChild();
		while (tempNode != NULL)
		{
			if(tempNode->GetName() == CUniString(ELEMENT_Rect))
			{
				int left = tempNode->GetIntAttribute(ATTR_left);
				int top = tempNode->GetIntAttribute(ATTR_top);
				int right = tempNode->GetIntAttribute(ATTR_right);
				int bottom = tempNode->GetIntAttribute(ATTR_bottom);
				CRect * rect = new CRect(left, top, right, bottom);
				rects->push_back(rect);
			}
			tempNode = tempNode->GetNextSibling();
		}
	}
}

/*
 * Parses overlap (e.g. for merge errors)
 */
COverlapRects * CXmlEvaluationReader::ParseOverlap(CMsXmlNode * node, CPageLayout * pageLayout)
{
	COverlapRects * overlapRects = new COverlapRects();

	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_Overlap))
		{
			//Region ID
			CUniString regionId = tempNode->GetAttribute(ATTR_regionId);
			//CLayoutObject * region = pageLayout->GetRegionById(regionId);
			if (!regionId.IsEmpty())
			{
				//Area
				int area = 0;
				if(tempNode->HasAttribute(ATTR_area))
					area = tempNode->GetIntAttribute(ATTR_area);
				//PixelCount
				int pixelCount = 0;
				if(tempNode->HasAttribute(ATTR_foregroundPixelCount))
					pixelCount = tempNode->GetIntAttribute(ATTR_foregroundPixelCount);
				//Add the overlap
				overlapRects->AddOverlap(regionId, area, pixelCount);
			}
		}
		tempNode = tempNode->GetNextSibling();
	}
	return overlapRects;
}

/*
 * Parses region overlap entries.
 */
vector<CUniString> CXmlEvaluationReader::ParseOverlapRegions(CMsXmlNode * parentNode)
{
	vector<CUniString> regions;
	CMsXmlNode * tempNode = parentNode->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_OverlapsRegion))
		{
			regions.push_back(tempNode->GetAttribute(ATTR_id));
		}
		tempNode = tempNode->GetNextSibling();
	}
	return regions;
}

/*
 * Evaluation Profile (weights, ...)
 */
void CXmlEvaluationReader::parseProfileNode(CEvaluationProfile * profile, 
											CMsXmlNode * profileNode)
{
	//Traverse children
	CMsXmlNode * tempNode = profileNode->GetFirstChild();

	//Name
	if (profileNode->HasAttribute(ATTR_name))
		profile->SetName(profileNode->GetAttribute(ATTR_name));

	while (tempNode != NULL)
	{
		//General Settings
		if(tempNode->GetName() == CUniString(ELEMENT_GeneralSettings))
		{
			CMsXmlParameterReader paramReader(tempNode);
			CParameterMap * generalSettings = paramReader.ReadParameters();

			if (generalSettings != NULL)
			{
				profile->SetGeneralSettings(generalSettings); //copies the params

				//Backwards compatibility
				if (m_SchemaVersion <= SCHEMA_2011_08_22)
				{
					CStringParameter * defTextType = (CStringParameter*)(profile->GetGeneralSettings()->GetParamForId(CEvaluationProfile::PARAM_DEFAULT_TEXT_TYPE));
					if (defTextType != NULL)
					{
						defTextType->SetValue(L"paragraph");
					}
				}
			}
			generalSettings->DeleteAll();
			delete generalSettings;
		}
		//Error Type Weights
		else if(tempNode->GetName() == CUniString(ELEMENT_ErrorTypeWeights))
		{
			parseErrorTypeWeights(profile, tempNode);
		}
		//Region Type Weights
		else if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeights))
		{
			parseRegionTypeWeights(profile, tempNode);
		}
		//Reading Order Weight
		else if(tempNode->GetName() == CUniString(ELEMENT_ReadingOrderWeight))
		{
			CReadingOrderWeight * weight = (CReadingOrderWeight*)profile->GetReadingOrderWeightObject();
			parseReadingOrderWeight(tempNode, weight);
		}
		//Text line weights
		else if(tempNode->GetName() == CUniString(ELEMENT_TextLineWeights))
		{
			parseTextSubStructureWeights(profile, CLayoutObject::TYPE_TEXT_LINE, tempNode);
		}
		//Word weights
		else if(tempNode->GetName() == CUniString(ELEMENT_WordWeights))
		{
			parseTextSubStructureWeights(profile, CLayoutObject::TYPE_WORD, tempNode);
		}
		//Glyph weights
		else if(tempNode->GetName() == CUniString(ELEMENT_GlyphWeights))
		{
			parseTextSubStructureWeights(profile, CLayoutObject::TYPE_GLYPH, tempNode);
		}
		//Reading order group weights
		else if (tempNode->GetName() == CUniString(ELEMENT_ReadingOrderGroupWeights))
		{
			parseReadingOrderGroupWeights(profile, tempNode);
		}
		//Border weights
		else if(tempNode->GetName() == CUniString(ELEMENT_BorderWeights))
		{
			parseBorderWeights(profile, tempNode);
		}
		//Reading order penalties
		else if(tempNode->GetName() == CUniString(ELEMENT_ReadingOrderPenalties))
		{
			parseReadingOrderPenalties(profile, tempNode);
		}
			
		tempNode = tempNode->GetNextSibling();
	}

	MigrateProfileFromOldFormat(profile);
}

void CXmlEvaluationReader::MigrateProfileFromOldFormat(CEvaluationProfile * profile)
{
	if (m_SchemaVersion < SCHEMA_2013_07_15)
	{
		//Handle new region types (Advert, Chem, Music)
		// -> Set the region type weights to the same as Maths regions

		CParameter * source = profile->GetRegionTypeWeightParam(CLayoutRegion::TYPE_MATHS);
		if (source != NULL)
		{
			CParameter * target = profile->GetRegionTypeWeightParam(CLayoutRegion::TYPE_ADVERT);
			if (target != NULL)
				target->SetValue(source);
			target = profile->GetRegionTypeWeightParam(CLayoutRegion::TYPE_CHEM);
			if (target != NULL)
				target->SetValue(source);
			target = profile->GetRegionTypeWeightParam(CLayoutRegion::TYPE_MUSIC);
			if (target != NULL)
				target->SetValue(source);
		}
	}
	if (m_SchemaVersion < SCHEMA_2018_07_15)
	{
		//Handle new region types (Map)
		// -> Set the region type weights to the same as Graphic regions
		CParameter * source = profile->GetRegionTypeWeightParam(CLayoutRegion::TYPE_GRAPHIC);
		if (source != NULL)
		{
			CParameter * target = profile->GetRegionTypeWeightParam(CLayoutRegion::TYPE_MAP);
			if (target != NULL)
				target->SetValue(source);
		}
		//Handle new region types (Custom)
		// -> Set the region type weights to the same as Unknown regions
		source = profile->GetRegionTypeWeightParam(CLayoutRegion::TYPE_UNKNOWN);
		if (source != NULL)
		{
			CParameter * target = profile->GetRegionTypeWeightParam(CLayoutRegion::TYPE_CUSTOM);
			if (target != NULL)
				target->SetValue(source);
		}
	}
}

/*
 * Parses the node for error type weight nodes and adds all found weights to the profile.
 */
void CXmlEvaluationReader::parseErrorTypeWeights(CEvaluationProfile * profile, CMsXmlNode * node)
{
	//Traverse children
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_ErrorTypeWeight))
		{
			parseErrorTypeWeight(profile, tempNode);
		}
		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Parses the weights for text line, word or glyph.
 */
void CXmlEvaluationReader::parseTextSubStructureWeights(CEvaluationProfile * profile, 
														int regionLevel, CMsXmlNode * node)
{
	//Traverse children
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_ErrorTypeWeight))
		{
			parseTextSubStructureWeight(profile, regionLevel, tempNode);
		}
		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Parses the weights for reading order group.
 */
void CXmlEvaluationReader::parseReadingOrderGroupWeights(CEvaluationProfile * profile, CMsXmlNode * node)
{
	//Traverse children
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if (tempNode->GetName() == CUniString(ELEMENT_ErrorTypeWeight))
		{
			parseReadingOrderGroupWeight(profile, tempNode);
		}
		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Parses border related weights.
 */
void CXmlEvaluationReader::parseBorderWeights(CEvaluationProfile * profile, CMsXmlNode * node)
{
	//Traverse children
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_IncludedBackgroundWeight))
		{
			CWeight * w = profile->GetIncludedBackgroundBorderWeightObject();
			if (w != NULL)
				parseWeight(w, tempNode, false);
		}
		else if(tempNode->GetName() == CUniString(ELEMENT_ExcludedForegroundWeight))
		{
			CWeight * w = profile->GetExcludedForegroundBorderWeightObject();
			if (w != NULL)
				parseWeight(w, tempNode, false);
		}
		else if(tempNode->GetName() == CUniString(ELEMENT_MissingRegionAreaWeights))
		{
			parseMissingRegionAreaBorderWeights(profile, tempNode);
		}
		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Parses the node for region type weight nodes and adds all found weights to the profile.
 */
void CXmlEvaluationReader::parseRegionTypeWeights(CEvaluationProfile * profile, CMsXmlNode * node)
{
	//Traverse children
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeight))
		{
			parseRegionTypeWeight(profile, tempNode, NULL);
		}
		tempNode = tempNode->GetNextSibling();
	}
}

void CXmlEvaluationReader::parseMissingRegionAreaBorderWeights(CEvaluationProfile * profile, CMsXmlNode * node)
{
	//Traverse children
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeight))
		{
			parseMissingRegionAreaBorderWeight(profile, tempNode, NULL);
		}
		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Parses error type weight node and children.
 */
void CXmlEvaluationReader::parseErrorTypeWeight(CEvaluationProfile * profile, CMsXmlNode * node)
{
	int type = 0;
	if (node->HasAttribute(ATTR_type))
	{
		if (m_SchemaVersion >= SCHEMA_2013_07_15)
			type = ErrorTypeStringToInt(node->GetAttribute(ATTR_type));
		else
			type = node->GetIntAttribute(ATTR_type);
	}

	CErrorTypeWeight * weight = profile->GetErrorTypeWeightObject(type);
	bool hasRegionTypeWeights = !weight->GetRegionTypeWeights()->empty();

	//Look for region type weight children
	CMsXmlNode * tempNode = NULL;
	/*tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeight))
		{
			hasRegionTypeWeights = true;
			break;
		}
		tempNode = tempNode->GetNextSibling();
	}*/

	parseWeight(weight, node, hasRegionTypeWeights);	//General weight attributes

	//Process children
	if (hasRegionTypeWeights)
	{
		tempNode = node->GetFirstChild();
		while (tempNode != NULL)
		{
			if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeight))
			{
				parseRegionTypeWeight(profile, tempNode, weight);
			}
			tempNode = tempNode->GetNextSibling();
		}
	}
}

/*
 * Parses error type weight for text line, word or glyph
 */
void CXmlEvaluationReader::parseTextSubStructureWeight(CEvaluationProfile * profile, int regionLevel, CMsXmlNode * node)
{
	int errType = 0;
	if (node->HasAttribute(ATTR_type))
	{
		if (m_SchemaVersion >= SCHEMA_2013_07_15)
			errType = ErrorTypeStringToInt(node->GetAttribute(ATTR_type));
		else
			errType = node->GetIntAttribute(ATTR_type);
	}

	CWeight * weight = profile->GetErrorTypeWeightObjectForTextSubStructure(errType, regionLevel);
	if (weight != NULL)
		parseWeight(weight, node, false);
}

/*
 * Parses error type weight for reading order group
 */
void CXmlEvaluationReader::parseReadingOrderGroupWeight(CEvaluationProfile * profile, CMsXmlNode * node)
{
	int errType = 0;
	if (node->HasAttribute(ATTR_type))
		errType = ErrorTypeStringToInt(node->GetAttribute(ATTR_type));

	CWeight * weight = profile->GetErrorTypeWeightObjectForReadingOrderGroup(errType);
	if (weight != NULL)
		parseWeight(weight, node, false);
}

/*
 * Parses region type weight node and children.
 */
void CXmlEvaluationReader::parseRegionTypeWeight(CEvaluationProfile * profile, 
												 CMsXmlNode * node, CWeight * parent)
{
	int type = 0;
	if (node->HasAttribute(ATTR_type))
	{
		if (m_SchemaVersion >= SCHEMA_2013_07_15)
			type = RegionTypeStringToInt(node->GetAttribute(ATTR_type));
		else
			type = node->GetIntAttribute(ATTR_type);
	}

	if (type == CLayoutRegion::DEPRECATED_TYPE_FRAME) // 19/07/2013 - Frame no longer supported
		return;

	bool enableAllowable = false;
	if (node->HasAttribute(ATTR_enableAllowable))
		enableAllowable = node->GetBoolAttribute(ATTR_enableAllowable);

	CLayoutObjectTypeWeight * weight = NULL;
	if (parent != NULL)
	{
		if (typeid(*parent) == typeid(CErrorTypeWeight))
			weight = ((CErrorTypeWeight*)parent)->GetRegionTypeWeight(type);
		else if (typeid(*parent) == typeid(CLayoutObjectTypeWeight))
			weight = ((CLayoutObjectTypeWeight*)parent)->GetRegionTypeWeight(type);
		else if (typeid(*parent) == typeid(CSubTypeWeight))
			weight = ((CSubTypeWeight*)parent)->GetRegionTypeWeight(type);
	}
	else //no parent (root region type weight)
	{
		weight = profile->GetRegionTypeWeightObject(type);
	}

	//Look for subtype weight children
	bool hasRegionTypeWeights = !weight->GetRegionTypeWeights()->empty();
	bool hasSubTypeWeights = !weight->GetSubTypeWeights()->empty();
	CMsXmlNode * tempNode = NULL;
	/*tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_SubTypeWeight))
			hasSubTypeWeights = true;
		if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeight))
			hasRegionTypeWeights = true;
		tempNode = tempNode->GetNextSibling();
	}*/

	parseWeight(weight, node, hasSubTypeWeights || hasRegionTypeWeights);	//General weight attributes

	//Process children
	if (hasSubTypeWeights || hasRegionTypeWeights)
	{
		bool foundSubTypeWeights = false;
		tempNode = node->GetFirstChild();
		while (tempNode != NULL)
		{
			if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeight))
			{
				parseRegionTypeWeight(profile, tempNode, weight);
			}
			else if(tempNode->GetName() == CUniString(ELEMENT_SubTypeWeight))
			{
				parseSubTypeWeight(tempNode, weight);
				foundSubTypeWeights = true;
			}
			tempNode = tempNode->GetNextSibling();
		}

		//Backwards compatibility
		if (m_SchemaVersion <= SCHEMA_2011_08_22 && hasSubTypeWeights && !foundSubTypeWeights)
		{
			//Subtype weights for Graphic and Chart have been introduced in schema version 2013-03-11
			//We have to pretend there are subtypes in the XML
			vector<CUniString> subtypes;
			if (type == CLayoutRegion::TYPE_GRAPHIC)
			{
				subtypes.push_back(L"logo");
				subtypes.push_back(L"letterhead");
				subtypes.push_back(L"handwritten-annotation");
				subtypes.push_back(L"stamp");
				subtypes.push_back(L"signature");
				subtypes.push_back(L"paper-grow");
				subtypes.push_back(L"punch-hole");
				subtypes.push_back(L"other");
			}
			else if (type == CLayoutRegion::TYPE_CHART)
			{
				subtypes.push_back(L"bar");
				subtypes.push_back(L"line");
				subtypes.push_back(L"pie");
				subtypes.push_back(L"scatter");
				subtypes.push_back(L"surface");
				subtypes.push_back(L"other");
			}
			for (unsigned int i=0; i<subtypes.size(); i++)
			{
				CSubTypeWeight * subTypeWeight = weight->GetSubTypeWeight(subtypes[i]);

				tempNode = node->GetFirstChild();
				while (tempNode != NULL)
				{
					if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeight))
					{
						parseRegionTypeWeight(profile, tempNode, subTypeWeight);
					}
					tempNode = tempNode->GetNextSibling();
				}
			}
		}
	}
}

/*
 * Parses region type weight node and children.
 */
void CXmlEvaluationReader::parseMissingRegionAreaBorderWeight(CEvaluationProfile * profile, 
															CMsXmlNode * node, CWeight * parent)
{
	int type = 0;
	if (node->HasAttribute(ATTR_type))
	{
		if (m_SchemaVersion >= SCHEMA_2013_07_15)
			type = RegionTypeStringToInt(node->GetAttribute(ATTR_type));
		else
			type = node->GetIntAttribute(ATTR_type);
		
		if (type == CLayoutRegion::DEPRECATED_TYPE_FRAME)
			return;
	}

	CLayoutObjectTypeWeight * weight = NULL;
	weight = profile->GetMissingRegionBorderWeightObject(type);

	//Look for subtype weight children
	bool hasSubTypeWeights = !weight->GetSubTypeWeights()->empty();
	CMsXmlNode * tempNode = NULL;
	/*bool hasSubTypeWeights = false;
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_SubTypeWeight))
			hasSubTypeWeights = true;
		tempNode = tempNode->GetNextSibling();
	}*/

	parseWeight(weight, node, hasSubTypeWeights);	//General weight attributes

	//Process children
	if (hasSubTypeWeights)
	{
		tempNode = node->GetFirstChild();
		while (tempNode != NULL)
		{
			if(tempNode->GetName() == CUniString(ELEMENT_SubTypeWeight))
			{
				parseMissingRegionAreaBorderSubTypeWeight(tempNode, weight);
			}
			tempNode = tempNode->GetNextSibling();
		}
	}
}

/*
 * Parses sub type weight node and children.
 */
void CXmlEvaluationReader::parseMissingRegionAreaBorderSubTypeWeight(CMsXmlNode * node, CWeight * parent)
{
	CUniString type;
	if (node->HasAttribute(ATTR_subtype))
		type = node->GetAttribute(ATTR_subtype);

	CSubTypeWeight * weight = NULL;
	weight = ((CLayoutObjectTypeWeight*)parent)->GetSubTypeWeight(type);

	parseWeight(weight, node, false);	//General weight attributes
}

/*
 * Parses sub type weight node and children.
 */
void CXmlEvaluationReader::parseSubTypeWeight(CMsXmlNode * node, CWeight * parent)
{
	CUniString type;
	if (node->HasAttribute(ATTR_subtype))
		type = node->GetAttribute(ATTR_subtype);

	bool enableAllowable = false;
	if (node->HasAttribute(ATTR_enableAllowable))
		enableAllowable = node->GetBoolAttribute(ATTR_enableAllowable);

	CSubTypeWeight * weight = NULL;
	weight = ((CLayoutObjectTypeWeight*)parent)->GetSubTypeWeight(type);

	//Look for region type weight children
	bool hasRegionTypeWeights = !weight->GetRegionTypeWeights()->empty();
	CMsXmlNode * tempNode = NULL;
	/*bool hasRegionTypeWeights = false;
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeight))
		{
			hasRegionTypeWeights = true;
			break;
		}
		tempNode = tempNode->GetNextSibling();
	}*/

	parseWeight(weight, node, hasRegionTypeWeights);	//General weight attributes

	//Process children
	if (hasRegionTypeWeights)
	{
		tempNode = node->GetFirstChild();
		while (tempNode != NULL)
		{
			if(tempNode->GetName() == CUniString(ELEMENT_RegionTypeWeight))
			{
				parseRegionTypeWeight(NULL, tempNode, weight);
			}
			tempNode = tempNode->GetNextSibling();
		}
	}
}

/*
 * Parses a reading order XML node and returns a reading order weight object.
 */
void CXmlEvaluationReader::parseReadingOrderWeight(CMsXmlNode * node, 
																	CReadingOrderWeight * weight)
{
	parseWeight(weight, node, false);
}

/*
 * Parses general weight attributes.
 */
void CXmlEvaluationReader::parseWeight(CWeight * weight, CMsXmlNode * node, bool hasChildren)
{
	if (weight == NULL)
		return;
	//Name
	if (node->HasAttribute(ATTR_name))
		weight->SetName(node->GetAttribute(ATTR_name));
	//if (!hasChildren)
	//{
		//Weight (non-allowable)
		if (node->HasAttribute(ATTR_weight))
			weight->SetValue(node->GetDoubleAttribute(ATTR_weight));
	//}
	//'Enable allowable' flag
	if (node->HasAttribute(ATTR_enableAllowable))
		weight->EnableAllowableWeight(node->GetBoolAttribute(ATTR_enableAllowable));
	//if (!hasChildren)
	//{
		if (weight->IsAllowableWeightEnabled())
		{
			//'Use allowable' flag
			if (node->HasAttribute(ATTR_useAllowable))
				weight->SetUseAllowableWeight(node->GetBoolAttribute(ATTR_useAllowable));
			//Allowable weight
			if (node->HasAttribute(ATTR_allowableWeight))
				weight->SetAllowableValue(node->GetDoubleAttribute(ATTR_allowableWeight));
		}
	//}
	//Description
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_Description))
		{
			if(tempNode->HasChildren())
				weight->SetDescription((tempNode->GetTextContent()));
			break;
		}
		tempNode = tempNode->GetNextSibling();
	}
}

/*
 * Returns the schema version of the given xml document.
 */
CUniString CXmlEvaluationReader::ExtractSchemaVersion(IXMLDOMDocument2 * xmlDoc)
{
	IXMLDOMElement * baseElement;
	char * tempString = NULL;

	CUniString version;
	if (xmlDoc->get_documentElement(&baseElement) == S_OK)
	{
		//Get the namespace
		BSTR temp = NULL;
		CUniString namesp;
		if (baseElement->get_namespaceURI(&temp) == S_OK)
		{
			namesp.Append(temp);

			if (!namesp.IsEmpty())
			{
				//Get the sub string right of the last '/'
				int pos = namesp.FindLast(CUniString(_T("/")));
				if (pos >= 0)
				{
					version = namesp.Right(namesp.GetLength()-pos-1);
				}
			}
		}
		SysFreeString(temp);
		baseElement->Release();
	}
	return version;
}

/*
 * Parses entries for the reading order penalty matrix.
 */
void CXmlEvaluationReader::parseReadingOrderPenalties(CEvaluationProfile * profile, CMsXmlNode * node)
{
	CReadingOrderPenalties * penalties = profile->GetReadingOrderPenalties();
	CMsXmlNode * tempNode = node->GetFirstChild();
	while (tempNode != NULL)
	{
		if(tempNode->GetName() == CUniString(ELEMENT_Penalty))
		{
			int row = tempNode->GetIntAttribute(ATTR_row);
			int col = tempNode->GetIntAttribute(ATTR_col);
			int value = tempNode->GetIntAttribute(ATTR_value);
			penalties->SetPenalty(row, col, value);
		}
		tempNode = tempNode->GetNextSibling();
	}
}
