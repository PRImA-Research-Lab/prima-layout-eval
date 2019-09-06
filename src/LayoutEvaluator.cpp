
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "stdafx.h"
#include "LayoutEvaluator.h"

using namespace PRImA;
using namespace std;

/*
 * Class CLayoutEvaluator
 *
 * Evaluates segmentation results using ground truth.
 *
 * CC 01.02.2010 - recreated this class (from daves old evaluator)
 * CC 15/03/2019 - Added reading order group evaluation
 */

/*
 * Constructor
 */
CLayoutEvaluator::CLayoutEvaluator(CLayoutEvaluation * layoutEval, CEvaluationProfile * profile,
									bool evaluateRegions, bool evaluateTextLines,
									bool evaluateWords, bool evaluateGlyphs, bool evaluateBorder,
									bool evaluateReadingOrderGroups, bool evaluateReadingOrder)
{
	m_EvaluateRegions = evaluateRegions;
	m_EvaluateTextLines = evaluateTextLines;
	m_EvaluateWords = evaluateWords;
	m_EvaluateGlyphs = evaluateGlyphs;
	m_EvaluateBorder = evaluateBorder;
	m_EvaluateReadingOrderGroups = evaluateReadingOrderGroups;
	m_EvaluateReadingOrder = evaluateReadingOrder;

	m_Profile = profile;
	m_LayoutEvaluation = layoutEval;
	m_LayoutEvaluation->SetProfile(profile);
	m_Image = layoutEval->GetBilevelImage();

	m_Progress = 0.0;

	m_UsePixelArea = profile->IsUsePixelArea();

	m_EnableErrorChecks.push_back(false);	//TYPE_NONE
	m_EnableErrorChecks.push_back(true);	//TYPE_SPLIT
	m_EnableErrorChecks.push_back(true);	//TYPE_MERGE
	m_EnableErrorChecks.push_back(true);	//TYPE_MISS
	m_EnableErrorChecks.push_back(true);	//TYPE_PART_MISS
	m_EnableErrorChecks.push_back(true);	//TYPE_MISCLASS
	m_EnableErrorChecks.push_back(true);	//TYPE_INVENT

	m_ConvertToIsothetic = true;
}


/*
 * Destructor
 */
CLayoutEvaluator::~CLayoutEvaluator()
{
}

/*
 * Entry method for evaluation
 */
void CLayoutEvaluator::RunEvaluation(CProgressMonitor * progressMonitor)
{
	//Delete old results
	CSingleLock * lockObject = m_LayoutEvaluation->Lock();
	m_LayoutEvaluation->DeleteResults(CLayoutObject::TYPE_LAYOUT_REGION);
	m_LayoutEvaluation->DeleteResults(CLayoutObject::TYPE_TEXT_LINE);
	m_LayoutEvaluation->DeleteResults(CLayoutObject::TYPE_WORD);
	m_LayoutEvaluation->DeleteResults(CLayoutObject::TYPE_GLYPH);
	m_LayoutEvaluation->DeleteResults(CLayoutObject::TYPE_BORDER);
	m_LayoutEvaluation->DeleteResults(CLayoutObject::TYPE_READING_ORDER_GROUP);
	m_LayoutEvaluation->Unlock(lockObject);

	m_ProgressMonitor = progressMonitor;

	int count = 0;
	if (m_EvaluateRegions)	count++;
	if (m_EvaluateTextLines) count++;
	if (m_EvaluateWords)	count++;
	if (m_EvaluateGlyphs)	count++;
	if (m_EvaluateReadingOrderGroups)	count++;
	if (m_EvaluateBorder)	count++;

	m_MaxPartialProgress = 100.0 / count; //Max progress value per region level
	
	//TODO use threads
	if (m_EvaluateRegions || m_EvaluateReadingOrder)
		Evaluate(CLayoutObject::TYPE_LAYOUT_REGION);
	if (m_EvaluateTextLines)
		Evaluate(CLayoutObject::TYPE_TEXT_LINE);
	if (m_EvaluateWords)
		Evaluate(CLayoutObject::TYPE_WORD);
	if (m_EvaluateGlyphs)
		Evaluate(CLayoutObject::TYPE_GLYPH);
	if (m_EvaluateReadingOrderGroups)
		Evaluate(CLayoutObject::TYPE_READING_ORDER_GROUP);
	if (m_EvaluateBorder)
		Evaluate(CLayoutObject::TYPE_BORDER);
}

/*
 * Layout object type specific evaluation.
 */
void CLayoutEvaluator::Evaluate(int layoutObjectType)
{
	//Evalute nested regions?
	if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION && m_Profile->IsEvaluateNestedRegions())
	{
		EvaluateRegionsAndNestedRegions();
	}
	else //Other layout objects or ignoring nested regions
	{
		//Prepare the relevant data (interval representations, overlap maps, ...)
		ProcessGroundTruthObjects(layoutObjectType, m_LayoutEvaluation);

		//Now find the errors
		FindErrors(layoutObjectType, m_LayoutEvaluation);

		CEvaluationResults * results = m_LayoutEvaluation->GetResults(layoutObjectType);
		results->CalculateMetrics();
	}
}

/*
 * Specialised evaluation when including nested regions
 */
void CLayoutEvaluator::EvaluateRegionsAndNestedRegions()
{
	//We evaluate four times.
	//Parent regions and nested regions are thereby evaluated separatly in different combinations.
	//For example, parent-to-nested means the ground truth parent regions are compared with the segmentation result nested regions

	CLayoutEvaluation evalParentToParent(false);
	evalParentToParent.InitialiseFrom(m_LayoutEvaluation);

	CLayoutEvaluation evalParentToNested(false);
	evalParentToNested.InitialiseFrom(m_LayoutEvaluation);

	CLayoutEvaluation evalNestedToParent(false);
	evalNestedToParent.InitialiseFrom(m_LayoutEvaluation);

	CLayoutEvaluation evalNestedToNested(false);
	evalNestedToNested.InitialiseFrom(m_LayoutEvaluation);

	//Prepare the relevant data (interval representations, overlap maps, ...)
	ProcessGroundTruthObjects(CLayoutObject::TYPE_LAYOUT_REGION, &evalParentToParent, NESTED_REGION_MODE_PARENT_TO_PARENT);
	ProcessGroundTruthObjects(CLayoutObject::TYPE_LAYOUT_REGION, &evalParentToNested, NESTED_REGION_MODE_PARENT_TO_NESTED);
	ProcessGroundTruthObjects(CLayoutObject::TYPE_LAYOUT_REGION, &evalNestedToParent, NESTED_REGION_MODE_NESTED_TO_PARENT);
	ProcessGroundTruthObjects(CLayoutObject::TYPE_LAYOUT_REGION, &evalNestedToNested, NESTED_REGION_MODE_NESTED_TO_NESTED);

	//Now find the errors
	FindErrors(CLayoutObject::TYPE_LAYOUT_REGION, &evalParentToParent, NESTED_REGION_MODE_PARENT_TO_PARENT);
	FindErrors(CLayoutObject::TYPE_LAYOUT_REGION, &evalParentToNested, NESTED_REGION_MODE_PARENT_TO_NESTED);
	FindErrors(CLayoutObject::TYPE_LAYOUT_REGION, &evalNestedToParent, NESTED_REGION_MODE_NESTED_TO_PARENT);
	FindErrors(CLayoutObject::TYPE_LAYOUT_REGION, &evalNestedToNested, NESTED_REGION_MODE_NESTED_TO_NESTED);

	//Combine the four results
	CEvaluationResults * targetResults = m_LayoutEvaluation->GetResults(CLayoutObject::TYPE_LAYOUT_REGION, true);

	CombineTopLevelAndNestedEvaluationResults(targetResults, evalParentToParent.GetResults(CLayoutObject::TYPE_LAYOUT_REGION),
															evalParentToNested.GetResults(CLayoutObject::TYPE_LAYOUT_REGION), 1L, 1L, 5L, 1L, 1L);

	CombineTopLevelAndNestedFalseDetectionResults(targetResults, evalParentToParent.GetResults(CLayoutObject::TYPE_LAYOUT_REGION),
																evalNestedToParent.GetResults(CLayoutObject::TYPE_LAYOUT_REGION));


	CombineTopLevelAndNestedEvaluationResults(targetResults, evalNestedToNested.GetResults(CLayoutObject::TYPE_LAYOUT_REGION),
															evalNestedToParent.GetResults(CLayoutObject::TYPE_LAYOUT_REGION), 1L, 1L, 1L, 1L, 1L);

	CombineTopLevelAndNestedFalseDetectionResults(targetResults, evalNestedToNested.GetResults(CLayoutObject::TYPE_LAYOUT_REGION),
																evalParentToNested.GetResults(CLayoutObject::TYPE_LAYOUT_REGION));

	// Reading order
	targetResults->SetReadingOrderResults(evalParentToParent.GetResults(CLayoutObject::TYPE_LAYOUT_REGION)->GetReadingOrderResults());
	evalParentToParent.GetResults(CLayoutObject::TYPE_LAYOUT_REGION)->ClearReadingOrderResults();

	// Border
	targetResults->SetBorderResults(evalParentToParent.GetResults(CLayoutObject::TYPE_LAYOUT_REGION)->GetBorderResults());
	evalParentToParent.GetResults(CLayoutObject::TYPE_LAYOUT_REGION)->ClearBorderResults();
	
	//TODO Copy anything else?

	targetResults->CalculateMetrics();
}

/*
 * Combines multiple evaluation results (separate for top-level and nested regions) using the minimum error of each combination.
 * For merge, split, miss, partial miss, misclassification.
 */
