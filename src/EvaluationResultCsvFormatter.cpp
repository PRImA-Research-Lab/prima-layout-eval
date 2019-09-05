
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "EvaluationResultCsvFormatter.h"
#include "ExtraFileHelper.h"
#include "XmlEvaluationReader.h"
#include "Region.h"

using namespace std;

namespace PRImA
{


/*
 * Class CEvaluationResultCsvFormatter
 *
 * CSV output for layout evaluation results.
 *
 * CC 24/09/2017 - created
 */

/*
 * Constructor
 */
CEvaluationResultCsvFormatter::CEvaluationResultCsvFormatter(CUniString lineBreak)
{
	m_LineBreak = lineBreak;
}

/*
 * Destructor
 */
CEvaluationResultCsvFormatter::~CEvaluationResultCsvFormatter()
{
}

/*
 * Output all result entries as CSV
 */
void CEvaluationResultCsvFormatter::OutputCsv(CLayoutEvaluation * eval, CUniString groundTruthFile, CUniString segResultFile,
	bool printHeaders, CUniString profile)
{
	bool printedHeaders = false;
	vector<int> regionLevels;
	regionLevels.push_back(CLayoutObject::TYPE_LAYOUT_REGION);
	regionLevels.push_back(CLayoutObject::TYPE_TEXT_LINE);
	regionLevels.push_back(CLayoutObject::TYPE_WORD);
	regionLevels.push_back(CLayoutObject::TYPE_GLYPH);
	regionLevels.push_back(CLayoutObject::TYPE_READING_ORDER_GROUP);
	regionLevels.push_back(CLayoutObject::TYPE_BORDER);

	for (unsigned int i = 0; i<regionLevels.size(); i++)
	{
		CUniString headers;
		CUniString values;

		if (!CreateCsv(headers, values, regionLevels[i], eval, groundTruthFile, segResultFile, profile))
			continue;

		//Print
		headers.Append(m_LineBreak);
		values.Append(m_LineBreak);

		if (printHeaders && !printedHeaders)
		{
			printf(headers.ToC_Str());
			printedHeaders = true;
		}
		printf(values.ToC_Str());
	}
}

/*
 * Create CSV and append to given headers/values.
 * Does NOT append line breaks.
 * returns true if CSV created
 */
bool CEvaluationResultCsvFormatter::CreateCsv(CUniString & headers, CUniString & values, int layoutObjectLevel, CLayoutEvaluation * eval, 
												CUniString groundTruthFile, CUniString segResultFile,
												CUniString profile)
{
	CEvaluationResults * results = eval->GetResults(layoutObjectLevel);
	if (results == NULL)
		return false;

	vector<int> errorTypes;
	GetAllErrorTypes(errorTypes);

	vector<int> regionTypes;
	GetAllRegionTypes(regionTypes);

	//Files
	headers.Append(_T("Ground-Truth"));
	headers.Append(_T(","));
	CUniString path, name;
	CExtraFileHelper::SplitPath(groundTruthFile, path, name);
	values.Append(name);
	values.Append(_T(","));

	headers.Append(_T("Segmentation Result"));
	headers.Append(_T(","));
	CExtraFileHelper::SplitPath(segResultFile, path, name);
	values.Append(name);
	values.Append(_T(","));

	//Structure level (regions, lines, words, glyphs)
	headers.Append(_T("Level"));
	headers.Append(_T(","));
	if (layoutObjectLevel == CLayoutObject::TYPE_LAYOUT_REGION)
		values.Append(_T("regions"));
	else if (layoutObjectLevel == CLayoutObject::TYPE_TEXT_LINE)
		values.Append(_T("lines"));
	else if (layoutObjectLevel == CLayoutObject::TYPE_WORD)
		values.Append(_T("words"));
	else if (layoutObjectLevel == CLayoutObject::TYPE_GLYPH)
		values.Append(_T("glyphs"));
	else if (layoutObjectLevel == CLayoutObject::TYPE_READING_ORDER_GROUP)
		values.Append(_T("groups"));
	else if (layoutObjectLevel == CLayoutObject::TYPE_BORDER)
		values.Append(_T("border"));
	values.Append(_T(","));


	if (layoutObjectLevel == CLayoutObject::TYPE_BORDER)
	{
		OutputBorderEvalResults(results, headers, values, errorTypes, regionTypes);
	}
	else //Region/line/word/glyph/group
	{
		OutputLayoutObjectEvalResults(results, headers, values, errorTypes, regionTypes);
	}

	headers.Append(_T("Profile"));
	headers.Append(_T(","));
	CExtraFileHelper::SplitPath(profile, path, name);
	values.Append(name);
	values.Append(_T(","));

	headers.Append(_T("SegResult Folder"));
	CExtraFileHelper::SplitPath(segResultFile, path, name);
	values.Append(path);

	return true;
}

/*
 * Fills the given vector with all evaluation error types
 */
void CEvaluationResultCsvFormatter::GetAllErrorTypes(vector<int> & errorTypes)
{
	if (errorTypes.empty())
	{
		errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_MERGE);
		errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_SPLIT);
		errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_MISS);
		errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_PART_MISS);
		errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_MISCLASS);
		errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_INVENT);
	}
}

