
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "stdafx.h"
#include "EvaluationProfile.h"

using namespace PRImA;
using namespace std;

/*
 * Class CEvaluationProfile
 *
 * Contains the settings (profile) for document layout evaluation.
 *
 * CC 15.01.2010 - created
 */

const double CEvaluationProfile::WEIGHT_MIN = 0.0;
const double CEvaluationProfile::WEIGHT_MAX = 10.0;
const double CEvaluationProfile::WEIGHT_INIT = 1.0;

const wchar_t * CEvaluationProfile::USAGE_AS_DEFINED_IN_GROUNDTRUTH				= _T("As specified in ground-truth");
const wchar_t * CEvaluationProfile::USAGE_USE_DEFAULT_IF_NOT_IN_GROUND_TRUTH	= _T("Use default if not set in ground-truth");
const wchar_t * CEvaluationProfile::USAGE_ALWAYS_USE_DEFAULT					= _T("Always use default value");

const wchar_t * CEvaluationProfile::STRING_SUBTYPE_UNDEFINED = L"<undefined>";

/*
 * Constructor
 */
CEvaluationProfile::CEvaluationProfile(CXmlValidator * pageLayoutXmlValidator)
{
	m_PageLayoutXmlValidator = pageLayoutXmlValidator;
	m_RegionTypes = NULL;

	m_UsePixelArea.Init(PARAM_USE_PIXEL_AREA,
						"Use foreground area", 
						"Use the number of foreground pixels instead of the area defined by polygons.",
						true);

	m_ReadingOrientationThreshold.Init(PARAM_READING_ORIENTATION_THRESHOLD,
										"Rd Orientation Threshold", 
										"Threshold until when a difference of two reading orientation angles is allowable.",
										10.0, 0.0, 90.0, 1.0); //(init, min, max, step)
	m_ReadingOrientationThreshold.SetReadOnly(true);

	m_DefaultReadingDirection.Init(PARAM_DEFAULT_READING_DIRECTION,
									"Def Rd Dir",
									"Default Reading Direction",
									CUniString(_T("left-to-right")));
	m_DefaultReadingDirection.SetTextType(CStringParameter::TEXTTYPE_LIST);
	set<CUniString> * validValues = m_DefaultReadingDirection.GetValidValues();
	validValues->insert(CUniString(_T("left-to-right")));
	validValues->insert(CUniString(_T("right-to-left")));
	validValues->insert(CUniString(_T("top-to-bottom")));
	validValues->insert(CUniString(_T("bottom-to-top")));

	m_DefaultReadingDirectionUsage.Init(PARAM_READING_DIRECTION_USAGE,
									"Rd Dir Usage",
									"Reading Direction Usage",
									CUniString(USAGE_AS_DEFINED_IN_GROUNDTRUTH));
	m_DefaultReadingDirectionUsage.SetTextType(CStringParameter::TEXTTYPE_LIST);
	validValues = m_DefaultReadingDirectionUsage.GetValidValues();
	validValues->insert(CUniString(USAGE_AS_DEFINED_IN_GROUNDTRUTH));
	validValues->insert(CUniString(USAGE_USE_DEFAULT_IF_NOT_IN_GROUND_TRUTH));
	validValues->insert(CUniString(USAGE_ALWAYS_USE_DEFAULT));

	m_DefaultReadingOrientation.Init(PARAM_DEFAULT_READING_ORIENTATION,
										"Def Rd Orientation", 
										"Default reading orientation",
										0.0, -179, 180.0, 1.0); //(init, min, max, step)

	m_DefaultReadingOrientationUsage.Init(PARAM_READING_ORIENTATION_USAGE,
									"Rd Orient. Usage",
									"Reading Orientation Usage",
									CUniString(USAGE_AS_DEFINED_IN_GROUNDTRUTH));
	m_DefaultReadingOrientationUsage.SetTextType(CStringParameter::TEXTTYPE_LIST);
	validValues = m_DefaultReadingOrientationUsage.GetValidValues();
	validValues->insert(CUniString(USAGE_AS_DEFINED_IN_GROUNDTRUTH));
	validValues->insert(CUniString(USAGE_USE_DEFAULT_IF_NOT_IN_GROUND_TRUTH));
	validValues->insert(CUniString(USAGE_ALWAYS_USE_DEFAULT));

	m_DefaultTextType.Init(PARAM_DEFAULT_TEXT_TYPE,
									"Default text type",
									"Default type for text regions of unspecified type",
									STRING_SUBTYPE_UNDEFINED);
	m_DefaultTextType.SetSet(true);
	m_DefaultTextType.SetTextType(CStringParameter::TEXTTYPE_LIST);
	validValues = m_DefaultTextType.GetValidValues();
	validValues->insert(STRING_SUBTYPE_UNDEFINED);
	validValues->insert(L"paragraph");
	validValues->insert(L"heading");
	validValues->insert(L"caption");
	validValues->insert(L"header");
	validValues->insert(L"footer");
	validValues->insert(L"page-number");
	validValues->insert(L"drop-capital");
	validValues->insert(L"credit");
	validValues->insert(L"floating");
	validValues->insert(L"signature-mark");
	validValues->insert(L"catch-word");
	validValues->insert(L"marginalia");
	validValues->insert(L"footnote");
	validValues->insert(L"footnote-continued");
	validValues->insert(L"TOC-entry");
	validValues->insert(L"list-label");
	validValues->insert(L"other");

	m_IgnoreEmbeddedTextMisclass.Init(PARAM_IGNORE_EMB_TEXT_MISCLASS,
						"Ignore embedded text misclassification", 
						"Misclassification of charts, graphics, tables, etc with embedded text is not penalised (if a text region was found instead)",
						false);

	m_EvaluateNestedRegions.Init(PARAM_EVALUATE_NESTED_REGIONS,
		"Evaluate nested regions",
		"Include nested regions (regions within regions) in the evaluation",
		false);

	m_MaxOverlapForAllowableSplitAndMerge.Init(PARAM_MAX_OVERLAP_FOR_ALLOWABLE_SPLIT_AND_MERGE,
		"Max overlap for allowable",
		"Maximum bounding box overlap in pixels for split/merge allowable check",
		20, 0, 200, 1); //(init, min, max, step)

	//General settings parameter map
	m_GeneralSettings.Add(&m_UsePixelArea);
	m_GeneralSettings.Add(&m_ReadingOrientationThreshold);
	m_GeneralSettings.Add(&m_MaxOverlapForAllowableSplitAndMerge);
	m_GeneralSettings.Add(&m_DefaultReadingDirection);
	m_GeneralSettings.Add(&m_DefaultReadingDirectionUsage);
	m_GeneralSettings.Add(&m_DefaultReadingOrientation);
	m_GeneralSettings.Add(&m_DefaultReadingOrientationUsage);
	m_GeneralSettings.Add(&m_DefaultTextType);
	m_GeneralSettings.Add(&m_IgnoreEmbeddedTextMisclass);
	m_GeneralSettings.Add(&m_EvaluateNestedRegions);

	m_ReadingOrderWeight = NULL;
	m_IncludedBackgroundBorderWeight = NULL;
	m_ExcludedForegroundBorderWeight = NULL;

	m_ReadingOrderPenalties = new CReadingOrderPenalties();

	InitWeights();
}

/*
 * Destructor
 */
CEvaluationProfile::~CEvaluationProfile(void)
{
	delete m_RegionTypes;

	//Error type weights
	map<int,CErrorTypeWeight*>::iterator it = m_ErrorTypeWeights.begin();
	while (it != m_ErrorTypeWeights.end())
	{
		delete (*it).second;
		it++;
	}

	//Region type weights
	map<int,CLayoutObjectTypeWeight*>::iterator it2 = m_RegionTypeWeights.begin();
	while (it2 != m_RegionTypeWeights.end())
	{
		delete (*it2).second;
		it2++;
	}

	//Reading Order weight
	delete m_ReadingOrderWeight;

	//Text line, word and glyph weights
	map<int, map<int, CWeight*>*>::iterator it3 = m_TextSubStructureWeights.begin();
	while (it3 != m_TextSubStructureWeights.end())
	{
		map<int, CWeight*> * errTypeMap = (*it3).second;
		map<int, CWeight*>::iterator itErrType = errTypeMap->begin();
		while (itErrType != errTypeMap->end())
		{
			delete (*itErrType).second;
			itErrType++;
		}
		delete errTypeMap;
		it3++;
	}

	//Reading order group weigths
	map<int, CWeight*>::iterator itErrType = m_ReadingOrderGroupWeights.begin();
	while (itErrType != m_ReadingOrderGroupWeights.end())
	{
		delete (*itErrType).second;
		itErrType++;
	}

	//Border weights
	delete m_IncludedBackgroundBorderWeight;
	delete m_ExcludedForegroundBorderWeight;
	map<int,CLayoutObjectTypeWeight*>::iterator it4 = m_MissingRegionAreaBorderWeights.begin();
	while (it4 != m_MissingRegionAreaBorderWeights.end())
	{
		delete (*it4).second;
		it4++;
	}

	delete m_ReadingOrderPenalties;
}

/*
 * Copies the values from the given parameter map into the internal map.
 */
void CEvaluationProfile::SetGeneralSettings(CParameterMap * settings)
{
	//Only copy the values, not the objects themselves
	for (int i=0; i<m_GeneralSettings.GetSize(); i++)
	{
		CParameter * target = m_GeneralSettings.Get(i);
		CParameter * source = settings->GetParamForId(target->GetId());
		if (source != NULL)
			target->SetValue(source);
	}
}

/*
 * Creates all weights with default values.
 */