void CLayoutEvaluator::CombineTopLevelAndNestedEvaluationResults(CEvaluationResults * targetResults, CEvaluationResults * sameLevelResults,
																CEvaluationResults * acrossLevelResults, long mergeMultiplier,
																long splitMultiplier, long missMultiplier, long partialMissMultiplier,
																long misclassMultiplier)
{
	//Iterate over all ground truth regions and choose the minimum error

	map<CUniString, CLayoutObjectEvaluationResult*> * targetgroundTruthObjectResults = targetResults->GetGroundTruthObjectResults();
	map<CUniString, CLayoutObjectEvaluationResult*> * sameLevelgroundTruthObjectResults = sameLevelResults->GetGroundTruthObjectResults();
	map<CUniString, CLayoutObjectEvaluationResult*> * acrossLevelgroundTruthObjectResults = acrossLevelResults->GetGroundTruthObjectResults();

	for (map<CUniString, CLayoutObjectEvaluationResult*>::iterator it = sameLevelgroundTruthObjectResults->begin(); it != sameLevelgroundTruthObjectResults->end(); it++)
	{
		CUniString groundTruthObjectId = (*it).first;
		CLayoutObjectEvaluationResult * sameLevelgroundTruthObjectResult = (*it).second;

		map<CUniString, CLayoutObjectEvaluationResult*>::iterator itAcross = acrossLevelgroundTruthObjectResults->find(groundTruthObjectId);
		CLayoutObjectEvaluationResult * acrossLevelgroundTruthObjectResult = NULL;
		if (itAcross != acrossLevelgroundTruthObjectResults->end())
			acrossLevelgroundTruthObjectResult = (*itAcross).second;

		long sameLevelError = CalculateErrorSum(sameLevelgroundTruthObjectResult, 1L, 1L, 1L, 1L, 1L);
		long acrossLevelError = CalculateErrorSum(acrossLevelgroundTruthObjectResult, mergeMultiplier,
												splitMultiplier, missMultiplier, partialMissMultiplier,
												misclassMultiplier);

		if (sameLevelError <= acrossLevelError)
			CopyLayoutObjectEvalResults(targetResults, targetgroundTruthObjectResults, sameLevelgroundTruthObjectResult);
		else
			CopyLayoutObjectEvalResults(targetResults, targetgroundTruthObjectResults, acrossLevelgroundTruthObjectResult);
	}
}

/*
 * Combines multiple evaluation results (separate for top-level and nested regions) using the minimum error of each combination.
 * For false detection.
 */
void CLayoutEvaluator::CombineTopLevelAndNestedFalseDetectionResults(CEvaluationResults * targetResults, CEvaluationResults * sameLevelResults,
																	CEvaluationResults * acrossLevelResults)
{
	//Iterate over all segmentation result regions and choose the minimum error (false detection)
	map<CUniString, CLayoutObjectEvaluationResult*> * targetsegResultObjectResults = targetResults->GetSegResultObjectResults();
	map<CUniString, CLayoutObjectEvaluationResult*> * sameLevelsegResultObjectResults = sameLevelResults->GetSegResultObjectResults();
	map<CUniString, CLayoutObjectEvaluationResult*> * acrossLevelsegResultObjectResults = acrossLevelResults->GetSegResultObjectResults();

	for (map<CUniString, CLayoutObjectEvaluationResult*>::iterator it = sameLevelsegResultObjectResults->begin(); it != sameLevelsegResultObjectResults->end(); it++)
	{
		CUniString segResultObjectId = (*it).first;
		CLayoutObjectEvaluationResult * sameLevelsegResultObjectResult = (*it).second;

		map<CUniString, CLayoutObjectEvaluationResult*>::iterator itAcross = acrossLevelsegResultObjectResults->find(segResultObjectId);
		CLayoutObjectEvaluationResult * acrossLevelsegResultObjectResult = NULL;
		if (itAcross != acrossLevelsegResultObjectResults->end())
			acrossLevelsegResultObjectResult = (*itAcross).second;

		long sameLevelError = CalculateErrorSum(sameLevelsegResultObjectResult, 1L, 1L, 1L, 1L, 1L);
		long acrossLevelError = CalculateErrorSum(acrossLevelsegResultObjectResult, 1L, 1L, 1L, 1L, 1L);

		if (sameLevelError <= acrossLevelError || acrossLevelsegResultObjectResult == NULL)
			CopyLayoutObjectEvalResults(targetResults, targetsegResultObjectResults, sameLevelsegResultObjectResult);
		else
			CopyLayoutObjectEvalResults(targetResults, targetsegResultObjectResults, acrossLevelsegResultObjectResult);
	}
}

/*
 * Copies the given error object to the specified target result map
 */
void CLayoutEvaluator::CopyLayoutObjectEvalResults(CEvaluationResults * targetResults, map<CUniString, CLayoutObjectEvaluationResult*> * target, CLayoutObjectEvaluationResult * evalRes)
{
	target->insert(pair<CUniString, CLayoutObjectEvaluationResult*>(evalRes->GetRegion(), evalRes->Clone(targetResults)));
}

/*
 * Calculates the sum of all area error values for merge, split, miss, partial miss, misclass. and false detection.
 */
long CLayoutEvaluator::CalculateErrorSum(CLayoutObjectEvaluationResult * evalRes, long mergeMultiplier,
											long splitMultiplier, long missMultiplier, long partialMissMultiplier,
											long misclassMultiplier)
{
	if (evalRes == NULL)
		return 0L;

	long sum = 0L;
	CLayoutObjectEvaluationError * err;
	
	err = evalRes->GetError(CLayoutObjectEvaluationError::TYPE_MERGE);
	if (err != NULL)
		sum += err->GetArea() * mergeMultiplier;

	err = evalRes->GetError(CLayoutObjectEvaluationError::TYPE_SPLIT);
	if (err != NULL)
		sum += err->GetArea() * splitMultiplier;

	err = evalRes->GetError(CLayoutObjectEvaluationError::TYPE_MISS);
	if (err != NULL)
		sum += err->GetArea() * missMultiplier;

	err = evalRes->GetError(CLayoutObjectEvaluationError::TYPE_PART_MISS);
	if (err != NULL)
		sum += err->GetArea() * partialMissMultiplier;

	err = evalRes->GetError(CLayoutObjectEvaluationError::TYPE_MISCLASS);
	if (err != NULL)
		sum += err->GetArea() * misclassMultiplier;

	err = evalRes->GetError(CLayoutObjectEvaluationError::TYPE_INVENT);
	if (err != NULL)
		sum += err->GetArea();

	return sum;
}


/*
 * Prepare the overlap maps and other relevant stuff.
 */
void CLayoutEvaluator::ProcessGroundTruthObjects(int layoutObjectType, CLayoutEvaluation * layoutEval, int nestedRegionsMode /*= NESTED_REGION_MODE_IGNORE*/)
{
	if (layoutObjectType == CLayoutObject::TYPE_BORDER)
		PrepareGroundTruthBorder(layoutEval);
	else if (layoutObjectType == CLayoutObject::TYPE_READING_ORDER_GROUP)
		PrepareGroundTruthReadingOrderGroups(layoutEval);
	else //Regions/lines/words/glyphs
		PrepareGroundTruthRegionsLinesWordsGlyphs(layoutObjectType, layoutEval, nestedRegionsMode);
}

/*
 * Prepare the overlap maps and other relevant stuff for page border.
 */
void CLayoutEvaluator::PrepareGroundTruthBorder(CLayoutEvaluation * layoutEval)
{
	//Convert to isothetic and remove loops
	if (m_ConvertToIsothetic)
	{
		if (layoutEval->GetGroundTruth()->GetBorder() != NULL)
		{
			layoutEval->GetGroundTruth()->GetBorder()->SetSynchronized(true);
			layoutEval->GetGroundTruth()->GetBorder()->ConvertToIsothetic(true);
			layoutEval->GetGroundTruth()->GetBorder()->SetSynchronized(false);
		}
		if (layoutEval->GetSegResult()->GetBorder() != NULL)
		{
			layoutEval->GetSegResult()->GetBorder()->SetSynchronized(true);
			layoutEval->GetSegResult()->GetBorder()->ConvertToIsothetic(true);
			layoutEval->GetSegResult()->GetBorder()->SetSynchronized(false);
		}

		if (!m_EvaluateRegions) //If regions are evaluated as well, this step has been done already
		{
			CLayoutObjectIterator * regionIterator = GetLayoutObjectIterator(layoutEval->GetGroundTruth(), CLayoutObject::TYPE_LAYOUT_REGION, true);
			CLayoutObject * region;
			while (regionIterator->HasNext())
			{
				region = regionIterator->Next();
				region->GetCoords()->SetSynchronized(true);
				region->GetCoords()->ConvertToIsothetic(true);
				region->GetCoords()->SetSynchronized(false);
			}
			delete regionIterator;
		}
	}
	IncreaseProgress(m_MaxPartialProgress * 0.4); //40%
}

/*
 * Prepare the overlap maps and other relevant stuff for reading order groups.
 */
void CLayoutEvaluator::PrepareGroundTruthReadingOrderGroups(CLayoutEvaluation * layoutEval)
{
	//Convert to isothetic and remove loops (regions)
	if (m_ConvertToIsothetic)
		ConvertToIsothetic(CLayoutObject::TYPE_LAYOUT_REGION, layoutEval);

	CLayoutObjectIterator * objectIterator = new CReadingOrderGroupIterator(layoutEval->GetSegResult(), 2, false);
	if (objectIterator == NULL)
		return;

	//Calc bounding boxes
	while (objectIterator->HasNext())
		((CReadingOrderGroup*)objectIterator->Next())->CalcCoords(layoutEval->GetSegResult());

	//Build a bounding box search map for the segmentation result
	CBoundingBoxMap * boundingBoxMap = NULL;
	objectIterator->Reset();
	boundingBoxMap = new CBoundingBoxMap(objectIterator);
	delete objectIterator;
	IncreaseProgress(m_MaxPartialProgress * 0.05); //5%

	//Now iterate over the ground truth regions and find the overlapping segmentation result regions
	CEvaluationResults * results = layoutEval->GetResults(CLayoutObject::TYPE_READING_ORDER_GROUP, true);
	objectIterator = new CReadingOrderGroupIterator(layoutEval->GetGroundTruth(), 2, false);

	while (objectIterator->HasNext())
	{
		CReadingOrderGroup * group = (CReadingOrderGroup*)objectIterator->Next();
		//Calc bounding boxes
		group->CalcCoords(layoutEval->GetSegResult());
		//Overlaps
		CalculateOverlaps(group, boundingBoxMap, results);
	}
	delete objectIterator;

	delete boundingBoxMap;
	IncreaseProgress(m_MaxPartialProgress * 0.3); //30%
}

/*
 * Prepare the overlap maps and other relevant stuff for regions/linse/words/glyphs.
 */
