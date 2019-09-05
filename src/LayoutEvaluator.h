#ifndef LAYOUTEVALUATOR_H
#define LAYOUTEVALUATOR_H

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

//#include <pstdint.h>

#include "BiLevelImage.h"
#include "ExtraArray.h"
#include "HiColorImage.h"
#include "EvaluationProfile.h"
#include "LayoutEvaluation.h"
#include "Algorithm.h"
#include "RegionIterator.h"
#include "BoundingBoxMap.h"
#include "IntervalRepresentation.h"
#include "RegionOverlap.h"
#include "DocumentLayout.h"

namespace PRImA
{

/*
 * Class CLayoutEvaluator
 *
 * Evaluates segmentation results using ground-truth.
 *
 * CC 01.02.2010 - recreated this class (from daves old evaluator)
 */

class CLayoutEvaluator
{
private:
	//Modes for dealing with nested regions. Parent regions and nested regions are thereby evaluate separatly in different combinations.
	//For example, parent-to-nested means the ground truth parent regions are compared with the segmentation result nested regions
	static const int NESTED_REGION_MODE_IGNORE				= 1; //Same as parent-to-parent
	static const int NESTED_REGION_MODE_PARENT_TO_PARENT	= 1;
	static const int NESTED_REGION_MODE_PARENT_TO_NESTED	= 2;
	static const int NESTED_REGION_MODE_NESTED_TO_PARENT	= 3;
	static const int NESTED_REGION_MODE_NESTED_TO_NESTED	= 4;

	// CONSTRUCTION
public:
	CLayoutEvaluator(CLayoutEvaluation * layoutEval, CEvaluationProfile * profile,
					 bool evaluateRegions, bool evaluateTextLines,
					 bool evaluateWords, bool evaluateGlyphs, bool evaluateBorder, bool evaluateReadingOrderGroups, bool evaluateReadingOrder);
	~CLayoutEvaluator();


	// METHODS
public:
	void	RunEvaluation(CProgressMonitor * progressMonitor = NULL);

	void	EnableEvaluationFeature(int errorType, bool enable);

	inline void SetConvertToIsothetic(bool convertToIsothetic) { m_ConvertToIsothetic = convertToIsothetic; };

	inline CLayoutEvaluation * GetLayoutEvaluationData() { return m_LayoutEvaluation; };

private:
	void				Evaluate(int layoutObjectType);
	void				EvaluateRegionsAndNestedRegions();

	void				ProcessGroundTruthObjects(int layoutObjectType, CLayoutEvaluation * layoutEval, int nestedRegionsMode = NESTED_REGION_MODE_IGNORE);
	void				PrepareGroundTruthBorder(CLayoutEvaluation * layoutEval);
	void				PrepareGroundTruthReadingOrderGroups(CLayoutEvaluation * layoutEval);
	void				PrepareGroundTruthRegionsLinesWordsGlyphs(int layoutObjectType, CLayoutEvaluation * layoutEval, int nestedRegionsMode = NESTED_REGION_MODE_IGNORE);

	void				ConvertToIsothetic(int layoutObjectType, CLayoutEvaluation * layoutEval);

	void				CalculateOverlaps(CLayoutObject * groundTruthObject, CBoundingBoxMap * boundingBoxMap, CEvaluationResults * results);

	void				IncreaseProgress(double amount);
	CLayoutObjectIterator *	GetLayoutObjectIterator(CPageLayout * pageLayout, int layoutObjectType);
	CLayoutObjectIterator *	GetLayoutObjectIterator(CPageLayout * pageLayout, int layoutObjectType, bool includeNested);

	void				FindErrors(int layoutObjectType, CLayoutEvaluation * layoutEval, int nestedRegionsMode = NESTED_REGION_MODE_IGNORE);
	void				FindGroundTruthBasedErrorsForLayoutObject(int regionType, CEvaluationResults * results, CLayoutObjectEvaluationResult * result,
																CLayoutObject * groundTruthObject, std::set<CUniString> * segResultObjects);


	void				FindBorderErrors(CLayoutEvaluation * layoutEval);
	void				CheckMerge(	int layoutObjectType, CEvaluationResults * results,
									CLayoutObjectEvaluationResult * result,
									CLayoutObject * groundTruthObject, 
									std::set<CUniString> * segResultObjects);
	void				CheckSplit(	int layoutObjectType, CEvaluationResults * results,
									CLayoutObjectEvaluationResult * result,
									CLayoutObject * groundTruthObject, 
									std::set<CUniString> * segResultObjects);
	void				CheckMiss(	int layoutObjectType, CEvaluationResults * results,
									CLayoutObjectEvaluationResult * result,
									CLayoutObject * groundTruthObject, 
									std::set<CUniString> * segResultObjects);
	void				CheckPartMiss(	int layoutObjectType, CEvaluationResults * results,
										CLayoutObjectEvaluationResult * result,
										CLayoutObject * groundTruthObject, 
										std::set<CUniString> * segResultObjects);
	void				CheckMisclass(	int layoutObjectType, CEvaluationResults * results,
										CLayoutObjectEvaluationResult * result,
										CLayoutObject * groundTruthObject, 
										std::set<CUniString> * segResultObjects);
	void				CheckMisclassForRegion(CEvaluationResults * results,
										CLayoutObjectEvaluationResult * result,
										CLayoutObject * groundTruthObject,
										std::set<CUniString> * segResultObjects);
	void				CheckMisclassForReadingOrderGroup(CEvaluationResults * results,
										CLayoutObjectEvaluationResult * result,
										CLayoutObject * groundTruthObject,
										std::set<CUniString> * segResultObjects);
	void				CheckInvented(	int layoutObjectType, CEvaluationResults * results,
										CLayoutObjectEvaluationResult * result,
										CLayoutObject * segResultObject, 
										std::set<CUniString> * groundTruthObjects);