void CEvaluationProfile::InitWeights()
{
	//Merge
	m_ErrorTypeWeights.insert(pair<int, CErrorTypeWeight *>(CLayoutObjectEvaluationError::TYPE_MERGE, 
															CreateMergeWeight()));
	//Split
	m_ErrorTypeWeights.insert(pair<int, CErrorTypeWeight *>(CLayoutObjectEvaluationError::TYPE_SPLIT, 
															CreateSplitWeight()));
	//Miss
	m_ErrorTypeWeights.insert(pair<int, CErrorTypeWeight *>(CLayoutObjectEvaluationError::TYPE_MISS, 
															CreateMissWeight()));
	//Part. miss
	m_ErrorTypeWeights.insert(pair<int, CErrorTypeWeight *>(CLayoutObjectEvaluationError::TYPE_PART_MISS, 
															CreatePartMissWeight()));
	//Misclass.
	m_ErrorTypeWeights.insert(pair<int, CErrorTypeWeight *>(CLayoutObjectEvaluationError::TYPE_MISCLASS, 
															CreateMisclassWeight()));
	//Invented
	m_ErrorTypeWeights.insert(pair<int, CErrorTypeWeight *>(CLayoutObjectEvaluationError::TYPE_INVENT, 
															CreateInventWeight()));

	//Region type weights
	CreateRegionTypeWeights();

	//Reading order weight
	m_ReadingOrderWeight = new CReadingOrderWeight();

	//Text lines, words and glyphs
	vector<int> errorTypes;
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_MERGE);
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_SPLIT);
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_MISS);
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_PART_MISS);
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_INVENT);
	//Lines
	map<int, CWeight*> * textLineWeights = new map<int, CWeight*>();
	m_TextSubStructureWeights.insert(pair<int, map<int, CWeight*>*>(CLayoutObject::TYPE_TEXT_LINE, textLineWeights));
	for (unsigned int i=0; i<errorTypes.size(); i++)
	{
		textLineWeights->insert(pair<int, CWeight*>(errorTypes[i],
													new CTextSubStructureWeight(errorTypes[i])));
	}
	//Words
	map<int, CWeight*> * wordWeights = new map<int, CWeight*>();
	m_TextSubStructureWeights.insert(pair<int, map<int, CWeight*>*>(CLayoutObject::TYPE_WORD, wordWeights));
	for (unsigned int i=0; i<errorTypes.size(); i++)
	{
		wordWeights->insert(pair<int, CWeight*>(errorTypes[i],
												new CTextSubStructureWeight(errorTypes[i])));
	}
	//Glyphs
	map<int, CWeight*> * glyphWeights = new map<int, CWeight*>();
	m_TextSubStructureWeights.insert(pair<int, map<int, CWeight*>*>(CLayoutObject::TYPE_GLYPH, glyphWeights));
	for (unsigned int i=0; i<errorTypes.size(); i++)
	{
		glyphWeights->insert(pair<int, CWeight*>(errorTypes[i],
												new CTextSubStructureWeight(errorTypes[i])));
	}

	//Reading Order groups (includes misclassification)
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_MISCLASS);
	for (unsigned int i = 0; i<errorTypes.size(); i++)
	{
		m_ReadingOrderGroupWeights.insert(pair<int, CWeight*>(errorTypes[i],
			new CTextSubStructureWeight(errorTypes[i])));
	}

	//Border
	m_IncludedBackgroundBorderWeight = new CBorderWeight("Included Background", "By border included background area");
	m_ExcludedForegroundBorderWeight = new CBorderWeight("Excluded Foreground", "By border excluded foreground area");

	vector<int> * regionTypes = GetRegionTypes();
	for (unsigned int i=0; i<regionTypes->size(); i++)
	{
		CLayoutObjectTypeWeight * rtWeight = CreateRegionTypeWeight(	regionTypes->at(i), 
																false,
																true,
																false, false);
		m_MissingRegionAreaBorderWeights.insert(pair<int, CLayoutObjectTypeWeight *>(regionTypes->at(i), 
															rtWeight));
	}

}

/*
 * Prints the weights to a string.
 */
CUniString CEvaluationProfile::ToString(bool errorTypeWeights, bool regionTypeWeights, 
										bool readingOrderWeight, bool printOnlyNonDefaultValues)
{
	CUniString ret;
	if (errorTypeWeights)
	{
		ret.Append(_T("Error Type Weights\n"));
		//Merge
		CErrorTypeWeight * errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_MERGE);
		if (errTypeWeight != NULL)
			ret.Append(errTypeWeight->ToString(printOnlyNonDefaultValues));
		//Split
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_SPLIT);
		if (errTypeWeight != NULL)
			ret.Append(errTypeWeight->ToString(printOnlyNonDefaultValues));
		//Miss
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_MISS);
		if (errTypeWeight != NULL)
			ret.Append(errTypeWeight->ToString(printOnlyNonDefaultValues));
		//Partial miss
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_PART_MISS);
		if (errTypeWeight != NULL)
			ret.Append(errTypeWeight->ToString(printOnlyNonDefaultValues));
		//Misclass
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_MISCLASS);
		if (errTypeWeight != NULL)
			ret.Append(errTypeWeight->ToString(printOnlyNonDefaultValues));
		//False Detection
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_INVENT);
		if (errTypeWeight != NULL)
			ret.Append(errTypeWeight->ToString(printOnlyNonDefaultValues));
		ret.Append(_T("\n"));
	}
	if (regionTypeWeights)
	{
		ret.Append(_T("Region Type Weights\n"));
		vector<int> regionTypes;
		regionTypes.push_back(CLayoutRegion::TYPE_TEXT); 
		regionTypes.push_back(CLayoutRegion::TYPE_IMAGE); 
		regionTypes.push_back(CLayoutRegion::TYPE_GRAPHIC);  
		regionTypes.push_back(CLayoutRegion::TYPE_CHART);
		regionTypes.push_back(CLayoutRegion::TYPE_LINEART); 
		regionTypes.push_back(CLayoutRegion::TYPE_MAP);
		regionTypes.push_back(CLayoutRegion::TYPE_TABLE);
		regionTypes.push_back(CLayoutRegion::TYPE_MATHS); 
		regionTypes.push_back(CLayoutRegion::TYPE_ADVERT); 
		regionTypes.push_back(CLayoutRegion::TYPE_CHEM); 
		regionTypes.push_back(CLayoutRegion::TYPE_MUSIC); 
		regionTypes.push_back(CLayoutRegion::TYPE_SEPARATOR);
		//regionTypes.push_back(CLayoutRegion::TYPE_FRAME);
		regionTypes.push_back(CLayoutRegion::TYPE_NOISE);
		regionTypes.push_back(CLayoutRegion::TYPE_CUSTOM);
		regionTypes.push_back(CLayoutRegion::TYPE_UNKNOWN);

		for (unsigned int i=0; i<regionTypes.size(); i++)
		{
			CLayoutObjectTypeWeight * regTypeWeight = GetRegionTypeWeightObject(regionTypes[i]);
			if (regTypeWeight != NULL)
				ret.Append(regTypeWeight->ToString(printOnlyNonDefaultValues));
		}
	}
	if (readingOrderWeight)
	{
		m_ReadingOrderWeight->ToString(printOnlyNonDefaultValues);
		ret.Append(_T("\n"));
	}
	return ret;
}

/*
 * Returns a vector of all non-default weights
 *
 * 'errorTypeWeights' - If true, error type weights are used, otherwise region type weights.
 */
vector<pair<int,CWeight*>> * CEvaluationProfile::GetImportantWeights(bool errorTypeWeights, bool includeAllowable /*=false*/,
																	 bool include_1_0_weights /*= false*/)
{
	vector<pair<int,CWeight*>> * weights = new vector<pair<int,CWeight*>>();

	//Error type weights
	if (errorTypeWeights)
	{
		//Merge
		CErrorTypeWeight * errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_MERGE);
		if (errTypeWeight != NULL)
			errTypeWeight->GetImportantWeights(weights, 0, !include_1_0_weights, includeAllowable);
		//Split
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_SPLIT);
		if (errTypeWeight != NULL)
			errTypeWeight->GetImportantWeights(weights, 0, !include_1_0_weights, includeAllowable);
		//Miss
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_MISS);
		if (errTypeWeight != NULL)
			errTypeWeight->GetImportantWeights(weights, 0, !include_1_0_weights, includeAllowable);
		//Partial miss
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_PART_MISS);
		if (errTypeWeight != NULL)
			errTypeWeight->GetImportantWeights(weights, 0, !include_1_0_weights, includeAllowable);
		//Misclass
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_MISCLASS);
		if (errTypeWeight != NULL)
			errTypeWeight->GetImportantWeights(weights, 0, !include_1_0_weights, includeAllowable);
		//False Detection
		errTypeWeight = GetErrorTypeWeightObject(CLayoutObjectEvaluationError::TYPE_INVENT);
		if (errTypeWeight != NULL)
			errTypeWeight->GetImportantWeights(weights, 0, !include_1_0_weights, includeAllowable);
	}
	else
	{
		//Region Type Weights
		vector<int> regionTypes;
		regionTypes.push_back(CLayoutRegion::TYPE_TEXT); 
		regionTypes.push_back(CLayoutRegion::TYPE_IMAGE); 
		regionTypes.push_back(CLayoutRegion::TYPE_GRAPHIC);  
		regionTypes.push_back(CLayoutRegion::TYPE_CHART);
		regionTypes.push_back(CLayoutRegion::TYPE_LINEART); 
		regionTypes.push_back(CLayoutRegion::TYPE_MAP);
		regionTypes.push_back(CLayoutRegion::TYPE_TABLE);
		regionTypes.push_back(CLayoutRegion::TYPE_MATHS); 
		regionTypes.push_back(CLayoutRegion::TYPE_ADVERT); 
		regionTypes.push_back(CLayoutRegion::TYPE_CHEM); 
		regionTypes.push_back(CLayoutRegion::TYPE_MUSIC); 
		regionTypes.push_back(CLayoutRegion::TYPE_SEPARATOR);
		//regionTypes.push_back(CLayoutRegion::TYPE_FRAME);
		regionTypes.push_back(CLayoutRegion::TYPE_NOISE);
		regionTypes.push_back(CLayoutRegion::TYPE_UNKNOWN);
		regionTypes.push_back(CLayoutRegion::TYPE_CUSTOM);

		for (unsigned int i=0; i<regionTypes.size(); i++)
		{
			CLayoutObjectTypeWeight * regTypeWeight = GetRegionTypeWeightObject(regionTypes[i]);
			if (regTypeWeight != NULL)
				regTypeWeight->GetImportantWeights(weights, 0, false, false);
		}
	}
	return weights;
}

/*
 * Resets all weights and settings to their initial value.
 */
void CEvaluationProfile::Reset()
{
	//General Settings
	m_UsePixelArea.SetValue(true);
	m_ReadingOrientationThreshold.SetValue(10.0);
	m_DefaultReadingDirection.SetValue(CUniString(_T("left-to-right")));
	m_DefaultReadingDirectionUsage.SetValue(CUniString(USAGE_AS_DEFINED_IN_GROUNDTRUTH));
	m_DefaultReadingOrientation.SetValue(0.0);
	m_DefaultReadingOrientationUsage.SetValue(CUniString(USAGE_AS_DEFINED_IN_GROUNDTRUTH));
	m_IgnoreEmbeddedTextMisclass.SetValue(false);

	//Error type weights
	map<int, CErrorTypeWeight *>::iterator itErrType = m_ErrorTypeWeights.begin();
	while (itErrType != m_ErrorTypeWeights.end())
	{
		CErrorTypeWeight * weight = (*itErrType).second;
		weight->SetValue(1.0);
		if (weight->IsAllowableWeightEnabled())
		{
			weight->SetUseAllowableWeight(true);
			weight->SetAllowableValue(1.0);
		}
		itErrType++;
	}

	//Region type weights
	map<int, CLayoutObjectTypeWeight *>::iterator itRegType = m_RegionTypeWeights.begin();
	while (itRegType != m_RegionTypeWeights.end())
	{
		CLayoutObjectTypeWeight * weight = (*itRegType).second;
		weight->SetValue(1.0);
		itRegType++;
	}

	//Reading Order Weight
	m_ReadingOrderWeight->SetValue(1.0);

	//Border weights
	m_IncludedBackgroundBorderWeight->SetValue(1.0);
	m_ExcludedForegroundBorderWeight->SetValue(1.0);
	map<int, CLayoutObjectTypeWeight *>::iterator itBorder = m_MissingRegionAreaBorderWeights.begin();
	while (itBorder != m_MissingRegionAreaBorderWeights.end())
	{
		CLayoutObjectTypeWeight * weight = (*itBorder).second;
		weight->SetValue(1.0);
		itBorder++;
	}

	m_FilePath.Clear();
}