void CLayoutEvaluator::PrepareGroundTruthRegionsLinesWordsGlyphs(int layoutObjectType, CLayoutEvaluation * layoutEval, int nestedRegionsMode /*= NESTED_REGION_MODE_IGNORE*/)
{
	CEvaluationResults * results = layoutEval->GetResults(layoutObjectType, true);
	CLayoutObjectIterator * objectIterator = NULL;

	//Convert to isothetic and remove loops
	if (m_ConvertToIsothetic)
		ConvertToIsothetic(layoutObjectType, layoutEval);

	//Build a bounding box search map for the segmentation result
	CBoundingBoxMap * boundingBoxMap = NULL;
	// Regions
	if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
	{
		vector<CLayoutObject*> segResRegions;
		if (nestedRegionsMode == NESTED_REGION_MODE_NESTED_TO_PARENT || nestedRegionsMode == NESTED_REGION_MODE_PARENT_TO_PARENT)
			GetParentRegions(layoutEval->GetSegResult(), &segResRegions);
		else //Nested only
			GetNestedRegions(layoutEval->GetSegResult(), &segResRegions);
		boundingBoxMap = new CBoundingBoxMap(&segResRegions);
	}
	else //Lines, words, glyphs
	{
		objectIterator = GetLayoutObjectIterator(layoutEval->GetSegResult(), layoutObjectType);
		boundingBoxMap = new CBoundingBoxMap(objectIterator);
		delete objectIterator;
	}
	IncreaseProgress(m_MaxPartialProgress * 0.05); //5%

	//Now iterate over the ground truth regions and find the overlapping segmentation result regions
	// Regions
	if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
	{
		vector<CLayoutObject*> groundTruthObjects;
		if (nestedRegionsMode == NESTED_REGION_MODE_PARENT_TO_PARENT || nestedRegionsMode == NESTED_REGION_MODE_PARENT_TO_NESTED)
			GetParentRegions(layoutEval->GetGroundTruth(), &groundTruthObjects);
		else //Nested only
			GetNestedRegions(layoutEval->GetGroundTruth(), &groundTruthObjects);

		for (vector<CLayoutObject*>::iterator it = groundTruthObjects.begin(); it != groundTruthObjects.end(); it++)
			CalculateOverlaps((*it), boundingBoxMap, results);
	}
	else //Lines, words, glyphs
	{
		objectIterator = GetLayoutObjectIterator(layoutEval->GetGroundTruth(), layoutObjectType);
		while (objectIterator->HasNext())
			CalculateOverlaps(objectIterator->Next(), boundingBoxMap, results);
		delete objectIterator;
	}

	delete boundingBoxMap;

	IncreaseProgress(m_MaxPartialProgress * 0.3); //30%
}

/*
 * Convert to isothetic and remove loops
 */
void CLayoutEvaluator::ConvertToIsothetic(int layoutObjectType, CLayoutEvaluation * layoutEval)
{
	CLayoutObjectIterator * objectIterator = GetLayoutObjectIterator(layoutEval->GetGroundTruth(), layoutObjectType, true); //Include nested
	CLayoutObject * region;
	while (objectIterator->HasNext())
	{
		region = objectIterator->Next();
		region->GetCoords()->SetSynchronized(true);
		region->GetCoords()->ConvertToIsothetic(true);
		region->GetCoords()->SetSynchronized(false);
	}
	delete objectIterator;
	IncreaseProgress(m_MaxPartialProgress * 0.05); //5%
												   // Seg Result
	objectIterator = GetLayoutObjectIterator(layoutEval->GetSegResult(), layoutObjectType, true);
	while (objectIterator->HasNext())
	{
		region = objectIterator->Next();
		region->GetCoords()->SetSynchronized(true);
		region->GetCoords()->ConvertToIsothetic(true);
		region->GetCoords()->SetSynchronized(false);
	}
	delete objectIterator;
}


void CLayoutEvaluator::CalculateOverlaps(CLayoutObject * groundTruthObject, CBoundingBoxMap * boundingBoxMap, CEvaluationResults * results)
{
	//Get the overlap candidates by bounding box check
	set<CLayoutObject*> * overlappingObjects = boundingBoxMap->GetOverlappingRegions(groundTruthObject);

	//Generate the interval representations
	CIntervalRepresentation * intReprGT = NULL;
	vector<CIntervalRepresentation *> intRepsForMultiOverlap;
	for (set<CLayoutObject*>::iterator it = overlappingObjects->begin(); it != overlappingObjects->end(); it++)
	{
		CLayoutObject * segObject = (*it);
		CIntervalRepresentation * intReprSeg = NULL;

		//Reading order group (use child regions)
		/*if (groundTruthObject->GetLayoutObjectType() == CLayoutObject::TYPE_READING_ORDER_GROUP)
		{
			//Generate the interval representations
			if (intReprGT == NULL)
			{
				intReprGT = results->GetIntervalRepresentation(groundTruthObject->GetId(), true, false);
				intReprGT = CalculateIntervalRepresentation((CReadingOrderGroup*)groundTruthObject);
				if (intReprGT == NULL)
					break;
				intReprGT->SetRegion(groundTruthObject);
				intRepsForMultiOverlap.push_back(intReprGT);
				results->AddIntervalRepresentation(groundTruthObject, intReprGT, true);
			}
		}
		else //Region/line/word/glyph
		{
			//Generate the interval representations
			if (intReprGT == NULL)
			{
				intReprGT = new CIntervalRepresentation(groundTruthObject->GetCoords(),
					false, groundTruthObject);
				intRepsForMultiOverlap.push_back(intReprGT);
				results->AddIntervalRepresentation(groundTruthObject, intReprGT, true);
			}
		}*/

		intReprGT = results->GetIntervalRepresentation(groundTruthObject->GetId(), true, true);
		intReprSeg = results->GetIntervalRepresentation(segObject->GetId(), true, false);
		//results->AddIntervalRepresentation(segRegion, intReprSeg, false); //CC 18.01.2013: is already done in GetIntervalRepresentation

		//Compute the overlap
		CLayoutObjectOverlap * overlap = new CLayoutObjectOverlap(intReprGT, intReprSeg);

		if (overlap->IsOverlapping())
		{
			intRepsForMultiOverlap.push_back(intReprSeg);
			results->AddLayoutObjectOverlap(groundTruthObject->GetId(), segObject->GetId(), overlap);
		}
		else
			delete overlap;
	}
	//Compute the multi overlap (ground truth region with all overlapping segmentation result regions)
	if (intRepsForMultiOverlap.size() > 2) //(only if gt + 2 or more seg result regions)
	{
		CLayoutObjectOverlap * overlap = new CLayoutObjectOverlap(&intRepsForMultiOverlap);
		results->AddMultiOverlapIntervalRep(groundTruthObject, overlap);
	}
	delete overlappingObjects;
}

/*
 * Finds the different errors (split, merge, ...) for all regions.
 */
void CLayoutEvaluator::FindErrors(int layoutObjectType, CLayoutEvaluation * layoutEval, int nestedRegionsMode /*= NESTED_REGION_MODE_IGNORE*/)
{
	//Border
	if (layoutObjectType == CLayoutObject::TYPE_BORDER)
	{
		FindBorderErrors(layoutEval);
		return;
	}

	//Ground Truth (for merge, split, miss, part. miss and misclassification)
	CEvaluationResults * results = layoutEval->GetResults(layoutObjectType);
	CLayoutObject * groundTruthObject;
	set<CUniString> * segResultObjects;
	CLayoutObjectEvaluationResult * result;
	if (m_EvaluateRegions || m_EvaluateTextLines || m_EvaluateWords || m_EvaluateGlyphs || m_EvaluateReadingOrderGroups)
	{
		CLayoutObjectIterator * it = NULL;

		if (	m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_MERGE]
			||	m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_SPLIT]
			||	m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_MISS]
			||	m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_PART_MISS]
			||	m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_MISCLASS])
		{
			//Regions
			if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
			{
				vector<CLayoutObject*> groundTruthObjects;
				if (nestedRegionsMode == NESTED_REGION_MODE_PARENT_TO_PARENT || nestedRegionsMode == NESTED_REGION_MODE_PARENT_TO_NESTED)
					GetParentRegions(layoutEval->GetGroundTruth(), &groundTruthObjects);
				else //Nested only
					GetNestedRegions(layoutEval->GetGroundTruth(), &groundTruthObjects);

				for (vector<CLayoutObject*>::iterator it = groundTruthObjects.begin(); it != groundTruthObjects.end(); it++) 
				{
					groundTruthObject = (*it);
					segResultObjects = results->GetGroundTruthOverlaps(groundTruthObject->GetId());
					result = results->GetGroundTruthObjectResult(groundTruthObject->GetId(), true);

					FindGroundTruthBasedErrorsForLayoutObject(layoutObjectType, results, result, groundTruthObject, segResultObjects);
				}
			}
			else //Textlines, words, glyphs, reading order groups
			{
				it = GetLayoutObjectIterator(layoutEval->GetGroundTruth(), layoutObjectType);
				while (it->HasNext())
				{
					groundTruthObject = it->Next();
					segResultObjects = results->GetGroundTruthOverlaps(groundTruthObject->GetId());
					result = results->GetGroundTruthObjectResult(groundTruthObject->GetId(), true);

					FindGroundTruthBasedErrorsForLayoutObject(layoutObjectType, results, result, groundTruthObject, segResultObjects);
				}
				delete it;
			}
		}

		//Extended allowable check (CC 13.06.2011)
		//  The basic allowable check (used above) only takes into account region pairs.
		//  Hence, chains of allowable merges are not detected. If for instance three regions
		//  A, B and C are merged (in reading order), then the merges between A and B as well
		//  as between B and C are allowable, but NOT the merge between A and C. 
		//  This issue is solved in this second pass which looks for sequences of allowable
		//  merges and corrects the allowable flag accordingly.
		if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
			AllowableSequenceDetection(results, layoutEval);

		IncreaseProgress(m_MaxPartialProgress * 0.3); //30%

		//Segmentation result (for false detection)
		if (m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_INVENT])
		{
			//Regions
			CLayoutObject * segResultObject;
			set<CUniString> * groundTruthObjects;
			if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
			{
				vector<CLayoutObject*> segResRegions;
				if (nestedRegionsMode == NESTED_REGION_MODE_PARENT_TO_PARENT || nestedRegionsMode == NESTED_REGION_MODE_NESTED_TO_PARENT)
					GetParentRegions(layoutEval->GetSegResult(), &segResRegions);
				else //Nested only
					GetNestedRegions(layoutEval->GetSegResult(), &segResRegions);

				for (vector<CLayoutObject*>::iterator it = segResRegions.begin(); it != segResRegions.end(); it++)
				{
					segResultObject = (*it);
					groundTruthObjects = results->GetSegResultOverlaps(segResultObject->GetId());
					result = results->GetSegResultObjectResult(segResultObject->GetId(), true);

					//False detection
					CheckInvented(layoutObjectType, results, result, segResultObject, groundTruthObjects);
				}
			}
			else //Textlines, words, glyphs, reading order groups
			{
				it = GetLayoutObjectIterator(layoutEval->GetSegResult(), layoutObjectType);
				while (it->HasNext())
				{
					segResultObject = it->Next();
					groundTruthObjects = results->GetSegResultOverlaps(segResultObject->GetId());
					result = results->GetSegResultObjectResult(segResultObject->GetId(), true);

					//False detection
					CheckInvented(layoutObjectType, results, result, segResultObject, groundTruthObjects);
				}
				delete it;
			}
		}
	}
	IncreaseProgress(m_MaxPartialProgress * 0.1); //10%

	//Reading Order Tree
	if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION && m_EvaluateReadingOrder) //only on block level
	{
		EvaluateReadingOrder(results, layoutEval);
	}
	IncreaseProgress(m_MaxPartialProgress * 0.1); //10%
}

