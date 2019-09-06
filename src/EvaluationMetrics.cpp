
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "EvaluationMetrics.h"


namespace PRImA
{

/*
 * Class CEvaluationMetrics
 *
 * Base class for metric results (success rates etc.).
 *
 * CC 23.08.2011
 */
CEvaluationMetrics::CEvaluationMetrics(CEvaluationResults * results, CEvaluationProfile * profile)
{
	m_Results = results;
	m_Profile = profile;
}

CEvaluationMetrics::~CEvaluationMetrics()
{
}


/*
 * Class CBorderEvaluationMetrics
 *
 * Border evaluation metric results.
 *
 * CC 23.08.2011
 */
CBorderEvaluationMetrics::CBorderEvaluationMetrics(CEvaluationResults * results, CEvaluationProfile * profile,
												   bool calculate) 
												   : CEvaluationMetrics(results, profile)
{
	m_IncludedBackgroundSuccessRate = 0.0;
	m_ExcludedForegroundSuccessRate = 0.0;
	m_MissingRegionAreaSuccessRate = 0.0;
	m_OverallSuccessRate = 0.0;
	m_ImageArea = 0;		
	m_GroundTruthBorderArea = 0;
	m_SegResultBorderArea = 0;
	m_TotalWeightedMissingRegionArea = 0.0;

	if (calculate && m_Results->GetBorderResults() != NULL)
	{
		CalculateGeneralFigures();
		CalculateWeightedErrors();
		CalculateSuccessRates();
	}
}

void CBorderEvaluationMetrics::CalculateGeneralFigures()
{
	CPageLayout * groundTruth = m_Results->GetLayoutEvaluation()->GetGroundTruth();
	CPageLayout * segResult = m_Results->GetLayoutEvaluation()->GetSegResult();

	//Image Area
	m_ImageArea = m_Results->GetLayoutEvaluation()->GetWidth() * m_Results->GetLayoutEvaluation()->GetHeight();

	//Border areas
	CIntervalRepresentation * intRepr = m_Results->GetBorderResults()->GetGroundTruthBorderIntervalRep();
	if (intRepr != NULL)
		m_GroundTruthBorderArea = intRepr->GetArea();

	intRepr = m_Results->GetBorderResults()->GetSegResultBorderIntervalRep();
	if (intRepr != NULL)
		m_SegResultBorderArea = intRepr->GetArea();

	//Combined region area / number of foreground pixels
	m_OverallGroundTruthRegionArea = 0;
	CLayoutObjectIterator * it = CLayoutObjectIterator::GetLayoutObjectIterator(groundTruth, CLayoutObject::TYPE_LAYOUT_REGION);
	while (it->HasNext())
	{
		CLayoutObject * reg = it->Next();
		m_OverallGroundTruthRegionArea += m_Results->GetRegionArea(reg->GetId(), true);
	}
	delete it;

}

void CBorderEvaluationMetrics::CalculateWeightedErrors()
{
	//Included background
	double weight = ((CDoubleParameter*)m_Profile->GetIncludedBackgroundBorderWeightParam())->GetValue();
	CBorderEvaluationError * err = m_Results->GetBorderResults()->GetIncludedBackgroundError();
	err->SetWeightedAreaError((double)err->GetArea() * weight);

	//Excluded foreground
	weight = ((CDoubleParameter*)m_Profile->GetExcludedForegroundBorderWeightParam())->GetValue();
	err = m_Results->GetBorderResults()->GetExcludedForegroundError();
	err->SetWeightedAreaError((double)err->GetArea() * weight);

	//Missed region area
	CPageLayout * groundTruth = m_Results->GetLayoutEvaluation()->GetGroundTruth();
	map<CUniString, CLayoutObjectEvaluationError*> * errs = m_Results->GetBorderResults()->GetMissingRegionAreaErrors();
	m_TotalWeightedMissingRegionArea = 0.0;
	double value;
	for (map<CUniString, CLayoutObjectEvaluationError*>::iterator it = errs->begin(); it != errs->end(); it++)
	{
		CLayoutRegion * reg = (CLayoutRegion*)groundTruth->FindLayoutObject(CLayoutObject::TYPE_LAYOUT_REGION, (*it).first);
		if (reg != NULL)
		{
			CParameter * weightParam = NULL;
			CUniString subType = CLayoutEvaluation::GetLayoutRegionSubtype(reg, m_Profile);
			if (!subType.IsEmpty())
				 weightParam = m_Profile->GetMissingRegionBorderWeightParam(reg->GetLayoutObjectType(), subType);
			else
				weightParam = m_Profile->GetMissingRegionBorderWeightParam(reg->GetLayoutObjectType());
			if (weightParam != NULL)
			{
				weight = ((CDoubleParameter*)weightParam)->GetValue();
				value = (double)((*it).second->GetArea()) * weight;
				(*it).second->SetWeightedAreaError(value);
				m_TotalWeightedMissingRegionArea += value;
			}
		}
	}
}

void CBorderEvaluationMetrics::CalculateSuccessRates()
{
	//There are two types of success rates:
	//  - relative to maximal possible error area (high contrast)
	//  - relative to image area (more objective)
	//For the overall success rate the latter type is used.

	//Included background
	CFunction * f = new CInverseSuccessRate(max(1.0, (m_ImageArea - m_GroundTruthBorderArea) / 2.0));
	CBorderEvaluationError * err = m_Results->GetBorderResults()->GetIncludedBackgroundError();
	double value = err->GetWeightedAreaError();
	m_IncludedBackgroundSuccessRate = f->GetY(value);
	delete f;
	
	f = new CInverseSuccessRate(m_ImageArea / 2.0);
	m_IncludedBackgroundSuccessRateRelativeToImage = f->GetY(value);
	delete f;

	//Excluded foreground
	f = new CInverseSuccessRate(max(1.0, (m_GroundTruthBorderArea) / 2.0));
	err = m_Results->GetBorderResults()->GetExcludedForegroundError();
	value = err->GetWeightedAreaError();
	m_ExcludedForegroundSuccessRate = f->GetY(value);
	delete f;

	f = new CInverseSuccessRate(m_ImageArea / 2.0);
	m_ExcludedForegroundSuccessRateRelativeToImage = f->GetY(value);
	delete f;

	//Missed region area
	f = new CInverseSuccessRate(max(1.0, (m_OverallGroundTruthRegionArea) / 2.0));
	m_MissingRegionAreaSuccessRate = f->GetY(m_TotalWeightedMissingRegionArea);
	delete f;

	f = new CInverseSuccessRate(m_ImageArea / 2.0);
	m_MissingRegionAreaSuccessRateRelativeToImage = f->GetY(value);
	delete f;

	//Overall (harmonic mean, relative to image area)
	m_OverallSuccessRate = 3.0 / (	1.0 / m_IncludedBackgroundSuccessRateRelativeToImage + 
									1.0 / m_ExcludedForegroundSuccessRateRelativeToImage +
									1.0 / m_MissingRegionAreaSuccessRateRelativeToImage);
}

CFunction * CBorderEvaluationMetrics::GetSuccessFunction()
{
	double param = m_ImageArea / 2.0;
	return new CInverseSuccessRate(param);
}


/*
 * Class CLayoutObjectEvaluationMetrics
 *
 * Combined penalties / error scores / success rates of the evaluation result.
 *
 * CC 25.06.2010
 */

/*
 * Constructor
 */
CLayoutObjectEvaluationMetrics::CLayoutObjectEvaluationMetrics(CEvaluationResults * results, CEvaluationProfile * profile,
									   bool calculate,
									   int layoutRegionType /*= CLayoutRegion::TYPE_ALL*/,
									   CLayoutObjectEvaluationMetrics * metricsForAllTypes /*= NULL*/)
									   : CEvaluationMetrics(results, profile)
{
	m_UsePixelArea = profile->IsUsePixelArea();
	m_LayoutRegionType = layoutRegionType;
	m_MetricsForAllTypes = metricsForAllTypes;

	m_NumberOfGroundTruthRegions = 0;
	m_NumberOfSegResultRegions = 0;
	m_ImageArea = 0;			
	m_ForeGroundPixelCount = 0;	
	m_OverallGroundTruthRegionArea = 0;				
	m_OverallGroundTruthRegionPixelCount = 0;		
	m_OverallSegResultRegionArea = 0;				
	m_OverallSegResultRegionPixelCount = 0;			
	m_OverallWeightedAreaError = 0;					
	m_OverallWeightedCountError = 0;				
	m_ReadingOrderError = 0;
	m_ReadingOrderSuccessRate = 0;
	m_OverallWeightedAreaSuccessRate = 0;		
	m_OverallWeightedCountSuccessRate = 0;		
	m_HarmonicWeightedAreaSuccessRate = 0;		
	m_HarmonicWeightedCountSuccessRate = 0;		
	m_ErrorRateBasedOnSimpleCount = 0;			
	m_RecallNonStrict = 0;		
	m_RecallStrict = 0;			
	m_PrecisionNonStrict = 0;	
	m_PrecisionStrict = 0;		
	m_FMeasureStrict = 0;		
	m_FMeasureNonStrict = 0;	
	m_OCRSuccessRate = 0.0;
	m_OCRSuccessRateExclReplacementChar = 0.0;
	m_GlyphStatistics = NULL;

	if (calculate)
	{
		CalculateGeneralFigures();
		CalculateWeightedErrors();
		CalculateReadingOrderError();
		CalculateSimpleCountBasedErrorRates();
		CalculateRecallAndPrecision();
		CalculateOverallSuccessRate();
		CalculateOCRSuccessRate();
	}
}

/*
 * Destructor
 */
CLayoutObjectEvaluationMetrics::~CLayoutObjectEvaluationMetrics()
{
	delete m_GlyphStatistics;
}

double CLayoutObjectEvaluationMetrics::GetErrorRatePerTypeBasedOnCount(int errorType)
{
	map<int,double>::iterator it = m_ErrorRatePerTypeBasedOnSimpleCount.find(errorType);
	if (it != m_ErrorRatePerTypeBasedOnSimpleCount.end())
		return (*it).second;
	return 0.0;
}

double CLayoutObjectEvaluationMetrics::GetOverallWeightedAreaErrorPerErrorType(int errorType)
{
	map<int,double>::iterator it = m_OverallWeightedAreaErrorPerErrorType.find(errorType);
	if (it != m_OverallWeightedAreaErrorPerErrorType.end())
		return (*it).second;
	return 0.0;
}

double CLayoutObjectEvaluationMetrics::GetOverallWeightedAreaErrorPerRegionType(int regionType)
{
	map<int,double>::iterator it = m_OverallWeightedAreaErrorPerRegionType.find(regionType);
	if (it != m_OverallWeightedAreaErrorPerRegionType.end())
		return (*it).second;
	return 0.0;
}

/*
 * Overall error per region type in relation to overall error ([0..1])
 */
double CLayoutObjectEvaluationMetrics::GetRelativeWeightedAreaErrorPerRegionType(int regionType)
{
	double errorPerType = GetOverallWeightedAreaErrorPerRegionType(regionType);
	if (m_OverallWeightedAreaError != 0)
		return errorPerType / m_OverallWeightedAreaError;
	else
		return 0.0;
}

double CLayoutObjectEvaluationMetrics::GetOverallWeightedCountErrorPerErrorType(int errorType)
{
	map<int,double>::iterator it = m_OverallWeightedCountErrorPerErrorType.find(errorType);
	if (it != m_OverallWeightedCountErrorPerErrorType.end())
		return (*it).second;
	return 0.0;
}

double CLayoutObjectEvaluationMetrics::GetOverallWeightedCountErrorPerRegionType(int regionType)
{
	map<int,double>::iterator it = m_OverallWeightedCountErrorPerRegionType.find(regionType);
	if (it != m_OverallWeightedCountErrorPerRegionType.end())
		return (*it).second;
	return 0.0;
}

/*
 * Overall error per region type in relation to overall error ([0..1])
 */
double CLayoutObjectEvaluationMetrics::GetRelativeWeightedCountErrorPerRegionType(int regionType)
{
	double errorPerType = GetOverallWeightedCountErrorPerRegionType(regionType);
	if (m_OverallWeightedCountError != 0)
		return errorPerType / m_OverallWeightedCountError;
	else
		return 0.0;
}

double CLayoutObjectEvaluationMetrics::GetWeightedAreaSuccessRatePerType(int errorType)
{
	map<int,double>::iterator it = m_WeightedAreaSuccessRatePerType.find(errorType);
	if (it != m_WeightedAreaSuccessRatePerType.end())
		return (*it).second;
	return 1.0;
}

double CLayoutObjectEvaluationMetrics::GetWeightedCountSuccessRatePerType(int errorType)
{
	map<int,double>::iterator it = m_WeightedCountSuccessRatePerType.find(errorType);
	if (it != m_WeightedCountSuccessRatePerType.end())
		return (*it).second;
	return 1.0;
}

/*
 * Retunrs the recall per region type (0..1).
 * Returns -1 if there are not regions of the given type.
 */
double CLayoutObjectEvaluationMetrics::GetRecall(int regionType)
{
	map<int,double>::iterator it = m_RecallPerType.find(regionType);
	if (it == m_RecallPerType.end())
		return -1.0;
	return (*it).second;
}

/*
 * Retunrs the precision per region type (0..1).
 * Returns -1 if there are not regions of the given type.
 */
double CLayoutObjectEvaluationMetrics::GetPrecision(int regionType)
{
	map<int,double>::iterator it = m_PrecisionPerType.find(regionType);
	if (it == m_PrecisionPerType.end())
		return -1.0;
	return (*it).second;
}

/*
 * Sets the number of ground-truth regions (used by XML reader).
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetNumberOfGroundTruthRegionsPerType(int type, int number)
{
	map<int,int>::iterator it = m_NumberOfGroundTruthRegionsPerType.find(type);
	if (it != m_NumberOfGroundTruthRegionsPerType.end()) //already there
		(*it).second = number;
	else //Insert
		m_NumberOfGroundTruthRegionsPerType.insert(pair<int,int>(type, number));
}

/*
 * Sets the number of seg result regions (used by XML reader).
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetNumberOfSegResultRegionsPerType(int type, int number)
{
	map<int,int>::iterator it = m_NumberOfSegResultRegionPerType.find(type);
	if (it != m_NumberOfSegResultRegionPerType.end()) //already there
		(*it).second = number;
	else //Insert
		m_NumberOfSegResultRegionPerType.insert(pair<int,int>(type, number));
}

/*
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetOverallGroundTruthRegionAreaPerType(int type, int area)
{
	map<int,int>::iterator it = m_GroundTruthRegionAreaPerType.find(type);
	if (it != m_GroundTruthRegionAreaPerType.end()) //already there
		(*it).second = area;
	else //Insert
		m_GroundTruthRegionAreaPerType.insert(pair<int,int>(type, area));
}

/*
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetOverallSegResultRegionAreaPerType(int type, int area)
{
	map<int,int>::iterator it = m_SegResultRegionAreaPerType.find(type);
	if (it != m_SegResultRegionAreaPerType.end()) //already there
		(*it).second = area;
	else //Insert
		m_SegResultRegionAreaPerType.insert(pair<int,int>(type, area));
}

/*
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetOverallGroundTruthRegionPixelCountPerType(int type, int count)
{
	map<int,int>::iterator it = m_GroundTruthRegionPixelCountPerType.find(type);
	if (it != m_GroundTruthRegionPixelCountPerType.end()) //already there
		(*it).second = count;
	else //Insert
		m_GroundTruthRegionPixelCountPerType.insert(pair<int,int>(type, count));
}

/*
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetOverallSegResultRegionPixelCountPerType(int type, int count)
{
	map<int,int>::iterator it = m_SegResultRegionPixelCountPerType.find(type);
	if (it != m_SegResultRegionPixelCountPerType.end()) //already there
		(*it).second = count;
	else //Insert
		m_SegResultRegionPixelCountPerType.insert(pair<int,int>(type, count));
}

/*
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetRecallAreaPerType(int type, int area)
{
	map<int,int>::iterator it = m_RecallAreaPerType.find(type);
	if (it != m_RecallAreaPerType.end()) //already there
		(*it).second = area;
	else //Insert
		m_RecallAreaPerType.insert(pair<int,int>(type, area));
}

/*
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetRecallPixelCountPerType(int type, int count)
{
	map<int,int>::iterator it = m_RecallPixelCountPerType.find(type);
	if (it != m_RecallPixelCountPerType.end()) //already there
		(*it).second = count;
	else //Insert
		m_RecallPixelCountPerType.insert(pair<int,int>(type, count));
}

/*
 * 'type' - error type (merge, split, ...)
 */
void CLayoutObjectEvaluationMetrics::SetOverallWeightedAreaErrorPerErrorType(int type, double area)
{
	map<int,double>::iterator it = m_OverallWeightedAreaErrorPerErrorType.find(type);
	if (it != m_OverallWeightedAreaErrorPerErrorType.end()) //already there
		(*it).second = area;
	else //Insert
		m_OverallWeightedAreaErrorPerErrorType.insert(pair<int,double>(type, area));
}

/*
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetOverallWeightedAreaErrorPerRegionType(int type, double area)
{
	map<int,double>::iterator it = m_OverallWeightedAreaErrorPerRegionType.find(type);
	if (it != m_OverallWeightedAreaErrorPerRegionType.end()) //already there
		(*it).second = area;
	else //Insert
		m_OverallWeightedAreaErrorPerRegionType.insert(pair<int,double>(type, area));
}

/*
 * 'type' - error type (merge, split, ...)
 */
void CLayoutObjectEvaluationMetrics::SetOverallWeightedCountErrorPerErrorType(int type, double error)
{
	map<int,double>::iterator it = m_OverallWeightedCountErrorPerErrorType.find(type);
	if (it != m_OverallWeightedCountErrorPerErrorType.end()) //already there
		(*it).second = error;
	else //Insert
		m_OverallWeightedCountErrorPerErrorType.insert(pair<int,double>(type, error));
}

/*
 * 'type' - region type (text, image, ...)
 */
void CLayoutObjectEvaluationMetrics::SetOverallWeightedCountErrorPerRegionType(int type, double error)
{
	map<int,double>::iterator it = m_OverallWeightedCountErrorPerRegionType.find(type);
	if (it != m_OverallWeightedCountErrorPerRegionType.end()) //already there
		(*it).second = error;
	else //Insert
		m_OverallWeightedCountErrorPerRegionType.insert(pair<int,double>(type, error));
}

/*
 * 'errorType' - (merge, split, ...)
 */
void CLayoutObjectEvaluationMetrics::SetWeightedAreaSuccessRatePerType(int errorType, double rate)
{
	map<int,double>::iterator it = m_WeightedAreaSuccessRatePerType.find(errorType);
	if (it != m_WeightedAreaSuccessRatePerType.end()) //already there
		(*it).second = rate;
	else //Insert
		m_WeightedAreaSuccessRatePerType.insert(pair<int,double>(errorType, rate));
}

/*
 * 'errorType' - (merge, split, ...)
 */
void CLayoutObjectEvaluationMetrics::SetWeightedCountSuccessRatePerType(int errorType, double rate)
{
	map<int,double>::iterator it = m_WeightedCountSuccessRatePerType.find(errorType);
	if (it != m_WeightedCountSuccessRatePerType.end()) //already there
		(*it).second = rate;
	else //Insert
		m_WeightedCountSuccessRatePerType.insert(pair<int,double>(errorType, rate));
}

/*
 * 'errorType' - (merge, split, ...)
 */
void CLayoutObjectEvaluationMetrics::SetErrorRatePerTypeBasedOnCount(int errorType, double rate)
{
	map<int,double>::iterator it = m_ErrorRatePerTypeBasedOnSimpleCount.find(errorType);
	if (it != m_ErrorRatePerTypeBasedOnSimpleCount.end()) //already there
		(*it).second = rate;
	else //Insert
		m_ErrorRatePerTypeBasedOnSimpleCount.insert(pair<int,double>(errorType, rate));
}

void CLayoutObjectEvaluationMetrics::SetRecall(int regionType, double recall)
{
	map<int,double>::iterator it = m_ErrorRatePerTypeBasedOnSimpleCount.find(regionType);
	if (it != m_ErrorRatePerTypeBasedOnSimpleCount.end()) //already there
		(*it).second = recall;
	else //Insert
		m_ErrorRatePerTypeBasedOnSimpleCount.insert(pair<int,double>(regionType, recall));
}

void CLayoutObjectEvaluationMetrics::SetPrecision(int regionType, double precision)
{
	map<int,double>::iterator it = m_ErrorRatePerTypeBasedOnSimpleCount.find(regionType);
	if (it != m_ErrorRatePerTypeBasedOnSimpleCount.end()) //already there
		(*it).second = precision;
	else //Insert
		m_ErrorRatePerTypeBasedOnSimpleCount.insert(pair<int,double>(regionType, precision));
}

int CLayoutObjectEvaluationMetrics::GetNumberOfGroundTruthRegionsPerType(int regionType)
{
	map<int,int>::iterator it = m_NumberOfGroundTruthRegionsPerType.find(regionType);
	if (it != m_NumberOfGroundTruthRegionsPerType.end())
		return (*it).second;
	return 0;
}

int CLayoutObjectEvaluationMetrics::GetNumberOfSegResultRegionsPerType(int regionType)
{
	map<int,int>::iterator it = m_NumberOfSegResultRegionPerType.find(regionType);
	if (it != m_NumberOfSegResultRegionPerType.end())
		return (*it).second;
	return 0;
}

int CLayoutObjectEvaluationMetrics::GetOverallGroundTruthRegionAreaPerType(int regionType)
{
	map<int,int>::iterator it = m_GroundTruthRegionAreaPerType.find(regionType);
	if (it != m_GroundTruthRegionAreaPerType.end())
		return (*it).second;
	return 0;
}

int CLayoutObjectEvaluationMetrics::GetOverallSegResultRegionAreaPerType(int regionType)
{
	map<int,int>::iterator it = m_SegResultRegionAreaPerType.find(regionType);
	if (it != m_SegResultRegionAreaPerType.end())
		return (*it).second;
	return 0;
}

int CLayoutObjectEvaluationMetrics::GetOverallGroundTruthRegionPixelCountPerType(int regionType)
{
	map<int,int>::iterator it = m_GroundTruthRegionPixelCountPerType.find(regionType);
	if (it != m_GroundTruthRegionPixelCountPerType.end())
		return (*it).second;
	return 0;
}

int CLayoutObjectEvaluationMetrics::GetOverallSegResultRegionPixelCountPerType(int regionType)
{
	map<int,int>::iterator it = m_SegResultRegionPixelCountPerType.find(regionType);
	if (it != m_SegResultRegionPixelCountPerType.end())
		return (*it).second;
	return 0;
}

void CLayoutObjectEvaluationMetrics::SetWeightedAreaInfluencePerType(int errorType, double influence)
{
	map<int, double>::iterator it = m_WeightedAreaInfluencePerType.find(errorType);
	if (it != m_WeightedAreaInfluencePerType.end())
		(*it).second = influence;
	else
		m_WeightedAreaInfluencePerType.insert(pair<int, double>(errorType, influence));
}

void CLayoutObjectEvaluationMetrics::SetWeightedCountInfluencePerType(int errorType, double influence)
{
	map<int, double>::iterator it = m_WeightedCountInfluencePerType.find(errorType);
	if (it != m_WeightedCountInfluencePerType.end())
		(*it).second = influence;
	else
		m_WeightedCountInfluencePerType.insert(pair<int, double>(errorType, influence));
}

double CLayoutObjectEvaluationMetrics::GetWeightedAreaInfluencePerType(int errorType)
{
	map<int, double>::iterator it = m_WeightedAreaInfluencePerType.find(errorType);
	if (it != m_WeightedAreaInfluencePerType.end())
		return (*it).second;
	return 0.0;
}

double CLayoutObjectEvaluationMetrics::GetWeightedCountInfluencePerType(int errorType)
{
	map<int, double>::iterator it = m_WeightedCountInfluencePerType.find(errorType);
	if (it != m_WeightedCountInfluencePerType.end())
		return (*it).second;
	return 0.0;
}

/*
 * Calculates evaluation result independent figures:
 *   - Number of regions (ground-truth and segmentation result)
 *   - Number of regions per type (for blocks only: text, image, table, ...; see CLayoutRegion::TYPE_...)
 *   - Image area
 *   - Overall number of foreground pixels
 *   - Combined area of all regions
 *   - Combined foreground pixel count of all regions
 *   - 
 */
void CLayoutObjectEvaluationMetrics::CalculateGeneralFigures()
{
	CPageLayout * groundTruth = m_Results->GetLayoutEvaluation()->GetGroundTruth();
	CPageLayout * segResult = m_Results->GetLayoutEvaluation()->GetSegResult();
	int regionType = m_Results->GetLayoutObjectType();

	//Number of ground-truth regions (of the used region type)
	CLayoutObjectIterator * it = NULL;
	if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
		it = CLayoutObjectIterator::GetLayoutObjectIterator(groundTruth, regionType);
	else
		it = CLayoutObjectIterator::GetLayoutObjectIterator(groundTruth, regionType, m_LayoutRegionType);
	int count = 0;
	map<int,int>::iterator itCountMap;
	while (it->HasNext())
	{
		CLayoutObject * reg = it->Next();
		count++;
		if (reg->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
		{
			int type =  ((CLayoutRegion*)reg)->GetType();
			itCountMap = m_NumberOfGroundTruthRegionsPerType.find(type);
			if (itCountMap == m_NumberOfGroundTruthRegionsPerType.end())
				m_NumberOfGroundTruthRegionsPerType.insert(pair<int,int>(type, 1));
			else
				(*itCountMap).second++;
		}
	}
	m_NumberOfGroundTruthRegions = count;
	delete it;

	//Number of segmentation result regions (of the used region type)
	if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
		it = CLayoutObjectIterator::GetLayoutObjectIterator(segResult, regionType);
	else
		it = CLayoutObjectIterator::GetLayoutObjectIterator(segResult, regionType, m_LayoutRegionType);
	count = 0;
	while (it->HasNext())
	{
		CLayoutObject * reg = it->Next();
		count++;
		if (reg->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
		{
			int type =  ((CLayoutRegion*)reg)->GetType();
			itCountMap = m_NumberOfSegResultRegionPerType.find(type);
			if (itCountMap == m_NumberOfSegResultRegionPerType.end())
				m_NumberOfSegResultRegionPerType.insert(pair<int,int>(type, 1));
			else
				(*itCountMap).second++;
		}
	}
	m_NumberOfSegResultRegions = count;
	delete it;

	//Image Area
	m_ImageArea = m_Results->GetLayoutEvaluation()->GetWidth() * m_Results->GetLayoutEvaluation()->GetHeight();

	//Number of foreground pixels
	m_ForeGroundPixelCount = 0;
	COpenCvBiLevelImage * img = m_Results->GetLayoutEvaluation()->GetBilevelImage();
	if (img != NULL)
		m_ForeGroundPixelCount = img->CountPixels(true);

	//Combined region area / number of foreground pixels
	m_OverallGroundTruthRegionArea = 0;
	m_OverallGroundTruthRegionPixelCount = 0;
	if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
		it = CLayoutObjectIterator::GetLayoutObjectIterator(groundTruth, regionType);
	else
		it = CLayoutObjectIterator::GetLayoutObjectIterator(groundTruth, regionType, m_LayoutRegionType);
	while (it->HasNext())
	{
		CLayoutObject * reg = it->Next();
		m_OverallGroundTruthRegionArea += m_Results->GetRegionArea(reg->GetId(), true);
		m_OverallGroundTruthRegionPixelCount += m_Results->GetPixelCount(reg->GetId(), true);
	}
	delete it;

	m_OverallSegResultRegionArea = 0;
	m_OverallSegResultRegionPixelCount = 0;
	if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
		it = CLayoutObjectIterator::GetLayoutObjectIterator(segResult, regionType);
	else
		it = CLayoutObjectIterator::GetLayoutObjectIterator(segResult, regionType, m_LayoutRegionType);
	while (it->HasNext())
	{
		CLayoutObject * reg = it->Next();
		m_OverallSegResultRegionArea += m_Results->GetRegionArea(reg->GetId(), false);
		m_OverallSegResultRegionPixelCount += m_Results->GetPixelCount(reg->GetId(), false);
	}
	delete it;
}

/*
 * Returns regionCountDeviation / (#ground-truth regions)
 */
double CLayoutObjectEvaluationMetrics::GetRelativeRegionCountDeviation()
{
	if (m_NumberOfGroundTruthRegions > 0)
		return (double)GetRegionCountDeviation() / (double)m_NumberOfGroundTruthRegions;
	else
		return GetRegionCountDeviation();
}

/*
 * Applies the weight to every evaluation error
 */
void CLayoutObjectEvaluationMetrics::CalculateWeightedErrors()
{
	CLayoutObjectErrorIterator it(m_Results);
	CLayoutObjectEvaluationError * err = NULL;
	int errorType = 0;
	double weight = 0.0;
	double weightedArea, weightedCount;
	m_OverallWeightedAreaError = 0.0;
	m_OverallWeightedCountError = 0.0;

	while (it.HasNext())
	{
		CLayoutObjectEvaluationError * err = it.GetNext();
		errorType = err->GetType();
		bool allowable = false;
		weightedArea = 0.0;
		weightedCount = 0.0;
		int layoutRegionType1 = CLayoutRegion::TYPE_INVALID;
		const double weightForNestedRegions = 0.5;

		if (!err->IsFalseAlarm())
		{
			//Miss, part. miss, false detection
			if (	errorType == CLayoutObjectEvaluationError::TYPE_MISS
				||	errorType == CLayoutObjectEvaluationError::TYPE_PART_MISS
				||	errorType == CLayoutObjectEvaluationError::TYPE_INVENT)
			{
				CUniString regId = err->GetLayoutObject();
				CLayoutObject * region = m_Results->GetDocumentLayoutObject(regId, errorType != CLayoutObjectEvaluationError::TYPE_INVENT);

				if (region->GetLayoutObjectType() != CLayoutObject::TYPE_LAYOUT_REGION
					||	(((CLayoutRegion*)region)->GetType() & m_LayoutRegionType) != 0) //Type filter
				{
					//Get type and subtype
					int regionType = region->GetLayoutObjectType();
					CUniString subType;

					if (regionType == CLayoutObject::TYPE_LAYOUT_REGION)
					{
						CLayoutRegion * layoutReg = (CLayoutRegion*)region;
						layoutRegionType1 = layoutReg->GetType();
						subType = CLayoutEvaluation::GetLayoutRegionSubtype(layoutReg, m_Profile);
					}

					//Error Type Weight
					if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP)
						weight = m_Profile->GetErrorTypeWeightForReadingOrderGroup(errorType);
					else if (m_Results->GetLayoutObjectType() != CLayoutObject::TYPE_LAYOUT_REGION) //Text line, word, glyph
						weight = m_Profile->GetErrorTypeWeightForTextSubStructure(errorType, regionType);
					else if (subType.IsEmpty()) //Layout region without subtype
						weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, false);
					else //Text region
						weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, subType, false);

					//Region Type Weight
					if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
					{
						if (subType.IsEmpty())
							weight *= m_Profile->GetRegionTypeWeight(layoutRegionType1);
						else
							weight *= m_Profile->GetRegionTypeWeight(layoutRegionType1, subType);
					}

					//Nested
					if (err->IsForNestedRegion())
						weight *= weightForNestedRegions;

					//Area error
					if (m_UsePixelArea)
						weightedArea = weight * err->GetPixelCount();
					else
						weightedArea = weight * err->GetArea();

					//Count error
					if (errorType == CLayoutObjectEvaluationError::TYPE_SPLIT)
						weightedCount = weight * err->GetCount();
					else
						weightedCount = weight;

					err->SetWeightedAreaError(weightedArea);
					err->SetWeightedCountError(weightedCount);
					m_OverallWeightedAreaError += weightedArea;
					m_OverallWeightedCountError += weightedCount;
				}
			}
			//Split
			else if (errorType == CLayoutObjectEvaluationError::TYPE_SPLIT)
			{
				CEvaluationErrorSplit * split = (CEvaluationErrorSplit*)err;
				allowable = split->IsAllowable();
				CUniString regId = err->GetLayoutObject();
				CLayoutObject * region = m_Results->GetDocumentLayoutObject(regId, true);

				if (region->GetLayoutObjectType() != CLayoutObject::TYPE_LAYOUT_REGION
					||	(((CLayoutRegion*)region)->GetType() & m_LayoutRegionType) != 0) //Type filter
				{
					//Get type and subtype
					int regionType = region->GetLayoutObjectType();
					CUniString subType;
					if (regionType == CLayoutObject::TYPE_LAYOUT_REGION)
					{
						CLayoutRegion * layoutReg = (CLayoutRegion*)region;
						layoutRegionType1 = layoutReg->GetType();
						subType = CLayoutEvaluation::GetLayoutRegionSubtype(layoutReg, m_Profile);
					}

					//Relative split area (largest overlap area divided by total overlap area)
					double relativeSplitArea = 0.0;
					double maxOverlapArea = 0.0;
					double totalOverlaArea = 0.0;
					COverlapRects * splittingRegions = split->GetSplittingRegions();
					vector<CUniString> * regions = splittingRegions->GetRegions();
					for (unsigned int i=0; i<regions->size(); i++)
					{
						CUniString regId = regions->at(i);
						//CLayoutObject * region = regions->at(i);
						double currArea = m_UsePixelArea ? splittingRegions->GetOverlapPixelCount(regId)
														 : splittingRegions->GetOverlapArea(regId);
						if (currArea > maxOverlapArea)
							maxOverlapArea = currArea;
						totalOverlaArea += currArea;
					}
					if (totalOverlaArea > 0.0)
						relativeSplitArea = maxOverlapArea / totalOverlaArea;
		
					//Ground-truth region area
					double groundTruthRegionArea = m_UsePixelArea ? m_Results->GetPixelCount(region->GetId(), true)
																  : m_Results->GetRegionArea(region->GetId(), true);

					//Error Type Weight
					if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP)
						weight = m_Profile->GetErrorTypeWeightForReadingOrderGroup(errorType);
					else if (m_Results->GetLayoutObjectType() != CLayoutObject::TYPE_LAYOUT_REGION) //Text line, word, glyph
						weight = m_Profile->GetErrorTypeWeightForTextSubStructure(errorType, regionType);
					else if (subType.IsEmpty()) //Layout region without subtype
						weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, allowable);
					else //Text region
						weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, subType, allowable);
					//Region Type Weight
					if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
					{
						if (subType.IsEmpty())
							weight *= m_Profile->GetRegionTypeWeight(layoutRegionType1);
						else
							weight *= m_Profile->GetRegionTypeWeight(layoutRegionType1, subType);
					}

					//Area error (includes count as well)
					weightedArea = weight * (1.0-relativeSplitArea) 
									* (log((double)err->GetCount()) + 0.31)		//natural logarithm (+0.31 so that the minimum (split into 2 regions) is 1.0 (ln(2)=0.69))
									* groundTruthRegionArea;

					//Count error
					weightedCount = weight * err->GetCount();

					err->SetWeightedAreaError(weightedArea);
					err->SetWeightedCountError(weightedCount);
					m_OverallWeightedAreaError += weightedArea;
					m_OverallWeightedCountError += weightedCount;
				}
			}
			//Merge
			else if (errorType == CLayoutObjectEvaluationError::TYPE_MERGE)
			{
				CEvaluationErrorMerge * merge = (CEvaluationErrorMerge*)err;

				CUniString regId = merge->GetLayoutObject();
				CLayoutObject * region1 = m_Results->GetDocumentLayoutObject(regId, true);

				if (region1->GetLayoutObjectType() != CLayoutObject::TYPE_LAYOUT_REGION
					||	(((CLayoutRegion*)region1)->GetType() & m_LayoutRegionType) != 0) //Type filter
				{
					//Get type and subtype of region 1
					int regionType1 = region1->GetLayoutObjectType();
					CUniString subType1;
					if (regionType1 == CLayoutObject::TYPE_LAYOUT_REGION)
					{
						CLayoutRegion * layoutReg = (CLayoutRegion*)region1;
						layoutRegionType1 = layoutReg->GetType();
						subType1 = CLayoutEvaluation::GetLayoutRegionSubtype(layoutReg, m_Profile);
					}

					//Get the seperate error for each merged region
					map<CUniString, COverlapRects *> * mergingRegions = merge->GetMergingRegions();

					map<CUniString, COverlapRects *>::iterator itMergingRegions = mergingRegions->begin();
					while (itMergingRegions != mergingRegions->end())
					{
						COverlapRects * overlap = (*itMergingRegions).second;
						vector<CUniString> * regions2 = overlap->GetRegions();

						double overlapWeight = 0.0;
						overlapWeight = 1.0 / ((double)regions2->size()); // - 1.0);  //CC 21.7.10 The -1 is not needed anymore
						//if (m_UsePixelArea)
						//	overlapWeight = (double)overlap->GetOverlapPixelCount(region1) / (double)overlap->GetPixelCount();
						//else
						//	overlapWeight = (double)overlap->GetOverlapArea(region1) / (double)overlap->GetArea();

						for (unsigned int i=0; i<regions2->size() ;i++)
						{
							CUniString regId2 = regions2->at(i);
							CLayoutObject * region2 = m_Results->GetDocumentLayoutObject(regId2, true);

							if (region2 == region1) //region1 isn't used for the error
								continue;

							allowable = merge->IsAllowable(regId2);

							//Get type and subtype of region 2
							int regionType2 = region2->GetLayoutObjectType();
							int layoutRegionType2 = CLayoutRegion::TYPE_INVALID;
							CUniString subType2;
							if (regionType2 == CLayoutObject::TYPE_LAYOUT_REGION)
							{
								CLayoutRegion * layoutReg = (CLayoutRegion*)region2;
								layoutRegionType2 = layoutReg->GetType();
								subType2 = CLayoutEvaluation::GetLayoutRegionSubtype(layoutReg, m_Profile);
							}

							//Error Type Weight
							if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP)
								weight = m_Profile->GetErrorTypeWeightForReadingOrderGroup(errorType);
							else if (m_Results->GetLayoutObjectType() != CLayoutObject::TYPE_LAYOUT_REGION) //Text line, word, glyph
								weight = m_Profile->GetErrorTypeWeightForTextSubStructure(errorType, regionType1);
							else if (!subType1.IsEmpty() && !subType2.IsEmpty())
								weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, subType1, 
																		layoutRegionType2, subType2, allowable);
							else if (!subType1.IsEmpty())
								weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, subType1, 
																		layoutRegionType2, allowable);
							else if (!subType2.IsEmpty())
								weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, 
																		layoutRegionType2, subType2, allowable);
							else
								weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, 
																		layoutRegionType2, allowable);
							//Region Type Weight
							if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
							{
								if (subType1.IsEmpty())
									weight *= m_Profile->GetRegionTypeWeight(layoutRegionType1);
								else
									weight *= m_Profile->GetRegionTypeWeight(layoutRegionType1, subType1);
							}

							//Apply overlap weight
							//  The overlap weight is the 1 / (number of involved regions - 1)
							//  The overlap weight is used to avoid double penalizing merge errors 
							//  of merges with many regions.
							weight *= overlapWeight;

							//Area error
							if (m_UsePixelArea)
							{
								weightedArea += weight * overlap->GetOverlapPixelCount(region2->GetId());
							}
							else
							{
								weightedArea += weight * overlap->GetOverlapArea(region2->GetId());
							}

							//Count error
							weightedCount += weight;

						}
						delete regions2;

						itMergingRegions++;
					}

					//Set the weighted error
					err->SetWeightedAreaError(weightedArea);
					err->SetWeightedCountError(weightedCount);
					m_OverallWeightedAreaError += weightedArea;
					m_OverallWeightedCountError += weightedCount;
				}
			}
			//Misclassification
			else if (errorType == CLayoutObjectEvaluationError::TYPE_MISCLASS
				&& 	(m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION //Misclassification only for layout regions
				  || m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP)) //or reading order groups
			{
				CEvaluationErrorMisclass * misclass = (CEvaluationErrorMisclass*)err;

				CUniString regId = misclass->GetLayoutObject();
				CLayoutObject * region1 = m_Results->GetDocumentLayoutObject(regId, true);

				if (region1->GetLayoutObjectType() != CLayoutObject::TYPE_LAYOUT_REGION
					||	(((CLayoutRegion*)region1)->GetType() & m_LayoutRegionType) != 0) //Type filter
				{
					//Get type and subtype of region 1
					int regionType1 = region1->GetLayoutObjectType();
					CUniString subType1;
					if (regionType1 == CLayoutObject::TYPE_LAYOUT_REGION)
					{
						CLayoutRegion * layoutReg = (CLayoutRegion*)region1;
						layoutRegionType1 = layoutReg->GetType();
						subType1 = CLayoutEvaluation::GetLayoutRegionSubtype(layoutReg, m_Profile);
					}

					//Get the seperate error for each misclassified region
					COverlapRects * overlap = misclass->GetMisclassRegions();
					vector<CUniString> * regions2 = overlap->GetRegions();

					for (unsigned int i=0; i<regions2->size() ;i++)
					{
						CUniString regId2 = regions2->at(i);
						CLayoutObject * region2 = m_Results->GetDocumentLayoutObject(regId2, false);
						//Get type and subtype of region 2
						int regionType2 = region2->GetLayoutObjectType();
						int layoutRegionType2 = 0;
						CUniString subType2;
						if (regionType2 == CLayoutObject::TYPE_LAYOUT_REGION)
						{
							CLayoutRegion * layoutReg = (CLayoutRegion*)region2;
							layoutRegionType2 = layoutReg->GetType();
							subType2 = CLayoutEvaluation::GetLayoutRegionSubtype(layoutReg, m_Profile);
						}

						//Error Type Weight
						if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP)
							weight = m_Profile->GetErrorTypeWeightForReadingOrderGroup(errorType);
						else //Layout region
						{
							if (!subType1.IsEmpty() && !subType2.IsEmpty())
								weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, subType1,
									layoutRegionType2, subType2, allowable);
							else if (!subType1.IsEmpty())
								weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1, subType1,
									layoutRegionType2, allowable);
							else if (!subType2.IsEmpty())
								weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1,
									layoutRegionType2, subType2, allowable);
							else
								weight = m_Profile->GetErrorTypeWeight(errorType, layoutRegionType1,
									layoutRegionType2, allowable);
						}
						//Region Type Weight
						if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
						{
							if (subType1.IsEmpty())
								weight *= m_Profile->GetRegionTypeWeight(layoutRegionType1);
							else
								weight *= m_Profile->GetRegionTypeWeight(layoutRegionType1, subType1);
						}

						//Area error
						if (m_UsePixelArea)
						{
							weightedArea += weight * overlap->GetOverlapPixelCount(region2->GetId());
						}
						else
						{
							weightedArea += weight * overlap->GetOverlapArea(region2->GetId());
						}

						//Count error
						weightedCount += weight;
					}
					delete regions2;

					//Set the weighted error
					err->SetWeightedAreaError(weightedArea);
					err->SetWeightedCountError(weightedCount);
					m_OverallWeightedAreaError += weightedArea;
					m_OverallWeightedCountError += weightedCount;
				}
			}
		}

		//Overall area error per error type
		map<int, double>::iterator itErr = m_OverallWeightedAreaErrorPerErrorType.find(errorType);
		if (itErr == m_OverallWeightedAreaErrorPerErrorType.end()) //Not found
			m_OverallWeightedAreaErrorPerErrorType.insert(pair<int, double>(errorType, weightedArea));
		else
			(*itErr).second += weightedArea;
		//Overall count error per error type
		itErr = m_OverallWeightedCountErrorPerErrorType.find(errorType);
		if (itErr == m_OverallWeightedCountErrorPerErrorType.end()) //Not found
			m_OverallWeightedCountErrorPerErrorType.insert(pair<int, double>(errorType, weightedCount));
		else
			(*itErr).second += weightedCount;

		if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
		{
			//Overall area error per region type
			itErr = m_OverallWeightedAreaErrorPerRegionType.find(layoutRegionType1);
			if (itErr == m_OverallWeightedAreaErrorPerRegionType.end()) //Not found
				m_OverallWeightedAreaErrorPerRegionType.insert(pair<int, double>(layoutRegionType1, weightedArea));
			else
				(*itErr).second += weightedArea;
			//Overall count error per region type
			itErr = m_OverallWeightedCountErrorPerRegionType.find(layoutRegionType1);
			if (itErr == m_OverallWeightedCountErrorPerRegionType.end()) //Not found
				m_OverallWeightedCountErrorPerRegionType.insert(pair<int, double>(layoutRegionType1, weightedCount));
			else
				(*itErr).second += weightedCount;
		}
	}

	//Area Success Rates
	map<int, double>::iterator itErr = m_OverallWeightedAreaErrorPerErrorType.begin();
	while (itErr != m_OverallWeightedAreaErrorPerErrorType.end())
	{
		int errorType = (*itErr).first;
		double value = (*itErr).second;
		CFunction * f = GetAreaSuccessFunction(errorType);
		double successRate = f->GetY(value);
		delete f;

		m_WeightedAreaSuccessRatePerType.insert(pair<int, double>(errorType, successRate));

		itErr++;
	}
	//Count Success Rates
	itErr = m_OverallWeightedCountErrorPerErrorType.begin();
	while (itErr != m_OverallWeightedCountErrorPerErrorType.end())
	{
		int errorType = (*itErr).first;
		double value = (*itErr).second;
		CFunction * f = GetCountSuccessFunction(errorType);
		double successRate = f->GetY(value);
		delete f;

		m_WeightedCountSuccessRatePerType.insert(pair<int, double>(errorType, successRate));

		itErr++;
	}
}