/*
 * Fills the given vector with all layout region types
 */
void CEvaluationResultCsvFormatter::GetAllRegionTypes(vector<int> & regionTypes)
{
	if (regionTypes.empty())
	{
		regionTypes.push_back(CLayoutRegion::TYPE_TEXT);
		regionTypes.push_back(CLayoutRegion::TYPE_IMAGE);
		regionTypes.push_back(CLayoutRegion::TYPE_GRAPHIC);
		regionTypes.push_back(CLayoutRegion::TYPE_CHART);
		regionTypes.push_back(CLayoutRegion::TYPE_LINEART);
		regionTypes.push_back(CLayoutRegion::TYPE_MAP);
		regionTypes.push_back(CLayoutRegion::TYPE_TABLE);
		regionTypes.push_back(CLayoutRegion::TYPE_MATHS);
		regionTypes.push_back(CLayoutRegion::TYPE_SEPARATOR);
		regionTypes.push_back(CLayoutRegion::TYPE_ADVERT);
		regionTypes.push_back(CLayoutRegion::TYPE_CHEM);
		regionTypes.push_back(CLayoutRegion::TYPE_MUSIC);
		regionTypes.push_back(CLayoutRegion::TYPE_NOISE);
		regionTypes.push_back(CLayoutRegion::TYPE_CUSTOM);
		regionTypes.push_back(CLayoutRegion::TYPE_UNKNOWN);
	}
}

/*
 * Output border evlaution results as CSV
 */