/*
 * Initializes the weight-subtree for error type MERGE.
 */
CErrorTypeWeight * CEvaluationProfile::CreateMergeWeight()
{
	CErrorTypeWeight * etWeight = new CErrorTypeWeight(CLayoutObjectEvaluationError::TYPE_MERGE);

	vector<int> * regionTypes = GetRegionTypes();
	if (regionTypes != NULL)
	{
		for (unsigned int i=0; i<regionTypes->size(); i++)
		{
			CLayoutObjectTypeWeight * rtWeight = CreateRegionTypeWeight(regionTypes->at(i), true, true, true, true);
			etWeight->AddRegionTypeWeight(regionTypes->at(i), rtWeight);
		}
	}
	return etWeight;
}

/*
 * Initializes the weight-subtree for error type MISCLASS.
 */
CErrorTypeWeight * CEvaluationProfile::CreateMisclassWeight()
{
	CErrorTypeWeight * etWeight = new CErrorTypeWeight(CLayoutObjectEvaluationError::TYPE_MISCLASS);

	vector<int> * regionTypes = GetRegionTypes();
	if (regionTypes != NULL)
	{
		for (unsigned int i=0; i<regionTypes->size(); i++)
		{
			CLayoutObjectTypeWeight * rtWeight = CreateRegionTypeWeight(regionTypes->at(i), false, true, true, true);
			etWeight->AddRegionTypeWeight(regionTypes->at(i), rtWeight);
		}
	}
	return etWeight;
}

/*
 * Initializes the weight-subtree for error type SPLIT.
 */
CErrorTypeWeight * CEvaluationProfile::CreateSplitWeight()
{
	CErrorTypeWeight * etWeight = new CErrorTypeWeight(CLayoutObjectEvaluationError::TYPE_SPLIT);

	vector<int> * regionTypes = GetRegionTypes();
	if (regionTypes != NULL)
	{
		for (unsigned int i=0; i<regionTypes->size(); i++)
		{
			CLayoutObjectTypeWeight * rtWeight = CreateRegionTypeWeight(regionTypes->at(i), true, true, false, false);
			etWeight->AddRegionTypeWeight(regionTypes->at(i), rtWeight);
		}
	}
	return etWeight;
}

/*
 * Initializes the weight-subtree for error type MISS.
 */
CErrorTypeWeight * CEvaluationProfile::CreateMissWeight()
{
	CErrorTypeWeight * etWeight = new CErrorTypeWeight(CLayoutObjectEvaluationError::TYPE_MISS);

	vector<int> * regionTypes = GetRegionTypes();
	if (regionTypes != NULL)
	{
		for (unsigned int i=0; i<regionTypes->size(); i++)
		{
			CLayoutObjectTypeWeight * rtWeight = CreateRegionTypeWeight(regionTypes->at(i), false, true, false, false);
			etWeight->AddRegionTypeWeight(regionTypes->at(i), rtWeight);
		}
	}
	return etWeight;
}

/*
 * Initializes the weight-subtree for error type PART_MISS.
 */
CErrorTypeWeight * CEvaluationProfile::CreatePartMissWeight()
{
	CErrorTypeWeight * etWeight = new CErrorTypeWeight(CLayoutObjectEvaluationError::TYPE_PART_MISS);

	vector<int> * regionTypes = GetRegionTypes();
	if (regionTypes != NULL)
	{
		for (unsigned int i=0; i<regionTypes->size(); i++)
		{
			CLayoutObjectTypeWeight * rtWeight = CreateRegionTypeWeight(regionTypes->at(i), false, true, false, false);
			etWeight->AddRegionTypeWeight(regionTypes->at(i), rtWeight);
		}
	}
	return etWeight;
}

/*
 * Initializes the weight-subtree for error type INVENTED.
 */
CErrorTypeWeight * CEvaluationProfile::CreateInventWeight()
{
	CErrorTypeWeight * etWeight = new CErrorTypeWeight(CLayoutObjectEvaluationError::TYPE_INVENT);

	vector<int> * regionTypes = GetRegionTypes();
	if (regionTypes != NULL)
	{
		for (unsigned int i=0; i<regionTypes->size(); i++)
		{
			CLayoutObjectTypeWeight * rtWeight = CreateRegionTypeWeight(regionTypes->at(i), false, true, false, false);
			etWeight->AddRegionTypeWeight(regionTypes->at(i), rtWeight);
		}
	}
	return etWeight;
}

/*
 * Weights by region type
 */
void CEvaluationProfile::CreateRegionTypeWeights()
{
	vector<int> * regionTypes = GetRegionTypes();
	for (unsigned int i=0; i<regionTypes->size(); i++)
	{
		CLayoutObjectTypeWeight * rtWeight = CreateRegionTypeWeight(	regionTypes->at(i), 
																false,
																true,
																false, false);
		m_RegionTypeWeights.insert(pair<int, CLayoutObjectTypeWeight *>(regionTypes->at(i), 
															rtWeight));
	}
}

/*
 * Creates a weight object for the given region subtype.
 *
 * 'appendRegionType2Weights' - should region type weights be appended as children
 * 'appendSubType2Weights' - should the region type children have subtype weights
 */
CSubTypeWeight * CEvaluationProfile::CreateSubTypeWeight(CUniString subtype, bool enableAllowable, 
														 bool appendRegionType2Weights,
														 bool appendSubType2Weights)
{
	CSubTypeWeight * stWeight = new CSubTypeWeight(subtype, enableAllowable, appendRegionType2Weights);
	
	if (appendRegionType2Weights)
	{
		vector<int> * regionTypes = GetRegionTypes();
		if (regionTypes != NULL)
		{
			for (unsigned int i=0; i<regionTypes->size(); i++)
			{
				CLayoutObjectTypeWeight * rtWeight = CreateRegionTypeWeight(	regionTypes->at(i), 
																		enableAllowable,
																		appendSubType2Weights, //is here subType1...
																		false, false);
				stWeight->AddRegionTypeWeight(regionTypes->at(i), rtWeight);
			}
		}
	}
	return stWeight;
}

/*
 * Creates a weight object for the given region type.
 *
 * 'appendSubTypeWeights' - should region subtype weights be appended as children
 * 'appendRegionType2Weights' - should the region subtype children have type weights
 * 'appendSubType2Weights' - should the region type children have subtype weights
 */
CLayoutObjectTypeWeight * CEvaluationProfile::CreateRegionTypeWeight(int regionType, bool enableAllowable, 
															   bool appendSubTypeWeights,
															   bool appendRegionType2Weights,
															   bool appendSubType2Weights)
{
	CLayoutObjectTypeWeight * rtWeight = NULL;
	
	if (appendSubTypeWeights)
	{
		vector<CUniString> * subTypes = GetSubTypes(regionType);
		if (subTypes != NULL)
		{
			rtWeight = new CLayoutObjectTypeWeight(regionType, enableAllowable, appendSubTypeWeights);
			for (unsigned int i=0; i<subTypes->size(); i++)
			{
				CSubTypeWeight * stWeight = CreateSubTypeWeight(subTypes->at(i), 
																enableAllowable,
																appendRegionType2Weights,
																appendSubType2Weights);
				rtWeight->AddSubTypeWeight(subTypes->at(i), stWeight);
			}
			//Add shortcuts to region type 2
			if (appendRegionType2Weights)
			{
				vector<int> * regionTypes = GetRegionTypes();
				for (unsigned int i=0; i<regionTypes->size(); i++)
				{
					CLayoutObjectTypeWeight * rtWeight2 = CreateRegionType2ShortCutWeight(regionTypes->at(i), 
																			enableAllowable,
																			appendSubType2Weights, //is here subType1...
																			rtWeight);
					rtWeight->AddRegionTypeShortcutWeight(regionTypes->at(i), rtWeight2);
				}
			}
			delete subTypes;
		}
		else //There are no sub types for this region type -> add region type 2
		{
			rtWeight = new CLayoutObjectTypeWeight(regionType, enableAllowable, appendRegionType2Weights);
			if (appendRegionType2Weights)
			{
				vector<int> * regionTypes = GetRegionTypes();
				for (unsigned int i=0; i<regionTypes->size(); i++)
				{
					CLayoutObjectTypeWeight * rtWeight2 = CreateRegionTypeWeight(	regionTypes->at(i), 
																			enableAllowable,
																			appendSubType2Weights, //is here subType1...
																			false, false);
					rtWeight->AddRegionTypeWeight(regionTypes->at(i), rtWeight2);
				}
			}
		}
	}
	else
		rtWeight = new CLayoutObjectTypeWeight(regionType, enableAllowable, false);
	return rtWeight;
}

/*
 * Shortcut from region type to region type avoiding the subtypes.
 */
CLayoutObjectTypeWeight * CEvaluationProfile::CreateRegionType2ShortCutWeight(int regionType, bool enableAllowable, 
																		bool appendSubTypeWeights,
																		CLayoutObjectTypeWeight * parentWeight)
{
	CLayoutObjectTypeWeight * rtWeight = new CLayoutObjectTypeWeight(regionType, enableAllowable, true);	
	map<CUniString, CSubTypeWeight*> * parentSubTypes = parentWeight->GetSubTypeWeights();
	map<CUniString, CSubTypeWeight*>::iterator it = parentSubTypes->begin();
	while (it != parentSubTypes->end())
	{
		CSubTypeWeight * subtTypeWeight = (*it).second;
		CLayoutObjectTypeWeight * rt2 = subtTypeWeight->GetRegionTypeWeight(regionType);
		if (rt2 != NULL)
			rtWeight->AddRegionTypeShortcutParam(rt2->GetParam(), rt2->GetAllowableParam(), rt2->GetUseAllowableParam());
		it++;
	}
	return rtWeight;
}

/*
 * Returns a list with all region type IDs (text, graphic, ...).
 */
vector<int> * CEvaluationProfile::GetRegionTypes()
{
	if (m_RegionTypes == NULL)
	{
		m_RegionTypes = new vector<int>();
		m_RegionTypes->push_back(CLayoutRegion::TYPE_TEXT);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_IMAGE);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_LINEART);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_GRAPHIC);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_TABLE);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_CHART);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_SEPARATOR);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_MATHS);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_NOISE);
		//m_RegionTypes->push_back(CLayoutRegion::TYPE_FRAME);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_ADVERT);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_CHEM);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_MUSIC);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_MAP);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_UNKNOWN);
		m_RegionTypes->push_back(CLayoutRegion::TYPE_CUSTOM);
	}
	return m_RegionTypes;
}

/*
 * Returns a list of all subtypes of the given region type.
 */
