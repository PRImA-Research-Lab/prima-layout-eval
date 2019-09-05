#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "Parameter.h"
#include <map>
#include "LayoutEvaluation.h"
#include "xmlvalidator.h"

using namespace std;

namespace PRImA
{

class CErrorTypeWeight;
class CLayoutObjectTypeWeight;
class CSubTypeWeight;
class CReadingOrderWeight;
class CWeight;
class CReadingOrderPenalties;

/*
 * Class CEvaluationProfile
 *
 * Contains the settings (profile) for document layout evaluation.
 *
 * CC 15.01.2010 - created
 * CC 06.08.2010 - added weights for lines, words and glyphs
 */
class CEvaluationProfile
{
public:
	static const double WEIGHT_MIN;
	static const double WEIGHT_MAX;
	static const double WEIGHT_INIT;

	static const wchar_t * USAGE_AS_DEFINED_IN_GROUNDTRUTH;
	static const wchar_t * USAGE_USE_DEFAULT_IF_NOT_IN_GROUND_TRUTH;
	static const wchar_t * USAGE_ALWAYS_USE_DEFAULT;

	CEvaluationProfile(CXmlValidator * pageLayoutXmlValidator);
	~CEvaluationProfile(void);

	inline bool					IsUsePixelArea() { return m_UsePixelArea.GetValue(); };
	inline double				GetReadingOrientationThreshold() { return m_ReadingOrientationThreshold.GetValue(); };

	inline int					GetMaxOverlapForAllowableSplitAndMerge() { return m_MaxOverlapForAllowableSplitAndMerge.IsSet() ? m_MaxOverlapForAllowableSplitAndMerge.GetValue() : 20; };

	//inline CBoolParameter *	GetUsePixelAreaParam() { return &m_UsePixelArea; };
	inline CParameterMap	*	GetGeneralSettings() { return &m_GeneralSettings; };
	void						SetGeneralSettings(CParameterMap * settings);

	//Default reading direction used for 'allowable' checks (usage: never, if not set in ground-truth, always)
	inline CUniString			GetDefaultReadingDirection() { return m_DefaultReadingDirection.GetValue(); };
	inline CUniString			GetDefaultReadingDirectionUsage() { return m_DefaultReadingDirectionUsage.GetValue(); };

	//Default reading orientation used for 'allowable' checks (usage: never, if not set in ground-truth, always)
	inline double				GetDefaultReadingOrientation() { return m_DefaultReadingOrientation.GetValue(); };
	inline CUniString			GetDefaultReadingOrientationUsage() { return m_DefaultReadingOrientationUsage.GetValue(); };

	//If following parameter is set to TRUE, misclassification of charts, graphics, tables, etc with embedded text is not penalised (if a text region was found instead)
	inline bool					IsIgnoreEmbeddedTextMisclass() { return m_IgnoreEmbeddedTextMisclass.GetValue(); };

	//If set to TRUE, nested regions are taken into account, otherwise they are ignored
	inline bool					IsEvaluateNestedRegions() { return m_EvaluateNestedRegions.GetValue(); };


	inline std::map<int, CErrorTypeWeight *> * GetErrorTypeWeights() { return &m_ErrorTypeWeights; };
	inline std::map<int, CLayoutObjectTypeWeight *> * GetRegionTypeWeights() { return &m_RegionTypeWeights; };

	double		GetErrorTypeWeight(int errorType, bool allowable = false);
	double		GetErrorTypeWeight(int errorType, int regionType, bool allowable = false);
	double		GetErrorTypeWeight(int errorType, int regionType, int regionType2, bool allowable = false);
	double		GetErrorTypeWeight(int errorType, int regionType, CUniString subType, bool allowable = false);
	double		GetErrorTypeWeight(int errorType, int regionType, CUniString subType, int regionType2, bool allowable = false);
	double		GetErrorTypeWeight(int errorType, int regionType, int regionType2, CUniString subType2, bool allowable = false);
	double		GetErrorTypeWeight(int errorType, int regionType, CUniString subType, int regionType2, CUniString subType2, bool allowable = false);