//Reading order tree error
void CLayoutObjectEvaluationMetrics::CalculateReadingOrderError()
{
	if (m_LayoutRegionType != CLayoutRegion::TYPE_ALL) //Type filter
	{
		m_ReadingOrderError = 0;
		m_ReadingOrderSuccessRate = 1.0;
		return;
	}

	CReadingOrderEvaluationResult * res = m_Results->GetReadingOrderResults();

	//Combined Error
	m_ReadingOrderError = 0.0;
	if (res != NULL)
	{
		for (int i=0; i<res->GetErrorCount(); i++)
			m_ReadingOrderError += res->GetError(i)->GetPenalty();
	}

	//Success Rate
	// Count text regions
	int count = 0;
	CLayoutObjectIterator * it = CLayoutObjectIterator::GetLayoutObjectIterator(m_Results->GetLayoutEvaluation()->GetGroundTruth(), CLayoutObject::TYPE_LAYOUT_REGION, CLayoutRegion::TYPE_TEXT);
	if (it != NULL)
	{
		while (it->HasNext())
		{
			count++;
			it->Next();
		}
		count = max(1,count);
		delete it;
	}

	double fiftyPercentX;
	fiftyPercentX = count * m_Profile->GetReadingOrderPenalties()->GetMaxPenalty() / 2;
	CFunction * f = new CInverseSuccessRate(fiftyPercentX);
	m_ReadingOrderSuccessRate = f->GetY(m_ReadingOrderError);
	delete f;
}