vector<CUniString> * CEvaluationProfile::GetSubTypes(int regionType)
{
	//TODO read from schema (and store in temporary map)
	vector<CUniString> * types = NULL;
	if (	regionType == CLayoutRegion::TYPE_TEXT 
		||	regionType == CLayoutRegion::TYPE_GRAPHIC
		||	regionType == CLayoutRegion::TYPE_CHART)
	{
		types = new vector<CUniString>();
		map<int, CUniString> * valueMap = NULL;
		if (m_PageLayoutXmlValidator != NULL)
		{
			CUniString schemaType;
			if (regionType == CLayoutRegion::TYPE_TEXT)
				schemaType = m_PageLayoutXmlValidator->GetType(_T("TextRegionType"), _T("type"));
			else if (regionType == CLayoutRegion::TYPE_GRAPHIC)
				schemaType = m_PageLayoutXmlValidator->GetType(_T("GraphicRegionType"), _T("type"));
			else if (regionType == CLayoutRegion::TYPE_CHART)
				schemaType = m_PageLayoutXmlValidator->GetType(_T("ChartRegionType"), _T("type"));
			valueMap = m_PageLayoutXmlValidator->GetSimpleTypeValuesByIndex(schemaType);
		}
		if (valueMap != NULL)
		{
			//Add <undefined>
			types->push_back(STRING_SUBTYPE_UNDEFINED);

			//Add the rest
			for (map<int, CUniString>::iterator it = valueMap->begin(); it != valueMap->end(); it++)
			{
				types->push_back((*it).second);
			}
		}
		else //Failsafe -> Use fixed subtypes
		{
			if (regionType == CLayoutRegion::TYPE_TEXT)
				types->push_back(CUniString(_T("paragraph")));
			else if (regionType == CLayoutRegion::TYPE_GRAPHIC)
				types->push_back(CUniString(_T("other")));
			else if (regionType == CLayoutRegion::TYPE_CHART)
				types->push_back(CUniString(_T("other")));
		}
		//types->push_back(CUniString(_T("paragraph")));
		//types->push_back(CUniString(_T("heading")));
		//types->push_back(CUniString(_T("caption")));
		//types->push_back(CUniString(_T("header")));
		//types->push_back(CUniString(_T("footer")));
		//types->push_back(CUniString(_T("page-number")));
		//types->push_back(CUniString(_T("drop-capital")));
		//types->push_back(CUniString(_T("credit")));
		//types->push_back(CUniString(_T("floating")));
		//types->push_back(CUniString(_T("signature-mark")));
		//types->push_back(CUniString(_T("catch-word")));
		//types->push_back(CUniString(_T("marginalia")));
		//types->push_back(CUniString(_T("footnote")));
		//types->push_back(CUniString(_T("footnote-continued")));
		//types->push_back(CUniString(_T("TOC-entry")));
	}
	return types;
}

double CEvaluationProfile::GetErrorTypeWeight(int errorType, bool allowable /*= false*/)
{
	CWeight * weight = GetWeightObject(errorType);
	if (weight != NULL)
		return allowable ? weight->GetAllowableValue() : weight->GetValue();
	return -1;
}

double CEvaluationProfile::GetErrorTypeWeight(int errorType, int regionType, bool allowable/*= false*/)
{
	CWeight * weight = GetWeightObject(errorType, regionType);
	if (weight != NULL)
		return allowable ? weight->GetAllowableValue() : weight->GetValue();
	return -1;
}

double CEvaluationProfile::GetErrorTypeWeight(int errorType, int regionType, CUniString subType, 
											  bool allowable /*= false*/)
{
	CWeight * weight = GetWeightObject(errorType, regionType, subType);
	if (weight != NULL)
		return allowable ? weight->GetAllowableValue() : weight->GetValue();
	return -1;
}

double CEvaluationProfile::GetErrorTypeWeight(int errorType, int regionType, int regionType2, 
											  bool allowable /*= false*/)
{
	CWeight * weight = GetWeightObject(errorType, regionType, regionType2);
	if (weight != NULL)
		return allowable ? weight->GetAllowableValue() : weight->GetValue();
	return -1;
}

double CEvaluationProfile::GetErrorTypeWeight(int errorType, int regionType, CUniString subType, 
									 int regionType2, bool allowable /*= false*/)
{
	CWeight * weight = GetWeightObject(errorType, regionType, subType, regionType2);
	if (weight != NULL)
		return allowable ? weight->GetAllowableValue() : weight->GetValue();
	return -1;
}

double CEvaluationProfile::GetErrorTypeWeight(int errorType, int regionType, int regionType2, 
											  CUniString subType2, bool allowable /*= false*/)
{
	CWeight * weight = GetWeightObject(errorType, regionType, regionType2, subType2);
	if (weight != NULL)
		return allowable ? weight->GetAllowableValue() : weight->GetValue();
	return -1;
}

double CEvaluationProfile::GetErrorTypeWeight(int errorType, int regionType, CUniString subType, 
									 int regionType2, CUniString subType2, bool allowable /*= false*/)
{
	CWeight * weight = GetWeightObject(errorType, regionType, subType, regionType2, subType2);
	if (weight != NULL)
		return allowable ? weight->GetAllowableValue() : weight->GetValue();
	return -1;
}

double CEvaluationProfile::GetRegionTypeWeight(int regionType)
{
	CWeight * weight = GetRegionTypeWeightObject(regionType);
	if (weight != NULL)
		return weight->GetValue();
	return -1;
}

double CEvaluationProfile::GetRegionTypeWeight(int regionType, CUniString subType)
{
	CWeight * weight = GetRTWeightObject(regionType, subType);
	if (weight != NULL)
		return weight->GetValue();
	return -1;
}

double CEvaluationProfile::GetReadingOrderWeight()
{
	return m_ReadingOrderWeight != NULL ? m_ReadingOrderWeight->GetValue() : 1.0;
}

void CEvaluationProfile::SetReadingOrderWeight(CReadingOrderWeight * weight) 
{ 
	delete m_ReadingOrderWeight; 
	m_ReadingOrderWeight = weight; 
}

double CEvaluationProfile::GetErrorTypeWeightForTextSubStructure(int errorType, 
																 int regionLevel)
{
	CWeight * weightObj = GetErrorTypeWeightObjectForTextSubStructure(errorType, regionLevel);
	if (weightObj != NULL)
		return weightObj->GetValue();
	return -1.0;
}

double CEvaluationProfile::GetErrorTypeWeightForReadingOrderGroup(int errorType)
{
	CWeight * weightObj = GetErrorTypeWeightObjectForReadingOrderGroup(errorType);
	if (weightObj != NULL)
		return weightObj->GetValue();
	return -1.0;
}

CParameter * CEvaluationProfile::GetErrorTypeWeightParamForReadingOrderGroup(int errorType)
{
	CWeight * weightObj = GetErrorTypeWeightObjectForReadingOrderGroup(errorType);
	if (weightObj != NULL)
		return weightObj->GetParam();
	return NULL;
}

CWeight * CEvaluationProfile::GetErrorTypeWeightObjectForReadingOrderGroup(int errorType)
{
	map<int, CWeight*>::iterator it2 = m_ReadingOrderGroupWeights.find(errorType);
	if (it2 != m_ReadingOrderGroupWeights.end()) //found
	{
		return (*it2).second;
	}
	return NULL;
}

CParameter * CEvaluationProfile::GetErrorTypeWeightParamForTextSubStructure(int errorType, 
																			int regionLevel)
{
	CWeight * weightObj = GetErrorTypeWeightObjectForTextSubStructure(errorType, regionLevel);
	if (weightObj != NULL)
		return weightObj->GetParam();
	return NULL;
}

CWeight * CEvaluationProfile::GetErrorTypeWeightObjectForTextSubStructure(int errorType, 
																		  int regionLevel)
{
	map<int, map<int, CWeight*>*>::iterator it = m_TextSubStructureWeights.find(regionLevel);
	if (it != m_TextSubStructureWeights.end()) //found
	{
		map<int, CWeight*> * errTypeMap = (*it).second;
		map<int, CWeight*>::iterator it2 = errTypeMap->find(errorType);
		if (it2 != errTypeMap->end()) //found
		{
			return (*it2).second;
		}
	}	
	return NULL;
}

CParameter * CEvaluationProfile::GetWeightParam(int errorType)
{
	CWeight * weight = GetWeightObject(errorType);
	if (weight != NULL)
		return weight->GetParam();
	return NULL;
}

CParameter * CEvaluationProfile::GetWeightParam(int errorType, int regionType)
{
	CWeight * weight = GetWeightObject(errorType, regionType);
	if (weight != NULL)
		return weight->GetParam();
	return NULL;
}

CParameter * CEvaluationProfile::GetWeightParam(int errorType, int regionType, CUniString subType)
{
	CWeight * weight = GetWeightObject(errorType, regionType, subType);
	if (weight != NULL)
		return weight->GetParam();
	return NULL;
}

CParameter * CEvaluationProfile::GetWeightParam(int errorType, int regionType, CUniString subType, 
												int regionType2)
{
	CWeight * weight = GetWeightObject(errorType, regionType, subType, regionType2);
	if (weight != NULL)
		return weight->GetParam();
	return NULL;
}

CParameter * CEvaluationProfile::GetWeightParam(int errorType, int regionType, CUniString subType, 
												int regionType2, CUniString subType2)
{
	CWeight * weight = GetWeightObject(errorType, regionType, subType, regionType2, subType2);
	if (weight != NULL)
		return weight->GetParam();
	return NULL;
}

CParameter * CEvaluationProfile::GetRegionTypeWeightParam(int regionType)
{
	CWeight * weight = GetRTWeightObject(regionType);
	if (weight != NULL)
		return weight->GetParam();
	return NULL;
}

CParameter * CEvaluationProfile::GetRegionTypeWeightParam(int regionType, CUniString subType)
{
	CWeight * weight = GetRTWeightObject(regionType, subType);
	if (weight != NULL)
		return weight->GetParam();
	return NULL;
}

CParameter * CEvaluationProfile::GetReadingOrderWeightParam() 
{ 
	return m_ReadingOrderWeight != NULL ? m_ReadingOrderWeight->GetParam() : NULL; 
}

CWeight * CEvaluationProfile::GetReadingOrderWeightObject()
{
	return m_ReadingOrderWeight;
}

CParameter * CEvaluationProfile::GetIncludedBackgroundBorderWeightParam()
{
	return m_IncludedBackgroundBorderWeight != NULL ? m_IncludedBackgroundBorderWeight->GetParam() : NULL; 
}

CParameter * CEvaluationProfile::GetExcludedForegroundBorderWeightParam()
{
	return m_ExcludedForegroundBorderWeight != NULL ? m_ExcludedForegroundBorderWeight->GetParam() : NULL; 
}

CWeight * CEvaluationProfile::GetIncludedBackgroundBorderWeightObject()
{
	return m_IncludedBackgroundBorderWeight; 
}

CWeight * CEvaluationProfile::GetExcludedForegroundBorderWeightObject()
{
	return m_ExcludedForegroundBorderWeight; 
}

CParameter * CEvaluationProfile::GetMissingRegionBorderWeightParam(int regionType)
{
	CWeight * weight = GetMissingRegionBorderWeightObject(regionType);
	if (weight != NULL)
		return weight->GetParam();
	return NULL;
}

