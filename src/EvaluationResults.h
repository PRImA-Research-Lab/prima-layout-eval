#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "LayoutEvaluation.h"
#include "EvaluationProfile.h"
#include "EvaluationMetrics.h"

namespace PRImA
{	
class CLayoutEvaluation;
class CLayoutObjectEvaluationResult;
class CEvaluationError;
class CLayoutObjectEvaluationError;
class CReadingOrderEvaluationResult;
class CLayoutObjectEvaluationMetrics;
class CLayoutObjectErrorIterator;
class CBorderEvaluationResults;
class CEvaluationMetrics;


/*
 * Class CEvaluationResults
 *
 * Evaluation results for an object type (layout region, text line, word, glyph).
 *
 * CC 22.01.2010 - created
 */

class CEvaluationResults
{
	friend class CLayoutObjectErrorIterator;

public:
	CEvaluationResults(CLayoutEvaluation * layoutEvaluation, CEvaluationProfile * profile, int layoutObjectType);
	~CEvaluationResults();

private:
	//Map [ground truth object, overlapping segmentation result objects]
	std::map<CUniString, set<CUniString>*>		m_GroundTruthOverlaps;

	//Map [segmentation result object, overlapping ground truth objects]
	std::map<CUniString, set<CUniString>*>		m_SegResultOverlaps;

	//Map with RegionOverlap for each ground truth object - segmentation result object pair
	std::map<CUniString, std::map<CUniString, CLayoutObjectOverlap*>*>	m_OverlapIntervalReps;

	//Map [ground truth object, RegionOverlap with all overlapping segmentation result objects]
	std::map<CUniString, CLayoutObjectOverlap*>		m_MultiOverlapIntervalReps;

	//Map [object, IntervalRepresentation]
	std::map<CUniString, CIntervalRepresentation*>	m_GroundTruthIntervalReps;
	std::map<CUniString, CIntervalRepresentation*>	m_SegResultIntervalReps;

	//Map [object, number of black pixels]
	std::map<CUniString, long>	m_GroundTruthPixelCounts;
	std::map<CUniString, long>	m_SegResultPixelCounts;

	//Map [object, EvaluationResult]
	std::map<CUniString, CLayoutObjectEvaluationResult*>	m_GroundTruthObjectResults;
	std::map<CUniString, CLayoutObjectEvaluationResult*>	m_SegResultObjectResults;

	//Map with objects per layout object error type
	std::map<int, std::map<CUniString, CLayoutObjectEvaluationError*>*>	m_ErrorTypeObjectsMap;

	CLayoutEvaluation	*	m_LayoutEvaluation;
	CEvaluationProfile	*	m_Profile;
	int						m_LayoutObjectType;	//layout region, text line, word, glyph (see CLayoutObject::TYPE_...)

	CReadingOrderEvaluationResult * m_ReadingOrderResults;	//Reading order evaluation results (only for blocks, not for lines, words or glyphs)

	CEvaluationMetrics					*	m_Metrics;						//Figures, numbers, success rates, ...
	std::map<int,CLayoutObjectEvaluationMetrics *>		m_MetricsPerLayoutRegionType;	//map [layout region type, metrics]

	CBorderEvaluationResults * m_BorderResults;

public:
	void						AddLayoutObjectOverlap(CUniString groundTruth, CUniString segResult, 
													 CLayoutObjectOverlap * overlap);
	std::set<CUniString>			*	GetGroundTruthOverlaps(CUniString groundTruth);
	std::set<CUniString>			*	GetSegResultOverlaps(CUniString segResult);
	inline std::map<CUniString, std::set<CUniString>*> * GetGroundTruthOverlaps() { return &m_GroundTruthOverlaps; };
	inline std::map<CUniString, std::set<CUniString>*> * GetSegResultOverlaps() { return &m_SegResultOverlaps; };

	CLayoutObjectOverlap			*	GetOverlapIntervalRep(CUniString groundTruth, CUniString segResult);

	CLayoutObjectOverlap			*	GetMultiOverlapIntervalRep(CUniString groundTruth);

	CLayoutObjectEvaluationResult		*	GetGroundTruthObjectResult(CUniString layoutObject, bool createIfNotExists = false);
	CLayoutObjectEvaluationResult		*	GetSegResultObjectResult(CUniString layoutObject, bool createIfNotExists = false);
	inline std::map<CUniString, CLayoutObjectEvaluationResult*> * GetGroundTruthObjectResults() { return &m_GroundTruthObjectResults; };
	inline std::map<CUniString, CLayoutObjectEvaluationResult*> * GetSegResultObjectResults() { return &m_SegResultObjectResults; };