/*
 * Simple counting of errors without applying weights.
 */
void CLayoutObjectEvaluationMetrics::CalculateSimpleCountBasedErrorRates()
{
	map<CUniString, CLayoutObjectEvaluationError*> * errmap = NULL;
	int count = 0;
	int maximum = 0;

	//False Detection
	errmap = m_Results->GetRegionsForErrorType(CLayoutObjectEvaluationError::TYPE_INVENT);
	count = 0;
	if (errmap != NULL)
	{
		if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
			count = (int)errmap->size();
		else //special layout region type (image, table, ...)
		{
			map<CUniString, CLayoutObjectEvaluationError*>::iterator it = errmap->begin();
			while (it != errmap->end())
			{
				CLayoutObject * region = m_Results->GetDocumentLayoutObject((*it).first, false);
				if (region->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION
					&& (((CLayoutRegion*)region)->GetType() & m_LayoutRegionType) != 0)
					count++;
				it++;
			}
		}
	}
	maximum = m_ImageArea / 100; //TODO make configurable
	if (count > maximum)
		count = maximum;
	maximum = max(maximum, 1);
	//TODO use logarithmic function
	m_ErrorRatePerTypeBasedOnSimpleCount.insert(pair<int,double>(CLayoutObjectEvaluationError::TYPE_INVENT, 
											(double)count / (double)maximum));
	
	//Merge
	errmap = m_Results->GetRegionsForErrorType(CLayoutObjectEvaluationError::TYPE_MERGE);
	count = 0;
	if (errmap != NULL)
	{
		if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
			count = (int)errmap->size();
		else //special layout region type (image, table, ...)
		{
			map<CUniString, CLayoutObjectEvaluationError*>::iterator it = errmap->begin();
			while (it != errmap->end())
			{
				CLayoutObject * region = m_Results->GetDocumentLayoutObject((*it).first, true);
				if (region->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION
					&& (((CLayoutRegion*)region)->GetType() & m_LayoutRegionType) != 0)
					count++;
				it++;
			}
		}
	}
	maximum = max(m_NumberOfGroundTruthRegions, 1); //Is fixed
	m_ErrorRatePerTypeBasedOnSimpleCount.insert(pair<int,double>(CLayoutObjectEvaluationError::TYPE_MERGE, 
											(double)count / (double)maximum));

	//Misclass
	if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION  //Only on block level (not line, word or glyph)
		|| m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP) //Or group
	{
		errmap = m_Results->GetRegionsForErrorType(CLayoutObjectEvaluationError::TYPE_MISCLASS);
		count = 0;
		if (errmap != NULL)
		{
			if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
				count = (int)errmap->size();
			else //special layout region type (image, table, ...)
			{
				map<CUniString, CLayoutObjectEvaluationError*>::iterator it = errmap->begin();
				while (it != errmap->end())
				{
					CLayoutObject * region = m_Results->GetDocumentLayoutObject((*it).first, true);
					if (region->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION
						&& (((CLayoutRegion*)region)->GetType() & m_LayoutRegionType) != 0)
						count++;
					it++;
				}
			}
		}
		maximum = max(m_NumberOfGroundTruthRegions, 1); //Is fixed
		m_ErrorRatePerTypeBasedOnSimpleCount.insert(pair<int,double>(CLayoutObjectEvaluationError::TYPE_MISCLASS, 
												(double)count / (double)maximum));
	}

	//Miss
	errmap = m_Results->GetRegionsForErrorType(CLayoutObjectEvaluationError::TYPE_MISS);
	count = 0;
	if (errmap != NULL)
	{
		if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
			count = (int)errmap->size();
		else //special layout region type (image, table, ...)
		{
			map<CUniString, CLayoutObjectEvaluationError*>::iterator it = errmap->begin();
			while (it != errmap->end())
			{
				CLayoutObject * region = m_Results->GetDocumentLayoutObject((*it).first, true);
				if (region->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION
					&& (((CLayoutRegion*)region)->GetType() & m_LayoutRegionType) != 0)
					count++;
				it++;
			}
		}
	}
	maximum = max(m_NumberOfGroundTruthRegions, 1); //Is fixed
	m_ErrorRatePerTypeBasedOnSimpleCount.insert(pair<int,double>(CLayoutObjectEvaluationError::TYPE_MISS, 
											(double)count / (double)maximum));

	//Partial Miss
	errmap = m_Results->GetRegionsForErrorType(CLayoutObjectEvaluationError::TYPE_PART_MISS);
	count = 0;
	if (errmap != NULL)
	{
		if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
			count = (int)errmap->size();
		else //special layout region type (image, table, ...)
		{
			map<CUniString, CLayoutObjectEvaluationError*>::iterator it = errmap->begin();
			while (it != errmap->end())
			{
				CLayoutObject * region = m_Results->GetDocumentLayoutObject((*it).first, true);
				if (region->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION
					&& (((CLayoutRegion*)region)->GetType() & m_LayoutRegionType) != 0)
					count++;
				it++;
			}
		}
	}
	maximum = max(m_NumberOfGroundTruthRegions, 1); //Is fixed
	m_ErrorRatePerTypeBasedOnSimpleCount.insert(pair<int,double>(CLayoutObjectEvaluationError::TYPE_PART_MISS, 
											(double)count / (double)maximum));

	//Split
	errmap = m_Results->GetRegionsForErrorType(CLayoutObjectEvaluationError::TYPE_SPLIT);
	count = 0;
	if (errmap != NULL)
	{
		if (m_LayoutRegionType == CLayoutRegion::TYPE_ALL)
			count = (int)errmap->size();
		else //special layout region type (image, table, ...)
		{
			map<CUniString, CLayoutObjectEvaluationError*>::iterator it = errmap->begin();
			while (it != errmap->end())
			{
				CLayoutObject * region = m_Results->GetDocumentLayoutObject((*it).first, true);
				if (region->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION
					&& (((CLayoutRegion*)region)->GetType() & m_LayoutRegionType) != 0)
					count++;
				it++;
			}
		}
	}
	maximum = max(m_NumberOfGroundTruthRegions, 1); //Is fixed
	m_ErrorRatePerTypeBasedOnSimpleCount.insert(pair<int,double>(CLayoutObjectEvaluationError::TYPE_SPLIT, 
											(double)count / (double)maximum));

	//Overall
	map<int,double>::iterator it = m_ErrorRatePerTypeBasedOnSimpleCount.begin();
	double overall = 0.0;
	while (it != m_ErrorRatePerTypeBasedOnSimpleCount.end())
	{
		overall += (*it).second;
		it++;
	}
	m_ErrorRateBasedOnSimpleCount = overall / m_ErrorRatePerTypeBasedOnSimpleCount.size();
}