CParameter * CEvaluationProfile::GetMissingRegionBorderWeightParam(int regionType, CUniString subType)
{
	CWeight * weight = GetMissingRegionBorderWeightObject(regionType, subType);
	if (weight != NULL)
		return weight->GetParam();
	return NULL;
}

CLayoutObjectTypeWeight * CEvaluationProfile::GetMissingRegionBorderWeightObject(int regionType)
{
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_MissingRegionAreaBorderWeights.find(regionType);
	if (it != m_MissingRegionAreaBorderWeights.end())
		return (*it).second;
	return NULL;
}

CSubTypeWeight * CEvaluationProfile::GetMissingRegionBorderWeightObject(int regionType, CUniString subType)
{
	CLayoutObjectTypeWeight * regionTypeWeight = GetRegionTypeWeightObject(regionType);
	if (regionTypeWeight != NULL)
		return regionTypeWeight->GetSubTypeWeight(subType);
	return NULL;
}

CErrorTypeWeight * CEvaluationProfile::GetErrorTypeWeightObject(int errorType)
{
	map<int, CErrorTypeWeight*>::iterator it = m_ErrorTypeWeights.find(errorType);
	if (it != m_ErrorTypeWeights.end())
		return (*it).second;
	return NULL;
}

CLayoutObjectTypeWeight * CEvaluationProfile::GetRegionTypeWeightObject(int regionType)
{
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_RegionTypeWeights.find(regionType);
	if (it != m_RegionTypeWeights.end())
		return (*it).second;
	return NULL;
}

CWeight * CEvaluationProfile::GetWeightObject(int errorType)
{
	return (CWeight*)GetErrorTypeWeightObject(errorType);
}

CLayoutObjectTypeWeight * CEvaluationProfile::GetRegionTypeWeightObject(int errorType, int regionType)
{
	CErrorTypeWeight * errTypeWeight = GetErrorTypeWeightObject(errorType);
	if (errTypeWeight != NULL)
		return errTypeWeight->GetRegionTypeWeight(regionType);
	return NULL;
}

CWeight * CEvaluationProfile::GetWeightObject(int errorType, int regionType)
{
	return (CWeight*)GetRegionTypeWeightObject(errorType, regionType);
}

CSubTypeWeight * CEvaluationProfile::GetSubTypeWeightObject(int errorType, int regionType, 
													  CUniString subType)
{
	CLayoutObjectTypeWeight * regionTypeWeight = GetRegionTypeWeightObject(errorType, regionType);
	if (regionTypeWeight != NULL)
		return regionTypeWeight->GetSubTypeWeight(subType);
	return NULL;
}

CWeight * CEvaluationProfile::GetWeightObject(int errorType, int regionType, CUniString subType)
{
	return (CWeight*)GetSubTypeWeightObject(errorType, regionType, subType);
}

CLayoutObjectTypeWeight * CEvaluationProfile::GetRegionTypeWeightObject(int errorType, int regionType, 
																  int regionType2)
{
	CLayoutObjectTypeWeight * rtWeight = GetRegionTypeWeightObject(errorType, regionType);
	if (rtWeight != NULL)
		return rtWeight->GetRegionTypeWeight(regionType2);
	return NULL;
}

CLayoutObjectTypeWeight * CEvaluationProfile::GetRegionTypeWeightObject(int errorType, int regionType, 
															 CUniString subType, int regionType2)
{
	CSubTypeWeight * subTypeWeight = GetSubTypeWeightObject(errorType, regionType, subType);
	if (subTypeWeight != NULL)
		return subTypeWeight->GetRegionTypeWeight(regionType2);
	return NULL;
}

CWeight * CEvaluationProfile::GetWeightObject(int errorType, int regionType, int regionType2)
{
	return (CWeight*)GetRegionTypeWeightObject(errorType, regionType, regionType2);
}

CWeight * CEvaluationProfile::GetWeightObject(int errorType, int regionType, CUniString subType, 
											  int regionType2)
{
	return (CWeight*)GetRegionTypeWeightObject(errorType, regionType, subType, regionType2);
}

CSubTypeWeight * CEvaluationProfile::GetSubTypeWeightObject(int errorType, int regionType, 
															int regionType2, CUniString subType2)
{
	CLayoutObjectTypeWeight * regionTypeWeight = GetRegionTypeWeightObject(errorType, regionType,  
																regionType2);
	if (regionTypeWeight != NULL)
		return regionTypeWeight->GetSubTypeWeight(subType2);
	return NULL;
}

CSubTypeWeight * CEvaluationProfile::GetSubTypeWeightObject	(int errorType, int regionType, 
														 CUniString subType, int regionType2, 
														 CUniString subType2)
{
	CLayoutObjectTypeWeight * regionTypeWeight = GetRegionTypeWeightObject(errorType, regionType, subType, 
																regionType2);
	if (regionTypeWeight != NULL)
		return regionTypeWeight->GetSubTypeWeight(subType2);
	return NULL;
}

CWeight * CEvaluationProfile::GetWeightObject(int errorType, int regionType, int regionType2, CUniString subType2)
{
	return (CWeight*)GetSubTypeWeightObject(errorType, regionType, regionType2, subType2);
}

CWeight * CEvaluationProfile::GetWeightObject(int errorType, int regionType, CUniString subType, 
											  int regionType2, CUniString subType2)
{
	return (CWeight*)GetSubTypeWeightObject(errorType, regionType, subType, regionType2, subType2);
}

CWeight * CEvaluationProfile::GetRTWeightObject(int regionType)
{
	return (CWeight*)GetRegionTypeWeightObject(regionType);
}

CWeight * CEvaluationProfile::GetRTWeightObject(int regionType, CUniString subType)
{
	return (CWeight*)GetSubTypeWeightObject(regionType, subType);
}

CSubTypeWeight * CEvaluationProfile::GetSubTypeWeightObject	(int regionType, CUniString subType)
{
	CLayoutObjectTypeWeight * regionTypeWeight = GetRegionTypeWeightObject(regionType);
	if (regionTypeWeight != NULL)
		return regionTypeWeight->GetSubTypeWeight(subType);
	return NULL;
}

void CEvaluationProfile::AddErrorTypeWeight(CErrorTypeWeight * weight)
{
	map<int, CErrorTypeWeight*>::iterator it = m_ErrorTypeWeights.find(weight->GetType());
	if (it != m_ErrorTypeWeights.end()) //already there
	{
		delete (*it).second;
		m_ErrorTypeWeights.erase(it);
	}
	m_ErrorTypeWeights.insert(pair<int, CErrorTypeWeight*>(weight->GetType(), weight));
}

void CEvaluationProfile::AddRegionTypeWeight(CLayoutObjectTypeWeight * weight)
{
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_RegionTypeWeights.find(weight->GetType());
	if (it != m_RegionTypeWeights.end()) //already there
	{
		delete (*it).second;
		m_RegionTypeWeights.erase(it);
	}
	m_RegionTypeWeights.insert(pair<int, CLayoutObjectTypeWeight*>(weight->GetType(), weight));
}


/*
 * Class CWeight
 *
 * Holds one single weight parameter. The parameter can be a MultiParameter
 * that is parent of other weight parameters.
 *
 * CC 09.02.2010 - created
 */
CWeight::CWeight(bool enableAllowableWeight /*= false */)
{
	m_WeightParam = NULL;
	m_AllowableWeight = NULL;
	m_UseAllowableWeight = NULL;
	m_EnableAllowableWeight = enableAllowableWeight;
}

CWeight::~CWeight()
{
	delete m_WeightParam;
	delete m_AllowableWeight;
	delete m_UseAllowableWeight;
}

CUniString CWeight::GetName()
{
	if (m_WeightParam != NULL)
		return m_WeightParam->GetName();
	return CUniString();
}

CUniString CWeight::GetDescription()
{
	if (m_WeightParam != NULL)
		return m_WeightParam->GetDescription();
	return CUniString();
}

void CWeight::SetValue(double val) 
{ 
	if (m_WeightParam == NULL)
		return;
	if (m_WeightParam->GetType() == CParameter::TYPE_DOUBLE) 
		((CDoubleParameter*)m_WeightParam)->SetValue(val);
	else if (m_WeightParam->GetType() == CParameter::TYPE_MULTI) 
		((CMultiParameter*)m_WeightParam)->SetValue(val);
}

void CWeight::SetAllowableValue(double val) 
{ 
	if (m_AllowableWeight == NULL)
		return;
	if (m_AllowableWeight->GetType() == CParameter::TYPE_DOUBLE) 
		((CDoubleParameter*)m_AllowableWeight)->SetValue(val);
	else if (m_AllowableWeight->GetType() == CParameter::TYPE_MULTI) 
		((CMultiParameter*)m_AllowableWeight)->SetValue(val);
}

/*
 * Returns the weigth value.
 * If there is no value, -1 is returned.
 */
double CWeight::GetValue()
{
	if (m_WeightParam == NULL)
		return -1;

	//Multi param
	if (m_WeightParam->GetType() == CParameter::TYPE_MULTI)
	{
		CParameter * value = ((CMultiParameter*)m_WeightParam)->GetValue();
		if (value != NULL && value->GetType() == CParameter::TYPE_DOUBLE)
			return ((CDoubleParameter*)value)->GetValue();
		return -1;
	}
	//Double param
	else if (m_WeightParam->GetType() == CParameter::TYPE_DOUBLE)
	{
		return ((CDoubleParameter*)m_WeightParam)->GetValue();
	}
	return -1;
}

double CWeight::GetAverage()
{
	if (m_WeightParam == NULL || m_WeightParam->GetType() == CParameter::TYPE_MULTI)
		return 0;
	//Double param
	else if (m_WeightParam->GetType() == CParameter::TYPE_DOUBLE)
		return ((CDoubleParameter*)m_WeightParam)->GetValue();
	return 0;
}

double CWeight::GetAverage(int & count)
{
	if (m_WeightParam == NULL || m_WeightParam->GetType() == CParameter::TYPE_MULTI)
		return 0;
	//Double param
	else if (m_WeightParam->GetType() == CParameter::TYPE_DOUBLE)
	{
		count++;
		return ((CDoubleParameter*)m_WeightParam)->GetValue();
	}
	return 0;
}

/*
 * Returns the weigth value for allowable errors (merge, split).
 * If there is no value, -1 is returned.
 */
double CWeight::GetAllowableValue()
{
	if (m_AllowableWeight == NULL)
		return -1;

	//If the allowable weight is not being used, the value of the normal weight is returned 
	if (m_EnableAllowableWeight && !IsUseAllowableWeight())
		return GetValue();

	//Multi param
	if (m_AllowableWeight->GetType() == CParameter::TYPE_MULTI)
	{
		CParameter * value = ((CMultiParameter*)m_AllowableWeight)->GetValue();
		if (value != NULL && value->GetType() == CParameter::TYPE_DOUBLE)
			return ((CDoubleParameter*)value)->GetValue();
		return -1;
	}
	//Double param
	else if (m_AllowableWeight->GetType() == CParameter::TYPE_DOUBLE)
	{
		return ((CDoubleParameter*)m_AllowableWeight)->GetValue();
	}
	return -1;
}

/*
 * Returns true, if the allowable weight is being used, otherwise false.
 */