void CLayoutEvaluator::FindGroundTruthBasedErrorsForLayoutObject(int layoutObjectType, CEvaluationResults * results, CLayoutObjectEvaluationResult * result,
																CLayoutObject * groundTruthObject, set<CUniString> * segResultObjects) {
	if (m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_MERGE])
		CheckMerge(layoutObjectType, results, result, groundTruthObject, segResultObjects);
	if (m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_SPLIT])
		CheckSplit(layoutObjectType, results, result, groundTruthObject, segResultObjects);
	if (m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_MISS])
		CheckMiss(layoutObjectType, results, result, groundTruthObject, segResultObjects);
	if (m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_PART_MISS])
		CheckPartMiss(layoutObjectType, results, result, groundTruthObject, segResultObjects);
	if (m_EnableErrorChecks[CLayoutObjectEvaluationError::TYPE_MISCLASS])
		CheckMisclass(layoutObjectType, results, result, groundTruthObject, segResultObjects);
}

void CLayoutEvaluator::FindBorderErrors(CLayoutEvaluation * layoutEval)
{
	CEvaluationResults * results = layoutEval->GetResults(CLayoutObject::TYPE_BORDER);
	CBorderEvaluationResults * borderResults = results->GetBorderResults(true);

	//Borders (create maximal if not exists)
	CLayoutPolygon * groundTruthBorder = layoutEval->GetGroundTruth()->GetBorder();
	bool deleteGtBorder = false;
	if (groundTruthBorder == NULL)
	{
		groundTruthBorder = new CLayoutPolygon();
		groundTruthBorder->AddPoint(0,0);
		groundTruthBorder->AddPoint(layoutEval->GetWidth()-1,0);
		groundTruthBorder->AddPoint(layoutEval->GetWidth()-1, layoutEval->GetHeight()-1);
		groundTruthBorder->AddPoint(0, layoutEval->GetHeight()-1);
		deleteGtBorder = true;
	}
	CLayoutPolygon * segResultBorder = layoutEval->GetSegResult()->GetBorder();
	bool deleteSegBorder = false;
	if (segResultBorder == NULL)
	{
		segResultBorder = new CLayoutPolygon();
		segResultBorder->AddPoint(0,0);
		segResultBorder->AddPoint(layoutEval->GetWidth()-1,0);
		segResultBorder->AddPoint(layoutEval->GetWidth()-1, layoutEval->GetHeight()-1);
		segResultBorder->AddPoint(0, layoutEval->GetHeight()-1);
		deleteSegBorder = true;
	}

	//Inverval representations and areas
	CIntervalRepresentation * gtIntervalRepr = new CIntervalRepresentation(groundTruthBorder, false, NULL);
	borderResults->SetGroundTruthBorderIntervalRep(gtIntervalRepr);
	long gtArea = gtIntervalRepr->GetArea();
	CIntervalRepresentation * segIntervalRepr = new CIntervalRepresentation(segResultBorder, false, NULL);
	borderResults->SetSegResultBorderIntervalRep(segIntervalRepr);
	long segArea = segIntervalRepr->GetArea();

	//Overlap GT with seg result
	CLayoutObjectOverlap overlapGtSeg(gtIntervalRepr, segIntervalRepr);
	long overlapAreaGtSeg = overlapGtSeg.GetOverlapArea();

	//Included background area
	CBorderEvaluationError * inclAreaError = borderResults->GetIncludedBackgroundError();
	inclAreaError->SetArea(segArea - overlapAreaGtSeg);

	//Missing region area errors
	map<CUniString, CLayoutObjectEvaluationError*> * missingRegionAreaErrors = borderResults->GetMissingRegionAreaErrors();
	long overallMissingRegionArea = 0L;
	CLayoutObjectIterator * regionIterator = GetLayoutObjectIterator(layoutEval->GetGroundTruth(), CLayoutObject::TYPE_LAYOUT_REGION);
	CLayoutObject * region;
	while (regionIterator->HasNext())
	{
		region = regionIterator->Next();
		if (region->GetCoords() == NULL || region->GetCoords()->GetNoPoints()==0)
			continue;
		//Interval repr. for the region
		CIntervalRepresentation regIntervalRepr(region->GetCoords(), false, NULL);
		long regArea = regIntervalRepr.GetArea();
		//Overlap reg - GT border
		CLayoutObjectOverlap overlap(&regIntervalRepr, segIntervalRepr);
		long overlapArea = overlap.GetOverlapArea();
		//Error area
		long errorArea = regArea - overlapArea;
		if (errorArea > 0L)
		{
			overallMissingRegionArea += errorArea;
			CLayoutObjectEvaluationError * err = new CLayoutObjectEvaluationError(CLayoutObjectEvaluationError::TYPE_BORDER, region->GetId());
			err->SetArea(errorArea);
			missingRegionAreaErrors->insert(pair<CUniString, CLayoutObjectEvaluationError*>(region->GetId(), err));
		}
	}
	delete regionIterator;

	//Excluded foreground area
	CBorderEvaluationError * exclAreaError = borderResults->GetExcludedForegroundError();
	exclAreaError->SetArea(gtArea - overlapAreaGtSeg - overallMissingRegionArea);

	if (deleteGtBorder)
		delete groundTruthBorder;
	if (deleteSegBorder)
		delete segResultBorder;
}

/* 
 * Checks for merge
 */
void CLayoutEvaluator::CheckMerge(	int layoutObjectType, CEvaluationResults * results,
								    CLayoutObjectEvaluationResult * result,
									CLayoutObject * groundTruthObject, 
									set<CUniString> * segResultObjects)
{
	if (segResultObjects == NULL || segResultObjects->empty()) //No merge
		return;

	//Merge: A ground truth object is connected to another ground truth object via a segmentation
	//       result object. The first ground truth object and all overlapping segmentation result
	//       objects are given. So we have to check for each segmentation result object if it 
	//       overlaps another ground truth object other than the given one.

	CEvaluationErrorMerge * err = new CEvaluationErrorMerge(groundTruthObject->GetId());
	set<CUniString>::iterator it = segResultObjects->begin();
	CUniString segResultObjectId;
	CLayoutObject * segResultObject;
	CUniString groundTruthObject2Id;
	CLayoutObject * groundTruthObject2;
	set<CUniString> * mergedgroundTruthObjects;
	long area = 0L;
	long pixelCount = 0L;
	//Iterate over the given segmentation result objects
	while (it != segResultObjects->end())
	{
		segResultObjectId = (*it);
		segResultObject = results->GetDocumentLayoutObject(segResultObjectId, false);;

		CLayoutObjectOverlap * overlap1 = results->GetOverlapIntervalRep(groundTruthObject->GetId(), 
																  segResultObjectId);

		COverlapRects * rects = new COverlapRects();
		//Find the overlapping ground truth objects for the current segmentation result object
		mergedgroundTruthObjects = results->GetSegResultOverlaps(segResultObjectId);
		if (mergedgroundTruthObjects != NULL && mergedgroundTruthObjects->size() > 1)
		{
			//Iterate over the ground truth objects, that overlap the current segmentation result object
			set<CUniString>::iterator it2 = mergedgroundTruthObjects->begin();
			while (it2 != mergedgroundTruthObjects->end())
			{
				groundTruthObject2Id = (*it2);
				groundTruthObject2 = results->GetDocumentLayoutObject(groundTruthObject2Id, true);
				if (groundTruthObject != groundTruthObject2) //exclude the given ground truth object
				{
					//Get the interval representation of the overlap
					CLayoutObjectOverlap * overlap2 = results->GetOverlapIntervalRep(groundTruthObject2Id, 
																			  segResultObjectId);
					if (overlap2 != NULL)
					{
						//Use the smaller of the two overlap areas
						CLayoutObjectOverlap * overlap = NULL;
						if (m_UsePixelArea)
						{
							//Pixel count
							long count1 = results->GetLayoutEvaluation()->GetBilevelImage()->CountPixels(overlap1->GetOverlapRects());
							long count2 = results->GetLayoutEvaluation()->GetBilevelImage()->CountPixels(overlap2->GetOverlapRects());
							if (count1 < count2)
								overlap = overlap1;
							else
								overlap = overlap2;
						}
						else if (overlap1->GetOverlapArea() < overlap2->GetOverlapArea())
							overlap = overlap1;
						else
							overlap = overlap2;

						rects->AddOverlapRects(	groundTruthObject2->GetId(), overlap, m_UsePixelArea,
												results->GetLayoutEvaluation()->GetBilevelImage());
						err->AddRects(overlap->GetOverlapRects());

						err->SetAllowable(groundTruthObject2->GetId(), CheckIfMergeAllowable(results->GetLayoutEvaluation()->GetGroundTruth()->GetReadingOrder(), 
																					groundTruthObject,
																					groundTruthObject2));
					}
					else
					{
						//TODO Exception
					}
				}
				it2++;
			}
		}
		err->AddErrorRects(segResultObject->GetId(), rects);
		area += rects->GetArea();
		pixelCount += rects->GetPixelCount();
		it++;
	}

	if (!err->GetRects()->empty()) //There is an error
	{
		if (m_UsePixelArea) //Using pixel area
			CheckFalseAlarm(err); //Check for false alarm
		err->SetArea(area);
		err->SetPixelCount(pixelCount);
		if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
			err->SetForNestedRegion(!groundTruthObject->GetParent().IsNull());
		result->AddError(err);
	}
	else
		delete err;
}

