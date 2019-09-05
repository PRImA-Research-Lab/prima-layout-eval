#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "evaluationreader.h"
#include "XmlValidator.h"
#include "XmlEvaluationWriter.h"
#include "MsXmlReader.h"
#include "MsXmlNode.h"
#include "Crypto.h"

namespace PRImA
{

/*
 * Class CXmlEvaluationReader
 *
 * Class to read layout evaluation results and profiles from XML files.
 *
 * CC 15.01.2010 - created
 * CC 10.06.2010 - switched to MS XML
 */

class CXmlEvaluationReader :
	public CEvaluationReader
{
friend class CXmlEvaluationWriter;

private:
	int SCHEMA_2010_01_15;
	int SCHEMA_2011_08_22;
	int SCHEMA_2013_07_15;
	int SCHEMA_2018_07_15;
	int SCHEMA_2019_07_15;

public:
	CXmlEvaluationReader(void);
	CXmlEvaluationReader(CXmlValidatorProvider * xmlValidatorProvider);
	~CXmlEvaluationReader(void);

	bool	ReadLayoutEvaluation(CUniString fileName, CLayoutEvaluation * layoutEval,
											CMetaData * metaData);

	bool	ReadEvaluationProfile(	CUniString fileName, CEvaluationProfile * profile,
											CMetaData * metaData, CCrypto * crypto = NULL);

	bool	Read(CUniString fileName,
							CLayoutEvaluation * layoutEvaluation,
							CEvaluationProfile * evaluationProfile,
							CMetaData * metaData);

	inline CUniString		GetErrorMsg() { return m_Validator != NULL ? m_Validator->GetErrorMsg() : CUniString(); };

	inline void SetListener(CEvaluationReaderListener * listener) { m_Listener = listener; };

public:
	static const wchar_t* ATTR_evalId;
	static const wchar_t* ATTR_imageFilename;
	static const wchar_t* ATTR_imageWidth;
	static const wchar_t* ATTR_imageHeight;
	static const wchar_t* ATTR_id;
	static const wchar_t* ATTR_groundTruthFilename;
	static const wchar_t* ATTR_segmentationResultFilename;
	static const wchar_t* ATTR_version;
	static const wchar_t* ATTR_producer;

	static const wchar_t* ATTR_name;
	static const wchar_t* ATTR_weight;
	static const wchar_t* ATTR_allowableWeight;
	static const wchar_t* ATTR_enableAllowable;
	static const wchar_t* ATTR_useAllowable;
	static const wchar_t* ATTR_type;
	static const wchar_t* ATTR_subtype;
	static const wchar_t* ATTR_value;

	static const wchar_t* ATTR_regionId;
	static const wchar_t* ATTR_regionId1;
	static const wchar_t* ATTR_regionId2;

	static const wchar_t* ATTR_weightedAreaError;
	static const wchar_t* ATTR_weightedCountError;
	static const wchar_t* ATTR_area;
	static const wchar_t* ATTR_foregroundPixelCount;
	static const wchar_t* ATTR_count;
	static const wchar_t* ATTR_falseAlarm;
	static const wchar_t* ATTR_allowable;

	static const wchar_t* ATTR_penalty;
	static const wchar_t* ATTR_groundTruthRelation;
	static const wchar_t* ATTR_segResultRelation;

	static const wchar_t* ATTR_rects;
	static const wchar_t* ATTR_left;
	static const wchar_t* ATTR_top;
	static const wchar_t* ATTR_right;
	static const wchar_t* ATTR_bottom;

	static const wchar_t* ATTR_numberOfGroundTruthRegions;
	static const wchar_t* ATTR_numberOfSegResultRegions;
	static const wchar_t* ATTR_imageArea;
	static const wchar_t* ATTR_overallGroundTruthRegionArea;
	static const wchar_t* ATTR_overallGroundTruthRegionPixelCount;
	static const wchar_t* ATTR_overallSegResultRegionArea;
	static const wchar_t* ATTR_overallSegResultRegionPixelCount;
	static const wchar_t* ATTR_overallWeightedAreaError;
	static const wchar_t* ATTR_overallWeightedCountError;
	static const wchar_t* ATTR_readingOrderError;
	static const wchar_t* ATTR_readingOrderSuccessRate;
	static const wchar_t* ATTR_overallWeightedAreaSuccessRate;
	static const wchar_t* ATTR_overallWeightedCountSuccessRate;
	static const wchar_t* ATTR_harmonicWeightedAreaSuccessRate;
	static const wchar_t* ATTR_harmonicWeightedCountSuccessRate;
	static const wchar_t* ATTR_errorRateBasedOnSimpleCount;
	static const wchar_t* ATTR_recallNonStrict;
	static const wchar_t* ATTR_recallStrict;
	static const wchar_t* ATTR_precisionNonStrict;
	static const wchar_t* ATTR_precisionStrict;
	static const wchar_t* ATTR_fMeasureStrict;
	static const wchar_t* ATTR_fMeasureNonStrict;
	static const wchar_t* ATTR_regionCountDeviation;
	static const wchar_t* ATTR_relativeRegionCountDeviation;
	static const wchar_t* ATTR_OCRSuccessRate;
	static const wchar_t* ATTR_OCRSuccessRateExclReplacementChar;
	static const wchar_t* ATTR_OCRSuccessRateForDigits;
	static const wchar_t* ATTR_OCRSuccessRateForNumericalChars;

	static const wchar_t* ATTR_row;
	static const wchar_t* ATTR_col;

	static const wchar_t* ATTR_includedBackgroundSuccessRate;
	static const wchar_t* ATTR_excludedForegroundSuccessRate;
	static const wchar_t* ATTR_missingRegionAreaSuccessRate;
	static const wchar_t* ATTR_overallSuccessRate;

	static const wchar_t* ELEMENT_Eval;
	static const wchar_t* ELEMENT_EvalData;
	static const wchar_t* ELEMENT_Profile;

	static const wchar_t* ELEMENT_Metadata;
	static const wchar_t* ELEMENT_Creator;
	static const wchar_t* ELEMENT_Created;
	static const wchar_t* ELEMENT_LastChange;
	static const wchar_t* ELEMENT_Comments;
	static const wchar_t* ELEMENT_Software;

	static const wchar_t* ELEMENT_GeneralSettings;
	static const wchar_t* ELEMENT_ErrorTypeWeights;
	static const wchar_t* ELEMENT_RegionTypeWeights;
	static const wchar_t* ELEMENT_TextLineWeights;
	static const wchar_t* ELEMENT_WordWeights;
	static const wchar_t* ELEMENT_GlyphWeights;
	static const wchar_t* ELEMENT_ReadingOrderGroupWeights;
	static const wchar_t* ELEMENT_BorderWeights;
	static const wchar_t* ELEMENT_ErrorTypeWeight;
	static const wchar_t* ELEMENT_RegionTypeWeight;
	static const wchar_t* ELEMENT_SubTypeWeight;
	static const wchar_t* ELEMENT_ReadingOrderWeight;
	static const wchar_t* ELEMENT_Description;
	static const wchar_t* ELEMENT_IncludedBackgroundWeight;
	static const wchar_t* ELEMENT_ExcludedForegroundWeight;
	static const wchar_t* ELEMENT_MissingRegionAreaWeights;
	
	static const wchar_t* ELEMENT_ReadingOrderPenalties;
	static const wchar_t* ELEMENT_Penalty;

	static const wchar_t* ELEMENT_Results;
	static const wchar_t* ELEMENT_PageObjectResults;
	static const wchar_t* ELEMENT_BorderResults;
	static const wchar_t* ELEMENT_RawData;
	static const wchar_t* ELEMENT_GroundTruthOverlap;
	static const wchar_t* ELEMENT_SegResultOverlap;
	static const wchar_t* ELEMENT_OverlapsRegion;
	static const wchar_t* ELEMENT_RegionResults;
	static const wchar_t* ELEMENT_ReadingOrderResults;
	static const wchar_t* ELEMENT_ReadingOrderError;

	static const wchar_t* ELEMENT_Metrics;

	static const wchar_t* ELEMENT_RegionError;
	static const wchar_t* ELEMENT_RegionErrorMerge;
	static const wchar_t* ELEMENT_RegionErrorSplit;
	static const wchar_t* ELEMENT_RegionErrorMisclass;
	static const wchar_t* ELEMENT_ErrorRects;
	static const wchar_t* ELEMENT_FalseAlarmRects;
	static const wchar_t* ELEMENT_Rect;
	static const wchar_t* ELEMENT_Merge;
	static const wchar_t* ELEMENT_Overlap;
	static const wchar_t* ELEMENT_AllowableEntry;

	static const wchar_t* ELEMENT_SegResultRelation;
	static const wchar_t* ELEMENT_GroundTruthRelation;
	static const wchar_t* ELEMENT_Relation;

	static const wchar_t* ELEMENT_NumberOfGroundTruthRegions;
	static const wchar_t* ELEMENT_NumberOfSegResultRegions;
	static const wchar_t* ELEMENT_GroundTruthRegionArea;
	static const wchar_t* ELEMENT_GroundTruthRegionPixelCount;
	static const wchar_t* ELEMENT_SegResultRegionArea;
	static const wchar_t* ELEMENT_SegResultRegionPixelCount;
	static const wchar_t* ELEMENT_RecallArea;
	static const wchar_t* ELEMENT_RecallPixelCount;
	static const wchar_t* ELEMENT_OverallWeightedAreaErrorPerErrorType;
	static const wchar_t* ELEMENT_InfluenceWeightedAreaErrorPerErrorType;
	static const wchar_t* ELEMENT_OverallWeightedAreaErrorPerRegionType;
	static const wchar_t* ELEMENT_WeightedAreaSuccessRate;
	static const wchar_t* ELEMENT_OverallWeightedCountErrorPerErrorType;
	static const wchar_t* ELEMENT_InfluenceWeightedCountErrorPerErrorType;
	static const wchar_t* ELEMENT_OverallWeightedCountErrorPerRegionType;
	static const wchar_t* ELEMENT_WeightedCountSuccessRate;
	static const wchar_t* ELEMENT_ErrorRateBasedOnSimpleCount;
	static const wchar_t* ELEMENT_Recall;
	static const wchar_t* ELEMENT_Precision;

private:
	IXMLDOMDocument2 * validate(CUniString fileName, CMsXmlReader * msXmlReader);

	bool	ReadLayoutEvaluation(IXMLDOMDocument2 * doc, CLayoutEvaluation * layoutEvaluation);
	bool	ReadEvaluationProfile(IXMLDOMDocument2 * doc, CEvaluationProfile * evaluationProfile);
	bool	ReadMetaData(IXMLDOMDocument2 * doc, CMetaData * metaData);

	void	parseMetadataNode(CMetaData * metaData, CMsXmlNode * metadataNode);
	void	parseEvalDataNode(CLayoutEvaluation * layoutEvaluation, CMsXmlNode * evalDataNode);

	void	parseProfileNode(CEvaluationProfile * profile, CMsXmlNode * profileNode);
	void	parseErrorTypeWeights(CEvaluationProfile * profile, CMsXmlNode * node);
	void	parseRegionTypeWeights(CEvaluationProfile * profile, CMsXmlNode * node);
	void	parseTextSubStructureWeights(CEvaluationProfile * profile, int regionLevel, CMsXmlNode * node);
	void	parseReadingOrderGroupWeights(CEvaluationProfile * profile, CMsXmlNode * node);
	void    parseBorderWeights(CEvaluationProfile * profile, CMsXmlNode * node);
	void	parseMissingRegionAreaBorderWeights(CEvaluationProfile * profile, CMsXmlNode * node);
	void	parseMissingRegionAreaBorderSubTypeWeight(CMsXmlNode * node, CWeight * parent);

	void	parseErrorTypeWeight(CEvaluationProfile * profile, CMsXmlNode * node);
	void	parseRegionTypeWeight(CEvaluationProfile * profile, CMsXmlNode * node, CWeight * parent);
	void	parseSubTypeWeight(CMsXmlNode * node, CWeight * parent);
	void	parseReadingOrderWeight(CMsXmlNode * node, CReadingOrderWeight * weight);
	void    parseTextSubStructureWeight(CEvaluationProfile * profile, int regionLevel, CMsXmlNode * node);
	void    parseReadingOrderGroupWeight(CEvaluationProfile * profile, CMsXmlNode * node);
	void	parseWeight(CWeight * weight, CMsXmlNode * node, bool hasChildren);
	void	parseMissingRegionAreaBorderWeight(CEvaluationProfile * profile, CMsXmlNode * node, CWeight * parent);

	void	parseReadingOrderPenalties(CEvaluationProfile * profile, CMsXmlNode * node);

	void ParsePageObjectResultsNode(CMsXmlNode * resultsNode, CEvaluationResults * results, CEvaluationProfile * profile);
	void ParseBorderResultsNode(CMsXmlNode * resultsNode, CEvaluationResults * results, CEvaluationProfile * profile);
	void ParseRawData(CMsXmlNode * rawDataNode, CEvaluationResults * results, CEvaluationProfile * profile);
	vector<CUniString> ParseOverlapRegions(CMsXmlNode * parentNode);
	void ParseRegionResults(CMsXmlNode * resultsNode, CEvaluationResults * results);
	void ParseRegionError(CMsXmlNode * errorNode, CLayoutObjectEvaluationError * error, CEvaluationResults * results);
	COverlapRects * ParseOverlap(CMsXmlNode * node, CPageLayout * pageLayout);
	void ParseReadingOrderResults(CMsXmlNode * resultsNode, CEvaluationResults * results, CEvaluationProfile * profile);
	CReadingOrderError * ParseReadingOrderError(CMsXmlNode * errorNode, CEvaluationResults * results, CEvaluationProfile * profile);
	set<int> ParseRelationSet(CMsXmlNode * node);
	void ParseRects(CMsXmlNode * node, list<CRect*> * rects);

	void ParseMetricResults(CMsXmlNode * metricsNode, CEvaluationResults * results);
	void ParseMetricsNode(CMsXmlNode * metricsNode, CLayoutObjectEvaluationMetrics * metrics);
	void ParseIntPerRegionType(CMsXmlNode * node, map<int,int> * valueMap);
	void ParseDoublePerRegionType(CMsXmlNode * node, map<int,double> * valueMap);
	void ParseIntPerErrorType(CMsXmlNode * node, map<int,int> * valueMap);
	void ParseDoublePerErrorType(CMsXmlNode * node, map<int,double> * valueMap);

	CUniString	ExtractSchemaVersion(IXMLDOMDocument2 * doc);

	int RegionTypeStringToInt(CUniString typeName);
	int PageObjectLevelStringToInt(CUniString level);
	int ErrorTypeStringToInt(CUniString typeName);
	int ReadingOrderRelationTypeStringToInt(CUniString type);

	void MigrateProfileFromOldFormat(CEvaluationProfile * profile);

private:
	CValidator				*	m_Validator;
	CXmlValidatorProvider	*	m_XmlValidatorProvider;
	CUniString				*	m_Warning;
	CUniString					m_SchemaName;
	int							m_SchemaVersion;
	CEvaluationReaderListener * m_Listener;

};

}