bool CWeight::IsUseAllowableWeight()
{
	if (!m_EnableAllowableWeight)
		return false;

	//Multi param
	if (m_UseAllowableWeight->GetType() == CParameter::TYPE_MULTI)
	{
		CParameter * value = ((CMultiParameter*)m_UseAllowableWeight)->GetValue();
		if (value != NULL && value->GetType() == CParameter::TYPE_BOOL)
			return ((CBoolParameter*)value)->GetValue();
		return false;
	}
	//Bool param
	else if (m_UseAllowableWeight->GetType() == CParameter::TYPE_BOOL)
	{
		return ((CBoolParameter*)m_UseAllowableWeight)->GetValue();
	}
	return false;
}

void CWeight::SetUseAllowableWeight(bool use)		
{ 
	CBoolParameter param("","",use);
	if (m_UseAllowableWeight != NULL) 
		m_UseAllowableWeight->SetValue(&param); 
}

/*
 * Prints the weight (and its children to a string)
 * 'printOnlyNonDefaultValues' - If true, only weight values that are not
 *                               the default (1.0) are printed.
 */
CUniString CWeight::ToString(bool printOnlyNonDefaultValues /*= true*/)
{
	return ToString(CUniString(_T("    ")), printOnlyNonDefaultValues);
}

/*
 * Prints the weight (and its children to a string)
 * 'printOnlyNonDefaultValues' - If true, only weight values that are not
 *                               the default (1.0) are printed.
 */
CUniString CWeight::ToString(CUniString prefix, bool printOnlyNonDefaultValues)
{
	CUniString ret;

	double weight = GetValue();
	double allowableWeight = GetAllowableValue();

	if (!printOnlyNonDefaultValues || (weight != 1.0 || (allowableWeight != 1.0 && m_EnableAllowableWeight)))
	{
		ret.Append(prefix);
		ret.Append(GetName());
		ret.Append(_T(":   "));

		if (weight >= 0.0 && (!printOnlyNonDefaultValues || weight != 1.0))
			ret.Append(weight, 2);

		if (m_EnableAllowableWeight)
		{
			if (allowableWeight >= 0.0 && (!printOnlyNonDefaultValues || allowableWeight != 1.0))
			{
				ret.Append(_T("   allowable:"));
				ret.Append(allowableWeight, 2);
			}
		}

		ret.Append(_T("\n"));

		//Children
		if (weight < 0 || (allowableWeight < 0 && m_EnableAllowableWeight)) //only if the children differ
		{
			prefix.Append(_T("    "));
			if (typeid(*this) == typeid(CErrorTypeWeight))
			{
				CErrorTypeWeight * w = (CErrorTypeWeight*)this;
				map<int, CLayoutObjectTypeWeight*> * children = w->GetRegionTypeWeights();
				map<int, CLayoutObjectTypeWeight*>::iterator it = children->begin();
				while (it != children->end())
				{
					ret.Append((*it).second->ToString(prefix, printOnlyNonDefaultValues));
					it++;
				}
			}
			else if (typeid(*this) == typeid(CSubTypeWeight))
			{
				CSubTypeWeight * w = (CSubTypeWeight*)this;
				map<int, CLayoutObjectTypeWeight*> * children = w->GetRegionTypeWeights();
				map<int, CLayoutObjectTypeWeight*>::iterator it = children->begin();
				while (it != children->end())
				{
					ret.Append((*it).second->ToString(prefix, printOnlyNonDefaultValues));
					it++;
				}
			}
			else if (typeid(*this) == typeid(CLayoutObjectTypeWeight))
			{
				CLayoutObjectTypeWeight * w = (CLayoutObjectTypeWeight*)this;
				map<int, CLayoutObjectTypeWeight*> * children = w->GetRegionTypeWeights();
				map<int, CLayoutObjectTypeWeight*>::iterator it = children->begin();
				while (it != children->end())
				{
					ret.Append((*it).second->ToString(prefix, printOnlyNonDefaultValues));
					it++;
				}

				map<CUniString, CSubTypeWeight*> * children2 = w->GetSubTypeWeights();
				map<CUniString, CSubTypeWeight*>::iterator it2 = children2->begin();
				while (it2 != children2->end())
				{
					ret.Append((*it2).second->ToString(prefix, printOnlyNonDefaultValues));
					it2++;
				}
			}
		}
	}

	return ret;
}

void CWeight::GetImportantWeights(vector<pair<int,CWeight*>> * weights, int level, bool useOnlyNonDefaultValues, 
								  bool includeAllowable)
{
	double weight = GetValue();
	double allowableWeight = GetAllowableValue();
	level++;

	if (!useOnlyNonDefaultValues || (weight != 1.0 || (allowableWeight != 1.0 && m_EnableAllowableWeight)))
	{
		weights->push_back(pair<int,CWeight*>(level, this));
		
		//Exclude allowable weights for non-text weights
		if (typeid(*this) == typeid(CLayoutObjectTypeWeight) && ((CLayoutObjectTypeWeight*)this)->GetType() != CLayoutRegion::TYPE_TEXT)
			includeAllowable = false;

		if (includeAllowable && m_EnableAllowableWeight && allowableWeight != 1.0)
			weights->push_back(pair<int,CWeight*>(-level, this));

		//Children
		if (weight < 0 || (allowableWeight < 0 && m_EnableAllowableWeight)) //only if the children differ
		{
			if (typeid(*this) == typeid(CErrorTypeWeight))
			{
				CErrorTypeWeight * w = (CErrorTypeWeight*)this;
				map<int, CLayoutObjectTypeWeight*> * children = w->GetRegionTypeWeights();
				map<int, CLayoutObjectTypeWeight*>::iterator it = children->begin();
				while (it != children->end())
				{
					(*it).second->GetImportantWeights(weights, level, useOnlyNonDefaultValues, includeAllowable);
					it++;
				}
			}
			else if (typeid(*this) == typeid(CSubTypeWeight))
			{
				CSubTypeWeight * w = (CSubTypeWeight*)this;
				map<int, CLayoutObjectTypeWeight*> * children = w->GetRegionTypeWeights();
				map<int, CLayoutObjectTypeWeight*>::iterator it = children->begin();
				while (it != children->end())
				{
					(*it).second->GetImportantWeights(weights, level, useOnlyNonDefaultValues, includeAllowable);
					it++;
				}
			}
			else if (typeid(*this) == typeid(CLayoutObjectTypeWeight))
			{
				CLayoutObjectTypeWeight * w = (CLayoutObjectTypeWeight*)this;
				if (w->GetSubTypeWeights()->empty())
				{
					map<int, CLayoutObjectTypeWeight*> * children = w->GetRegionTypeWeights();
					map<int, CLayoutObjectTypeWeight*>::iterator it = children->begin();
					while (it != children->end())
					{
						(*it).second->GetImportantWeights(weights, level, useOnlyNonDefaultValues, includeAllowable);
						it++;
					}
				}

				map<CUniString, CSubTypeWeight*> * children2 = w->GetSubTypeWeights();
				map<CUniString, CSubTypeWeight*>::iterator it2 = children2->begin();
				while (it2 != children2->end())
				{
					(*it2).second->GetImportantWeights(weights, level, useOnlyNonDefaultValues, includeAllowable);
					it2++;
				}
			}
		}
	}
}


/*
 * Class CTextSubStructureWeight
 *
 * Weight for text line, word or glyph per error type
 *
 * CC 06.08.2010 - created
 */

/*
 * Constructor
 * 'errorType' - merge, split, ... (see CLayoutObjectEvaluationError::TYPE_...)
 */
CTextSubStructureWeight::CTextSubStructureWeight(int errorType)
{
	CUniString name = CLayoutObjectEvaluationError::GetTypeName(errorType);

	m_WeightParam = (CParameter*) new CDoubleParameter(PARAM_NORMAL_WEIGHT, name.ToC_Str(), "", 
														CEvaluationProfile::WEIGHT_INIT, 
														CEvaluationProfile::WEIGHT_MIN, 
														CEvaluationProfile::WEIGHT_MAX);
}

/*
 * Destructor
 */
CTextSubStructureWeight::~CTextSubStructureWeight()
{
}


/*
 * Class CErrorTypeWeight
 *
 * Weight for an evaluation error type (e.g. MERGE, SPLIT, ...).
 * Can have region weights as children.
 *
 * CC 09.02.2010 - created
 */

CErrorTypeWeight::CErrorTypeWeight(int errorType, bool hasRegionTypes /*= true*/) : CWeight()
{
	this->m_Type = errorType;
	CUniString name = CLayoutObjectEvaluationError::GetTypeName(errorType);
	if (hasRegionTypes)
		m_WeightParam = (CParameter*) new CMultiParameter(	PARAM_NORMAL_WEIGHT,
															name.ToC_Str(), "");
	else
		m_WeightParam = (CParameter*) new CDoubleParameter(PARAM_NORMAL_WEIGHT, name.ToC_Str(), "", 
															CEvaluationProfile::WEIGHT_INIT, 
															CEvaluationProfile::WEIGHT_MIN, 
															CEvaluationProfile::WEIGHT_MAX);
	if (errorType == CLayoutObjectEvaluationError::TYPE_MERGE
		|| errorType == CLayoutObjectEvaluationError::TYPE_SPLIT)
	{
		EnableAllowableWeight(true);
		if (hasRegionTypes)
			m_AllowableWeight = (CParameter*) new CMultiParameter(	PARAM_ALLOWABLE_WEIGHT,
																	"Allowable", "");
		else
			m_AllowableWeight = (CParameter*) new CDoubleParameter(	PARAM_ALLOWABLE_WEIGHT,
																	"Allowable", "", 
																	CEvaluationProfile::WEIGHT_INIT, 
																	CEvaluationProfile::WEIGHT_MIN, 
																	CEvaluationProfile::WEIGHT_MAX);
		if (hasRegionTypes)
			m_UseAllowableWeight = (CParameter*) new CMultiParameter(	PARAM_USE_ALLOWABLE,
																		"Enable allowable", "");
		else
			m_UseAllowableWeight = (CParameter*) new CBoolParameter(PARAM_USE_ALLOWABLE,
																	"Enable allowable", "", true);
		m_UseAllowableWeight->SetSet(true);
	}
}

CErrorTypeWeight::~CErrorTypeWeight()
{
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_RegionTypeWeights.begin();
	while (it != m_RegionTypeWeights.end())
	{
		delete (*it).second;
		it++;
	}
}

double CErrorTypeWeight::GetAverage()
{
	int count = 0;
	double avg = GetAverage(count);
	return count > 0 ? avg / (double)count : 0;
}

double CErrorTypeWeight::GetAverage(int & count)
{
	//Children
	double val = 0.0;
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_RegionTypeWeights.begin();
	while (it != m_RegionTypeWeights.end())
	{
		val += (*it).second->GetAverage(count);
		it++;
	}
	return val;
}

CLayoutObjectTypeWeight * CErrorTypeWeight::GetRegionTypeWeight(int regionType)
{
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_RegionTypeWeights.find(regionType);
	if (it != m_RegionTypeWeights.end())
		return (*it).second;
	return NULL;
}