	void						AddMultiOverlapIntervalRep(CLayoutObject * groundTruth, CLayoutObjectOverlap * overlap);

	void						AddIntervalRepresentation(CLayoutObject * region, CIntervalRepresentation * intRepr, bool isGroundTruth);
	CIntervalRepresentation *	GetIntervalRepresentation(CUniString region, bool createIfNotExists, bool isGroundTruth);

	long						GetPixelCount(CUniString region, bool isGroundTruth);

	long						GetRegionArea(CUniString region, bool isGroundTruth);

	inline CLayoutEvaluation *	GetLayoutEvaluation() { return m_LayoutEvaluation; };

	inline int					GetLayoutObjectType() { return m_LayoutObjectType; };	//Region type of these results (block ,text line, word or glyph)

	std::map<CUniString, CLayoutObjectEvaluationError*> *	GetRegionsForErrorType(int errorType, bool createIfNotExists = false);

	inline CReadingOrderEvaluationResult * GetReadingOrderResults() {return m_ReadingOrderResults; };
	void ClearReadingOrderResults();
	void SetReadingOrderResults(CReadingOrderEvaluationResult * res);

	inline CEvaluationMetrics	*	GetMetrics() { return m_Metrics; };
	CLayoutObjectEvaluationMetrics	*	GetMetrics(int layoutRegionType);
	inline std::map<int,CLayoutObjectEvaluationMetrics *>	*	GetMetricsPerType() { return &m_MetricsPerLayoutRegionType; };
	void						CalculateMetrics();
	void						SetMetrics(CEvaluationMetrics * metrics);

	inline CEvaluationProfile * GetProfile() { return m_Profile; }

	CLayoutObject					  * GetDocumentLayoutObject(CUniString objectId, bool isGroundTruth);

	CBorderEvaluationResults * GetBorderResults(bool create = false);
	void ClearBorderResults();
	void SetBorderResults(CBorderEvaluationResults * res);

private:
	void						AddOverlapIntervalRep(CUniString groundTruth, CUniString segResult, 
													  CLayoutObjectOverlap * overlap);
	void						RestrictToDocumentDimensions(CLayoutPolygon * coords, int width, int height);

	CIntervalRepresentation * CalculateIntervalRepresentation(CReadingOrderGroup * group, CPageLayout * pageLayout);

};


/*
 * Class CLayoutObjectErrorIterator
 *
 * To iterate over all region based errors (merge, split, ...) 
 * of a EvaluationResults object.
 *
 * CC 28.06.2010
 */

class CLayoutObjectErrorIterator
{
public:
	CLayoutObjectErrorIterator(CEvaluationResults * results);
	~CLayoutObjectErrorIterator();

	bool HasNext();
	CLayoutObjectEvaluationError * GetNext();

private:
	CEvaluationResults * m_Results;
	std::map<int, std::map<CUniString, CLayoutObjectEvaluationError*>*>::iterator m_ItType;
	std::map<CUniString, CLayoutObjectEvaluationError*>::iterator m_ItRegion;
};


/*
 * Class CEvaluationError
 *
 * Generic class for different evaluation errors.
 *
 * CC 22.01.2010 - created
 */

class CEvaluationError
{
public:
	CEvaluationError();
	virtual ~CEvaluationError();

	virtual CUniString GetName() = 0;

	inline void		SetWeightedAreaError(double err) { m_WeightedAreaError = err; };
	inline double	GetWeightedAreaError() {return m_WeightedAreaError; };
	inline void		SetWeightedCountError(double err) { m_WeightedCountError = err; };
	inline double	GetWeightedCountError() {return m_WeightedCountError; };

private:
	double m_WeightedAreaError;		//Final error amount based on area and weighted by the weights defined in the evaluation profile
	double m_WeightedCountError;	//Final error amount based on count and weighted by the weights defined in the evaluation profile
};


/*
 * Class CLayoutObjectEvaluationError
 *
 * Generic class for different region based evaluation errors.
 *
 * CC 22.01.2010 - created
 */

class CLayoutObjectEvaluationError : public CEvaluationError
{
public:
	CLayoutObjectEvaluationError(int type, CUniString region);
	virtual ~CLayoutObjectEvaluationError();

	virtual CLayoutObjectEvaluationError * Clone();

