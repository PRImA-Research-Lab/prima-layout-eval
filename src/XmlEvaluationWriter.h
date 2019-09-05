#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "evaluationwriter.h"
#include "XmlValidator.h"
#include "XmlEvaluationReader.h"
#include "ExtraString.h"
#include "ExtraFileHelper.h"
#include "MsXmlWriter.h"
#include "MsXmlNode.h"
#include "MsXmlParameterWriter.h"

namespace PRImA
{

class CXmlEvaluationReader;
class CWeightSetting;


/*
 * Class CXmlEvaluationWriter
 *
 * Writer for evaluation profile and results.
 *
 * CC
 */

class CXmlEvaluationWriter :
	public CEvaluationWriter
{
public:
	CXmlEvaluationWriter(void);
	CXmlEvaluationWriter(CValidator * validator);
	~CXmlEvaluationWriter(void);

	void WriteLayoutEvaluation(	CLayoutEvaluation * layoutEval, 
								CMetaData * metaData, CUniString fileName);

	void WriteEvaluationProfile(CEvaluationProfile * profile, 
								CMetaData * metaData, CUniString fileName);

	void Write(	CLayoutEvaluation * layoutEval, 
				CEvaluationProfile * profile, 
				CMetaData * metaData,
				CUniString fileName);
private:
	//xmlNsPtr	SetNameSpace(xmlNodePtr node);
	void WriteProfile(CEvaluationProfile * profile, CMsXmlNode * parentNode);
	CMsXmlNode * WriteWeightNode(	CWeight * weight, CMsXmlNode * parentNode, 
									CWeightSetting * defaultSetting,
									const wchar_t * elementName = NULL);
	void WriteReadingOrderPenalties(CEvaluationProfile * profile, CMsXmlNode * profileNode);

	void WriteRawDataAndMetrics(CLayoutEvaluation * layoutEval, CMsXmlNode * parentNode);
	void WriteRawData(CEvaluationResults * results, CMsXmlNode * parentNode);
	void WriteMetricResults(CEvaluationResults * results, CMsXmlNode * metricsNode);
	void WriteMetricResult(CLayoutObjectEvaluationMetrics * metricResult, CMsXmlNode * metricsNode);
	void WriteOverlapEntries(CUniString region1, set<CUniString> * regions2, CMsXmlNode * overlapNode);
	void WriteRegionResults(CUniString region, CLayoutObjectEvaluationResult * results, CMsXmlNode * resultsNode);
	void WriteRegionError(CLayoutObjectEvaluationError * error, CMsXmlNode * errorNode);
	void WriteRects(list<CRect*> * rects, CMsXmlNode * rectsNode);
	void WriteOverlaps(COverlapRects * overlapRects, CMsXmlNode * node);

	void WriteReadingOrderError(CReadingOrderError * error, CMsXmlNode * readingOrderErrorNode);
	void WriteRelation(set<int> * relation, CMsXmlNode * node);

	void WriteIntPerRegionTypeNode(map<int, int> * values, const wchar_t * elementName, CMsXmlNode * parentNode);
	void WriteDoublePerRegionTypeNode(map<int, double> * values, const wchar_t * elementName, CMsXmlNode * parentNode);
	void WriteIntPerErrorTypeNode(map<int, int> * values, const wchar_t * elementName, CMsXmlNode * parentNode);
	void WriteDoublePerErrorTypeNode(map<int, double> * values, const wchar_t * elementName, CMsXmlNode * parentNode);

	CWeightSetting * FindMostCommonSetting(CWeight * weight);
	void FindMostCommonSetting(CWeight * weight, std::vector<CWeightSetting> * weightSettings);
	bool CompareAllchildWeightsAgainstSetting(CWeight * weight, CWeightSetting * weightSetting);

	CUniString RegionTypeIntToString(int type);
	CUniString PageObjectLevelIntToString(int level);
	CUniString ErrorTypeIntToString(int type);
	CUniString ReadingOrderRelationTypeIntToString(int type);

private:
	CValidator	*	m_XmlValidator;
};

/*
 * Class CWeightSetting
 *
 * CC 11.03.2013
 */
class CWeightSetting
{
public:
	CWeightSetting(double value, double allowableValue, bool enableAllowable, bool useAllowable);
	CWeightSetting(CWeightSetting * toCopy);
	~CWeightSetting();

	bool operator==(const CWeightSetting& other);

	inline void IncrementCount() { m_Count++; };
	inline int GetCount() { return m_Count; };

	inline double GetValue() { return m_Value; };
	inline double GetAllowableValue() { return m_AllowableValue; };
	inline bool IsAllowableWeightEnabled() { return m_EnableAllowable; };
	inline bool IsUseAllowableWeight() { return m_UseAllowable; };

private:
	int m_Count;
	double m_Value;
	double m_AllowableValue;
	bool m_EnableAllowable;
	bool m_UseAllowable;
};


}