	double		GetErrorTypeWeightForTextSubStructure(int errorType, int regionLevel);
	double		GetErrorTypeWeightForReadingOrderGroup(int errorType);

	double		GetRegionTypeWeight(int regionType);
	double		GetRegionTypeWeight(int regionType, CUniString subType);

	double		GetReadingOrderWeight();
	void		SetReadingOrderWeight(CReadingOrderWeight * weight);

	CParameter * GetWeightParam(int errorType);
	CParameter * GetWeightParam(int errorType, int regionType);
	CParameter * GetWeightParam(int errorType, int regionType, CUniString subType);
	CParameter * GetWeightParam(int errorType, int regionType, CUniString subType, int regionType2);
	CParameter * GetWeightParam(int errorType, int regionType, CUniString subType, int regionType2, CUniString subType2);

	CParameter * GetRegionTypeWeightParam(int regionType);
	CParameter * GetRegionTypeWeightParam(int regionType, CUniString subType);

	CParameter * GetReadingOrderWeightParam();
	CWeight    * GetReadingOrderWeightObject();

	CParameter * GetErrorTypeWeightParamForTextSubStructure(int errorType, int regionLevel);
	CWeight	   * GetErrorTypeWeightObjectForTextSubStructure(int errorType, int regionLevel);

	CParameter * GetErrorTypeWeightParamForReadingOrderGroup(int errorType);
	CWeight	   * GetErrorTypeWeightObjectForReadingOrderGroup(int errorType);

	CParameter	* GetIncludedBackgroundBorderWeightParam();
	CParameter	* GetExcludedForegroundBorderWeightParam();
	CWeight		* GetIncludedBackgroundBorderWeightObject();
	CWeight		* GetExcludedForegroundBorderWeightObject();
	CParameter	* GetMissingRegionBorderWeightParam(int regionType);
	CParameter	* GetMissingRegionBorderWeightParam(int regionType, CUniString subType);
	CLayoutObjectTypeWeight	* GetMissingRegionBorderWeightObject(int regionType);
	CSubTypeWeight		* GetMissingRegionBorderWeightObject(int regionType, CUniString subType);
	inline std::map<int, CLayoutObjectTypeWeight *> * GetMissingRegionAreaBorderWeights() { return &m_MissingRegionAreaBorderWeights; };

	void AddErrorTypeWeight(CErrorTypeWeight * weight);
	void AddRegionTypeWeight(CLayoutObjectTypeWeight * weight);

	CErrorTypeWeight	*	GetErrorTypeWeightObject	(int errorType);
	CLayoutObjectTypeWeight	*	GetRegionTypeWeightObject	(int regionType);

	CUniString ToString(bool errorTypeWeights, bool regionTypeWeights, bool readingOrderWeight,
						bool printOnlyNonDefaultValues);
	std::vector<std::pair<int,CWeight*>> * GetImportantWeights(bool errorTypeWeights, bool includeAllowable = false, bool include_1_0_weights = false);

	void Reset();

	inline CUniString		GetFilePath() { return m_FilePath; };
	inline void				SetFilePath(CUniString path) { m_FilePath = path; };

	inline CReadingOrderPenalties * GetReadingOrderPenalties() { return m_ReadingOrderPenalties; };

	inline void			SetName(CUniString name) { m_Name = name; };
	inline CUniString	GetName() { return m_Name; };

private:
	void	InitWeights();

	CErrorTypeWeight		*	CreateMergeWeight();
	CErrorTypeWeight		*	CreateMisclassWeight();
	CErrorTypeWeight		*	CreateSplitWeight();
	CErrorTypeWeight		*	CreateMissWeight();
	CErrorTypeWeight		*	CreatePartMissWeight();
	CErrorTypeWeight		*	CreateInventWeight();

	void						CreateRegionTypeWeights();

	CSubTypeWeight			*	CreateSubTypeWeight(CUniString subtype, bool enableAllowable, bool appendRegionType2Weights,
													bool appendSubType2Weights);
	CLayoutObjectTypeWeight		*	CreateRegionTypeWeight(int regionType, bool enableAllowable, bool appendSubTypeWeights,
														bool appendRegionType2Weights,
														bool appendSubType2Weights);
	CLayoutObjectTypeWeight		*	CreateRegionType2ShortCutWeight(int regionType, bool enableAllowable, 
																bool appendSubTypeWeights,
																CLayoutObjectTypeWeight * parentWeight);