/*
 * Artihmetic and harmonic mean of all error rates
 */
void CLayoutObjectEvaluationMetrics::CalculateOverallSuccessRate()
{
	double readingOrderWeight = m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION
									? m_Profile->GetReadingOrderWeight()
									: 0.0;	//No reading order for text lines, words or glyphs

	vector<int> errorTypes;
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_INVENT);
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_MERGE);
	if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION //only on block level (not for lines, words or glyphs)
		|| m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP) //Or group
	{
		errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_MISCLASS);
	}
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_MISS);
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_PART_MISS);
	errorTypes.push_back(CLayoutObjectEvaluationError::TYPE_SPLIT);

	//Weights for the error types
	vector<double> errorTypeCountWeights; //(same order as errorTypes vector!)
	vector<double> errorTypeAreaWeights; 
	double relCount, relArea;
	double sumCountWeights = 0.0;
	double sumAreaWeights = 0.0;
	// False detection
	GetRelativeAreaAndCountPerErrorType(CLayoutObjectEvaluationError::TYPE_INVENT, relCount, relArea);
	errorTypeCountWeights.push_back(relCount);	//Allways 1.0
	errorTypeAreaWeights.push_back(relArea);	
	// Merge
	GetRelativeAreaAndCountPerErrorType(CLayoutObjectEvaluationError::TYPE_MERGE, relCount, relArea);
	errorTypeCountWeights.push_back(relCount);
	errorTypeAreaWeights.push_back(relArea);
	// Misclassification
	if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION //only on block level (not for lines, words or glyphs)
		|| m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP) //Or group
	{
		GetRelativeAreaAndCountPerErrorType(CLayoutObjectEvaluationError::TYPE_MISCLASS, relCount, relArea);
		errorTypeCountWeights.push_back(relCount);
		errorTypeAreaWeights.push_back(relArea);
	}
	// Miss
	GetRelativeAreaAndCountPerErrorType(CLayoutObjectEvaluationError::TYPE_MISS, relCount, relArea);
	errorTypeCountWeights.push_back(relCount);
	errorTypeAreaWeights.push_back(relArea);
	// Partial Miss
	GetRelativeAreaAndCountPerErrorType(CLayoutObjectEvaluationError::TYPE_PART_MISS, relCount, relArea);
	errorTypeCountWeights.push_back(relCount);
	errorTypeAreaWeights.push_back(relArea);
	// Split
	GetRelativeAreaAndCountPerErrorType(CLayoutObjectEvaluationError::TYPE_SPLIT, relCount, relArea);
	errorTypeCountWeights.push_back(relCount);
	errorTypeAreaWeights.push_back(relArea);

	// Sum
	for (unsigned int i=0; i<errorTypeCountWeights.size(); i++)
		sumCountWeights += errorTypeCountWeights[i];
	sumCountWeights += readingOrderWeight;
	for (unsigned int i=0; i<errorTypeAreaWeights.size(); i++)
		sumAreaWeights += errorTypeAreaWeights[i];
	sumAreaWeights += readingOrderWeight;


	//====== Weighted Arithmetic Mean (average) ======
	//*** Area Error ***
	m_OverallWeightedAreaSuccessRate = 0.0;

	//Sum up all error rates
	for (unsigned int i=0; i<errorTypes.size(); i++)
	{
		m_OverallWeightedAreaSuccessRate += GetWeightedAreaSuccessRatePerType(errorTypes[i])
												* errorTypeAreaWeights[i];
	}
	m_OverallWeightedAreaSuccessRate += readingOrderWeight * m_ReadingOrderSuccessRate;
	//Divide by count (sum of the weights)
	m_OverallWeightedAreaSuccessRate = m_OverallWeightedAreaSuccessRate / sumAreaWeights;

	//*** Count Error ***
	m_OverallWeightedCountSuccessRate = 0.0;

	//Sum up all error rates
	for (unsigned int i=0; i<errorTypes.size(); i++)
	{
		m_OverallWeightedCountSuccessRate += GetWeightedCountSuccessRatePerType(errorTypes[i])
												* errorTypeCountWeights[i];
	}
	m_OverallWeightedCountSuccessRate += readingOrderWeight * m_ReadingOrderSuccessRate;
	//Divide by count
	m_OverallWeightedCountSuccessRate = m_OverallWeightedCountSuccessRate / sumCountWeights;


	//====== Weighted Harmonic Mean ======
	//*** Area Error ***
	double sumRates = 0.0;
	bool oneIsZero = false;
	for (unsigned int i=0; i<errorTypes.size(); i++)
	{
		if (GetWeightedAreaSuccessRatePerType(errorTypes[i]) <= 0.0)
		{
			oneIsZero = true;
			break;
		}
		sumRates += errorTypeAreaWeights[i] / GetWeightedAreaSuccessRatePerType(errorTypes[i]);
	}
	if (m_ReadingOrderSuccessRate <= 0.0)
		oneIsZero = true;
	else
		sumRates += readingOrderWeight / m_ReadingOrderSuccessRate;

	if (oneIsZero)
		m_HarmonicWeightedAreaSuccessRate = 0.0;
	else
		m_HarmonicWeightedAreaSuccessRate = sumAreaWeights / sumRates;

	//*** Count Error ***
	sumRates = 0.0;
	oneIsZero = false;
	for (unsigned int i=0; i<errorTypes.size(); i++)
	{
		if (GetWeightedCountSuccessRatePerType(errorTypes[i]) <= 0.0)
		{
			oneIsZero = true;
			break;
		}
		sumRates += errorTypeCountWeights[i] / GetWeightedCountSuccessRatePerType(errorTypes[i]);
	}
	if (m_ReadingOrderSuccessRate <= 0.0)
		oneIsZero = true;
	else
		sumRates += readingOrderWeight / m_ReadingOrderSuccessRate;

	if (oneIsZero)
		m_HarmonicWeightedCountSuccessRate = 0.0;
	else
		m_HarmonicWeightedCountSuccessRate = sumCountWeights / sumRates;
}