void CErrorTypeWeight::AddRegionTypeWeight(int regionType, CLayoutObjectTypeWeight * weight)
{
	m_RegionTypeWeights.insert(pair<int,CLayoutObjectTypeWeight*>(regionType, weight));

	//Add param as child for the multi param
	((CMultiParameter*)m_WeightParam)->AddChild(weight->GetParam());
	if (m_AllowableWeight != NULL)
	{
		((CMultiParameter*)m_AllowableWeight)->AddChild(weight->GetAllowableParam());
		((CMultiParameter*)m_UseAllowableWeight)->AddChild(weight->GetUseAllowableParam());
	}
}

/*
 * Class CLayoutObjectTypeWeight
 *
 * Weight for a layout region type (e.g. Text, Graphic, ...).
 * Can have region subtype weights or region weights as children.
 *
 * CC 09.02.2010 - created
 */
CLayoutObjectTypeWeight::CLayoutObjectTypeWeight(int regionType, bool enableAllowable, bool hasChildren /*= false*/) 
								: CWeight(enableAllowable)
{
	m_Type = regionType;
	CUniString name = CLayoutRegion::GetTypeName(regionType);
	if (hasChildren)
		m_WeightParam = (CParameter*) new CMultiParameter(PARAM_NORMAL_WEIGHT, name.ToC_Str(), "");
	else
		m_WeightParam = (CParameter*) new CDoubleParameter(	PARAM_NORMAL_WEIGHT,
															name.ToC_Str(), "", 
															CEvaluationProfile::WEIGHT_INIT, 
															CEvaluationProfile::WEIGHT_MIN, 
															CEvaluationProfile::WEIGHT_MAX);
	if (enableAllowable)
	{
		if (hasChildren)
			m_AllowableWeight = (CParameter*) new CMultiParameter(	PARAM_ALLOWABLE_WEIGHT,
																	"Allowable", "");
		else
			m_AllowableWeight = (CParameter*) new CDoubleParameter(	PARAM_ALLOWABLE_WEIGHT,
																	"Allowable", "", 
																	CEvaluationProfile::WEIGHT_INIT, 
																	CEvaluationProfile::WEIGHT_MIN, 
																	CEvaluationProfile::WEIGHT_MAX);
		if (hasChildren)
			m_UseAllowableWeight = (CParameter*) new CMultiParameter(	PARAM_USE_ALLOWABLE,
																		"Enable allowable", "");
		else
			m_UseAllowableWeight = (CParameter*) new CBoolParameter(PARAM_USE_ALLOWABLE,
																	"Enable allowable", "", true);
		m_UseAllowableWeight->SetSet(true);
	}
}

CLayoutObjectTypeWeight::~CLayoutObjectTypeWeight()
{
	map<CUniString, CSubTypeWeight*>::iterator it = m_SubTypeWeights.begin();
	while (it != m_SubTypeWeights.end())
	{
		delete (*it).second;
		it++;
	}

	map<int, CLayoutObjectTypeWeight*>::iterator it2 = m_RegionTypeWeights.begin();
	while (it2 != m_RegionTypeWeights.end())
	{
		delete (*it2).second;
		it2++;
	}
}

double CLayoutObjectTypeWeight::GetAverage()
{
	int count = 0;
	double avg = GetAverage(count);
	return count > 0 ? avg / (double)count : 0;
}

double CLayoutObjectTypeWeight::GetAverage(int & count)
{
	//Children
	bool hasChildren = false;
	double val = 0.0;
	map<CUniString, CSubTypeWeight*>::iterator it = m_SubTypeWeights.begin();
	while (it != m_SubTypeWeights.end())
	{
		val += (*it).second->GetAverage(count);
		hasChildren = true;
		it++;
	}

	map<int, CLayoutObjectTypeWeight*>::iterator it2 = m_RegionTypeWeights.begin();
	while (it2 != m_RegionTypeWeights.end())
	{
		val += (*it2).second->GetAverage(count);
		hasChildren = true;
		it2++;
	}
	if (!hasChildren)
	{
		count++;
		val = GetValue();
	}
	return val;
}

CSubTypeWeight * CLayoutObjectTypeWeight::GetSubTypeWeight(CUniString subType)
{
	map<CUniString, CSubTypeWeight*>::iterator it = m_SubTypeWeights.find(subType);
	if (it != m_SubTypeWeights.end())
		return (*it).second;
	return NULL;
}

void CLayoutObjectTypeWeight::AddSubTypeWeight(CUniString subType, CSubTypeWeight * weight)
{
	m_SubTypeWeights.insert(pair<CUniString,CSubTypeWeight*>(subType, weight));

	//Add param as child for the multi param
	((CMultiParameter*)m_WeightParam)->AddChild(weight->GetParam());

	if (m_AllowableWeight != NULL)
	{
		((CMultiParameter*)m_AllowableWeight)->AddChild(weight->GetAllowableParam());
		((CMultiParameter*)m_UseAllowableWeight)->AddChild(weight->GetUseAllowableParam());
	}
}

CLayoutObjectTypeWeight * CLayoutObjectTypeWeight::GetRegionTypeWeight(int regionType)
{
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_RegionTypeWeights.find(regionType);
	if (it != m_RegionTypeWeights.end())
		return (*it).second;
	return NULL;
}

void CLayoutObjectTypeWeight::AddRegionTypeWeight(int regionType, CLayoutObjectTypeWeight * weight)
{
	m_RegionTypeWeights.insert(pair<int,CLayoutObjectTypeWeight*>(regionType, weight));

	//Add param as child for the multi param
	((CMultiParameter*)m_WeightParam)->AddChild(weight->GetParam());

	if (m_AllowableWeight != NULL)
	{
		((CMultiParameter*)m_AllowableWeight)->AddChild(weight->GetAllowableParam());
		((CMultiParameter*)m_UseAllowableWeight)->AddChild(weight->GetUseAllowableParam());
	}
}

void CLayoutObjectTypeWeight::AddRegionTypeShortcutWeight(int regionType, CLayoutObjectTypeWeight * weight)
{
	m_RegionTypeWeights.insert(pair<int,CLayoutObjectTypeWeight*>(regionType, weight));
	//Add param as child for the multi param
	((CMultiParameter*)m_WeightParam)->AddChild(weight->GetParam());
	if (m_AllowableWeight != NULL)
	{
		((CMultiParameter*)m_AllowableWeight)->AddChild(weight->GetAllowableParam());
		((CMultiParameter*)m_UseAllowableWeight)->AddChild(weight->GetUseAllowableParam());
	}
}

void CLayoutObjectTypeWeight::AddRegionTypeShortcutParam(CParameter * childParam, 
												   CParameter * childAllowableParam,
												   CParameter * childUseAllowableParam)
{
	((CMultiParameter*)m_WeightParam)->AddChild(childParam);
	if (m_EnableAllowableWeight && childAllowableParam != NULL)
	{
		((CMultiParameter*)m_AllowableWeight)->AddChild(childAllowableParam);
		((CMultiParameter*)m_UseAllowableWeight)->AddChild(childUseAllowableParam);
	}
}


/*
 * Class CSubTypeWeight
 *
 * Weight for a region subtype (e.g. Paragraph, Heading, ...).
 *
 * CC 09.02.2010 - created
 */

CSubTypeWeight::CSubTypeWeight(CUniString subType, bool enableAllowable, bool hasRegionTypes2 /*= false */) 
			: CWeight(enableAllowable)
{
	m_Type = subType;
	if (hasRegionTypes2)
		m_WeightParam = (CParameter*) new CMultiParameter(	PARAM_NORMAL_WEIGHT,
															subType.ToC_Str(), "");
	else
		m_WeightParam = (CParameter*) new CDoubleParameter(	PARAM_NORMAL_WEIGHT,
															subType.ToC_Str(), "", 
															CEvaluationProfile::WEIGHT_INIT, 
															CEvaluationProfile::WEIGHT_MIN, 
															CEvaluationProfile::WEIGHT_MAX);
	if (enableAllowable)
	{
		if (hasRegionTypes2)
			m_AllowableWeight = (CParameter*) new CMultiParameter(	PARAM_ALLOWABLE_WEIGHT,
																	"Allowable", "");
		else
			m_AllowableWeight = (CParameter*) new CDoubleParameter(	PARAM_ALLOWABLE_WEIGHT,
																	"Allowable", "", 
																	CEvaluationProfile::WEIGHT_INIT, 
																	CEvaluationProfile::WEIGHT_MIN, 
																	CEvaluationProfile::WEIGHT_MAX);
		if (hasRegionTypes2)
			m_UseAllowableWeight = (CParameter*) new CMultiParameter(	PARAM_USE_ALLOWABLE,
																		"Enable allowable", "");
		else
			m_UseAllowableWeight = (CParameter*) new CBoolParameter(PARAM_USE_ALLOWABLE,
																	"Enable allowable", "", true);
		m_UseAllowableWeight->SetSet(true);
	}
}

CSubTypeWeight::~CSubTypeWeight()
{
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_RegionTypeWeights.begin();
	while (it != m_RegionTypeWeights.end())
	{
		delete (*it).second;
		it++;
	}
}

double CSubTypeWeight::GetAverage()
{
	int count = 0;
	double avg = GetAverage(count);
	return count > 0 ? avg / (double)count : 0;
}

double CSubTypeWeight::GetAverage(int & count)
{
	//Children
	bool hasChildren = false;
	double val = 0.0;
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_RegionTypeWeights.begin();
	while (it != m_RegionTypeWeights.end())
	{
		val += (*it).second->GetAverage(count);
		hasChildren = true;
		it++;
	}
	if (!hasChildren)
	{
		count++;
		val = GetValue();
	}
	return val;
}

CLayoutObjectTypeWeight * CSubTypeWeight::GetRegionTypeWeight(int regionType)
{
	map<int, CLayoutObjectTypeWeight*>::iterator it = m_RegionTypeWeights.find(regionType);
	if (it != m_RegionTypeWeights.end())
		return (*it).second;
	return NULL;
}

void CSubTypeWeight::AddRegionTypeWeight(int regionType, CLayoutObjectTypeWeight * weight)
{
	m_RegionTypeWeights.insert(pair<int,CLayoutObjectTypeWeight*>(regionType, weight));

	//Add param as child for the multi param
	((CMultiParameter*)m_WeightParam)->AddChild(weight->GetParam());
	if (m_AllowableWeight != NULL)
	{
		((CMultiParameter*)m_AllowableWeight)->AddChild(weight->GetAllowableParam());
		((CMultiParameter*)m_UseAllowableWeight)->AddChild(weight->GetUseAllowableParam());
	}
}

/*
 * Class CReadingOrderWeight
 *
 *
 *
 * CC 01.07.2010 - created
 */

CReadingOrderWeight::CReadingOrderWeight() : CWeight()
{
	m_WeightParam = (CParameter*) new CDoubleParameter(	PARAM_NORMAL_WEIGHT,
														"Reading Order", "Influence of the reading order error", 
														CEvaluationProfile::WEIGHT_INIT, 
														CEvaluationProfile::WEIGHT_MIN, 
														CEvaluationProfile::WEIGHT_MAX);
}