/*
 * Checks the rects of the given error, if they have black pixels.
 * Each rect without any black pixel is removed from the rect list
 * and added to the false alarm list.
 * Also sets the area and pixel count for the error.
 * Returns true, if at least one rect contains black pixels, false otherwise.
 */
bool CLayoutEvaluator::CheckFalseAlarm(CLayoutObjectEvaluationError * err)
{
	bool isError = false;
	list<CRect*> * falseAlarm = err->GetFalseAlarmRects();
	if (!falseAlarm->empty())
	{
		for (list<CRect*>::iterator it = falseAlarm->begin(); it != falseAlarm->end(); it++)
			delete (*it);
		falseAlarm->clear();
	}

	map<CRect*, long> realErrors;
	list<CRect*> * origRects = err->GetRects();
	long pixels = SplitByPixelArea(origRects, falseAlarm, &realErrors);
	if (!falseAlarm->empty())
	{
		if (realErrors.size() == 0) //No error at all
			isError = false;
		else //There were real errors
			isError = true;

		//Clear the old rect list and insert the realError rects.
		for (list<CRect*>::iterator it = origRects->begin(); it != origRects->end(); it++)
			delete (*it);
		origRects->clear();
		
		map<CRect*, long>::iterator it = realErrors.begin();
		while (it != realErrors.end())
		{
			origRects->push_back((*it).first);
			it++;
		}
	}
	else //There was no false alarm -> leave everything as it is
	{
		isError = !realErrors.empty();
		//Delete
		map<CRect*, long>::iterator it = realErrors.begin();
		while (it != realErrors.end())
		{
			delete (*it).first;
			it++;
		}
	}
	err->SetPixelCount(pixels);
	err->SetFalseAlarm(!isError);
	return isError;
}

/* 
 * Checks for split
 */
void CLayoutEvaluator::CheckSplit(	int layoutObjectType, CEvaluationResults * results,
								    CLayoutObjectEvaluationResult * result,
									CLayoutObject * groundTruthObject, 
									set<CUniString> * segResultObjects)
{
	if (segResultObjects == NULL || segResultObjects->size() <= 1) //No split
		return;

	CEvaluationErrorSplit * err = new CEvaluationErrorSplit(groundTruthObject->GetId());

	//Number of splitting regions
	err->SetCount((int)segResultObjects->size());

	//Error area
	set<CUniString>::iterator it = segResultObjects->begin();
	CLayoutObject * segResultObject, * segResultObject2;
	CLayoutObjectOverlap * overlap;
	bool allowable = true;
	while (it != segResultObjects->end())
	{
		segResultObject = results->GetDocumentLayoutObject((*it), false);
		overlap = results->GetOverlapIntervalRep(groundTruthObject->GetId(), segResultObject->GetId());

		if (overlap != NULL)
		{
			err->AddErrorRects(	segResultObject->GetId(), overlap, m_UsePixelArea, 
								results->GetLayoutEvaluation()->GetBilevelImage());

			//Allowable?
			if (allowable) //still allowable
			{
				if (groundTruthObject->GetLayoutObjectType() == CLayoutObject::TYPE_LAYOUT_REGION)
				{
					CLayoutRegion * layoutReg = (CLayoutRegion*)groundTruthObject;
					if (layoutReg->GetType() == CLayoutRegion::TYPE_TEXT)
					{
						CTextRegionInfo * info = (CTextRegionInfo*)layoutReg->GetRegionInfo();
						CUniString direction = GetReadingDirection(info);
						double orientation = GetReadingOrientation(info);
						set<CUniString>::iterator it2 = segResultObjects->begin();
						while (it2 != segResultObjects->end())
						{
							segResultObject2 = results->GetDocumentLayoutObject((*it2), false);
							if (segResultObject != segResultObject2)
							{
								if (!CheckIfMergeAllowable(segResultObject, segResultObject2, direction, orientation))
								{
									allowable = false;
									break;
								}
							}
							it2++;
						}
					}
				}
			}
			
		}
		else
		{
			//TODO error
		}
		it++;
	}
	err->SetAllowable(allowable);

	if (!err->GetRects()->empty()) //There is an error
	{
		if (m_UsePixelArea) //Using pixel area
			CheckFalseAlarm(err); //Check for false alarm
		if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
			err->SetForNestedRegion(!groundTruthObject->GetParent().IsNull());
		result->AddError(err);
	}
	else
		delete err;
}

/* 
 * Checks complete miss
 */
void CLayoutEvaluator::CheckMiss(	int layoutObjectType, CEvaluationResults * results,
								    CLayoutObjectEvaluationResult * result,
									CLayoutObject * groundTruthObject, 
									set<CUniString> * segResultObjects)
{
	if (segResultObjects == NULL || segResultObjects->empty())
	{
		CLayoutObjectEvaluationError * err = new CLayoutObjectEvaluationError(CLayoutObjectEvaluationError::TYPE_MISS, groundTruthObject->GetId());
		err->SetArea(results->GetRegionArea(groundTruthObject->GetId(), true));
		if (m_UsePixelArea)
		{
			long pixelCount = results->GetPixelCount(groundTruthObject->GetId(), true);
			err->SetPixelCount(pixelCount);
			if (pixelCount == 0L)
				err->SetFalseAlarm(true);
		}
		if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
			err->SetForNestedRegion(!groundTruthObject->GetParent().IsNull());
		result->AddError(err);
	}
}

/* 
 * Checks for partly miss
 */
void CLayoutEvaluator::CheckPartMiss(	int layoutObjectType, CEvaluationResults * results,
										CLayoutObjectEvaluationResult * result,
										CLayoutObject * groundTruthObject, 
										set<CUniString> * segResultObjects)
{
	if (segResultObjects == NULL || segResultObjects->empty()) //No partly miss
		return;

	CLayoutObjectOverlap * overlap = NULL;

	if (segResultObjects->size() == 1) //only one region overlaps -> look in the single overlap map
	{
		CLayoutObject * segResultObject = results->GetDocumentLayoutObject((*segResultObjects->begin()), false);
		overlap = results->GetOverlapIntervalRep(groundTruthObject->GetId(), segResultObject->GetId());
	}
	else //more than one region overlap -> look in the multi overlap map
	{
		overlap = results->GetMultiOverlapIntervalRep(groundTruthObject->GetId());
	}
	if (overlap != NULL)
	{
		vector<CRect *> * rects = overlap->GetUniqueRects(groundTruthObject);
		if (rects != NULL && !rects->empty()) //A part of the ground truth region does not overlap with any segmentation result region
		{
			CLayoutObjectEvaluationError * err = new CLayoutObjectEvaluationError(CLayoutObjectEvaluationError::TYPE_PART_MISS, 
																	groundTruthObject->GetId());
			err->AddRects(rects);

			if (!err->GetRects()->empty()) //There is an error
			{
				if (m_UsePixelArea) //Using pixel area
					CheckFalseAlarm(err); //Check for false alarm

				if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
					err->SetForNestedRegion(!groundTruthObject->GetParent().IsNull());

				result->AddError(err);

				//Area
				long area = 0L;
				for (unsigned int i=0; i<rects->size(); i++)
					area += (rects->at(i)->Width()+1) * (rects->at(i)->Height()+1);
				err->SetArea(area);
			}
			else
				delete err;

			//Delete the rects, because err->AddRects(rects) copies the rects
			for (unsigned int i=0; i<rects->size(); i++)
				delete rects->at(i);
		}
		delete rects;
	}
	else
	{
		//TODO error
	}
}

/* 
 * Checks misclassification
 */
void CLayoutEvaluator::CheckMisclass(int layoutObjectType, CEvaluationResults * results,
	CLayoutObjectEvaluationResult * result,
	CLayoutObject * groundTruthObject,
	set<CUniString> * segResultObjects)
{
	if (segResultObjects == NULL || segResultObjects->empty()) //No misclassification
		return;

	if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
		CheckMisclassForRegion(results, result, groundTruthObject, segResultObjects);
	else if (layoutObjectType == CLayoutObject::TYPE_READING_ORDER_GROUP)
		CheckMisclassForReadingOrderGroup(results, result, groundTruthObject, segResultObjects);
}

/* 
 * Checks misclassification for layout regions
 */
void CLayoutEvaluator::CheckMisclassForRegion(CEvaluationResults * results,
										CLayoutObjectEvaluationResult * result,
										CLayoutObject * groundTruthObject, 
										set<CUniString> * segResultObjects)
{
	CEvaluationErrorMisclass * err = new CEvaluationErrorMisclass(groundTruthObject->GetId());

	set<CUniString>::iterator it = segResultObjects->begin();
	CLayoutRegion * segResultLayoutRegion;
	CLayoutRegion * groundTruthLayoutRegion = (CLayoutRegion*)groundTruthObject;
	CLayoutObjectOverlap * overlap;
	int overallArea = 0;
	while (it != segResultObjects->end())
	{
		segResultLayoutRegion = (CLayoutRegion*)results->GetDocumentLayoutObject((*it), false);
		//Compare region type
		bool misclass = segResultLayoutRegion->GetType() != groundTruthLayoutRegion->GetType();

		//Subtype?
		if (!misclass)
		{
			CUniString gtSubType = CLayoutEvaluation::GetLayoutRegionSubtype(groundTruthLayoutRegion, m_Profile);
			CUniString segResultSubType = CLayoutEvaluation::GetLayoutRegionSubtype(segResultLayoutRegion, m_Profile);
			if (!gtSubType.IsEmpty())
				misclass = gtSubType != segResultSubType;
		}

		if (misclass && !CheckIfMisclassCanBeIgnored(groundTruthLayoutRegion, segResultLayoutRegion->GetType()))
		{
			overlap = results->GetOverlapIntervalRep(groundTruthObject->GetId(), segResultLayoutRegion->GetId());
			if (overlap != NULL)
			{
				err->AddErrorRects(	segResultLayoutRegion->GetId(), overlap,
									m_UsePixelArea, 
									results->GetLayoutEvaluation()->GetBilevelImage());
				overallArea += overlap->GetOverlapArea();
			}
			else
			{
				//TODO error
			}
		}
		it++;
	}
	err->SetArea(overallArea);

	if (!err->GetRects()->empty()) //There is an error
	{
		if (m_UsePixelArea) //Using pixel area
			CheckFalseAlarm(err); //Check for false alarm

		err->SetForNestedRegion(!groundTruthObject->GetParent().IsNull());

		result->AddError(err);
	}
	else
		delete err;
}