	static const int TYPE_NONE			= 0;
	static const int TYPE_SPLIT			= 1;	//A ground-truth region is split in two or more regions in the segmentation result
	static const int TYPE_MERGE			= 2;	//Two or more ground-truth regions are merged to one region in the segmentation result
	static const int TYPE_MISS			= 3;	//A ground-truth region was not detected by the segmentation
	static const int TYPE_PART_MISS		= 4;	//Parts of a ground-truth region are not covered by a region in the segmentation result
	static const int TYPE_MISCLASS		= 5;	//The segmentation classified a region differently than the ground-truth (only on block level)
	static const int TYPE_INVENT		= 6;	//The segmentation found a region where the ground-truth has only background (false detection)
	static const int TYPE_BORDER		= 7;	//

	inline int				GetType() { return m_Type; };
	static CUniString		GetTypeName(int errorType);		//returns 'Merge', 'Split', ... depending on the given type ID

	inline CUniString		GetName() { return GetTypeName(m_Type) ;};	//returns 'Merge', 'Split', ... depending on the error type

	inline std::list<CRect*> *	GetRects() { return &m_Rects; };						//Collection of rectangles that decribe the error region
	inline std::list<CRect*> *	GetFalseAlarmRects() { return &m_FalseAlarmRects; };	//Collection of rectangles that decribe the error region that doesn't contain foreground pixels
	void					AddRects(vector<CRect*> * rects);
	inline int				GetCount() { return m_Count; };
	inline void				SetCount(int count) { m_Count = count; };
	inline bool				IsFalseAlarm() { return m_FalseAlarm; };
	inline void				SetFalseAlarm(bool b) { m_FalseAlarm = b; };
	inline long				GetPixelCount() { return m_PixelCount; };
	inline void				SetPixelCount(long count) { m_PixelCount = count; };
	inline long				GetArea() { return m_Area; };
	inline void				SetArea(long area) { m_Area = area; };
	inline CUniString		GetLayoutObject() { return m_LayoutObject; };
	inline bool				IsForNestedRegion() { return m_NestedRegion; };
	inline void				SetForNestedRegion(bool nested) { m_NestedRegion = nested; };

protected:
	int				m_Type;				//error type (merge, split, ...) (see constants TYPE_...)
	CUniString		m_LayoutObject;		//Involved document layout object (for false detection this is an object from the segmentation result, otherwise a ground-truth object)
	long			m_Area;				//Error area
	long			m_PixelCount;		//Error foreground pixel count
	int				m_Count;			//Error count
	std::list<CRect*>	m_Rects;			//Error region
	std::list<CRect*>	m_FalseAlarmRects;	//Error false alarm region
	bool			m_FalseAlarm;		//Is the error a false alarm (is the case if there are no foreground pixels at all invloved)
	bool			m_NestedRegion;		//Is the error for a nested region?
};


/*
 * Class COverlapRects
 *
 * Class for storing overlapping areas per layout object.
 *
 * CC 25.01.2010 - created
 */

class COverlapRects
{
public:
	COverlapRects();
	~COverlapRects();
	void AddOverlapRects(CUniString overlappingObject, CLayoutObjectOverlap * overlap,
						bool countPixels, COpenCvBiLevelImage * image);
	long GetOverlapArea(CUniString region);
	long GetOverlapPixelCount(CUniString region);
	inline long	GetArea() { return m_OverallArea; };
	inline long	GetPixelCount() { return m_OverallPixelCount; };
	std::vector<CUniString> * GetRegions();

	void AddOverlap(CUniString overlappingObject, long area, long pixelCount);

	void CopyFrom(COverlapRects * rects, bool deepCopy = false);

public:
	//Map [overlappingObject, list of overlap rects]
	std::map<CUniString, std::vector<CRect*>*> m_Overlaps;

private:
	//Overlap area per object
	std::map<CUniString, long>	m_OverlapArea;

	//Number of black pixels per object
	std::map<CUniString, long>	m_PixelCount;

	long					m_OverallArea;
	long					m_OverallPixelCount;
};


/*
 * Class CEvaluationErrorMerge
 *
 * Class for merge evaluation error.
 *
 * CC 25.01.2010 - created
 */

class CEvaluationErrorMerge : public CLayoutObjectEvaluationError
{
public:
	CEvaluationErrorMerge(CUniString region);
	~CEvaluationErrorMerge();

	virtual CLayoutObjectEvaluationError * Clone();

	void AddErrorRects(CUniString segResultRegion, COverlapRects* overlapRects, bool addArea = true);
	inline std::map<CUniString, COverlapRects *> * GetMergingRegions() { return &m_MergingRegions; };

	void SetAllowable(CUniString groundTruthRegion, bool allowable);
	bool IsAllowable(CUniString groundTruthRegion);