/*
 * Caclulates the influence of the specified error type on the overall success rates.
 *
 * 'errorType' (in) - merge, split, ... (see CLayoutObjectEvaluationError::TYPE_...)
 * 'relativeCount' (out) - number of ground-truth regions with an error of the specified type 
 *                         divided by the overall number of ground-truth regions.
 * 'relativeArea' (out) - total area of ground-truth regions with an error of the specified type 
 *                        divided by the total area of all ground-truth regions.
 */
void CLayoutObjectEvaluationMetrics::GetRelativeAreaAndCountPerErrorType(int errorType, 
															 double & relativeCount, 
															 double & relativeArea)
{
	map<CUniString, CLayoutObjectEvaluationError*> * errmap = m_Results->GetRegionsForErrorType(errorType);
	relativeCount = 0.0;
	relativeArea = 0.0;

	relativeCount = 1.0 - GetWeightedCountSuccessRatePerType(errorType);
	relativeArea = 1.0 - GetWeightedAreaSuccessRatePerType(errorType);

	//Relativate the relative values (so that there is a minimum of 1/6 influence)
	relativeCount = (5*relativeCount + 1) / 6;
	relativeArea = (5*relativeArea + 1) / 6;

	SetWeightedCountInfluencePerType(errorType, relativeCount);
	SetWeightedAreaInfluencePerType(errorType, relativeArea);
}