/* 
 * Checks misclassification for reading order groups
 */
void CLayoutEvaluator::CheckMisclassForReadingOrderGroup(CEvaluationResults * results,
										CLayoutObjectEvaluationResult * result,
										CLayoutObject * groundTruthObject, 
										set<CUniString> * segResultObjects)
{
	CEvaluationErrorMisclass * err = new CEvaluationErrorMisclass(groundTruthObject->GetId());

	set<CUniString>::iterator it = segResultObjects->begin();
	CReadingOrderGroup * segResultGroup;
	CReadingOrderGroup * groundTruthGroup = (CReadingOrderGroup*)groundTruthObject;
	CLayoutObjectOverlap * overlap;
	int overallArea = 0;
	while (it != segResultObjects->end())
	{
		segResultGroup = (CReadingOrderGroup*)results->GetDocumentLayoutObject((*it), false);
		//Compare group type
		bool misclass = segResultGroup->GetGroupType() != groundTruthGroup->GetGroupType();

		if (misclass)
		{
			overlap = results->GetOverlapIntervalRep(groundTruthObject->GetId(), segResultGroup->GetId());
			if (overlap != NULL)
			{
				err->AddErrorRects(segResultGroup->GetId(), overlap,
									m_UsePixelArea, 
									results->GetLayoutEvaluation()->GetBilevelImage());
				overallArea += overlap->GetOverlapArea();
			}
			else
			{
				//TODO error
			}
		}
		it++;
	}
	err->SetArea(overallArea);

	if (!err->GetRects()->empty()) //There is an error
	{
		if (m_UsePixelArea) //Using pixel area
			CheckFalseAlarm(err); //Check for false alarm

		result->AddError(err);
	}
	else
		delete err;
}

/*
 * Checks if misclassification can be ignored.
 * At the moment this can only happen if a chart/graphic/table/... with embedded text flag is misclassified as text region (and a corresponding switch is set in the profile)
 *
 * returns 'true' if the misclassification of the given regions can be ignored
 */
bool CLayoutEvaluator::CheckIfMisclassCanBeIgnored(CLayoutRegion * groundTruthObject, int resultType)
{
	if (groundTruthObject == NULL)
		return false;

	//Check flag first
	if (!m_Profile->IsIgnoreEmbeddedTextMisclass()) //-> do not ignore
		return false;

	//Check if result type is TEXT
	if (resultType != CLayoutRegion::TYPE_TEXT)
		return false;

	//Check if embedded text flag is set to TRUE
	bool embeddedTextFlag = false;
	if (groundTruthObject->GetRegionInfo() != NULL)
	{
		if (groundTruthObject->GetType() == CLayoutRegion::TYPE_CHART)
			embeddedTextFlag = ((CChartRegionInfo*)groundTruthObject->GetRegionInfo())->IsEmbeddedTextSet();
		else if (groundTruthObject->GetType() == CLayoutRegion::TYPE_GRAPHIC)
			embeddedTextFlag = ((CGraphicRegionInfo*)groundTruthObject->GetRegionInfo())->IsEmbeddedTextSet();
		else if (groundTruthObject->GetType() == CLayoutRegion::TYPE_TABLE)
			embeddedTextFlag = ((CTableRegionInfo*)groundTruthObject->GetRegionInfo())->IsEmbeddedTextSet();
		else if (groundTruthObject->GetType() == CLayoutRegion::TYPE_IMAGE)
			embeddedTextFlag = ((CImageRegionInfo*)groundTruthObject->GetRegionInfo())->IsEmbeddedTextSet();
		else if (groundTruthObject->GetType() == CLayoutRegion::TYPE_LINEART)
			embeddedTextFlag = ((CLineDrawingRegionInfo*)groundTruthObject->GetRegionInfo())->IsEmbeddedTextSet();
	}
	if (!embeddedTextFlag)
		return false;

	//All conditions fulfilled -> ignore misclassification
	return true;
}

/* 
 * Checks for false detection
 */
void CLayoutEvaluator::CheckInvented(	int layoutObjectType, CEvaluationResults * results,
										CLayoutObjectEvaluationResult * result,
										CLayoutObject * segResultObject, 
										set<CUniString> * groundTruthObjects)
{
	if (groundTruthObjects == NULL || groundTruthObjects->empty())
	{
		CLayoutObjectEvaluationError * err = new CLayoutObjectEvaluationError(CLayoutObjectEvaluationError::TYPE_INVENT,
																segResultObject->GetId());
		err->SetArea(results->GetRegionArea(segResultObject->GetId(), false));
		if (m_UsePixelArea)
			err->SetPixelCount(results->GetPixelCount(segResultObject->GetId(), false));
		err->SetArea(results->GetRegionArea(segResultObject->GetId(), false));

		if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
			err->SetForNestedRegion(!segResultObject->GetParent().IsNull());

		result->AddError(err);
	}
}

/*
 * Splits the given list of rects into a list of false alarm rects
 * and a list of real error rects, by counting the black pixels
 * in the bi-level image.
 * 'input' - the original list
 * 'falseAlarm' - rectangles without black pixels
 * 'realErrors' - map of rectangles with black pixels and the number of the pixels
 *   NOTE: Creates new rects for falseAlarm and realErrors, so they have to be deleted later!
 * Returns the overall number of black pixels
 */
long CLayoutEvaluator::SplitByPixelArea(list<CRect*> * input, 
										list<CRect*> * falseAlarm, 
										map<CRect*, long> * realErrors)
{
	CRect * rect;
	COpenCvBiLevelImage * image = m_Image;
	long count, overall = 0;
	for (list<CRect*>::iterator it = input->begin(); it != input->end(); it++)
	{
		rect = (*it);
		count = image->CountPixels(rect);
		if (count > 0)
		{
			overall += count;
			realErrors->insert(pair<CRect*, long>(new CRect(rect), count));
		}
		else
			falseAlarm->push_back(new CRect(rect));
	}
	return overall;
}

/*
 * Returns a region iterator for the given region type.
 */
CLayoutObjectIterator * CLayoutEvaluator::GetLayoutObjectIterator(CPageLayout * pageLayout, int layoutObjectType)
{
	if (layoutObjectType == CLayoutObject::TYPE_LAYOUT_REGION)
		return CLayoutObjectIterator::GetLayoutObjectIterator(pageLayout, layoutObjectType, false);
	else if (layoutObjectType == CLayoutObject::TYPE_READING_ORDER_GROUP)
		return new CReadingOrderGroupIterator(pageLayout, 2, false);
	return CLayoutObjectIterator::GetLayoutObjectIterator(pageLayout, layoutObjectType, true);
}

/*
 * Returns a region iterator for the given region type.
 */
CLayoutObjectIterator *	CLayoutEvaluator::GetLayoutObjectIterator(CPageLayout * pageLayout, int layoutObjectType, bool includeNested)
{
	return CLayoutObjectIterator::GetLayoutObjectIterator(pageLayout, layoutObjectType, includeNested);
}

/*
 * Adds all top-level regions to the specified list (and ignores nested regions)
 */
void CLayoutEvaluator::GetParentRegions(CPageLayout * pageLayout, vector<CLayoutObject*> * targetList)
{
	CLayoutObjectIterator * it = GetLayoutObjectIterator(pageLayout, CLayoutObject::TYPE_LAYOUT_REGION, false);

	if (it == NULL)
		return;

	while (it->HasNext())
		targetList->push_back((CLayoutRegion*)it->Next());
}

/*
 * Adds all nested regions to the specified list (and ignores top-level regions)
 */
void CLayoutEvaluator::GetNestedRegions(CPageLayout * pageLayout, vector<CLayoutObject*> * targetList)
{
	CLayoutObjectIterator * it = GetLayoutObjectIterator(pageLayout, CLayoutObject::TYPE_LAYOUT_REGION, true);

	if (it == NULL)
		return;

	CLayoutRegion * region;
	while (it->HasNext())
	{
		region = (CLayoutRegion*)it->Next();
		if (!region->GetParent().IsNull())
			targetList->push_back(region);
	}
}


//TODO make thread safe if using threads
void CLayoutEvaluator::IncreaseProgress(double amount)
{
	m_Progress += amount;
	if (m_ProgressMonitor != NULL)
		m_ProgressMonitor->SetProgress((int)m_Progress);
}

/*
 * Evaluate the reading order of the segmentation result
 */