	inline std::map<CUniString, bool> * GetAllowableEntries() { return &m_Allowable; };

protected:
	//Map [segmentation result region, overlapping ground truth regions + overlap rects]
	std::map<CUniString, COverlapRects *>	m_MergingRegions;
	std::map<CUniString, bool>				m_Allowable;	//map [ground-truth region, isAllowable]
};


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
class CFuzzyReadingOrderRelation
{
public:
	CFuzzyReadingOrderRelation(double weight, CUniString reg1, CUniString reg2, std::set<int> relation);
	~CFuzzyReadingOrderRelation();

	inline double GetWeight() { return m_Weight; };
	inline std::set<int> GetRelation() {return m_Relation; };

	inline CUniString GetRegion1() { return m_Region1; };
	inline CUniString GetRegion2() { return m_Region2; };

	inline void		SetPenalty(double penalty) { m_Penalty = penalty; };
	inline double	GetPenalty() { return m_Penalty; };
	inline void		SetCausingGroundTruthRelation(int rel) { m_CausingGroundTruthRelation = rel; };
	inline void		SetCausingSegResultRelation(int rel) { m_CausingSegResultRelation = rel; };
	inline int		GetCausingGroundTruthRelation() { return m_CausingGroundTruthRelation; };
	inline int		GetCausingSegResultRelation() { return m_CausingSegResultRelation; };

private:
	double			m_Weight;
	CUniString		m_Region1;
	CUniString		m_Region2;
	std::set<int>		m_Relation;		//Set of possible relations

	double			m_Penalty;						//Penalty for
	int				m_CausingGroundTruthRelation;	//Reading order relations
	int				m_CausingSegResultRelation;		//that caused the penalty
};


/*
 * Class CReadingOrderError
 *
 * Class for reading order evaluation error.
 * Represents a wrong relation between two layout regions.
 *
 * CC 14.06.2010 - created
 */

class CReadingOrderError : public CEvaluationError
{
public:
	CReadingOrderError(CUniString reg1, CUniString reg2, std::set<int> segResultRelation,
		std::vector<CFuzzyReadingOrderRelation> groundTruthRelation, CEvaluationProfile * profile);
	~CReadingOrderError();

	inline CUniString		GetName() { return CUniString(_T("Reading Order")); };

	inline bool		HasPenalty() { return m_Penalty > 0.0;};
	inline double	GetPenalty() { return m_Penalty; };

	inline std::vector<CFuzzyReadingOrderRelation> * GetGroundTruthRelations() { return &m_GroundTruthRelation; };

	inline CUniString	GetRegion1() { return m_Region1; };
	inline CUniString	GetRegion2() { return m_Region2; };

	static double GetMaxPenalty(CEvaluationProfile * profile);  //Max error per region pair

	inline std::set<int> * GetSegResultRelation() { return &m_SegResultRelation; };


protected:
	void CalculatePenalty(CEvaluationProfile * profile);
	double CalculatePenalty(CEvaluationProfile * profile, std::set<int> & groundTruth, std::set<int> & segmentation,
							int & selectedGroundTruthRel, int & selectedSegResultRel);
	double CalculatePenalty(CEvaluationProfile * profile, int groundTruthRel, int segmentationRel);

	CUniString		m_Region1;
	CUniString		m_Region2;
	std::set<int>		m_SegResultRelation;					//Relation between Reg1 and Reg2 according to the segmentation result reading order
	std::vector<CFuzzyReadingOrderRelation>	m_GroundTruthRelation;	//Relation between Reg1 and Reg2 according to the ground-truth
	double			m_Penalty;								//Penalty points
};


/*
 * Class CEvaluationErrorMisclass
 *
 * Class for misclassification evaluation error.
 *
 * CC 27.01.2010 - created
 */

class CEvaluationErrorMisclass : public CLayoutObjectEvaluationError
{
public:
	CEvaluationErrorMisclass(CUniString region);
	~CEvaluationErrorMisclass();

	virtual CLayoutObjectEvaluationError * Clone();

	void AddErrorRects(	CUniString overlappingRegion, CLayoutObjectOverlap * overlap,
						bool countPixels, COpenCvBiLevelImage * image);
	inline COverlapRects * GetMisclassRegions() { return &m_ErrorAreas; };

	inline void SetMisclassRegions(COverlapRects * rects) { m_ErrorAreas.CopyFrom(rects); };

protected:
	COverlapRects 	m_ErrorAreas;		
};


/*
 * Class CEvaluationErrorSplit
 *
 * Class for split evaluation error.
 *
 * CC 05.02.2010 - created
 */

class CEvaluationErrorSplit : public CLayoutObjectEvaluationError
{
public:
	CEvaluationErrorSplit(CUniString  region);
	~CEvaluationErrorSplit();