/*
 * Recall and Precision
 *
 * Recall (of text regions) = Number of pixels within ground-truth text regions
 *                            that are also in a text region within the segmentation
 *                            result divided by the overall number of pixels in
 *                            ground-truth text regions.
 *
 * Precision (text region)  = Number of pixels within ground-truth text regions
 *                            that are also in a text region within the segmentation
 *                            result divided by the overall number of pixels in
 *                            segmentation result text regions.
 */
void CLayoutObjectEvaluationMetrics::CalculateRecallAndPrecision()
{
	if (m_LayoutRegionType != CLayoutRegion::TYPE_ALL) //ignore for region type specific metrics
	{
		m_RecallNonStrict = -1.0;
		m_PrecisionNonStrict = -1.0;
		m_RecallStrict = -1.0;
		m_PrecisionStrict = -1.0;
		return;
	}

	COpenCvBiLevelImage * image = m_Results->GetLayoutEvaluation()->GetBilevelImage();
	CLayoutObjectIterator * it = CLayoutObjectIterator::GetLayoutObjectIterator(m_Results->GetLayoutEvaluation()->GetGroundTruth(),
																m_Results->GetLayoutObjectType());
	int overallRecallAreaNonStrict = 0;
	CLayoutObject * groundTruthReg;
	int gtLayoutRegType;
	//Iterate over all regions
	while (it->HasNext())
	{
		groundTruthReg = it->Next();
		gtLayoutRegType = CLayoutRegion::TYPE_INVALID;
		if (groundTruthReg->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
			gtLayoutRegType = ((CLayoutRegion*)groundTruthReg)->GetType();

		//Area
		if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION) //only on block level
		{
			map<int,int>::iterator itArea = m_GroundTruthRegionAreaPerType.find(gtLayoutRegType);
			if (itArea == m_GroundTruthRegionAreaPerType.end()) //not in map yet
				m_GroundTruthRegionAreaPerType.insert(pair<int,int>(gtLayoutRegType, 
																m_Results->GetRegionArea(groundTruthReg->GetId(), true)));
			else //already in map
				(*itArea).second += m_Results->GetRegionArea(groundTruthReg->GetId(), true);
			//Pixel count
			if (m_UsePixelArea)
			{
				map<int,int>::iterator itCount = m_GroundTruthRegionPixelCountPerType.find(gtLayoutRegType);
				if (itCount == m_GroundTruthRegionPixelCountPerType.end()) //not in map yet
					m_GroundTruthRegionPixelCountPerType.insert(pair<int,int>(gtLayoutRegType, 
																			m_Results->GetPixelCount(groundTruthReg->GetId(), true)));
				else //already in map
					(*itCount).second += m_Results->GetPixelCount(groundTruthReg->GetId(), true);
			}
		}

		//Get overlapping segmentation result regions
		set<CUniString> * overlappingRegions = m_Results->GetGroundTruthOverlaps(groundTruthReg->GetId());

		if (overlappingRegions != NULL && overlappingRegions->size() > 0)
		{
			CLayoutObjectOverlap * overlap = NULL;
			if (overlappingRegions->size() == 1) //only one region overlaps -> look in the single overlap map
			{
				CUniString segResRegionId = (*overlappingRegions->begin());
				CLayoutObject * segResultRegion = m_Results->GetDocumentLayoutObject(segResRegionId, false);
				overlap = m_Results->GetOverlapIntervalRep(groundTruthReg->GetId(), segResultRegion->GetId());
			}
			else //more than one region overlap -> look in the multi overlap map
			{
				overlap = m_Results->GetMultiOverlapIntervalRep(groundTruthReg->GetId());
			}
			if (overlap != NULL)
			{
				//strict
				if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION) //only on block level
				{
					vector<CRect *> * rects = overlap->GetRecalledRects(groundTruthReg, true);
					//Area
					int area = 0;
					if (rects != NULL && !rects->empty()) //Found recalled intervall rects
					{
						for (unsigned int i=0; i<rects->size(); i++)
							area += (rects->at(i)->Width()+1) * (rects->at(i)->Height()+1);
					}
					map<int,int>::iterator itArea = m_RecallAreaPerType.find(gtLayoutRegType);
					if (itArea == m_RecallAreaPerType.end()) //not in map yet
						m_RecallAreaPerType.insert(pair<int,int>(gtLayoutRegType, area));
					else //already in map
						(*itArea).second += area;

					//Pixel Count
					if (m_UsePixelArea)
					{
						int pixelCount = 0;
						if (rects != NULL && !rects->empty()) //Found recalled intervall rects
						{
							if (image != NULL)
								pixelCount = image->CountPixels(rects, true);
						}
						map<int,int>::iterator itCount = m_RecallPixelCountPerType.find(gtLayoutRegType);
						if (itCount == m_RecallPixelCountPerType.end()) //not in map yet
							m_RecallPixelCountPerType.insert(pair<int,int>(gtLayoutRegType, pixelCount));
						else //already in map
							(*itCount).second += pixelCount;
					}

					for (unsigned int i=0; i<rects->size(); i++)
						delete rects->at(i);
					delete rects;
				}

				//non-strict
				vector<CRect *> * rects = overlap->GetRecalledRects(groundTruthReg, false);
				if (rects != NULL && !rects->empty()) //Found recalled intervall rects
				{
					int area = 0;
					if (m_UsePixelArea) //Pixel Count
					{
						if (image != NULL)
							area = image->CountPixels(rects, true);
					}
					else //Area
					{
						for (unsigned int i=0; i<rects->size(); i++)
							area += (rects->at(i)->Width()+1) * (rects->at(i)->Height()+1);
					}
					overallRecallAreaNonStrict += area;
				}
				for (unsigned int i=0; i<rects->size(); i++)
					delete rects->at(i);
				delete rects;
			}
			else
			{
				//TODO error
			}
		}
	}
	delete it;

	//Area / pixel count per type for segmentation result
	if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION) //only on block level
	{
		it = CLayoutObjectIterator::GetLayoutObjectIterator(m_Results->GetLayoutEvaluation()->GetSegResult(),
												m_Results->GetLayoutObjectType());
		CLayoutObject * segResultReg;
		int segLayoutRegType;
		//Iterate over all regions
		while (it->HasNext())
		{
			segResultReg = it->Next();
			segLayoutRegType = 0;
			if (segResultReg->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
				segLayoutRegType = ((CLayoutRegion*)segResultReg)->GetType();

			//Area
			map<int,int>::iterator itArea = m_SegResultRegionAreaPerType.find(segLayoutRegType);
			if (itArea == m_SegResultRegionAreaPerType.end()) //not in map yet
				m_SegResultRegionAreaPerType.insert(pair<int,int>(segLayoutRegType, 
															m_Results->GetRegionArea(segResultReg->GetId(), false)));
			else //already in map
				(*itArea).second += m_Results->GetRegionArea(segResultReg->GetId(), false);
			//Pixel count
			if (m_UsePixelArea)
			{
				map<int,int>::iterator itCount = m_SegResultRegionPixelCountPerType.find(segLayoutRegType);
				if (itCount == m_SegResultRegionPixelCountPerType.end()) //not in map yet
					m_SegResultRegionPixelCountPerType.insert(pair<int,int>(segLayoutRegType, 
																			m_Results->GetPixelCount(segResultReg->GetId(), false)));
				else //already in map
					(*itCount).second += m_Results->GetPixelCount(segResultReg->GetId(), false);
			}

			//Initialize recall map entries
			map<int,int>::iterator itRecallArea;
			if (m_UsePixelArea)
			{
				itRecallArea = m_RecallPixelCountPerType.find(segLayoutRegType);
				if (itRecallArea == m_RecallPixelCountPerType.end())
					m_RecallPixelCountPerType.insert(pair<int,int>(segLayoutRegType,0));
			}
			else
			{
				itRecallArea = m_RecallAreaPerType.find(segLayoutRegType);
				if (itRecallArea == m_RecallAreaPerType.end())
					m_RecallAreaPerType.insert(pair<int,int>(segLayoutRegType,0));
			}
		}
		delete it;
	}

	//Recall / Precision per type
	int overallRecallAreaStrict = 0;
	if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION) //only on block level
	{
		map<int,int>::iterator itRecallArea;
		if (m_UsePixelArea)
			itRecallArea = m_RecallPixelCountPerType.begin();
		else
			itRecallArea = m_RecallAreaPerType.begin();

		while (m_UsePixelArea && itRecallArea != m_RecallPixelCountPerType.end()
			|| !m_UsePixelArea && itRecallArea != m_RecallAreaPerType.end())
		{
			int layoutRegionType = (*itRecallArea).first;
			int recallArea = (*itRecallArea).second;
			overallRecallAreaStrict += recallArea;

			//Recall
			int totalArea = 0;
			map<int,int>::iterator itTotalArea;

			if (m_UsePixelArea)
			{
				itTotalArea = m_GroundTruthRegionPixelCountPerType.find(layoutRegionType);
				totalArea = itTotalArea == m_GroundTruthRegionPixelCountPerType.end() ? 0 : (*itTotalArea).second;
			}
			else
			{
				itTotalArea = m_GroundTruthRegionAreaPerType.find(layoutRegionType);
				totalArea = itTotalArea == m_GroundTruthRegionAreaPerType.end() ? 0 : (*itTotalArea).second;
			}

			// For region types that are not in the ground-truth
			// the recall is not defined (division by zero)!

			if (totalArea > 0)
			{
				double recall = (double)recallArea / (double)totalArea;
				m_RecallPerType.insert(pair<int,double>(layoutRegionType, recall));
			}

			//Precision
			if (m_UsePixelArea)
			{
				itTotalArea = m_SegResultRegionPixelCountPerType.find(layoutRegionType);
				totalArea = itTotalArea == m_SegResultRegionPixelCountPerType.end() ? 0 : (*itTotalArea).second;
			}
			else
			{
				itTotalArea = m_SegResultRegionAreaPerType.find(layoutRegionType);
				totalArea = itTotalArea == m_SegResultRegionAreaPerType.end() ? 0 : (*itTotalArea).second;
			}

			double precision = 0.0;
			if (totalArea > 0.0)
				precision = (double)recallArea / (double)totalArea;

			bool foundType = m_SegResultRegionAreaPerType.find(layoutRegionType) != m_SegResultRegionAreaPerType.end();

			if (foundType) //Segmentation result has regions of layoutRegionType
				m_PrecisionPerType.insert(pair<int,double>(layoutRegionType, precision));

			itRecallArea++;
		}
	}

	//Overall Recall / Precision
	double overallGroundTruthRegionArea = 0.0;
	double overallSegResultRegionArea = 0.0;
	if (m_UsePixelArea)
	{
		overallGroundTruthRegionArea = (double)m_OverallGroundTruthRegionPixelCount;
		overallSegResultRegionArea = (double)m_OverallSegResultRegionPixelCount;
	}
	else
	{
		overallGroundTruthRegionArea = (double)m_OverallGroundTruthRegionArea;
		overallSegResultRegionArea = (double)m_OverallSegResultRegionArea;
	}
	//Avoid div by zero
	if (overallGroundTruthRegionArea <= 0.0)
		overallGroundTruthRegionArea = 0.0001;
	if (overallSegResultRegionArea <= 0.0)
		overallSegResultRegionArea = 0.0001;

	if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION) //only on block level
	{
		m_RecallStrict = (double)overallRecallAreaStrict / overallGroundTruthRegionArea;
		m_PrecisionStrict = (double)overallRecallAreaStrict / overallSegResultRegionArea;
	}
	m_RecallNonStrict	= (double)overallRecallAreaNonStrict / overallGroundTruthRegionArea;
	m_PrecisionNonStrict = (double)overallRecallAreaNonStrict / overallSegResultRegionArea;

	//F-Measure
	double sum = m_RecallNonStrict + m_PrecisionNonStrict;
	m_FMeasureNonStrict = sum != 0.0 
							? 2 * m_RecallNonStrict * m_PrecisionNonStrict / sum 
							: 0.0;
	if (m_Results->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION) //only on block level
	{
		sum = m_RecallStrict + m_PrecisionStrict;
		m_FMeasureStrict = sum != 0.0 
								? 2 * m_RecallStrict * m_PrecisionStrict / sum 
								: 0.0;
	}
}