void CLayoutEvaluator::EvaluateReadingOrder(CEvaluationResults * results, CLayoutEvaluation * layoutEval)
{
	CPageLayout * groundTruth = layoutEval->GetGroundTruth();
	CPageLayout * segResult = layoutEval->GetSegResult();

	if (groundTruth->GetReadingOrder() == NULL)
		return; //No reading order in ground truth
	
	CReadingOrder * segResultReadingOrder = segResult->GetReadingOrder();
	CReadingOrder * groundTruthReadingOrder = groundTruth->GetReadingOrder();
	CReadingOrderEvaluationResult * result = results->GetReadingOrderResults();

	CLayoutObjectIterator * regionIterator1 = GetLayoutObjectIterator(segResult, CLayoutObject::TYPE_LAYOUT_REGION);

	//Check all region pairs of the segmentation result
	set<int> relation;
	while (regionIterator1->HasNext())
	{
		CLayoutRegion * reg1 = (CLayoutRegion*)regionIterator1->Next();

		//Text region only!
		if (reg1->GetType() != CLayoutRegion::TYPE_TEXT)
			continue;

		CLayoutObjectIterator * regionIterator2 = GetLayoutObjectIterator(segResult, CLayoutObject::TYPE_LAYOUT_REGION);
		while (regionIterator2->HasNext())
		{
			CLayoutRegion * reg2 = (CLayoutRegion*)regionIterator2->Next();
			//Text region only and not the same region as above!
			if (reg2->GetType() != CLayoutRegion::TYPE_TEXT
				|| reg1 == reg2 
				|| reg1 < reg2)	//To avoid double results (r1->r2  r2<-r1)
				continue;
				
			//Calculate the relation between reg1 and reg2 using the reading order of the seg result
			if (segResultReadingOrder != NULL)
				relation = segResultReadingOrder->CalculateRelation(reg1->GetId(), reg2->GetId());
			else //Not defined
			{
				relation = set<int>();
				relation.insert(CReadingOrder::RELATION_NOT_DEFINED);
			}

			//Find the overlapping ground-truth regions and calculate the relations
			vector<CFuzzyReadingOrderRelation> fuzzyRelations;
			set<CUniString> * overlaps1 = results->GetSegResultOverlaps(reg1->GetId());
			set<CUniString> * overlaps2 = results->GetSegResultOverlaps(reg2->GetId());

			if (overlaps1 != NULL && overlaps2 != NULL && !overlaps1->empty() && !overlaps2->empty())
			{
				set<CUniString>::iterator it1 = overlaps1->begin();
				while (it1 != overlaps1->end())
				{
					CLayoutObject * groundTruthReg1 = results->GetDocumentLayoutObject((*it1), true);
					set<CUniString>::iterator it2 = overlaps2->begin();
					while (it2 != overlaps2->end())
					{
						CLayoutObject * groundTruthReg2 = results->GetDocumentLayoutObject((*it2), true);

						//Calculate the relation using the reading order of the ground-truth
						set<int> rel;
						if (groundTruthReadingOrder != NULL)
						{
							rel = groundTruthReadingOrder->CalculateRelation(groundTruthReg1->GetId(), groundTruthReg2->GetId());
						}
						else //Not defined
						{
							rel.insert(CReadingOrder::RELATION_NOT_DEFINED);
						}

						//Calculate the weight for the relation (based on overlap)
						double weight = 0.0, weightSeg1, weightSeg2, weightGt1, weightGt2;
						if (m_UsePixelArea) //We are counting pixels
						{
							//Seg Reg1
							double pixelCountReg1 = (double)results->GetPixelCount(reg1->GetId(), false);
							CLayoutObjectOverlap * overlap = results->GetOverlapIntervalRep(groundTruthReg1->GetId(), reg1->GetId());

							COverlapRects rects1;
							rects1.AddOverlapRects(groundTruthReg1->GetId(), overlap, true, layoutEval->GetBilevelImage());
							
							double overlapPixels1 = overlap != NULL ? (double)rects1.GetPixelCount() : 0.0;
							weightSeg1 = overlapPixels1 / pixelCountReg1;

							//Seg Reg2
							double pixelCountReg2 = (double)results->GetPixelCount(reg2->GetId(), false);
							overlap = results->GetOverlapIntervalRep(groundTruthReg2->GetId(), reg2->GetId());

							COverlapRects rects2;
							rects2.AddOverlapRects(groundTruthReg2->GetId(), overlap, true, layoutEval->GetBilevelImage());
							
							double overlapPixels2 = overlap != NULL ? (double)rects2.GetPixelCount() : 0.0;
							weightSeg2 = overlapPixels2 / pixelCountReg2;

							//Gt Reg1
							double pixelCountGtReg1 = (double)results->GetPixelCount(groundTruthReg1->GetId(), true);
							weightGt1 = overlapPixels1 / pixelCountGtReg1;

							//Gt Reg2
							double pixelCountGtReg2 = (double)results->GetPixelCount(groundTruthReg2->GetId(), true);
							weightGt2 = overlapPixels2 / pixelCountGtReg2;
						}
						else //Not using pixel area (using region area)
						{
							//Reg1
							double areaReg1 = results->GetRegionArea(reg1->GetId(), false);
							CLayoutObjectOverlap * overlap = results->GetOverlapIntervalRep(groundTruthReg1->GetId(), reg1->GetId());

							COverlapRects rects1;
							rects1.AddOverlapRects(groundTruthReg1->GetId(), overlap, false, layoutEval->GetBilevelImage());
							
							double overlapArea1 = overlap != NULL ? (double)rects1.GetArea() : 0.0;
							weightSeg1 = overlapArea1 / areaReg1;

							//Reg2
							double areaReg2 = (double)results->GetRegionArea(reg2->GetId(), false);
							overlap = results->GetOverlapIntervalRep(groundTruthReg2->GetId(), reg2->GetId());

							COverlapRects rects2;
							rects2.AddOverlapRects(groundTruthReg2->GetId(), overlap, false, layoutEval->GetBilevelImage());
							
							double overlapArea2 = overlap != NULL ? (double)rects2.GetArea() : 0.0;
							weightSeg2 = overlapArea2 / areaReg2;

							//Gt Reg1
							double areaGtReg1 = (double)results->GetRegionArea(groundTruthReg1->GetId(), true);
							weightGt1 = overlapArea1 / areaGtReg1;

							//Gt Reg2
							double areaGtReg2 = (double)results->GetRegionArea(groundTruthReg2->GetId(), true);
							weightGt2 = overlapArea2 / areaGtReg2;
						}
						weight = (weightSeg1 + weightSeg2) / 2 * ((weightGt1 + weightGt2) / 2.0);
						//weight = (weightSeg1 + weightSeg2 + weightGt1 + weightGt2) / 4.0;
						//weight = weightSeg1 * weightSeg2 * weightGt1 * weightGt2;					//CC didn't work well

						//Save
						fuzzyRelations.push_back(CFuzzyReadingOrderRelation(weight, groundTruthReg1->GetId(), groundTruthReg2->GetId(), rel));

						it2++;
					}
					it1++;
				}
			}

			CReadingOrderError * error = new CReadingOrderError(reg1->GetId(), reg2->GetId(), relation, fuzzyRelations, m_Profile);
			if (error->HasPenalty())
				result->AddError(error);
			else
				delete error;
		}
	}
}

/*
 * Checks if a merge is in reading direction...
 * Interface for Split
 */
bool CLayoutEvaluator::CheckIfMergeAllowable(CLayoutObject * object1, CLayoutObject * object2,
											 CUniString readingDirection, double readingOrientation)
{
	return CheckIfMergeAllowable(NULL, object1, object2, readingDirection, readingOrientation, false, true);
}

/*
 * Checks if a merge is in reading direction...
 * Interface for Merge
 */
bool CLayoutEvaluator::CheckIfMergeAllowable(CReadingOrder * readingOrder, 
											  CLayoutObject * object1, CLayoutObject * object2)
{
	return CheckIfMergeAllowable(readingOrder, object1, object2, CUniString(), 0.0, true, false);
}

/*
 * Checks if a merge is in direction of the reading order and reading direction.
 */
bool CLayoutEvaluator::CheckIfMergeAllowable(CReadingOrder * readingOrder, 
											  CLayoutObject * object1, CLayoutObject * object2,
											  CUniString readingDirection, double readingOrientation,
											  bool useReadingOrder,
											  bool useReadingdirectionAndOrientation)
{
	if (object1 == NULL || object2 == NULL) //Should not happen
		return false;

	if (object1->GetLayoutObjectType() != CLayoutObject::TYPE_LAYOUT_REGION		//Not block regions
		|| object2->GetLayoutObjectType() != CLayoutObject::TYPE_LAYOUT_REGION)
		return false;

	CLayoutRegion * layoutReg1 = (CLayoutRegion*)object1;
	if (layoutReg1->GetType() != CLayoutRegion::TYPE_TEXT)	//Region 1 is no text region
		return false;

	CLayoutRegion * layoutReg2 = (CLayoutRegion*)object2;
	if (layoutReg2->GetType() != CLayoutRegion::TYPE_TEXT)	//Region 2 is no text region
		return false;

	int allowedOverlap = m_Profile->GetMaxOverlapForAllowableSplitAndMerge();
	bool allowable = true;
	bool hasRelationPredecessor = false;		//'->'
	bool hasRelationSuccessor = false;			//'<-'

	if (useReadingOrder)
	{
		//Check reading order (if defined)
		if (readingOrder != NULL && readingOrder->GetRoot() != NULL && readingOrder->GetRoot()->GetSize() > 0)
		{
			if (readingOrder->ContainsRegionRef(object1->GetId())
				&& readingOrder->ContainsRegionRef(object2->GetId())) //Both regions in reading order
			{
				//Calculate the relation of the two regions
				set<int> rel = readingOrder->CalculateRelation(object1->GetId(), object2->GetId());

				set<int>::iterator it = rel.find(CReadingOrder::RELATION_PREDECESSOR);	//'->'
				hasRelationPredecessor = it != rel.end();
				it = rel.find(CReadingOrder::RELATION_SUCCESSOR);						//'<-'
				hasRelationSuccessor = it != rel.end();
				if (!hasRelationPredecessor && !hasRelationSuccessor) //No '->' or '<-' relation
					allowable = false;
			}
			else //One or both regions not in the reading order
				allowable = false;
		}
		else //Reading order not defined or empty
			allowable = false;
	}

	//Check position
	if (allowable) //(Allowable by reading order)
	{
		if (hasRelationSuccessor && useReadingOrder)	//'<-'
		{
			//Swap the regions, so that we don't need so many cases for the position testing
			CLayoutObject * temp = object1;
			object1 = object2;
			object2 = temp;
			CLayoutRegion * temp2 = layoutReg1;
			layoutReg1 = layoutReg2;
			layoutReg2 = temp2;
		}
		//Now we only have to deal with a '->' relation

		//Check reading direction and orientation
		CTextRegionInfo * info1 = (CTextRegionInfo*)layoutReg1->GetRegionInfo();
		CTextRegionInfo * info2 = (CTextRegionInfo*)layoutReg2->GetRegionInfo();
		double threshold = m_Profile->GetReadingOrientationThreshold();

		if (GetReadingDirection(info1) == GetReadingDirection(info2)
			&& abs(GetReadingOrientation(info1) - GetReadingOrientation(info1)) <= threshold)
		{
			//Bounding boxes
			int left1 = object1->GetCoords()->GetBBX1();
			int right1 = object1->GetCoords()->GetBBX2();
			int top1 = object1->GetCoords()->GetBBY1();
			int bottom1 = object1->GetCoords()->GetBBY2();
			int left2 = object2->GetCoords()->GetBBX1();
			int right2 = object2->GetCoords()->GetBBX2();
			int top2 = object2->GetCoords()->GetBBY1();
			int bottom2 = object2->GetCoords()->GetBBY2();

			//Rotate the regions using the reading orientation
			double orientation = useReadingdirectionAndOrientation 
									? readingOrientation	
									: GetReadingOrientation(info1);
			if (orientation != 0.0)
			{
				//The reading orienation is an angle -180 to 180 rotating clockwise.
				//So we have to ratate anti clockwise to rerotate it.
				CPointList * clone1 = object1->GetCoords()->Clone();
				CPointList * clone2 = object2->GetCoords()->Clone();
				CPointList * points;
				int x, y, xRot, yRot;
				double rad = orientation * M_PI / 180.0;
				for (int i=0; i<2; i++)
				{
					points = i==0 ? clone1 : clone2;
					CPolygonPoint * p = points->GetHeadPoint();
					while (p!=NULL)
					{
						x = p->GetX();
						y = -p->GetY();		//The coordinate system is flipped
						//Rotate
						xRot =  (int)(x*cos(rad) - y*sin(rad));
						yRot = -(int)(x*sin(rad) + y*cos(rad));
						p->SetXY(xRot, yRot);
						p = p->GetNextPoint();
					}
				}
				left1	= clone1->GetBBX1();
				right1	= clone1->GetBBX2();
				top1	= clone1->GetBBY1();
				bottom1 = clone1->GetBBY2();
				left2	= clone2->GetBBX1();
				right2	= clone2->GetBBX2();
				top2	= clone2->GetBBY1();
				bottom2 = clone2->GetBBY2();

				delete clone1;
				delete clone2;
			}

			CUniString direction = useReadingdirectionAndOrientation
											? readingDirection
											: GetReadingDirection(info1);
			//Reading direction left-to-right or right-to-left
			if (direction == CUniString(_T("left-to-right"))
				|| direction == CUniString(_T("right-to-left")))
			{
				if (useReadingOrder)
				{
					if (top2 < bottom1-allowedOverlap) //Region 2 not completely underneath region 1
						allowable = false;
				}
				else //not using reading order -> both directions allowed
				{
					if (top2 <= bottom1-allowedOverlap && top2 >= top1 
						|| top1 <= bottom2-allowedOverlap && top1 >= top2
						|| top1 < top2 && bottom1 > bottom2
						|| top2 < top1 && bottom2 > bottom1) //they overlap
						allowable = false;
				}
			}
			//Reading direction top-to-bottom or bottom-to-top
			else if (direction == CUniString(_T("top-to-bottom"))
				|| direction == CUniString(_T("bottom-to-top")))
			{
				if (useReadingOrder)
				{
					if (left2 < right1-allowedOverlap)	//Region 2 not completely right of region 1
						allowable = false;
				}
				else //not using reading order -> both directions allowed
				{
					if (left2 <= right1-allowedOverlap && left2 >= left1 
						|| left1 <= right2-allowedOverlap && left1 >= left2
						|| left1 < left2 && right1 > right2
						|| left2 < left1 && right2 > right1) //they overlap
						allowable = false;
				}
			}
			else //Unknown or unspecified reading direction
			{
				allowable = false;
			}
		}
		else
		{
			allowable = false;
		}
	}

	return allowable;
}