	virtual CLayoutObjectEvaluationError * Clone();

	void AddErrorRects(	CUniString overlappingRegion, CLayoutObjectOverlap * overlap,
						bool countPixels, COpenCvBiLevelImage * image);
	inline COverlapRects * GetSplittingRegions() { return &m_SplittingRegions; };

	inline void SetAllowable(bool allowable) { m_Allowable = allowable; };
	inline bool IsAllowable() { return m_Allowable;};

	inline void SetSplittingRegions(COverlapRects * rects) { m_SplittingRegions.CopyFrom(rects); };

protected:
	COverlapRects 	m_SplittingRegions;		
	bool	m_Allowable;	
};


/*
 * Class CLayoutObjectEvaluationResult
 *
 * Result for one single region.
 *
 * CC 22.01.2010 - created
 */

class CLayoutObjectEvaluationResult
{
public:
	CLayoutObjectEvaluationResult(CEvaluationResults * results, CUniString region);
	~CLayoutObjectEvaluationResult();

	CLayoutObjectEvaluationResult * Clone(CEvaluationResults * parentResults);

	void						AddError(CLayoutObjectEvaluationError * error);
	CLayoutObjectEvaluationError	*	GetError(int errType);
	inline CUniString			GetRegion() { return m_Region; };

	inline std::map<int, CLayoutObjectEvaluationError*> * GetErrors() { return &m_Errors; };
	
private:
	std::map<int, CLayoutObjectEvaluationError*> m_Errors;	//map [errType, error object]
	CEvaluationResults		*	m_Results;
	CUniString					m_Region;
};


/*
 * Class CReadingOrderEvaluationResult
 *
 * Result for reading order relations.
 *
 * CC 16.06.2010 - created
 */

class CReadingOrderEvaluationResult
{
public:
	CReadingOrderEvaluationResult(CEvaluationResults * results);
	~CReadingOrderEvaluationResult();

	void						AddError(CReadingOrderError * error);
	CReadingOrderError		*	GetError(int index);
	int							GetErrorCount();
	vector<CReadingOrderError*>	GetErrors(CUniString * regionid);



private:
	bool	AddToErrorMap(CUniString regionId1, CUniString regionId2, CReadingOrderError * error);

	vector<CReadingOrderError*> m_Errors;	
	std::map<CUniString, std::map<CUniString, CReadingOrderError*>>	m_ErrorMap;	//map [regId1, map[regId2, error object] 
	CEvaluationResults		*	m_Results;
};



/*
 * Class CBorderEvaluationError 
 *
 * CC 22.08.2011 - created
 */
class CBorderEvaluationError : public CEvaluationError
{
public:
	CBorderEvaluationError(CUniString name);
	~CBorderEvaluationError();

	inline CUniString GetName() { return m_Name; };

	inline long GetArea() { return m_Area; };
	inline void SetArea(long area) { m_Area = area; };

private:
	CUniString m_Name;
	long m_Area;
};


/*
 * Class CBorderEvaluationResults 
 *
 * CC 22.08.2011 - created
 */
class CBorderEvaluationResults
{
public:
	CBorderEvaluationResults();
	~CBorderEvaluationResults();

	inline CBorderEvaluationError * GetIncludedBackgroundError() { return m_IncludedBackground; };
	inline CBorderEvaluationError * GetExcludedForegroundError() { return m_ExcludedForeground; };

	inline std::map<CUniString, CLayoutObjectEvaluationError*> * GetMissingRegionAreaErrors() { return &m_MissingRegionAreaErrors; };

	inline CIntervalRepresentation * GetGroundTruthBorderIntervalRep() { return m_GroundTruthBorderIntervalRep; };
	inline CIntervalRepresentation * GetSegResultBorderIntervalRep() { return m_SegResultBorderIntervalRep; };
	inline void SetGroundTruthBorderIntervalRep(CIntervalRepresentation * intRep) { m_GroundTruthBorderIntervalRep = intRep; };
	inline void SetSegResultBorderIntervalRep(CIntervalRepresentation * intRep) { m_SegResultBorderIntervalRep = intRep; };

private:
	CIntervalRepresentation * m_GroundTruthBorderIntervalRep;
	CIntervalRepresentation * m_SegResultBorderIntervalRep;

	CBorderEvaluationError * m_IncludedBackground;
	CBorderEvaluationError * m_ExcludedForeground;
	std::map<CUniString, CLayoutObjectEvaluationError*> m_MissingRegionAreaErrors;
	
};
}