/*
 * Calculates the OCR success rate
 * Note: Only implemented for glyph level.
 */
void CLayoutObjectEvaluationMetrics::CalculateOCRSuccessRate()
{
	m_OCRSuccessRate = 0.0;
	m_OCRSuccessRateExclReplacementChar = 0.0;

	//Glyph level?
	if (m_Results->GetLayoutObjectType() != CLayoutObject::TYPE_GLYPH)
		return;

	delete m_GlyphStatistics;
	m_GlyphStatistics = new CGlyphStatistics();
	
	CPageLayout * groundTruth = m_Results->GetLayoutEvaluation()->GetGroundTruth();
	CPageLayout * segResult = m_Results->GetLayoutEvaluation()->GetSegResult();

	//Iterate over all glyphs within the ground truth
	CGlyphIterator it(groundTruth, true);
	CGlyph * glyph1;
	CGlyph * glyph2;

	int glyphCount = 0;
	int matches = 0;

	int glyphCountExclReplacementChar = 0;
	int matchesExclReplacementChar = 0;

	int glyphCountDigits = 0;
	int matchesDigits = 0;

	int glyphCountNumerical = 0;
	int matchesNumerical = 0;

	bool isReplacementChar = false;
	bool isDigit = false;
	bool isNumerical = false;

	while (it.HasNext())
	{
		glyph1 = (CGlyph*)it.Next();

		if (glyph1->GetTextUnicode().IsEmpty()) //Ignore empty glyphs
			continue;

		//Find overlapping glyphs from the segmentation result
		glyph2 = NULL;
		set<CUniString> * overlappingRegions = m_Results->GetGroundTruthOverlaps(glyph1->GetId());
		if (overlappingRegions != NULL)
		{
			//Find the most overlapping glyph
			long maxOverlapArea = 0;
			CGlyph * curr = NULL;
			for (set<CUniString>::iterator it2 = overlappingRegions->begin(); it2 != overlappingRegions->end(); it2++)
			{
				curr = (CGlyph*)segResult->FindLayoutObject(CLayoutObject::TYPE_GLYPH, (*it2));
				if (curr == NULL)
					continue;
				CLayoutObjectOverlap * overlap = m_Results->GetOverlapIntervalRep(glyph1->GetId(), curr->GetId());
				if (overlap != NULL && overlap->GetOverlapArea() > maxOverlapArea)
				{
					maxOverlapArea = overlap->GetOverlapArea();
					glyph2 = curr;
				}
			}
		}

		isReplacementChar = glyph1->GetTextUnicode().Find(L"�") >= 0;
		isDigit = IsDigitCharacter(glyph1->GetTextUnicode());
		isNumerical = IsNumericalCharacter(glyph1->GetTextUnicode());

		//Found matching glyph in segmentation/OCR result
		if (glyph2 != NULL)
		{
			//Compare text content
			if (glyph1->GetTextUnicode() == glyph2->GetTextUnicode())
			{
				matches++;

				if (!isReplacementChar)
					matchesExclReplacementChar++;

				if (isDigit)
					matchesDigits++;

				if (isNumerical)
					matchesNumerical++;
			}
		}

		//Update glyph statistics
		m_GlyphStatistics->AddEntry(glyph1, glyph2);

		glyphCount++;

		if (!isReplacementChar)
			glyphCountExclReplacementChar++;

		if (isDigit)
			glyphCountDigits++;

		if (isNumerical)
			glyphCountNumerical++;
	}

	//Calculate success rates
	if (glyphCount > 0)
		m_OCRSuccessRate = (double)matches / (double)glyphCount;
	
	if (glyphCountExclReplacementChar > 0)
		m_OCRSuccessRateExclReplacementChar = (double)matchesExclReplacementChar / (double)glyphCountExclReplacementChar;

	if (glyphCountDigits > 0)
		m_OCRSuccessRateForDigits = (double)matchesDigits / (double)glyphCountDigits;

	if (glyphCountNumerical > 0)
		m_OCRSuccessRateForNumericalChars = (double)matchesNumerical / (double)glyphCountNumerical;
}