/*
 * Extended allowable check (CC 13.06.2011)
 *  The basic allowable check (used above) only takes into account region pairs.
 *  Hence, chains of allowable merges are not detected. If for instance three regions
 *  A, B and C are merged (in reading order), then the merges between A and B as well
 *  as between B and C are allowable, but NOT the merge between A and C. 
 *  This issue is solved in this second pass which looks for sequences of allowable
 *  merges and corrects the allowable flag accordingly.
 */
void CLayoutEvaluator::AllowableSequenceDetection(CEvaluationResults * results, CLayoutEvaluation * layoutEval)
{
	//Check if reading order exists
	if (layoutEval->GetGroundTruth() == NULL
		|| layoutEval->GetGroundTruth()->GetReadingOrder() == NULL
		|| layoutEval->GetGroundTruth()->GetReadingOrder()->GetRoot() == NULL
		|| layoutEval->GetGroundTruth()->GetReadingOrder()->GetRoot()->GetSize() == 0)
	{
		return;
	}

	CReadingOrder * readingOrder = layoutEval->GetGroundTruth()->GetReadingOrder();

	//Get all merges
	map<CUniString, CLayoutObjectEvaluationError*> * merges = results->GetRegionsForErrorType(CLayoutObjectEvaluationError::TYPE_MERGE);
	if (merges == NULL)
		return;

	//Iterate over all ground truth regions involved in a merge
	CUniString gtRegionId;
	CLayoutObjectEvaluationError * merge;
	for (map<CUniString, CLayoutObjectEvaluationError*>::iterator itGtRegion = merges->begin(); itGtRegion != merges->end(); itGtRegion++)
	{
		gtRegionId = (*itGtRegion).first;
		merge = (*itGtRegion).second;

		//Region in reading order?
		if (!readingOrder->ContainsRegionRef(gtRegionId))
			continue;

		//Is the group of the region ordered?
		CReadingOrderGroup * group = FindReadingOrderGroup(gtRegionId, readingOrder->GetRoot());
		if (group == NULL || !group->IsSorted())
			continue;

		//Get the position of the region in the group
		CReadingOrderRegionRef * gtRegionRef = group->FindRegionRef(gtRegionId);
		if (gtRegionRef == NULL)
			continue;

		int index = group->GetIndexOfElement(gtRegionRef);
		if (index < 0)
			continue;

		//Follow the reading order forwards and look for allowable
		CUniString region1 = gtRegionId;
		CUniString region2;
		for (int i=index+1; i<group->GetSize(); i++)
		{
			CReadingOrderElement * element2 = group->GetElement(i);
			if (element2->GetType() != CReadingOrderElement::TYPE_REGION_REF)
				break;
			region2 = ((CReadingOrderRegionRef*)element2)->GetIdRef();

			//Check if pair allowable
			map<CUniString, CLayoutObjectEvaluationError*>::iterator itFind = merges->find(region1);
			if (itFind == merges->end())
				break;
			CEvaluationErrorMerge * currError = (CEvaluationErrorMerge*)(*itFind).second;
			if (currError == NULL)
				break;

			if(currError->IsAllowable(region2))
			{
				//Check if the merge error contains region2
				map<CUniString, bool> * allowableEntries = ((CEvaluationErrorMerge*)merge)->GetAllowableEntries();
				if (allowableEntries == NULL || allowableEntries->find(region2) == allowableEntries->end())
					break;

				//The merge of the current region with the 'start' region is also allowable
				((CEvaluationErrorMerge*)merge)->SetAllowable(region2, true);
			}
			else
				break; //not allowable (end of sequence)

			region1 = region2;
		}

		//Now also follow the reading order backwards and look for allowable
		region1 = gtRegionId;
		for (int i=index-1; i>=0; i--)
		{
			CReadingOrderElement * element2 = group->GetElement(i);
			if (element2->GetType() != CReadingOrderElement::TYPE_REGION_REF)
				break;
			region2 = ((CReadingOrderRegionRef*)element2)->GetIdRef();

			//Check if pair allowable
			map<CUniString, CLayoutObjectEvaluationError*>::iterator itFind = merges->find(region1);
			if (itFind == merges->end())
				break;
			CEvaluationErrorMerge * currError = (CEvaluationErrorMerge*)(*itFind).second;
			if (currError == NULL)
				break;

			if(currError->IsAllowable(region2))
			{
				//Check if the merge error contains region2
				map<CUniString, bool> * allowableEntries = ((CEvaluationErrorMerge*)merge)->GetAllowableEntries();
				if (allowableEntries == NULL || allowableEntries->find(region2) == allowableEntries->end())
					break;

				//The merge of the current region with the 'start' region is also allowable
				((CEvaluationErrorMerge*)merge)->SetAllowable(region2, true);
			}
			else
				break; //not allowable (end of sequence)

			region1 = region2;
		}
	}
}

CReadingOrderGroup * CLayoutEvaluator::FindReadingOrderGroup(CUniString regionRef, 
															 CReadingOrderGroup * startGroup)
{
	if (startGroup == NULL)
		return NULL;
	if (startGroup->ContainsRegionRef(regionRef, false))
		return startGroup;
	for (int i=0; i<startGroup->GetSize(); i++)
	{
		CReadingOrderElement * element = startGroup->GetElement(i);
		if (element->GetType() == CReadingOrderElement::TYPE_GROUP)
		{
			CReadingOrderGroup * group = FindReadingOrderGroup(regionRef, (CReadingOrderGroup*)element);
			if (group != NULL)
				return group;
		}
	}
	return NULL;
}

/*
 * Returns the reading direction.
 * The original direction may be overridden by default values!
 */
CUniString CLayoutEvaluator::GetReadingDirection(CTextRegionInfo * info)
{
	CUniString usage = m_Profile->GetDefaultReadingDirectionUsage();
	if (usage == CUniString(CEvaluationProfile::USAGE_AS_DEFINED_IN_GROUNDTRUTH))
	{
		if (!info->IsReadingDirectionSet())
			return CUniString();
		return info->GetReadingDirection();
	}
	else if (usage == CUniString(CEvaluationProfile::USAGE_USE_DEFAULT_IF_NOT_IN_GROUND_TRUTH))
	{
		if (!info->IsReadingDirectionSet())
			return m_Profile->GetDefaultReadingDirection();
		return info->GetReadingDirection();
	}
	else //always use default value
	{
		return m_Profile->GetDefaultReadingDirection();
	}
}

/*
 * Returns the reading orientation.
 * The original orientation may be overridden by default values!
 */
double CLayoutEvaluator::GetReadingOrientation(CTextRegionInfo * info)
{
	CUniString usage = m_Profile->GetDefaultReadingOrientationUsage();
	if (usage == CUniString(CEvaluationProfile::USAGE_AS_DEFINED_IN_GROUNDTRUTH))
	{
		if (!info->IsReadingOrientationSet())
			return 0.0;
		return info->GetReadingOrientation();
	}
	else if (usage == CUniString(CEvaluationProfile::USAGE_USE_DEFAULT_IF_NOT_IN_GROUND_TRUTH))
	{
		if (!info->IsReadingOrientationSet())
			return m_Profile->GetDefaultReadingOrientation();
		return info->GetReadingOrientation();
	}
	else //always use default value
	{
		return m_Profile->GetDefaultReadingOrientation();
	}
}

/*
 * Enables or disables a specific evaluation feature (e.g. check for merge errors)
 * 'errorType' - 
 */
void CLayoutEvaluator::EnableEvaluationFeature(int errorType, bool enable)
{
	m_EnableErrorChecks[errorType] = enable;
}

/*
 * Class CLayoutEvaluationAlgorithm
 *
 * Implementation of the algorithm interface.
 */

CLayoutEvaluationAlgorithm::CLayoutEvaluationAlgorithm(CLayoutEvaluator * evaluator) 
{
	m_Evaluator = evaluator;
}

CLayoutEvaluationAlgorithm::~CLayoutEvaluationAlgorithm()
{
}

void CLayoutEvaluationAlgorithm::DoRun()
{
	m_Evaluator->RunEvaluation(this);
}