CReadingOrderWeight::~CReadingOrderWeight()
{
}


/*
 * Class CBorderWeight
 *
 *
 * CC 22.08.2011 - created
 */
CBorderWeight::CBorderWeight(const char * name, const char * description) : CWeight()
{
	m_WeightParam = (CParameter*) new CDoubleParameter(	PARAM_NORMAL_WEIGHT,
														name, description, 
														CEvaluationProfile::WEIGHT_INIT, 
														CEvaluationProfile::WEIGHT_MIN, 
														CEvaluationProfile::WEIGHT_MAX);
}

CBorderWeight::~CBorderWeight()
{
}



/*
 * Class CReadingOrderPenalties
 *
 * Penalty matrix for reading order evaluation.
 *
 * CC 12.03.2013
 */

/*
 * Constructor
 */
CReadingOrderPenalties::CReadingOrderPenalties()
{
	m_Matrix = new int * [9];
	for (int i=0; i<9; i++)
		m_Matrix[i] = new int [9];
	Reset();
}

/*
 * Destructor
 */
CReadingOrderPenalties::~CReadingOrderPenalties()
{
	for (int i=0; i<9; i++)
		delete [] m_Matrix[i];
	delete [] m_Matrix;
}

/*
 * Resets all penalties to the default.
 */
void CReadingOrderPenalties::Reset()
{
	m_MaxPenalty = -1;

	//Init all with 0
	for (int i=0; i<9; i++)
		for (int j=0; j<9; j++)
			m_Matrix[i][j] = 0;

	//Row REL_DIRECTLY_BEFORE ->
	m_Matrix[REL_DIRECTLY_BEFORE][REL_DIRECTLY_BEFORE]			= 0;
	m_Matrix[REL_DIRECTLY_BEFORE][REL_DIRECTLY_AFTER]			= 40;
	m_Matrix[REL_DIRECTLY_BEFORE][REL_UNORDERED]				= 10;
	m_Matrix[REL_DIRECTLY_BEFORE][REL_UNKNOWN]					= 0;
	m_Matrix[REL_DIRECTLY_BEFORE][REL_NOT_DIRECT_OR_UNORDERED]	= 20;
	m_Matrix[REL_DIRECTLY_BEFORE][REL_NOT_DEFINED]				= 0;
	m_Matrix[REL_DIRECTLY_BEFORE][REL_SOMEWHERE_BEFORE]			= 0;
	m_Matrix[REL_DIRECTLY_BEFORE][REL_SOMEWHERE_AFTER]			= 10;

	//Row REL_DIRECTLY_AFTER <-
	m_Matrix[REL_DIRECTLY_AFTER][REL_DIRECTLY_BEFORE]			= 40;
	m_Matrix[REL_DIRECTLY_AFTER][REL_DIRECTLY_AFTER]			= 0;
	m_Matrix[REL_DIRECTLY_AFTER][REL_UNORDERED]					= 10;
	m_Matrix[REL_DIRECTLY_AFTER][REL_UNKNOWN]					= 0;
	m_Matrix[REL_DIRECTLY_AFTER][REL_NOT_DIRECT_OR_UNORDERED]	= 20;
	m_Matrix[REL_DIRECTLY_AFTER][REL_NOT_DEFINED]				= 0;
	m_Matrix[REL_DIRECTLY_AFTER][REL_SOMEWHERE_BEFORE]			= 10;
	m_Matrix[REL_DIRECTLY_AFTER][REL_SOMEWHERE_AFTER]			= 0;

	//Row REL_UNORDERED --
	m_Matrix[REL_UNORDERED][REL_DIRECTLY_BEFORE]			= 20;
	m_Matrix[REL_UNORDERED][REL_DIRECTLY_AFTER]				= 20;
	m_Matrix[REL_UNORDERED][REL_UNORDERED]					= 0;
	m_Matrix[REL_UNORDERED][REL_UNKNOWN]					= 0;
	m_Matrix[REL_UNORDERED][REL_NOT_DIRECT_OR_UNORDERED]	= 10;
	m_Matrix[REL_UNORDERED][REL_NOT_DEFINED]				= 0;
	m_Matrix[REL_UNORDERED][REL_SOMEWHERE_BEFORE]			= 10;
	m_Matrix[REL_UNORDERED][REL_SOMEWHERE_AFTER]			= 10;

	//Row REL_UNKNOWN ?
	m_Matrix[REL_UNKNOWN][REL_DIRECTLY_BEFORE]			= 0;
	m_Matrix[REL_UNKNOWN][REL_DIRECTLY_AFTER]			= 0;
	m_Matrix[REL_UNKNOWN][REL_UNORDERED]				= 0;
	m_Matrix[REL_UNKNOWN][REL_UNKNOWN]					= 0;
	m_Matrix[REL_UNKNOWN][REL_NOT_DIRECT_OR_UNORDERED]	= 0;
	m_Matrix[REL_UNKNOWN][REL_NOT_DEFINED]				= 0;
	m_Matrix[REL_UNKNOWN][REL_SOMEWHERE_BEFORE]			= 0;
	m_Matrix[REL_UNKNOWN][REL_SOMEWHERE_AFTER]			= 0;

	//Row REL_NOT_DIRECT_OR_UNORDERED -x-
	m_Matrix[REL_NOT_DIRECT_OR_UNORDERED][REL_DIRECTLY_BEFORE]			= 20;
	m_Matrix[REL_NOT_DIRECT_OR_UNORDERED][REL_DIRECTLY_AFTER]			= 20;
	m_Matrix[REL_NOT_DIRECT_OR_UNORDERED][REL_UNORDERED]				= 10;
	m_Matrix[REL_NOT_DIRECT_OR_UNORDERED][REL_UNKNOWN]					= 0;
	m_Matrix[REL_NOT_DIRECT_OR_UNORDERED][REL_NOT_DIRECT_OR_UNORDERED]	= 0;
	m_Matrix[REL_NOT_DIRECT_OR_UNORDERED][REL_NOT_DEFINED]				= 0;
	m_Matrix[REL_NOT_DIRECT_OR_UNORDERED][REL_SOMEWHERE_BEFORE]			= 10;
	m_Matrix[REL_NOT_DIRECT_OR_UNORDERED][REL_SOMEWHERE_AFTER]			= 10;

	//Row REL_NOT_DEFINED n.d.
	m_Matrix[REL_NOT_DEFINED][REL_DIRECTLY_BEFORE]			= 20;
	m_Matrix[REL_NOT_DEFINED][REL_DIRECTLY_AFTER]			= 20;
	m_Matrix[REL_NOT_DEFINED][REL_UNORDERED]				= 10;
	m_Matrix[REL_NOT_DEFINED][REL_UNKNOWN]					= 0;
	m_Matrix[REL_NOT_DEFINED][REL_NOT_DIRECT_OR_UNORDERED]	= 0;
	m_Matrix[REL_NOT_DEFINED][REL_NOT_DEFINED]				= 0;
	m_Matrix[REL_NOT_DEFINED][REL_SOMEWHERE_BEFORE]			= 10;
	m_Matrix[REL_NOT_DEFINED][REL_SOMEWHERE_AFTER]			= 10;

	//Row REL_SOMEWHERE_BEFORE ->->
	m_Matrix[REL_SOMEWHERE_BEFORE][REL_DIRECTLY_BEFORE]			= 0;
	m_Matrix[REL_SOMEWHERE_BEFORE][REL_DIRECTLY_AFTER]			= 20;
	m_Matrix[REL_SOMEWHERE_BEFORE][REL_UNORDERED]				= 5;
	m_Matrix[REL_SOMEWHERE_BEFORE][REL_UNKNOWN]					= 0;
	m_Matrix[REL_SOMEWHERE_BEFORE][REL_NOT_DIRECT_OR_UNORDERED]	= 5;
	m_Matrix[REL_SOMEWHERE_BEFORE][REL_NOT_DEFINED]				= 0;
	m_Matrix[REL_SOMEWHERE_BEFORE][REL_SOMEWHERE_BEFORE]		= 0;
	m_Matrix[REL_SOMEWHERE_BEFORE][REL_SOMEWHERE_AFTER]			= 10;

	//Row REL_SOMEWHERE_AFTER <-<-
	m_Matrix[REL_SOMEWHERE_AFTER][REL_DIRECTLY_BEFORE]			= 20;
	m_Matrix[REL_SOMEWHERE_AFTER][REL_DIRECTLY_AFTER]			= 0;
	m_Matrix[REL_SOMEWHERE_AFTER][REL_UNORDERED]				= 5;
	m_Matrix[REL_SOMEWHERE_AFTER][REL_UNKNOWN]					= 0;
	m_Matrix[REL_SOMEWHERE_AFTER][REL_NOT_DIRECT_OR_UNORDERED]	= 5;
	m_Matrix[REL_SOMEWHERE_AFTER][REL_NOT_DEFINED]				= 0;
	m_Matrix[REL_SOMEWHERE_AFTER][REL_SOMEWHERE_BEFORE]			= 10;
	m_Matrix[REL_SOMEWHERE_AFTER][REL_SOMEWHERE_AFTER]			= 0;
}

/*
 * Returns the penalty for a single combination of relations.
 * The column specifies the relation as defined in the ground truth and
 * the row specifies the relation as defined in the reading order that is to be evaluated.
 * 'row', 'column' - Relation type (see REL_... constants)
 * Return - Penalty value
 */
int CReadingOrderPenalties::GetPenalty(int row, int column)
{
	if (row >= 0 && row < 9 && column >= 0 && column < 9)
		return m_Matrix[row][column];
	return 0;
}

/*
 * Sets the penalty for a single combination of relations.
 * The column specifies the relation as defined in the ground truth and
 * the row specifies the relation as defined in the reading order that is to be evaluated.
 * 'row', 'column' - Relation type (see REL_... constants)
 * 'penalty' - Penalty value
 */
void CReadingOrderPenalties::SetPenalty(int row, int column, int penalty)
{
	if (row >= 0 && row < 9 && column >= 0 && column < 9)
		m_Matrix[row][column] = penalty;

	m_MaxPenalty = -1;
}

/*
 * Returns the maximum single penalty within the matrix.
 */
int CReadingOrderPenalties::GetMaxPenalty()
{
	if (m_MaxPenalty < 0)
	{
		m_MaxPenalty = 0;
		for (int i=0; i<9; i++)
			for (int j=0; j<9; j++)
			{
				if (m_Matrix[i][j] > m_MaxPenalty)
					m_MaxPenalty = m_Matrix[i][j];
			}
	}
	return m_MaxPenalty;
}

/*
 * Checks if all values of this matrix are the default values.
 * Returns true if this matrix equals the default matrix, returns false if one or more values are not the default.
 */
bool CReadingOrderPenalties::IsDefaultMatrix()
{
	CReadingOrderPenalties defaultMatrix;

	for (int i=0; i<9; i++)
		for (int j=0; j<9; j++)
		{
			if (m_Matrix[i][j] != defaultMatrix.GetPenalty(i, j))
				return false;
		}
	return true;
}
