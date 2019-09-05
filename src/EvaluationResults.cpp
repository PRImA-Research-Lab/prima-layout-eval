
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "EvaluationResults.h"

namespace PRImA
{


/*
 * Class CEvaluationResults
 *
 * Evaluation results for a region type (layout region, text line, word, glyph).
 *
 * CC 22.01.2010 - created
 */

/*
 * Constructor
 */
CEvaluationResults::CEvaluationResults(CLayoutEvaluation * layoutEvaluation, 
									   CEvaluationProfile * profile, int layoutObjectType)
{
	m_LayoutEvaluation = layoutEvaluation;
	m_Profile = profile;
	m_LayoutObjectType = layoutObjectType;
	if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
		m_ReadingOrderResults = new CReadingOrderEvaluationResult(this);
	else
		m_ReadingOrderResults = NULL; //No reading order for lines, words, glyphs
	m_Metrics = NULL;
	m_BorderResults = NULL;
}

/*
 * Destructor
 */
CEvaluationResults::~CEvaluationResults()
{
	//Ground truth map
	map<CUniString, set<CUniString>*>::iterator it = m_GroundTruthOverlaps.begin();
	while (it != m_GroundTruthOverlaps.end())
	{
		delete (*it).second;
		it++;
	}

	//Seg result map
	it = m_SegResultOverlaps.begin();
	while (it != m_SegResultOverlaps.end())
	{
		delete (*it).second;
		it++;
	}

	//Overlap Interval Representations
	map<CUniString, map<CUniString, CLayoutObjectOverlap*>*>::iterator gtIt = m_OverlapIntervalReps.begin();
	while (gtIt != m_OverlapIntervalReps.end())
	{
		map<CUniString, CLayoutObjectOverlap*>* segMap = (*gtIt).second;

		map<CUniString, CLayoutObjectOverlap*>::iterator segIt = segMap->begin();
		while (segIt != segMap->end())
		{
			delete (*segIt).second; //Deletes the interval repr.
			segIt++;
		}

		delete segMap; //deletes the map
		gtIt++;
	}

	//Multi Overlap Interval Representations
	map<CUniString, CLayoutObjectOverlap*>::iterator mIt = m_MultiOverlapIntervalReps.begin();
	while (mIt != m_MultiOverlapIntervalReps.end())
	{
		delete (*mIt).second; //Deletes the interval repr.
		mIt++;
	}

	//Interval Representations
	map<CUniString, CIntervalRepresentation*>::iterator intRepIt = m_GroundTruthIntervalReps.begin();
	while (intRepIt != m_GroundTruthIntervalReps.end())
	{
		delete (*intRepIt).second; //Deletes the interval repr.
		intRepIt++;
	}
	intRepIt = m_SegResultIntervalReps.begin();
	while (intRepIt != m_SegResultIntervalReps.end())
	{
		delete (*intRepIt).second; //Deletes the interval repr.
		intRepIt++;
	}

	//Error Type - regions map
	map<int, map<CUniString,CLayoutObjectEvaluationError*>*>::iterator itErrTpReg = m_ErrorTypeObjectsMap.begin();
	while (itErrTpReg != m_ErrorTypeObjectsMap.end())
	{
		delete (*itErrTpReg).second;
		itErrTpReg++;
	}

	//Layout object results
	map<CUniString, CLayoutObjectEvaluationResult*>::iterator itRegRes = m_GroundTruthObjectResults.begin();
	while (itRegRes != m_GroundTruthObjectResults.end())
	{
		delete (*itRegRes).second;
		itRegRes++;
	}
	itRegRes = m_SegResultObjectResults.begin();
	while (itRegRes != m_SegResultObjectResults.end())
	{
		delete (*itRegRes).second;
		itRegRes++;
	}

	//Reading Order Results
	delete m_ReadingOrderResults;

	//Border results
	delete m_BorderResults;

	//Metrics
	delete m_Metrics;

	map<int,CLayoutObjectEvaluationMetrics *>::iterator itMetrics = m_MetricsPerLayoutRegionType.begin();
	while (itMetrics != m_MetricsPerLayoutRegionType.end())
	{
		delete (*itMetrics).second;
		itMetrics++;
	}
}

CBorderEvaluationResults * CEvaluationResults::GetBorderResults(bool create /*= false*/)
{
	if (m_BorderResults == NULL && create)
		m_BorderResults = new CBorderEvaluationResults();
	return m_BorderResults;
}

/*
 * Add an overlap to the object overlap maps.
 */
void CEvaluationResults::AddLayoutObjectOverlap(CUniString groundTruth, CUniString segResult,
										  CLayoutObjectOverlap * overlap)
{
	//Ground truth map
	map<CUniString, set<CUniString>*>::iterator it = m_GroundTruthOverlaps.find(groundTruth);

	set<CUniString> * overlaps = NULL;
	if (it != m_GroundTruthOverlaps.end())
		overlaps = (*it).second;
	else
	{
		overlaps = new set<CUniString>();
		m_GroundTruthOverlaps.insert(pair<CUniString, set<CUniString>*>(groundTruth, overlaps));
	}
	overlaps->insert(segResult);

	//Segmentation result map
	it = m_SegResultOverlaps.find(segResult);

	overlaps = NULL;
	if (it != m_SegResultOverlaps.end())
		overlaps = (*it).second;
	else
	{
		overlaps = new set<CUniString>();
		m_SegResultOverlaps.insert(pair<CUniString, set<CUniString>*>(segResult, overlaps));
	}
	overlaps->insert(groundTruth);

	//Interval Representation
	if (overlap != NULL)
	{
		AddOverlapIntervalRep(groundTruth, segResult, overlap);
	}
}

/*
 * Returns all overlapping segmentation result regions for the given ground truth region (or NULL).
 */
set<CUniString> * CEvaluationResults::GetGroundTruthOverlaps(CUniString groundTruth)
{
	map<CUniString, set<CUniString>*>::iterator it = m_GroundTruthOverlaps.find(groundTruth);

	set<CUniString> * ret = NULL;
	if (it != m_GroundTruthOverlaps.end())
		ret = (*it).second;
	return ret;
}

/*
 * Returns all overlapping ground truth result regions for the given segmentation region (or NULL).
 */
set<CUniString> * CEvaluationResults::GetSegResultOverlaps(CUniString segResult)
{
	map<CUniString, set<CUniString>*>::iterator it = m_SegResultOverlaps.find(segResult);

	set<CUniString> * ret = NULL;
	if (it != m_SegResultOverlaps.end())
		ret = (*it).second;
	return ret;
}

/*
 * Gets the overlap interval representation for the given regions
 */
CLayoutObjectOverlap * CEvaluationResults::GetOverlapIntervalRep(CUniString groundTruth, CUniString segResult)
{
	map<CUniString, map<CUniString, CLayoutObjectOverlap*>*>::iterator itg = m_OverlapIntervalReps.find(groundTruth);

	if (itg == m_OverlapIntervalReps.end())
		return NULL;
	map<CUniString, CLayoutObjectOverlap*>* mapseg = (*itg).second;

	map<CUniString, CLayoutObjectOverlap*>::iterator its = mapseg->find(segResult);
	if (its == mapseg->end())
		return NULL;
	return (*its).second;
}

/*
 * Gets the overlap interval representation for the given ground truth and all overlapping
 * segentation result regions.
 */
CLayoutObjectOverlap * CEvaluationResults::GetMultiOverlapIntervalRep(CUniString groundTruth)
{
	map<CUniString, CLayoutObjectOverlap*>::iterator it = m_MultiOverlapIntervalReps.find(groundTruth);

	if (it == m_MultiOverlapIntervalReps.end())
		return NULL;
	return (*it).second;
}

/*
 * Add an overlap interval representation for the given regions
 */
void CEvaluationResults::AddOverlapIntervalRep(CUniString groundTruth, CUniString segResult, 
											   CLayoutObjectOverlap * overlap)
{
	map<CUniString, map<CUniString, CLayoutObjectOverlap*>*>::iterator itg = m_OverlapIntervalReps.find(groundTruth);

	map<CUniString, CLayoutObjectOverlap*>* mapseg = NULL;
	if (itg == m_OverlapIntervalReps.end())
	{
		mapseg = new map<CUniString, CLayoutObjectOverlap*>();
		m_OverlapIntervalReps.insert(pair<CUniString, map<CUniString, CLayoutObjectOverlap*>*>(groundTruth, mapseg));
	}
	else
		mapseg = (*itg).second;

	mapseg->insert(pair<CUniString, CLayoutObjectOverlap*>(segResult, overlap));
}

/*
 * Add an overlap interval representation for the given ground truth region and 
 * all overlapping segmentation result regions.
 */
void CEvaluationResults::AddMultiOverlapIntervalRep(CLayoutObject * groundTruth, CLayoutObjectOverlap * overlap)
{
	m_MultiOverlapIntervalReps.insert(pair<CUniString, CLayoutObjectOverlap*>(groundTruth->GetId(), overlap));
}

/*
 * Returns the result object for a single layout object.
 *
 * 'createIfNotExists' - If true and the object doesn't exist yet, a LayoutObjectEvaluationResult
 *                       object for the given object will be created and stored.
 */
CLayoutObjectEvaluationResult * CEvaluationResults::GetGroundTruthObjectResult(CUniString layoutObject, bool createIfNotExists)
{
	map<CUniString, CLayoutObjectEvaluationResult*>::iterator it = m_GroundTruthObjectResults.find(layoutObject);

	CLayoutObjectEvaluationResult * ret = NULL;
	if (it != m_GroundTruthObjectResults.end())
		ret = (*it).second;
	else if (createIfNotExists)
	{
		ret = new CLayoutObjectEvaluationResult(this, layoutObject);
		m_GroundTruthObjectResults.insert(pair<CUniString, CLayoutObjectEvaluationResult*>(layoutObject, ret));
	}
	return ret;
}

/*
 * Returns the result object for a single layout object.
 *
 * 'createIfNotExists' - If true and the object doesn't exist yet, a LayoutObjectEvaluationResult
 *                       object for the given object will be created and stored.
 */
CLayoutObjectEvaluationResult * CEvaluationResults::GetSegResultObjectResult(CUniString layoutObject, bool createIfNotExists)
{
	map<CUniString, CLayoutObjectEvaluationResult*>::iterator it = m_SegResultObjectResults.find(layoutObject);

	CLayoutObjectEvaluationResult * ret = NULL;
	if (it != m_SegResultObjectResults.end())
		ret = (*it).second;
	else if (createIfNotExists)
	{
		ret = new CLayoutObjectEvaluationResult(this, layoutObject);
		m_SegResultObjectResults.insert(pair<CUniString, CLayoutObjectEvaluationResult*>(layoutObject, ret));
	}
	return ret;
}

/*
 * Add the given interval representation to the internal map.
 */
void CEvaluationResults::AddIntervalRepresentation(CLayoutObject * object, 
												   CIntervalRepresentation * intRepr, 
												   bool isGroundTruth)
{
	if (isGroundTruth)
		m_GroundTruthIntervalReps.insert(pair<CUniString, CIntervalRepresentation*>(object->GetId(), intRepr));
	else //seg result
		m_SegResultIntervalReps.insert(pair<CUniString, CIntervalRepresentation*>(object->GetId(), intRepr));
}

/*
 * Returns the interval representation for the given layout object. If it's not found in the internal map it is created and stored.
 */
CIntervalRepresentation * CEvaluationResults::GetIntervalRepresentation(CUniString objectId, 
																		bool createIfNotExists, 
																		bool isGroundTruth)
{
	map<CUniString, CIntervalRepresentation*>::iterator it = isGroundTruth ? m_GroundTruthIntervalReps.find(objectId) : m_SegResultIntervalReps.find(objectId);
	map<CUniString, CIntervalRepresentation*>::iterator end = isGroundTruth ? m_GroundTruthIntervalReps.end() : m_SegResultIntervalReps.end();

	CIntervalRepresentation * ret = NULL;

	if (it == end && createIfNotExists) //not found
	{
		CLayoutObject * layoutObject = GetDocumentLayoutObject(objectId, isGroundTruth);
		if (layoutObject->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP)
			ret = CalculateIntervalRepresentation((CReadingOrderGroup*)layoutObject, isGroundTruth ? m_LayoutEvaluation->GetGroundTruth() : m_LayoutEvaluation->GetSegResult());
		else
		{
			RestrictToDocumentDimensions(layoutObject->GetCoords(), m_LayoutEvaluation->GetGroundTruth()->GetWidth(), m_LayoutEvaluation->GetGroundTruth()->GetHeight());
			ret = new CIntervalRepresentation(layoutObject->GetCoords(), false, layoutObject);
		}
		AddIntervalRepresentation(layoutObject, ret, isGroundTruth);
	}
	else
		ret = (*it).second;

	return ret;
}

/*
 * Calculates an interval representation containing all regions referenced from the given group
 */
CIntervalRepresentation * CEvaluationResults::CalculateIntervalRepresentation(CReadingOrderGroup * group, CPageLayout * pageLayout)
{
	if (group == NULL)
		return NULL;

	//Collect regions
	list<CIntervalRepresentation*> intReps;
	for (int i = 0; i < group->GetSize(); i++)
	{
		if (group->GetElement(i)->GetType() == CReadingOrderElement::TYPE_REGION_REF)
		{
			CUniString regionId = ((CReadingOrderRegionRef*)group->GetElement(i))->GetIdRef();
			CLayoutRegion * reg = pageLayout->GetRegionById(regionId);
			if (reg != NULL)
				intReps.push_back(new CIntervalRepresentation(reg->GetCoords(), false, reg));
		}
	}
	//Create merged interval representation
	CIntervalRepresentation * ret = new CIntervalRepresentation(&intReps);

	//Clean up
	for (list<CIntervalRepresentation*>::iterator it = intReps.begin(); it != intReps.end(); it++)
		delete (*it);

	return ret;
}

/*
 * Cuts of coord that are <0 or >=width/height.
 */
void CEvaluationResults::RestrictToDocumentDimensions(CLayoutPolygon * coords, int width, int height)
{
	CPolygonPoint * p = coords->GetHeadPoint();
	while (p != NULL)
	{
		//Left side
		if (p->GetX() < 0)
			p->SetX(0);
		//Top side
		if (p->GetY() < 0)
			p->SetY(0);
		//Right side
		if (p->GetX() >= width)
			p->SetX(width-1);
		//Bottom side
		if (p->GetY() >= height)
			p->SetY(height-1);

		p = p->GetNextPoint();
	}
}

/*
 * Returns the number of black pixels within the given region.
 */
long CEvaluationResults::GetPixelCount(CUniString region, bool isGroundTruth)
{
	//Look in the map first
	map<CUniString,long>::iterator it;
	if (isGroundTruth)
	{
		it = m_GroundTruthPixelCounts.find(region);
		if (it != m_GroundTruthPixelCounts.end()) //found
			return (*it).second;
	}
	else //seg result
	{
		it = m_SegResultPixelCounts.find(region);
		if (it != m_SegResultPixelCounts.end()) //found
			return (*it).second;
	}

	COpenCvBiLevelImage * img = m_LayoutEvaluation->GetBilevelImage();
	CIntervalRepresentation * intRepr = GetIntervalRepresentation(region, true, isGroundTruth);
	long pixelCount = 0L;

	//Count (using interval representation)
	if (img != NULL)
	{
		for (int i=0; i<intRepr->GetIntervalCount(); i++)
		{
			CInterval * interval = intRepr->GetInterval(i);
			vector<int> * parts = interval->GetIntervalSegments();
			for (unsigned int j=0; (int)j<(int)parts->size()-1; j+=2)
			{
				pixelCount += img->CountPixels(	parts->at(j), interval->GetStart(),
												parts->at(j+1), interval->GetEnd());
			}
		}
	}
	if (isGroundTruth)
		m_GroundTruthPixelCounts.insert(pair<CUniString,long>(region, pixelCount)); //store
	else //seg result
		m_SegResultPixelCounts.insert(pair<CUniString,long>(region, pixelCount)); //store

	return pixelCount;
}

/*
 * Retunrs the area of the specified region.
 * The area is retrieved from the interval representation.
 */
long CEvaluationResults::GetRegionArea(CUniString region, bool isGroundTruth)
{
	CIntervalRepresentation * intRepr = GetIntervalRepresentation(region, true, isGroundTruth);
	return intRepr->GetArea();
}

/*
 * Returns all layout objects having an evaluation error of the specified type (merge, split, ...).
 */
map<CUniString, CLayoutObjectEvaluationError*> * CEvaluationResults::GetRegionsForErrorType(int errorType, 
																					bool createIfNotExists /*= false*/)
{
	map<int, map<CUniString, CLayoutObjectEvaluationError*>*>::iterator it = m_ErrorTypeObjectsMap.find(errorType);

	map<CUniString, CLayoutObjectEvaluationError*>* ret = NULL;
	if (it == m_ErrorTypeObjectsMap.end())
	{
		if (createIfNotExists)
		{
			ret = new map<CUniString, CLayoutObjectEvaluationError*>();
			m_ErrorTypeObjectsMap.insert(pair<int, map<CUniString, CLayoutObjectEvaluationError*> *>(errorType, ret));
		}
		else
			return NULL;
	}
	else
		ret = (*it).second;
	return ret;
}

//Removes reading order result without deleting
void CEvaluationResults::ClearReadingOrderResults()
{ 
	m_ReadingOrderResults = NULL; 
} 

void CEvaluationResults::SetReadingOrderResults(CReadingOrderEvaluationResult * res)
{ 
	delete m_ReadingOrderResults; 
	m_ReadingOrderResults = res; 
}

//Removes border result without deleting
void CEvaluationResults::ClearBorderResults()
{ 
	m_BorderResults = NULL; 
} 

void CEvaluationResults::SetBorderResults(CBorderEvaluationResults * res)
{ 
	delete m_BorderResults; 
	m_BorderResults = res; 
}

void CEvaluationResults::SetMetrics(CEvaluationMetrics * metrics)
{
	delete m_Metrics;
	m_Metrics = metrics;
}

CLayoutObject * CEvaluationResults::GetDocumentLayoutObject(CUniString objectId, bool isGroundTruth)
{
	if (isGroundTruth)
	{
		if (m_LayoutEvaluation != NULL && m_LayoutEvaluation->GetGroundTruth() != NULL)
			return (CLayoutObject*)m_LayoutEvaluation->GetGroundTruth()->GetIdRegister()->GetRegion(objectId);
	}
	else //seg result
	{
		if (m_LayoutEvaluation != NULL && m_LayoutEvaluation->GetSegResult() != NULL)
			return (CLayoutObject*)m_LayoutEvaluation->GetSegResult()->GetIdRegister()->GetRegion(objectId);
	}
	return NULL;
}

/*
 * Calculate the measures, metrics, rates based on the raw evaluation data.
 */
void CEvaluationResults::CalculateMetrics()
{
	delete m_Metrics;
	//Border
	if (m_LayoutObjectType == CLayoutObject::TYPE_BORDER)
	{
		m_Metrics = new CBorderEvaluationMetrics(this, m_Profile, true);
	}
	else //Layout object
	{
		m_Metrics = new CLayoutObjectEvaluationMetrics(this, m_Profile, true);

		if (m_LayoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
		{
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_CHART,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_CHART, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			//m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_FRAME,
			//	new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_FRAME, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_GRAPHIC,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_GRAPHIC, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_IMAGE,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_IMAGE, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_LINEART,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_LINEART, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int, CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_MAP,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_MAP, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_MATHS,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_MATHS, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_NOISE,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_NOISE, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_SEPARATOR,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_SEPARATOR, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_TABLE,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_TABLE, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_TEXT,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_TEXT, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_ADVERT,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_ADVERT, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_CHEM,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_CHEM, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_MUSIC,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_MUSIC, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int, CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_CUSTOM,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_CUSTOM, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
			m_MetricsPerLayoutRegionType.insert(pair<int,CLayoutObjectEvaluationMetrics *>(CLayoutRegion::TYPE_UNKNOWN,
				new CLayoutObjectEvaluationMetrics(this, m_Profile, true, CLayoutRegion::TYPE_UNKNOWN, (CLayoutObjectEvaluationMetrics*)m_Metrics)));
		}
	}
}

/*
 * Returns the specialized metrics for a layout region type (e.g. TABLE or IMAGE; see CLayoutRegion::TYPE_...).
 */
CLayoutObjectEvaluationMetrics * CEvaluationResults::GetMetrics(int layoutRegionType)
{
	map<int,CLayoutObjectEvaluationMetrics *>::iterator it = m_MetricsPerLayoutRegionType.find(layoutRegionType);
	if (it != m_MetricsPerLayoutRegionType.end())
		return (*it).second;
	return NULL;
}


/*
 * Class CEvaluationError
 *
 * Generic class for different evaluation errors.
 *
 * CC 22.01.2010 - created
 */

/*
 * Constructor
 */
CEvaluationError::CEvaluationError()
{
	m_WeightedAreaError = 0.0;
	m_WeightedCountError = 0.0;
}

/*
 * Destructor
 */
CEvaluationError::~CEvaluationError()
{
}


/*
 * Class CLayoutObjectEvaluationError
 *
 * Generic class for different layout-object-based evaluation errors.
 *
 * CC 22.01.2010 - created
 */

/*
 * Constructor
 *
 * 'type' - error type (merge, split, ...) (see constants TYPE_...)
 */
CLayoutObjectEvaluationError::CLayoutObjectEvaluationError(int type, CUniString objectId) : CEvaluationError()
{
	m_Type = type;
	m_LayoutObject = objectId;
	m_Area = 0L;
	m_PixelCount = 0L;
	m_Count = 0;
	m_FalseAlarm = false;
	m_NestedRegion = false;
}

/*
 * Destructor
 */
CLayoutObjectEvaluationError::~CLayoutObjectEvaluationError()
{
	for (list<CRect*>::iterator it = m_Rects.begin(); it != m_Rects.end(); it++)
		delete (*it);
	for (list<CRect*>::iterator it = m_FalseAlarmRects.begin(); it != m_FalseAlarmRects.end(); it++)
		delete (*it);
}

/*
 * Creates a deep copy
 */
CLayoutObjectEvaluationError * CLayoutObjectEvaluationError::Clone()
{
	CLayoutObjectEvaluationError * copy = new CLayoutObjectEvaluationError(m_Type, m_LayoutObject);

	copy->SetArea(m_Area);
	copy->SetPixelCount(m_PixelCount);
	copy->SetCount(m_Count);
	copy->SetFalseAlarm(m_FalseAlarm);
	copy->SetForNestedRegion(m_NestedRegion);

	for (list<CRect*>::iterator it = m_Rects.begin(); it != m_Rects.end(); it++)
		copy->m_Rects.push_back(new CRect((*it)));
	for (list<CRect*>::iterator it = m_FalseAlarmRects.begin(); it != m_FalseAlarmRects.end(); it++)
		copy->m_FalseAlarmRects.push_back(new CRect((*it)));

	return copy;
}

/*
 * Adds the given rects to the internal rect list.
 * Note: Copies the rects!
 */
void CLayoutObjectEvaluationError::AddRects(vector<CRect*> * rects)
{
	if (rects == NULL)
		return;
	for (unsigned int i=0; i<rects->size(); i++)
		m_Rects.push_back(new CRect(rects->at(i))); //Copy the rects to avoid errors on destruction
}

CUniString CLayoutObjectEvaluationError::GetTypeName(int errorType)
{
	if (errorType == TYPE_MERGE)
		return CUniString(_T("Merge"));
	if (errorType == TYPE_SPLIT)
		return CUniString(_T("Split"));
	if (errorType == TYPE_MISS)
		return CUniString(_T("Miss"));
	if (errorType == TYPE_PART_MISS)
		return CUniString(_T("Partial Miss"));
	if (errorType == TYPE_MISCLASS)
		return CUniString(_T("Misclassification"));
	if (errorType == TYPE_INVENT)
		return CUniString(_T("False Detection"));
	return CUniString(_T("Unknown"));
}


/*
 * Class CEvaluationErrorMerge
 *
 * Class for merge evaluation error.
 *
 * CC 25.01.2010 - created
 */

/*
 * Constructor
 */
CEvaluationErrorMerge::CEvaluationErrorMerge(CUniString region) : CLayoutObjectEvaluationError(TYPE_MERGE, region)
{
}

/*
 * Destructor
 */
CEvaluationErrorMerge::~CEvaluationErrorMerge()
{
	map<CUniString, COverlapRects *>::iterator it = m_MergingRegions.begin();
	while (it != m_MergingRegions.end())
	{
		delete (*it).second;
		it++;
	}
}

/*
 * Creates a deep copy
 */
CLayoutObjectEvaluationError * CEvaluationErrorMerge::Clone()
{
	CEvaluationErrorMerge * copy = new CEvaluationErrorMerge(m_LayoutObject);

	copy->SetArea(m_Area);
	copy->SetPixelCount(m_PixelCount);
	copy->SetCount(m_Count);
	copy->SetFalseAlarm(m_FalseAlarm);
	copy->SetForNestedRegion(m_NestedRegion);

	for (list<CRect*>::iterator it = m_Rects.begin(); it != m_Rects.end(); it++)
		copy->m_Rects.push_back(new CRect((*it)));
	for (list<CRect*>::iterator it = m_FalseAlarmRects.begin(); it != m_FalseAlarmRects.end(); it++)
		copy->m_FalseAlarmRects.push_back(new CRect((*it)));

	for (map<CUniString, bool>::iterator it = m_Allowable.begin(); it != m_Allowable.end(); it++)
		copy->m_Allowable.insert(pair<CUniString, bool>((*it).first, (*it).second));

	for (map<CUniString, COverlapRects *>::iterator it = m_MergingRegions.begin(); it != m_MergingRegions.end(); it++)
	{
		COverlapRects * rects = new COverlapRects();
		rects->CopyFrom((*it).second, true);
		copy->m_MergingRegions.insert(pair<CUniString, COverlapRects *>((*it).first, rects));
	}

	return copy;
}

/*
 * Adds the given error rects to the internal map
 */
void CEvaluationErrorMerge::AddErrorRects(CUniString segResultRegion, COverlapRects* overlapRects, 
										  bool addArea /*= true*/)
{
	//map<CLayoutObject *, vector<CRect *>*>::iterator it = m_ErrorRegions.find(segResultRegion);
	m_MergingRegions.insert(pair<CUniString, COverlapRects *>(segResultRegion, overlapRects));

	//Also add copies of the rects to the global rect list for this error
	map<CUniString, vector<CRect*>*>::iterator it = overlapRects->m_Overlaps.begin();
	while (it != overlapRects->m_Overlaps.end())
	{
		vector<CRect*> * rects = (*it).second;
		if (rects != NULL)
		{
			for (unsigned int i=0; i<rects->size(); i++)
				m_Rects.push_back(new CRect(rects->at(i)));
		}
		it++;
	}

	//Area / pixel count
	if (addArea)
	{
		m_Area += overlapRects->GetArea();
		m_PixelCount += overlapRects->GetPixelCount();
	}
}

void CEvaluationErrorMerge::SetAllowable(CUniString groundTruthRegion, bool allowable)
{
	map<CUniString, bool>::iterator it = m_Allowable.find(groundTruthRegion);
	if (it != m_Allowable.end())
	{
		(*it).second = allowable;
	}
	else
	{
		m_Allowable.insert(pair<CUniString, bool>(groundTruthRegion, allowable));
	}
}

bool CEvaluationErrorMerge::IsAllowable(CUniString groundTruthRegion)
{
	map<CUniString, bool>::iterator it = m_Allowable.find(groundTruthRegion);
	if (it != m_Allowable.end())
		return (*it).second;
	return false;
}


/*
 * Class CFuzzyReadingOrderRelation
 *
 * Reading order relation between two segmentation result regions
 * based on the ground-truth reading order. Because each segmentation
 * result region can overlap with many ground-truth regions, multiple
 * relations are allowed. The relations are weighted by their overlap
 * percentage.
 *
 */

CFuzzyReadingOrderRelation::CFuzzyReadingOrderRelation(double weight, 
													   CUniString reg1, CUniString reg2, 
													   set<int> relation)
{
	m_Weight = weight;
	m_Region1 = reg1;
	m_Region2 = reg2;
	m_Relation = relation;
}

CFuzzyReadingOrderRelation::~CFuzzyReadingOrderRelation()
{
}


/*
 * Class CReadingOrderError
 *
 * Class for reading order tree evaluation error.
 *
 * CC 14.06.2010 - created
 */

/*
 * Constructor
 */
CReadingOrderError::CReadingOrderError(CUniString reg1, CUniString reg2, 
									   set<int> segResultRelation, 
									   vector<CFuzzyReadingOrderRelation> groundTruthRelation,
									   CEvaluationProfile * profile) 
							: CEvaluationError()
{
	m_Region1 = reg1;
	m_Region2 = reg2;
	m_SegResultRelation = segResultRelation;
	m_GroundTruthRelation = groundTruthRelation;
	m_Penalty = 0;
	CalculatePenalty(profile);
}

/*
 * Destructor
 */
CReadingOrderError::~CReadingOrderError()
{
}

/*
 * Calculates the penalty of the relation
 */
void CReadingOrderError::CalculatePenalty(CEvaluationProfile * profile)
{
	m_Penalty = 0.0;
	
	for (unsigned int i=0; i<m_GroundTruthRelation.size(); i++)
	{
		double currPenalty = 0.0;
		int selectedGroundTruthRel = 0;
		int selectedSegResultRel = 0;

		currPenalty = CalculatePenalty(profile, m_GroundTruthRelation[i].GetRelation(), m_SegResultRelation,
										selectedGroundTruthRel, selectedSegResultRel);

		m_Penalty += currPenalty * m_GroundTruthRelation[i].GetWeight();

		//Collect data for detailed evaluation
		m_GroundTruthRelation[i].SetPenalty(currPenalty);
		m_GroundTruthRelation[i].SetCausingGroundTruthRelation(selectedGroundTruthRel);
		m_GroundTruthRelation[i].SetCausingSegResultRelation(selectedSegResultRel);
	}
}

/*
 * Compares all combinations of the given relation sets and returns the minimum penalty.
 *
 * 'selectedGroundTruthRel' (out) - The ground-truth reading order relation involved in the minimal penalty.
 * 'selectedSegResultRel' (out) - The sementation result reading order relation involved in the minimal penalty.
 */
double CReadingOrderError::CalculatePenalty(CEvaluationProfile * profile, 
											set<int> & groundTruth, set<int> & segmentation,
											int & selectedGroundTruthRel, int & selectedSegResultRel)
{
	double minPenalty = 999999.9, currPenalty;
	set<int>::iterator itGroundTruth = groundTruth.begin();
	while (itGroundTruth != groundTruth.end())
	{
		set<int>::iterator itSeg = segmentation.begin();
		while (itSeg != segmentation.end())
		{
			currPenalty = CalculatePenalty(profile, (*itGroundTruth), (*itSeg));
			if (currPenalty < minPenalty)
			{
				minPenalty = currPenalty;
				selectedGroundTruthRel = (*itGroundTruth);
				selectedSegResultRel = (*itSeg);
			}
			itSeg++;
		}
		itGroundTruth++;
	}
	return minPenalty;
}

//Maximum error of the penalty matrix below.
double CReadingOrderError::GetMaxPenalty(CEvaluationProfile * profile)
{
	return profile->GetReadingOrderPenalties()->GetMaxPenalty();
}

/*
 * Compares two relations and calculates a penalty, if they are not the same.
 */
double CReadingOrderError::CalculatePenalty(CEvaluationProfile * profile, int groundTruthRel, int segmentationRel)
{
	// ->    r1 preceeds r2 directly
	// <-    r1 succeeds r2 directly
	// --    unordered relation
	// ?     unknown relation (used for splits)
	// -x-   no direct relation
	// n.d.  not defined (e.g. one of the two regions not in the reading order)
	// ->->  r1 somewhere before r2
	// <-<-  r1 somewhere after r2
	//

	///
	/// NOTE: Mind GetMaxPenalty above when making changes !!!!!
	///

	//int penaltyMatrix[9][9] = {	
	//			
	//				//Ground-Truth
	////Seg.Result 
	//			/*			->	   <-     --     ?     -x-    n.d.  ->->    <-<-  */
	///*     */	{     0,     0,     0,     0,     0,     0,     0,     0,     0},

	///*  -> */	{     0,     0,    40,    10,     0,    20,     0,     0,    10},

	///*  <- */	{     0,    40,     0,    10,     0,    20,     0,    10,     0},

	///*  -- */	{     0,    20,    20,     0,     0,    10,     0,    10,    10},

	///*  ?  */	{     0,     0,     0,     0,     0,     0,     0,     0,     0},	//cannot happen

	///* -x- */	{     0,    20,    20,    10,     0,     0,     0,    10,    10},

	///* n.d.*/	{     0,    20,    20,    10,     0,     0,     0,    10,    10},

	///* ->->*/	{     0,     0,    20,     5,     0,     5,     0,     0,    10},

	///* <-<-*/	{     0,    20,     0,     5,     0,     5,     0,    10,     0}
 //   };

	int penalty = 0;

	//if (segmentationRel>0 && segmentationRel<9 
	//	&& groundTruthRel>0 && groundTruthRel<9)
	//{
		//penalty = penaltyMatrix[segmentationRel][groundTruthRel];
		penalty = profile->GetReadingOrderPenalties()->GetPenalty(segmentationRel, groundTruthRel);
	//}

	return (double)penalty;
}


/*
 * Class CEvaluationErrorMisclass
 *
 * Class for misclassification evaluation error.
 *
 * CC 27.01.2010 - created
 */

/*
 * Constructor
 */
CEvaluationErrorMisclass::CEvaluationErrorMisclass(CUniString region) : CLayoutObjectEvaluationError(TYPE_MISCLASS, region)
{
}

/*
 * Destructor
 */
CEvaluationErrorMisclass::~CEvaluationErrorMisclass()
{
}

/*
 * Creates a deep copy
 */
CLayoutObjectEvaluationError * CEvaluationErrorMisclass::Clone()
{
	CEvaluationErrorMisclass * copy = new CEvaluationErrorMisclass(m_LayoutObject);

	copy->SetArea(m_Area);
	copy->SetPixelCount(m_PixelCount);
	copy->SetCount(m_Count);
	copy->SetFalseAlarm(m_FalseAlarm);
	copy->SetForNestedRegion(m_NestedRegion);

	for (list<CRect*>::iterator it = m_Rects.begin(); it != m_Rects.end(); it++)
		copy->m_Rects.push_back(new CRect((*it)));
	for (list<CRect*>::iterator it = m_FalseAlarmRects.begin(); it != m_FalseAlarmRects.end(); it++)
		copy->m_FalseAlarmRects.push_back(new CRect((*it)));

	copy->m_ErrorAreas.CopyFrom(&m_ErrorAreas, true);

	return copy;
}

/*
 * Adds the given error rects to the internal maps (also to the base class map!)
 */
void CEvaluationErrorMisclass::AddErrorRects(CUniString overlappingRegion, CLayoutObjectOverlap * overlap,
											 bool countPixels, COpenCvBiLevelImage * image)
{
	m_ErrorAreas.AddOverlapRects(	overlappingRegion, overlap, 
									countPixels, image);
	AddRects(overlap->GetOverlapRects());
}


/*
 * Class CEvaluationErrorSplit
 *
 * Class for split evaluation error.
 *
 * CC 05.02.2010 - created
 */

/*
 * Constructor
 */
CEvaluationErrorSplit::CEvaluationErrorSplit(CUniString region) : CLayoutObjectEvaluationError(TYPE_SPLIT, region)
{
}

/*
 * Destructor
 */
CEvaluationErrorSplit::~CEvaluationErrorSplit()
{
}

/*
 * Creates a deep copy
 */
CLayoutObjectEvaluationError * CEvaluationErrorSplit::Clone()
{
	CEvaluationErrorSplit * copy = new CEvaluationErrorSplit(m_LayoutObject);

	copy->SetArea(m_Area);
	copy->SetPixelCount(m_PixelCount);
	copy->SetCount(m_Count);
	copy->SetFalseAlarm(m_FalseAlarm);
	copy->SetAllowable(m_Allowable);
	copy->SetForNestedRegion(m_NestedRegion);

	for (list<CRect*>::iterator it = m_Rects.begin(); it != m_Rects.end(); it++)
		copy->m_Rects.push_back(new CRect((*it)));
	for (list<CRect*>::iterator it = m_FalseAlarmRects.begin(); it != m_FalseAlarmRects.end(); it++)
		copy->m_FalseAlarmRects.push_back(new CRect((*it)));

	copy->m_SplittingRegions.CopyFrom(&m_SplittingRegions, true);

	return copy;
}

/*
 * Adds the given error rects to the internal maps (also to the base class map!)
 */
void CEvaluationErrorSplit::AddErrorRects(CUniString overlappingRegion, CLayoutObjectOverlap * overlap,
											 bool countPixels, COpenCvBiLevelImage * image)
{
	m_SplittingRegions.AddOverlapRects(	overlappingRegion, overlap, 
									countPixels, image);
	m_Area = m_SplittingRegions.GetArea();
	AddRects(overlap->GetOverlapRects());
}


/*
 * Class CLayoutObjectEvaluationResult
 *
 * Single result for one region.
 *
 * CC 22.01.2010 - created
 */

/*
 * Constructor
 */
CLayoutObjectEvaluationResult::CLayoutObjectEvaluationResult(CEvaluationResults * results,
									 CUniString region)
{
	m_Results = results;
	m_Region = region;
}

/*
 * Destructor
 */
CLayoutObjectEvaluationResult::~CLayoutObjectEvaluationResult()
{
	map<int, CLayoutObjectEvaluationError*>::iterator it = m_Errors.begin();
	while (it != m_Errors.end())
	{
		delete (*it).second;
		it++;
	}
}

/*
 * Creates a deep copy
 */
CLayoutObjectEvaluationResult * CLayoutObjectEvaluationResult::Clone(CEvaluationResults * parentResults)
{
	CLayoutObjectEvaluationResult * copy = new CLayoutObjectEvaluationResult(parentResults, m_Region);

	map<int, CLayoutObjectEvaluationError*>::iterator it = m_Errors.begin();
	while (it != m_Errors.end())
	{
		copy->AddError((*it).second->Clone());
		it++;
	}
	return copy;
}

/*
 * Adds the given error to the internal maps.
 */
void CLayoutObjectEvaluationResult::AddError(CLayoutObjectEvaluationError * error)
{
	m_Errors.insert(pair<int, CLayoutObjectEvaluationError*>(error->GetType(), error));
	//Add it also to the error type specific list
	map<CUniString, CLayoutObjectEvaluationError*> * regions = m_Results->GetRegionsForErrorType(error->GetType(), true);
	regions->insert(pair<CUniString, CLayoutObjectEvaluationError*>(m_Region, error));
}

/*
 * Returns the evaluation error of the specified type (merge, split, ...).
 */
CLayoutObjectEvaluationError * CLayoutObjectEvaluationResult::GetError(int errType)
{
	map<int, CLayoutObjectEvaluationError*>::iterator it = m_Errors.find(errType);
	if (it == m_Errors.end())
		return NULL;
	return (*it).second;
}


/*
 * Class COverlapRects
 *
 * Class for storing overlapping areas.
 *
 * CC 25.01.2010 - created
 */

COverlapRects::COverlapRects()
{
	m_OverallArea = 0L;
	m_OverallPixelCount = 0L;
}

/*
 * Destructor
 */
COverlapRects::~COverlapRects()
{
	map<CUniString, vector<CRect*>*>::iterator it = m_Overlaps.begin();
	while (it != m_Overlaps.end())
	{
		vector<CRect*> * vec = (*it).second;
		if (vec != NULL)
		{
			for (unsigned int i=0; i<vec->size(); i++)
				delete vec->at(i);
			delete vec;
		}
		it++;
	}
	m_Overlaps.clear();
}

/*
 * Adds the given CRects to the internal map.
 * Note: The vector and the rects are copied.
 */
void COverlapRects::AddOverlapRects(CUniString overlappingObject, CLayoutObjectOverlap * overlap,
									bool countPixels, COpenCvBiLevelImage * image)
{
	vector<CRect*> * rects = overlap->GetOverlapRects();
	vector<CRect*> * copy = new vector<CRect*>();
	for (unsigned int i=0; i<rects->size(); i++)
		copy->push_back(new CRect(rects->at(i)));
	m_Overlaps.insert(pair<CUniString, vector<CRect*>*>(overlappingObject, copy));

	//Set area and pixel count as well
	m_OverlapArea.insert(pair<CUniString, long>(overlappingObject, overlap->GetOverlapArea()));
	m_OverallArea += overlap->GetOverlapArea();

	if (countPixels && image != NULL)
	{
		long count = image->CountPixels(copy);
		m_PixelCount.insert(pair<CUniString, long>(overlappingObject,
													count));
		m_OverallPixelCount += count;
	}
}

/*
 * Adds the overlap to the internal map. (Used by XML reader)
 */
void COverlapRects::AddOverlap(CUniString overlappingObject, long area, long pixelCount)
{
	m_Overlaps.insert(pair<CUniString, vector<CRect*>*>(overlappingObject, NULL)); //No rects available

	m_OverlapArea.insert(pair<CUniString, long>(overlappingObject, area));
	m_OverallArea += area;

	m_PixelCount.insert(pair<CUniString, long>(overlappingObject,
												pixelCount));
	m_OverallPixelCount += pixelCount;
}

long COverlapRects::GetOverlapArea(CUniString region)
{
	map<CUniString, long>::iterator it = m_OverlapArea.find(region);
	if (it == m_OverlapArea.end())
		return 0L;
	return (*it).second;
}

long COverlapRects::GetOverlapPixelCount(CUniString region)
{
	map<CUniString, long>::iterator it = m_PixelCount.find(region);
	if (it == m_PixelCount.end())
		return 0L;
	return (*it).second;
}

vector<CUniString> * COverlapRects::GetRegions()
{
	vector<CUniString> * ret = new vector<CUniString>();

	map<CUniString, long>::iterator it = m_OverlapArea.begin();
	while (it != m_OverlapArea.end())
	{
		ret->push_back((*it).first);
		it++;
	}
	return ret;
}

void COverlapRects::CopyFrom(COverlapRects * rects, bool deepCopy /*= false*/)
{
	map<CUniString, long>::iterator itArea = rects->m_OverlapArea.begin();
	while (itArea != rects->m_OverlapArea.end())
	{
		m_OverlapArea.insert(pair<CUniString, long>((*itArea).first, (*itArea).second));
		itArea++;
	}

	//Number of black pixels per region
	map<CUniString, long>::iterator itCount = rects->m_PixelCount.begin();
	while (itCount != rects->m_PixelCount.end())
	{
		m_PixelCount.insert(pair<CUniString, long>((*itCount).first, (*itCount).second));
		itCount++;
	}

	map<CUniString, vector<CRect*>*>::iterator it = rects->m_Overlaps.begin();
	while (it != rects->m_Overlaps.end())
	{
		vector<CRect*> * vec = (*it).second;
		vector<CRect*> * copyVec = new vector<CRect*>();
		if (vec != NULL)
		{
			if (deepCopy) //Create new objects
			{
				for (unsigned int i = 0; i < vec->size(); i++)
					copyVec->push_back(new CRect(vec->at(i)));
			}
			else //Copy pointers
			{
				for (unsigned int i = 0; i < vec->size(); i++)
					copyVec->push_back(vec->at(i));
			}
		}

		m_Overlaps.insert(pair<CUniString, vector<CRect*>*>((*it).first, copyVec));
		it++;
	}

	m_OverallArea = rects->m_OverallArea;
	m_OverallPixelCount = rects->m_OverallPixelCount;
}


/*
 * Class CReadingOrderEvaluationResult
 *
 * Result for reading order tree relations.
 *
 * CC 16.06.2010 - created
 */

CReadingOrderEvaluationResult::CReadingOrderEvaluationResult(CEvaluationResults * results)
{
	m_Results = results;
}

CReadingOrderEvaluationResult::~CReadingOrderEvaluationResult()
{
	for (unsigned int i=0; i<m_Errors.size(); i++)
		delete m_Errors[i];
}

void CReadingOrderEvaluationResult::AddError(CReadingOrderError * error)
{
	//Add it to the internal vector and map (with region1 as first key and region2 as first key)
	if (AddToErrorMap(error->GetRegion1(), error->GetRegion2(), error))
	{
		AddToErrorMap(error->GetRegion2(), error->GetRegion1(), error);
		m_Errors.push_back(error);
	}
}

/*
 * Adds the error to the error map.
 * Returns true, if added and false if already in map.
 */
bool CReadingOrderEvaluationResult::AddToErrorMap(CUniString regionId1, 
												  CUniString regionId2, 
												  CReadingOrderError * error)
{
	map<CUniString, map<CUniString, CReadingOrderError*>>::iterator it = m_ErrorMap.find(regionId1);

	
	if (it == m_ErrorMap.end()) //Not in map yet
	{
		map<CUniString, CReadingOrderError*> map2;
		m_ErrorMap.insert(pair<CUniString, map<CUniString, CReadingOrderError*>>(regionId1, map2));
		map2.insert(pair<CUniString, CReadingOrderError*>(regionId2, error));
		return true;
	}

	map<CUniString, CReadingOrderError*> map2 = (*it).second;
	map<CUniString, CReadingOrderError*>::iterator it2 = map2.find(regionId2);

	if (it2 == map2.end()) //Not in map2 yet
	{
		map2.insert(pair<CUniString, CReadingOrderError*>(regionId2, error));
		return true;
	}
	return false;
}

CReadingOrderError * CReadingOrderEvaluationResult::GetError(int index)
{
	return m_Errors[index];
}

int CReadingOrderEvaluationResult::GetErrorCount()
{
	return (int)m_Errors.size();
}

vector<CReadingOrderError*> CReadingOrderEvaluationResult::GetErrors(CUniString * regionId)
{
	vector<CReadingOrderError*> ret;

	map<CUniString, map<CUniString, CReadingOrderError*>>::iterator it = m_ErrorMap.find(regionId);
	if (it != m_ErrorMap.end())
	{
		map<CUniString, CReadingOrderError*> map2 = (*it).second;
		map<CUniString, CReadingOrderError*>::iterator it2 = map2.begin();
		while (it2 != map2.end())
		{
			ret.push_back((*it2).second);
			it2++;
		}
	}
	return ret;
}


/*
 * Class CBorderEvaluationError 
 *
 * CC 22.08.2011 - created
 */

CBorderEvaluationError::CBorderEvaluationError(CUniString name)
{
	m_Name = name;
	m_Area = 0L;
}

CBorderEvaluationError::~CBorderEvaluationError()
{
}


/*
 * Class CBorderEvaluationResults 
 *
 * CC 22.08.2011 - created
 */

CBorderEvaluationResults::CBorderEvaluationResults()
{
	m_IncludedBackground = new CBorderEvaluationError(_T("Included background area"));
	m_ExcludedForeground = new CBorderEvaluationError(_T("Excluded foreground area"));
	m_GroundTruthBorderIntervalRep = NULL;
	m_SegResultBorderIntervalRep = NULL;
}

CBorderEvaluationResults::~CBorderEvaluationResults()
{
	delete m_IncludedBackground;
	delete m_ExcludedForeground;
	delete m_GroundTruthBorderIntervalRep;
	delete m_SegResultBorderIntervalRep;
}



}