	std::vector<int>			*	GetRegionTypes();
	std::vector<CUniString>	*	GetSubTypes(int regionType);

	CWeight * GetWeightObject(int errorType);
	CWeight * GetWeightObject(int errorType, int regionType);
	CWeight * GetWeightObject(int errorType, int regionType, CUniString subType);
	CWeight * GetWeightObject(int errorType, int regionType, int regionType2);
	CWeight * GetWeightObject(int errorType, int regionType, CUniString subType, int regionType2);
	CWeight * GetWeightObject(int errorType, int regionType, int regionType2, CUniString subType2);
	CWeight * GetWeightObject(int errorType, int regionType, CUniString subType, int regionType2, CUniString subType2);

	CWeight * GetRTWeightObject(int regionType);
	CWeight * GetRTWeightObject(int regionType, CUniString subType);

	CLayoutObjectTypeWeight	*	GetRegionTypeWeightObject	(int errorType, int regionType);
	CLayoutObjectTypeWeight	*	GetRegionTypeWeightObject	(int errorType, int regionType, int regionType2);
	CSubTypeWeight		*	GetSubTypeWeightObject		(int errorType, int regionType, CUniString subType);
	CLayoutObjectTypeWeight	*	GetRegionTypeWeightObject	(int errorType, int regionType, CUniString subType, int regionType2);
	CSubTypeWeight		*	GetSubTypeWeightObject		(int errorType, int regionType, int regionType2, CUniString subType2);
	CSubTypeWeight		*	GetSubTypeWeightObject		(int errorType, int regionType, CUniString subType, int regionType2, CUniString subType2);

	CSubTypeWeight		*	GetSubTypeWeightObject		(int regionType, CUniString subType);


public:
	static const int	PARAM_USE_PIXEL_AREA = 2001;
	static const int	PARAM_DEFAULT_TEXT_TYPE = 2007;
	static const wchar_t * STRING_SUBTYPE_UNDEFINED;


private:
	CBoolParameter		m_UsePixelArea;					//Using the black pixel area instead of the area defined by the polygons

	static const int	PARAM_READING_ORIENTATION_THRESHOLD = 2002;
	CDoubleParameter	m_ReadingOrientationThreshold;	//Threshold until when the difference of two reading orientation angles is allowable (used for allowable merges and splits)

	static const int	PARAM_DEFAULT_READING_DIRECTION = 2003;
	CStringParameter	m_DefaultReadingDirection;

	static const int	PARAM_READING_DIRECTION_USAGE = 2004;
	CStringParameter	m_DefaultReadingDirectionUsage;

	static const int	PARAM_DEFAULT_READING_ORIENTATION = 2005;
	CDoubleParameter	m_DefaultReadingOrientation;

	static const int	PARAM_READING_ORIENTATION_USAGE = 2006;
	CStringParameter	m_DefaultReadingOrientationUsage;

	static const int	PARAM_IGNORE_EMB_TEXT_MISCLASS = 2008;
	CBoolParameter		m_IgnoreEmbeddedTextMisclass;	//If set to TRUE, misclassification of charts, graphics, tables, etc with embedded text is not penalised (if a text region was found instead)

	static const int	PARAM_EVALUATE_NESTED_REGIONS = 2009;
	CBoolParameter		m_EvaluateNestedRegions;	//If set to TRUE, nested regions are taken into account, otherwise they are ignored

	static const int	PARAM_MAX_OVERLAP_FOR_ALLOWABLE_SPLIT_AND_MERGE = 2010;
	CIntParameter		m_MaxOverlapForAllowableSplitAndMerge;	//Maximum bounding box overlap in pixels (used for allowable merges and splits)

	CStringParameter	m_DefaultTextType;

	CParameterMap		m_GeneralSettings;

	std::map<int, CErrorTypeWeight *>	m_ErrorTypeWeights;
	std::map<int, CLayoutObjectTypeWeight *>	m_RegionTypeWeights;