void CEvaluationResultCsvFormatter::OutputBorderEvalResults(CEvaluationResults * results, CUniString & headers, CUniString & values,
	vector<int> & errorTypes, vector<int> & regionTypes)
{
	CBorderEvaluationMetrics * metrics = (CBorderEvaluationMetrics*)results->GetMetrics();

	//Moslty empty fields (not used for border eval)

	//Overall success rates

	headers.Append(CXmlEvaluationReader::ATTR_overallWeightedAreaSuccessRate);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_harmonicWeightedAreaSuccessRate);
	headers.Append(_T(","));
	values.Append(metrics->GetOverallSuccessRate(), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallWeightedCountSuccessRate);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_harmonicWeightedCountSuccessRate);
	headers.Append(_T(","));
	values.Append(_T(","));

	//F-Measure, Precision and Recall
	headers.Append(CXmlEvaluationReader::ATTR_fMeasureStrict);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_fMeasureNonStrict);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_recallStrict);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_recallNonStrict);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_precisionStrict);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_precisionNonStrict);
	headers.Append(_T(","));
	values.Append(_T(","));
	
	//Region Count Deviation
	headers.Append(_T("regionCountDeviation"));
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(_T("relativeRegionCountDeviation"));
	headers.Append(_T(","));
	values.Append(_T(","));

	//Weighted area success rate per error type (merge, split, ...)
	for (unsigned int i = 0; i<errorTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ELEMENT_WeightedAreaSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutObjectEvaluationError::GetTypeName(errorTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		values.Append(_T(","));
	}

	//Weighted count success rate per error type (merge, split, ...)
	for (unsigned int i = 0; i<errorTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ELEMENT_WeightedCountSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutObjectEvaluationError::GetTypeName(errorTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		values.Append(_T(","));
	}

	//Reading Order Success Rate
	headers.Append(CXmlEvaluationReader::ATTR_readingOrderSuccessRate);
	headers.Append(_T(","));
	values.Append(_T(","));

	//Arithmetic weighted area success rate per region type (text, image, ...)
	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ATTR_overallWeightedAreaSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		values.Append(_T(","));
	}

	//Harmonic weighted area success rate per region type (text, image, ...)
	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ATTR_harmonicWeightedAreaSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		values.Append(_T(","));
	}

	//Arithmetic weighted count success rate per region type (text, image, ...)
	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ATTR_overallWeightedCountSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		values.Append(_T(","));
	}

	//Harmonic weighted count success rate per region type (text, image, ...)
	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ATTR_harmonicWeightedCountSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		values.Append(_T(","));
	}

	//Precision / Recall per Region type (text, image, ...)
	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ELEMENT_Recall);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		values.Append(_T(","));
	}

	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ELEMENT_Precision);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		values.Append(_T(","));
	}

	//Error values
	headers.Append(CXmlEvaluationReader::ATTR_overallWeightedAreaError);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallWeightedCountError);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_readingOrderError);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_OCRSuccessRate);
	headers.Append(_T(","));
	values.Append(_T(","));

	//Border
	headers.Append(_T("BorderIncludedBackgroundSuccessRate"));
	headers.Append(_T(","));
	values.Append(metrics->GetIncludedBackgroundSuccessRate(), 3);
	values.Append(_T(","));

	headers.Append(_T("BorderExcludedForegroundSuccessRate"));
	headers.Append(_T(","));
	values.Append(metrics->GetExcludedForegroundSuccessRate(), 3);
	values.Append(_T(","));

	headers.Append(_T("BorderMissedRegionAreaSuccessRate"));
	headers.Append(_T(","));
	values.Append(metrics->GetMissingRegionAreaSuccessRate(), 3);
	values.Append(_T(","));

	//Statistics (partly empty)
	headers.Append(CXmlEvaluationReader::ATTR_numberOfGroundTruthRegions);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_numberOfSegResultRegions);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_imageArea);
	headers.Append(_T(","));
	values.Append(metrics->GetImageArea());
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallGroundTruthRegionArea);
	headers.Append(_T(","));
	values.Append(metrics->GetGroundTruthBorderArea());
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallGroundTruthRegionPixelCount);
	headers.Append(_T(","));
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallSegResultRegionArea);
	headers.Append(_T(","));
	values.Append(metrics->GetSegResultBorderArea());
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallSegResultRegionPixelCount);
	headers.Append(_T(","));
	values.Append(_T(","));
}

/*
 * Output evaluatino results for regions / text lines / words / glyphs / groups
 */