/*
 * Checks if the given character is a digit (0-9). Returns false if the string contains more than one character.
 */
bool CLayoutObjectEvaluationMetrics::IsDigitCharacter(CUniString c)
{
	if (c.GetLength() > 1)
		return false;
	return c.FindOneOf(L"0123456789") >= 0;
}

/*
 * Checks if the given character is numerical (0-9 + - * / ( ) , . ; = % < > : ) 
 * Returns false if the string contains more than one character.
 */
bool CLayoutObjectEvaluationMetrics::IsNumericalCharacter(CUniString c)
{
	if (c.GetLength() > 1)
		return false;
	return c.FindOneOf(L"0123456789+-*/(),.;=%<>:") >= 0;
}

/*
 * Returns the appropriate success rate function for the given error type (area based).
 */
CFunction * CLayoutObjectEvaluationMetrics::GetAreaSuccessFunction(int errorType)
{
	//TODO make configurable
	double param = 500.0;
	if (errorType == CLayoutObjectEvaluationError::TYPE_INVENT)
	{
		if (m_UsePixelArea)
			param = m_ForeGroundPixelCount / 10.0;
		else
			param = m_ImageArea / 10.0;
	}
	else if (errorType == CLayoutObjectEvaluationError::TYPE_MERGE)
	{
		if (m_UsePixelArea)
			param = m_OverallGroundTruthRegionPixelCount / 4.0;
		else
			param = m_OverallGroundTruthRegionArea / 4.0;
	}
	else if (	errorType == CLayoutObjectEvaluationError::TYPE_SPLIT
			||	errorType == CLayoutObjectEvaluationError::TYPE_MISS
			||	errorType == CLayoutObjectEvaluationError::TYPE_PART_MISS
			||	errorType == CLayoutObjectEvaluationError::TYPE_MISCLASS)
	{
		if (m_UsePixelArea)
			param = m_OverallGroundTruthRegionPixelCount / 2.0;
		else
			param = m_OverallGroundTruthRegionArea / 2.0;
	}
	return new CInverseSuccessRate(param);
}

/*
 * Returns the appropriate success rate function for the given error type (count based).
 */
CFunction * CLayoutObjectEvaluationMetrics::GetCountSuccessFunction(int errorType)
{
	//TODO make configurable
	double param = 10.0;
	if (errorType == CLayoutObjectEvaluationError::TYPE_INVENT)
	{
		if (m_MetricsForAllTypes == NULL)
			param = max(m_NumberOfGroundTruthRegions/2.0, 1.0);
		else //Type specific -> use the overall numbers anyway
			param = max(m_MetricsForAllTypes->GetNumberOfGroundTruthRegions()/2.0, 1.0);
	}
	else if (errorType == CLayoutObjectEvaluationError::TYPE_SPLIT)
	{
		//If half of the ground-truth regions are split in two
		//we have 50% error (and 50% success)
		param = m_NumberOfGroundTruthRegions;
	}
	else if (	errorType == CLayoutObjectEvaluationError::TYPE_MERGE)
	{
		//If all ground-truth regions are merged as pairs,
		//we have an count of (#regions). This should be
		//100% merge error. So we use (#regions / 2) as
		//fiftyPercentX.
		param = m_NumberOfGroundTruthRegions / 2.0;
	}
	else if (	errorType == CLayoutObjectEvaluationError::TYPE_MISS
			||	errorType == CLayoutObjectEvaluationError::TYPE_PART_MISS
			||	errorType == CLayoutObjectEvaluationError::TYPE_MISCLASS)
	{
		param = m_NumberOfGroundTruthRegions / 2.0;
	}
	return new CInverseSuccessRate(param);
}


/*
 * Class CLayoutObjectErrorIterator
 *
 *
 * CC 28.06.2010
 */

CLayoutObjectErrorIterator::CLayoutObjectErrorIterator(CEvaluationResults * results)
{
	m_Results = results;
	m_ItType = m_Results->m_ErrorTypeObjectsMap.begin();
	if (m_ItType != m_Results->m_ErrorTypeObjectsMap.end())
	{
		m_ItRegion = (*m_ItType).second->begin();
	}
}

CLayoutObjectErrorIterator::~CLayoutObjectErrorIterator()
{
}

bool CLayoutObjectErrorIterator::HasNext()
{
	return m_ItType != m_Results->m_ErrorTypeObjectsMap.end()
		&& m_ItRegion != (*m_ItType).second->end();
}

CLayoutObjectEvaluationError * CLayoutObjectErrorIterator::GetNext()
{
	if (HasNext())
	{
		CLayoutObjectEvaluationError * ret = (*m_ItRegion).second;
		m_ItRegion++;
		if (m_ItRegion == (*m_ItType).second->end())
		{
			m_ItType++;
			if (m_ItType != m_Results->m_ErrorTypeObjectsMap.end())
			{
				m_ItRegion = (*m_ItType).second->begin();
			}
		}
		return ret;
	}
	else
		return NULL;
}


/*
 * Class CLogSuccessRate
 *
 * Function that returns a success rate (as percentage 0-100) for a given x.
 * Uses a logarithmic function.
 *
 * CC 29.06.2010 - created
 */

/*
 * Constructor
 * 'fiftyPercentX' - x value where the y-value should be 50.
 */
CLogSuccessRate::CLogSuccessRate(double fiftyPercentX)
{
	//Note:
	//Formula: y = a - b*ln(c*(x+d))
	//  ln(1) = 0
	//  ln(0.5) = -0.69
	//  ln(2) = 0.69

	a = 100.0;
	b = 8.0;

	//8*ln(500) = 49.7 (50% point)
	if (fiftyPercentX < 1.0)
		fiftyPercentX = 1.0;
	c = 500.0 / fiftyPercentX;

	d = 1 / c;
}

CLogSuccessRate::~CLogSuccessRate()
{
}

double CLogSuccessRate::GetY(double x)
{
	double y = 100.0;

	double v = c * (x + d);

	if (v > 0.0)
	{
		y = a - b * log(v); //(log is ln)
		if (y < 0.0)
			y = 0.0;
		if (y > 100.0)
			y = 100.0;
	}
	return y;
}


/*
 * Class CInverseSuccessRate
 *
 * Function that returns a success rate (0-1 interpretable as percentage) for a given x.
 * Uses an inverse function (a/x).
 *
 * CC 29.06.2010 - created
 */

/*
 * Constructor
 * 'fiftyPercentX' - x value where the y-value should be 50.
 */
CInverseSuccessRate::CInverseSuccessRate(double fiftyPercentX)
{
	//Note:
	//Formula: y = 1 / (a*x+1)

	//1 / (1+1) = 50 (50% point)
	if (fiftyPercentX < 1.0)
		fiftyPercentX = 1.0;
	a = 1.0 / fiftyPercentX;
}

CInverseSuccessRate::~CInverseSuccessRate()
{
}

double CInverseSuccessRate::GetY(double x)
{
	double y = 1.0;

	double v = a * x + 1;

	if (v > 0.0)
	{
		y = 1.0 / v;
		if (y < 0.0)
			y = 0.0;
		if (y > 1.0)
			y = 1.0;
	}
	return y;
}


}//end namespace