	void				EvaluateReadingOrder(CEvaluationResults * results, CLayoutEvaluation * layoutEval);

	bool				CheckFalseAlarm(CLayoutObjectEvaluationError * err);

	long				SplitByPixelArea(std::list<CRect*> * input,
		std::list<CRect*> * falseAlarm,
		std::map<CRect*, long> * realErrors);

	//long				CountForegroundPixels(COpenCvBiLevelImage * image, CRect * rect);

	bool				CheckIfMergeAllowable(CLayoutObject * object1, CLayoutObject * object2,
											  CUniString readingDirection, double readingOrientation);
	bool				CheckIfMergeAllowable(CReadingOrder * readingOrder, 
											  CLayoutObject * object1, CLayoutObject * object2);
	bool				CheckIfMergeAllowable(CReadingOrder * readingOrder, 
											  CLayoutObject * object1, CLayoutObject * object2,
											  CUniString readingDirection, double readingOrientation,
											  bool useReadingOrder,
											  bool useReadingdirectionAndOrientation);
	void				AllowableSequenceDetection(CEvaluationResults * results, CLayoutEvaluation * layoutEval);
	CReadingOrderGroup * FindReadingOrderGroup(CUniString regionRef, CReadingOrderGroup * startGroup);
	
	CUniString			GetReadingDirection(CTextRegionInfo * info);
	double				GetReadingOrientation(CTextRegionInfo * info);

	bool				CheckIfMisclassCanBeIgnored(CLayoutRegion * groundTruthObject, int resultType);

	void				GetParentRegions(CPageLayout * pageLayout, std::vector<CLayoutObject*> * targetList);
	void				GetNestedRegions(CPageLayout * pageLayout, std::vector<CLayoutObject*> * targetList);

	void				CombineTopLevelAndNestedEvaluationResults(CEvaluationResults * targetResults, CEvaluationResults * sameLevelResults,
																CEvaluationResults * acrossLevelResults, long mergeMultiplier,
																long splitMultiplier, long missMultiplier, long partialMissMultiplier, 
																long misclassMultiplier);

	void				CombineTopLevelAndNestedFalseDetectionResults(CEvaluationResults * targetResults, CEvaluationResults * sameLevelResults,
																	CEvaluationResults * acrossLevelResults);

	long				CalculateErrorSum(CLayoutObjectEvaluationResult * evalRes, long mergeMultiplier,
											long splitMultiplier, long missMultiplier, long partialMissMultiplier,
											long misclassMultiplier);

	void				CopyLayoutObjectEvalResults(CEvaluationResults * targetResults, std::map<CUniString, CLayoutObjectEvaluationResult*> * target, CLayoutObjectEvaluationResult * evalRes);

	// DATA ITEMS
private:
	bool m_EvaluateRegions;
	bool m_EvaluateTextLines;
	bool m_EvaluateWords;
	bool m_EvaluateGlyphs;
	bool m_EvaluateBorder;
	bool m_EvaluateReadingOrderGroups;
	bool m_EvaluateReadingOrder;

	CLayoutEvaluation *		m_LayoutEvaluation;
	CEvaluationProfile *	m_Profile;
	COpenCvBiLevelImage *	m_Image;

	CProgressMonitor *	m_ProgressMonitor;
	double				m_Progress;
	double				m_MaxPartialProgress;

	bool m_UsePixelArea;		//Using number of foreground pixels instead of region area (for area based errors)

	std::vector<bool> m_EnableErrorChecks;

	//Option to convert all polygons to isothetic format before running the evaluation (also removes possible loops) (default: true)
	bool m_ConvertToIsothetic;
};



/*
 * Class CLayoutEvaluationAlgorithm
 *
 */

 class CLayoutEvaluationAlgorithm : public CAlgorithm
 {
	 public:
		CLayoutEvaluationAlgorithm(CLayoutEvaluator * evaluator);
		~CLayoutEvaluationAlgorithm();

		inline CLayoutEvaluator * GetLayoutEvaluator() { return m_Evaluator;};

		inline CLayoutEvaluator * GetEvaluator() { return m_Evaluator; };

	protected:
		void	DoRun();
		inline bool	CanRun() { return true; };

	private:
		CLayoutEvaluator * m_Evaluator;
};

}

#endif // LAYOUTEVALUATOR_H