void CEvaluationResultCsvFormatter::OutputLayoutObjectEvalResults(CEvaluationResults * results, CUniString & headers, CUniString & values,
	vector<int> & errorTypes, vector<int> & regionTypes)
{
	CLayoutObjectEvaluationMetrics * metrics = (CLayoutObjectEvaluationMetrics*)results->GetMetrics();

	//Overall success rates

	headers.Append(CXmlEvaluationReader::ATTR_overallWeightedAreaSuccessRate);
	headers.Append(_T(","));
	values.Append(metrics->GetOverallWeightedAreaSuccessRate(), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_harmonicWeightedAreaSuccessRate);
	headers.Append(_T(","));
	values.Append(metrics->GetHarmonicWeightedAreaSuccessRate(), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallWeightedCountSuccessRate);
	headers.Append(_T(","));
	values.Append(metrics->GetOverallWeightedCountSuccessRate(), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_harmonicWeightedCountSuccessRate);
	headers.Append(_T(","));
	values.Append(metrics->GetHarmonicWeightedCountSuccessRate(), 3);
	values.Append(_T(","));

	//F-Measure, Precision and Recall

	headers.Append(CXmlEvaluationReader::ATTR_fMeasureStrict);
	headers.Append(_T(","));
	values.Append(metrics->GetFMeasure(true), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_fMeasureNonStrict);
	headers.Append(_T(","));
	values.Append(metrics->GetFMeasure(false), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_recallStrict);
	headers.Append(_T(","));
	values.Append(metrics->GetRecall(true), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_recallNonStrict);
	headers.Append(_T(","));
	values.Append(metrics->GetRecall(false), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_precisionStrict);
	headers.Append(_T(","));
	values.Append(metrics->GetPrecision(true), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_precisionNonStrict);
	headers.Append(_T(","));
	values.Append(metrics->GetPrecision(false), 3);
	values.Append(_T(","));


	//Region Count Deviation

	headers.Append(_T("regionCountDeviation"));
	headers.Append(_T(","));
	values.Append(metrics->GetRegionCountDeviation());
	values.Append(_T(","));

	headers.Append(_T("relativeRegionCountDeviation"));
	headers.Append(_T(","));
	values.Append(metrics->GetRelativeRegionCountDeviation(), 3);
	values.Append(_T(","));


	//Weighted area success rate per error type (merge, split, ...)

	for (unsigned int i = 0; i<errorTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ELEMENT_WeightedAreaSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutObjectEvaluationError::GetTypeName(errorTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		if (results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION ||
			results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP ||
			errorTypes[i] != CLayoutObjectEvaluationError::TYPE_MISCLASS)
		{
			values.Append(metrics->GetWeightedAreaSuccessRatePerType(errorTypes[i]), 3);
		}
		values.Append(_T(","));
	}

	//Weighted count success rate per error type (merge, split, ...)

	for (unsigned int i = 0; i<errorTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ELEMENT_WeightedCountSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutObjectEvaluationError::GetTypeName(errorTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		if (results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION ||
			results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP ||
			errorTypes[i] != CLayoutObjectEvaluationError::TYPE_MISCLASS)
		{
			values.Append(metrics->GetWeightedCountSuccessRatePerType(errorTypes[i]), 3);
		}
		values.Append(_T(","));
	}

	//Reading Order Success Rate
	headers.Append(CXmlEvaluationReader::ATTR_readingOrderSuccessRate);
	headers.Append(_T(","));
	values.Append(metrics->GetReadingOrderSuccessRate(), 3);
	values.Append(_T(","));

	//Arithmetic weighted area success rate per region type (text, image, ...)

	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		CLayoutObjectEvaluationMetrics * metPerType = results->GetMetrics(regionTypes[i]);
		headers.Append(CXmlEvaluationReader::ATTR_overallWeightedAreaSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		if (metPerType != NULL)
			values.Append(metPerType->GetOverallWeightedAreaSuccessRate(), 3);
		values.Append(_T(","));
	}

	//Harmonic weighted area success rate per region type (text, image, ...)

	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		CLayoutObjectEvaluationMetrics * metPerType = results->GetMetrics(regionTypes[i]);
		headers.Append(CXmlEvaluationReader::ATTR_harmonicWeightedAreaSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		if (metPerType != NULL)
			values.Append(metPerType->GetHarmonicWeightedAreaSuccessRate(), 3);
		values.Append(_T(","));
	}

	//Arithmetic weighted count success rate per region type (text, image, ...)

	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		CLayoutObjectEvaluationMetrics * metPerType = results->GetMetrics(regionTypes[i]);
		headers.Append(CXmlEvaluationReader::ATTR_overallWeightedCountSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		if (metPerType != NULL)
			values.Append(metPerType->GetOverallWeightedCountSuccessRate(), 3);
		values.Append(_T(","));
	}

	//Harmonic weighted count success rate per region type (text, image, ...)

	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		CLayoutObjectEvaluationMetrics * metPerType = results->GetMetrics(regionTypes[i]);
		headers.Append(CXmlEvaluationReader::ATTR_harmonicWeightedCountSuccessRate);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		if (metPerType != NULL)
			values.Append(metPerType->GetHarmonicWeightedCountSuccessRate(), 3);
		values.Append(_T(","));
	}

	//Precision / Recall per Region type (text, image, ...)

	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ELEMENT_Recall);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		if (results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
		{
			if (metrics->GetRecall(regionTypes[i]) >= 0.0)
				values.Append(metrics->GetRecall(regionTypes[i]), 3);
		}
		values.Append(_T(","));
	}

	for (unsigned int i = 0; i<regionTypes.size(); i++)
	{
		headers.Append(CXmlEvaluationReader::ELEMENT_Precision);
		headers.Append(_T("("));
		headers.Append(CLayoutRegion::GetTypeName(regionTypes[i]));
		headers.Append(_T(")"));
		headers.Append(_T(","));
		if (results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
		{
			if (metrics->GetPrecision(regionTypes[i]) >= 0.0)
				values.Append(metrics->GetPrecision(regionTypes[i]), 3);
		}
		values.Append(_T(","));
	}

	//Error values
	headers.Append(CXmlEvaluationReader::ATTR_overallWeightedAreaError);
	headers.Append(_T(","));
	values.Append(metrics->GetOverallWeightedAreaError(), 1);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallWeightedCountError);
	headers.Append(_T(","));
	values.Append(metrics->GetOverallWeightedCountError(), 1);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_readingOrderError);
	headers.Append(_T(","));
	if (results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
		values.Append(metrics->GetReadingOrderError(), 1);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_OCRSuccessRate);
	headers.Append(_T(","));
	if (results->GetLayoutObjectType() == CLayoutObject::TYPE_GLYPH)
		values.Append(metrics->GetOCRSuccessRate(), 3);
	values.Append(_T(","));

	//More OCR Rates (was Border placeholder)
	headers.Append(CXmlEvaluationReader::ATTR_OCRSuccessRateExclReplacementChar);
	headers.Append(_T(","));
	if (results->GetLayoutObjectType() == CLayoutObject::TYPE_GLYPH)
		values.Append(metrics->GetOCRSuccessRateExclReplacementChar(), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_OCRSuccessRateForDigits);
	headers.Append(_T(","));
	if (results->GetLayoutObjectType() == CLayoutObject::TYPE_GLYPH)
		values.Append(metrics->GetOCRSuccessRateForDigits(), 3);
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_OCRSuccessRateForNumericalChars);
	headers.Append(_T(","));
	if (results->GetLayoutObjectType() == CLayoutObject::TYPE_GLYPH)
		values.Append(metrics->GetOCRSuccessRateForNumericalChars(), 3);
	values.Append(_T(","));

	//Statistics
	headers.Append(CXmlEvaluationReader::ATTR_numberOfGroundTruthRegions);
	headers.Append(_T(","));
	values.Append(metrics->GetNumberOfGroundTruthRegions());
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_numberOfSegResultRegions);
	headers.Append(_T(","));
	values.Append(metrics->GetNumberOfSegResultRegions());
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_imageArea);
	headers.Append(_T(","));
	values.Append(metrics->GetImageArea());
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallGroundTruthRegionArea);
	headers.Append(_T(","));
	values.Append(metrics->GetOverallGroundTruthRegionArea());
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallGroundTruthRegionPixelCount);
	headers.Append(_T(","));
	values.Append(metrics->GetOverallGroundTruthRegionPixelCount());
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallSegResultRegionArea);
	headers.Append(_T(","));
	values.Append(metrics->GetOverallSegResultRegionArea());
	values.Append(_T(","));

	headers.Append(CXmlEvaluationReader::ATTR_overallSegResultRegionPixelCount);
	headers.Append(_T(","));
	values.Append(metrics->GetOverallSegResultRegionPixelCount());
	values.Append(_T(","));
}