	std::map<int, std::map<int, CWeight*>*>	m_TextSubStructureWeights;	//map [regionType, map[errorType, weight]] (regionType: line, word or glyph)

	std::map<int, CWeight*>	m_ReadingOrderGroupWeights;	//map[errorType, weight] 

	CReadingOrderWeight			*	m_ReadingOrderWeight;

	CWeight						*	m_IncludedBackgroundBorderWeight;
	CWeight						*	m_ExcludedForegroundBorderWeight;
	std::map<int, CLayoutObjectTypeWeight *>	m_MissingRegionAreaBorderWeights;

	std::vector<int>		*	 m_RegionTypes;

	CUniString			m_Name;
	CUniString			m_FilePath;

	CXmlValidator	*	m_PageLayoutXmlValidator;

	CReadingOrderPenalties * m_ReadingOrderPenalties;
};


/*
 * Class CWeight
 *
 *
 *
 * CC 09.02.2010 - created
 */
class CWeight
{
public:	
	CWeight(bool enableAllowableWeight = false);
	virtual ~CWeight();

	static const int PARAM_NORMAL_WEIGHT	= 5001;
	static const int PARAM_ALLOWABLE_WEIGHT = 5002;
	static const int PARAM_USE_ALLOWABLE	= 5003;

	inline CParameter *	GetParam() { return m_WeightParam; };
	inline CParameter *	GetAllowableParam() { return m_EnableAllowableWeight ? m_AllowableWeight : NULL; };
	inline CParameter * GetUseAllowableParam() { return m_UseAllowableWeight; }
	double				GetValue();
	double				GetAllowableValue();
	CUniString			GetName();
	CUniString			GetDescription();

	inline void			SetName(CUniString name) { if (m_WeightParam != NULL) m_WeightParam->SetName(name); };
	inline void			SetDescription(CUniString descr) { if (m_WeightParam != NULL) m_WeightParam->SetDescription(descr); };
	void				SetValue(double val);
	void				SetAllowableValue(double val);

	inline bool IsAllowableWeightEnabled()			{ return m_EnableAllowableWeight; };
	inline void EnableAllowableWeight(bool enable)	{ m_EnableAllowableWeight = enable; };
	bool IsUseAllowableWeight();
	void SetUseAllowableWeight(bool use);

	CUniString ToString(bool printOnlyNonDefaultValues = true);
	void GetImportantWeights(std::vector<std::pair<int,CWeight*>> * weights, int level, bool useOnlyNonDefaultValues, bool includeAllowable);

	virtual double GetAverage();

protected:
	CUniString ToString(CUniString prefix, bool printOnlyNonDefaultValues);
	virtual double GetAverage(int & count);


protected:
	CParameter		*	m_WeightParam;				//(non allowable) weight
	CParameter		*	m_AllowableWeight;			//allowable weight (for allowable merge/split)
	bool				m_EnableAllowableWeight;	//Has this error an allowable weight?
	CParameter		*	m_UseAllowableWeight;		//Is the allowable weight used? (otherwise allowable and normal weight are the same)
};


/*
 * Class CErrorTypeWeight
 *
 *
 *
 * CC 09.02.2010 - created
 */
class CErrorTypeWeight : public CWeight
{
public:	
	CErrorTypeWeight(int errorType, bool hasRegionTypes = true);
	~CErrorTypeWeight();

	CLayoutObjectTypeWeight * GetRegionTypeWeight(int regionType);
	void				AddRegionTypeWeight(int regionType, CLayoutObjectTypeWeight * weight);

	inline std::map<int, CLayoutObjectTypeWeight*> * GetRegionTypeWeights() { return &m_RegionTypeWeights; };
	inline int	GetType() { return m_Type; };

	virtual double GetAverage();

protected:
	virtual double GetAverage(int & count);

private:
	int m_Type;
	std::map<int, CLayoutObjectTypeWeight*> m_RegionTypeWeights;
};


/*
 * Class CLayoutObjectTypeWeight
 *
 *
 *
 * CC 09.02.2010 - created
 */
class CLayoutObjectTypeWeight : public CWeight
{
public:	
	CLayoutObjectTypeWeight(int regionType, bool enableAllowable, bool hasChildren = false);
	~CLayoutObjectTypeWeight();

	CSubTypeWeight *	GetSubTypeWeight(CUniString subType);
	void				AddSubTypeWeight(CUniString subType, CSubTypeWeight * weight);

	inline std::map<CUniString, CSubTypeWeight*> * GetSubTypeWeights() { return &m_SubTypeWeights; };

	CLayoutObjectTypeWeight * GetRegionTypeWeight(int regionType);
	void				AddRegionTypeWeight(int regionType, CLayoutObjectTypeWeight * weight);
	void				AddRegionTypeShortcutWeight(int regionType, CLayoutObjectTypeWeight * weight);

	inline std::map<int, CLayoutObjectTypeWeight*> * GetRegionTypeWeights() { return &m_RegionTypeWeights; };

	void				AddRegionTypeShortcutParam(	CParameter * childParam, 
													CParameter * childAllowableParam,
													CParameter * childUseAllowableParam);

	inline int			GetType() { return m_Type; };

	virtual double GetAverage();
	virtual double GetAverage(int & count);

private:
	int m_Type;
	std::map<CUniString, CSubTypeWeight*> m_SubTypeWeights;
	std::map<int, CLayoutObjectTypeWeight*> m_RegionTypeWeights;
};


/*
 * Class CSubTypeWeight
 *
 *
 *
 * CC 09.02.2010 - created
 */
class CSubTypeWeight : public CWeight
{
public:	
	CSubTypeWeight(CUniString subType, bool enableAllowable, bool hasRegionTypes2 = false);
	~CSubTypeWeight();

	CLayoutObjectTypeWeight * GetRegionTypeWeight(int regionType);
	void				AddRegionTypeWeight(int regionType, CLayoutObjectTypeWeight * weight);

	inline std::map<int, CLayoutObjectTypeWeight*> * GetRegionTypeWeights() { return &m_RegionTypeWeights; };

	inline CUniString	GetType() { return m_Type; };

	virtual double GetAverage();
	virtual double GetAverage(int & count);

private:
	CUniString m_Type;
	std::map<int, CLayoutObjectTypeWeight*> m_RegionTypeWeights;
};


/*
 * Class CReadingOrderWeight
 *
 *
 *
 * CC 01.07.2010 - created
 */
class CReadingOrderWeight : public CWeight
{
public:	
	CReadingOrderWeight();
	~CReadingOrderWeight();
};


/*
 * Class CTextSubStructureWeight
 *
 * Weight for text line, word or glyph per error type
 *
 * CC 06.08.2010 - created
 */
class CTextSubStructureWeight : public CWeight
{
public:	
	CTextSubStructureWeight(int errorType);
	~CTextSubStructureWeight();
};


/*
 * Class CBorderWeight
 *
 *
 * CC 22.08.2011 - created
 */
class CBorderWeight : public CWeight
{
public:	
	CBorderWeight(const char * name, const char * description);
	~CBorderWeight();
};


/*
 * Class CReadingOrderPenalties
 *
 * Penalty matrix for reading order evaluation.
 *
 * CC 12.03.2013
 */
class CReadingOrderPenalties
{
public:
	//Relation types (for row and column)
	static const int REL_DIRECTLY_BEFORE			= 1;
	static const int REL_DIRECTLY_AFTER				= 2;
	static const int REL_UNORDERED					= 3;
	static const int REL_UNKNOWN					= 4;
	static const int REL_NOT_DIRECT_OR_UNORDERED	= 5;
	static const int REL_NOT_DEFINED				= 6;
	static const int REL_SOMEWHERE_BEFORE			= 7;
	static const int REL_SOMEWHERE_AFTER			= 8;

public:
	CReadingOrderPenalties();
	~CReadingOrderPenalties();

	void Reset();

	int GetPenalty(int row, int column);

	void SetPenalty(int row, int column, int penalty);

	int GetMaxPenalty();

	bool IsDefaultMatrix();

private:
	int** m_Matrix;
	int m_MaxPenalty;
};


}