/*
 * Prints the glyph statistics in CSV format
 */
void CEvaluationResultCsvFormatter::OutputGlyphStatistics(CLayoutEvaluation * eval, CUniString groundTruthFile, CUniString segResultFile,
			   bool printHeaders)
{
	CEvaluationResults * results = eval->GetResults(CLayoutObject::TYPE_GLYPH);
	if (results == NULL)
		return;

	CLayoutObjectEvaluationMetrics * metrics = (CLayoutObjectEvaluationMetrics*)results->GetMetrics();

	CGlyphStatistics * glyphStatistics = metrics->GetGlyphStatistics();

	if (glyphStatistics == NULL)
		return;

	//Headers
	if (printHeaders)
	{
		printf("\"Ground truth file\",\"OCR result file\",\"Ground truth character\",\"Ground truth Unicode\",\"OCR result character\",\"OCR result unicode\",\"Count\"\n");
	}

	//Values
	map<CUniString, CGlyphStatisticsItem*> * items = glyphStatistics->GetItems();

	for (map<CUniString, CGlyphStatisticsItem*>::iterator it = items->begin(); it != items->end(); it++)
	{
		CGlyphStatisticsItem * item = (*it).second;

		//Correct (no OCR error)
		OutputGlyphStatisticsRow(groundTruthFile, segResultFile, item->m_GroundTruthCharacter, L"", item->m_GroundTruthCharacter, L"", L"None", item->m_Matches);

		//Miss (no corresponding glyph in OCR result)
		OutputGlyphStatisticsRow(groundTruthFile, segResultFile, item->m_GroundTruthCharacter, L"", L"", L"", L"Miss", item->m_Misses);

		//Mismatches
		for (map<CUniString,int>::iterator itMismatch = item->m_OcrErrors.begin(); itMismatch != item->m_OcrErrors.end(); itMismatch++)
		{
			OutputGlyphStatisticsRow(groundTruthFile, segResultFile, item->m_GroundTruthCharacter, L"", (*itMismatch).first, L"", L"Mismatch", (*itMismatch).second);
		}
	}
}

/*
 * Prints a single CSV row for glyph statistics
 */
void CEvaluationResultCsvFormatter::OutputGlyphStatisticsRow(CUniString groundTruthFile, CUniString segResultFile,
							  CUniString groundTruthCharacter, CUniString groundTruthUnicode,
							  CUniString ocrResultCharacter, CUniString ocrResultUnicode,
							  CUniString ocrError, int count)
{
	//Ground truth file
	printf("\"");
	wprintf(groundTruthFile.GetBuffer());
	printf("\"");

	//OCR result file
	printf(",\"");
	wprintf(segResultFile.GetBuffer());
	printf("\"");

	//Ground truth character
	printf(",\"");
	CUniString character = groundTruthCharacter;
	//character.Replace(L",", L",,"); //Escape comma
	character.Replace(L"\"", L"\"\""); //Escape quotation mark
	wprintf(character);
	printf("\"");

	//Ground truth unicode
	printf(",\"");
	CharsToHexString(groundTruthCharacter);
	printf("\"");

	//OCR result character
	printf(",\"");
	character = ocrResultCharacter;
	//character.Replace(L",", L",,"); //Escape comma
	character.Replace(L"\"", L"\"\""); //Escape quotation mark
	wprintf(character);
	printf("\"");

	//OCR result unicode
	printf(",\"");
	CharsToHexString(ocrResultCharacter);
	printf("\"");

	//OCR error
	printf(",\"");
	wprintf(ocrError);
	printf("\"");

	//Count
	printf(",\"%d\"",count);

	wprintf(m_LineBreak);
}


/*
 * Outputs 4-digit hex codes for all characters of the given string to stdout (: separated)
 */
void CEvaluationResultCsvFormatter::CharsToHexString(CUniString str)
{
	for (int i=0; i<str.GetLength(); i++)
	{
		if (i > 0)
			printf(":");
		wchar_t c = str[i];
		printf("%#06x", c);
	}
}


} //end namespace