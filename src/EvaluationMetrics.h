#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "EvaluationResults.h"
#include "EvaluationProfile.h"

namespace PRImA
{

class CFunction;
class CEvaluationResults;
class CEvaluationProfile;

/*
 * Class CEvaluationMetrics
 *
 * Base class for metric results (success rates etc.).
 *
 * CC 23.08.2011
 */
class CEvaluationMetrics
{
public:
	CEvaluationMetrics(CEvaluationResults * results, CEvaluationProfile * profile);
	virtual ~CEvaluationMetrics();

	inline CEvaluationResults * GetEvaluationResults() { return m_Results; };


protected:
	CEvaluationResults * m_Results;
	CEvaluationProfile * m_Profile;
};


/*
 * Class CBorderEvaluationMetrics
 *
 * Border evaluation metric results.
 *
 * CC 23.08.2011
 */
class CBorderEvaluationMetrics : public CEvaluationMetrics
{
public:
	CBorderEvaluationMetrics(CEvaluationResults * results, CEvaluationProfile * profile, bool calculate);
	
	inline double GetIncludedBackgroundSuccessRate() { return m_IncludedBackgroundSuccessRate; };
	inline double GetExcludedForegroundSuccessRate() { return m_ExcludedForegroundSuccessRate; };
	inline double GetMissingRegionAreaSuccessRate() { return m_MissingRegionAreaSuccessRate; };
	inline double GetOverallSuccessRate() { return m_OverallSuccessRate; };

	inline void SetIncludedBackgroundSuccessRate(double val) { m_IncludedBackgroundSuccessRate = val; };
	inline void SetExcludedForegroundSuccessRate(double val) { m_ExcludedForegroundSuccessRate = val; };
	inline void SetMissingRegionAreaSuccessRate(double val) { m_MissingRegionAreaSuccessRate = val; };
	inline void SetOverallSuccessRate(double val) { m_OverallSuccessRate = val; };

	inline int GetImageArea() { return m_ImageArea; };
	inline void SetImageArea(int area) { m_ImageArea = area; };

	inline int GetGroundTruthBorderArea() { return m_GroundTruthBorderArea; };
	inline void SetGroundTruthBorderArea(int area) { m_GroundTruthBorderArea = area; };
	inline int GetSegResultBorderArea() { return m_SegResultBorderArea; };
	inline void SetSegResultBorderArea(int area) { m_SegResultBorderArea = area; };

	inline int GetOverallGroundTruthRegionArea() { return m_OverallGroundTruthRegionArea; };
	inline void SetOverallGroundTruthRegionArea(int area) { m_OverallGroundTruthRegionArea = area; };

private:
	void CalculateGeneralFigures();
	void CalculateWeightedErrors();
	void CalculateSuccessRates();
	CFunction * GetSuccessFunction();

private:
	double m_IncludedBackgroundSuccessRate;
	double m_IncludedBackgroundSuccessRateRelativeToImage;
	double m_ExcludedForegroundSuccessRate;
	double m_ExcludedForegroundSuccessRateRelativeToImage;
	double m_MissingRegionAreaSuccessRate;
	double m_MissingRegionAreaSuccessRateRelativeToImage;
	double m_OverallSuccessRate;
	double m_TotalWeightedMissingRegionArea;

	int m_ImageArea;			//image width * height
	int m_GroundTruthBorderArea;
	int m_SegResultBorderArea;
	int m_OverallGroundTruthRegionArea;					//Combined area of all ground-truth regions
};


/*
 * Class CLayoutObjectEvaluationMetrics
 *
 * Combined penalties / error scores / success rates of the evaluation result.
 *
 * CC 25.06.2010
 */
class CLayoutObjectEvaluationMetrics : public CEvaluationMetrics
{
public:
	CLayoutObjectEvaluationMetrics(CEvaluationResults * results, CEvaluationProfile * profile, bool calculate,
						int layoutRegionType = CLayoutRegion::TYPE_ALL,
						CLayoutObjectEvaluationMetrics * metricsForAllTypes = NULL);
	~CLayoutObjectEvaluationMetrics();

	inline int GetRegionType() { return m_LayoutRegionType; };

	//*** General figures ***//
	inline int GetNumberOfGroundTruthRegions() { return m_NumberOfGroundTruthRegions; };
	inline int GetNumberOfSegResultRegions() { return m_NumberOfSegResultRegions; };
	inline void SetNumberOfGroundTruthRegions(int number) { m_NumberOfGroundTruthRegions = number; };
	inline void SetNumberOfSegResultRegions(int number) { m_NumberOfSegResultRegions = number; };

	inline std::map<int,int> * GetNumberOfGroundTruthRegionsPerType() { return &m_NumberOfGroundTruthRegionsPerType; };
	inline std::map<int,int> * GetNumberOfSegResultRegionsPerType() { return &m_NumberOfSegResultRegionPerType; };
	int GetNumberOfGroundTruthRegionsPerType(int regionType);
	int GetNumberOfSegResultRegionsPerType(int regionType);
	void SetNumberOfGroundTruthRegionsPerType(int type, int number);
	void SetNumberOfSegResultRegionsPerType(int type, int number);

	inline int GetImageArea() { return m_ImageArea; };
	inline int GetImageForegroundPixelCount() { return m_ForeGroundPixelCount; };
	inline void SetImageArea(int area) { m_ImageArea = area; };
	inline void SetImageForegroundPixelCount(int count) { m_ForeGroundPixelCount = count; };

	inline int GetOverallGroundTruthRegionArea() { return m_OverallGroundTruthRegionArea; };
	inline int GetOverallSegResultRegionArea() { return m_OverallSegResultRegionArea; };
	inline void SetOverallGroundTruthRegionArea(int area) { m_OverallGroundTruthRegionArea = area; };
	inline void SetOverallSegResultRegionArea(int area) { m_OverallSegResultRegionArea = area; };

	inline std::map<int,int> * GetOverallGroundTruthRegionAreaPerType() { return &m_GroundTruthRegionAreaPerType; };
	inline std::map<int,int> * GetOverallSegResultRegionAreaPerType() { return &m_SegResultRegionAreaPerType; };
	int GetOverallGroundTruthRegionAreaPerType(int regionType);
	int GetOverallSegResultRegionAreaPerType(int regionType);
	void SetOverallGroundTruthRegionAreaPerType(int type, int area);
	void SetOverallSegResultRegionAreaPerType(int type, int area);

	inline int GetOverallGroundTruthRegionPixelCount() { return m_OverallGroundTruthRegionPixelCount; };
	inline int GetOverallSegResultRegionPixelCount() { return m_OverallSegResultRegionPixelCount; };
	inline void SetOverallGroundTruthRegionPixelCount(int count) { m_OverallGroundTruthRegionPixelCount = count; };
	inline void SetOverallSegResultRegionPixelCount(int count) { m_OverallSegResultRegionPixelCount = count; };

	inline std::map<int,int> * GetOverallGroundTruthRegionPixelCountPerType() { return &m_GroundTruthRegionPixelCountPerType; };
	inline std::map<int,int> * GetOverallSegResultRegionPixelCountPerType() { return &m_SegResultRegionPixelCountPerType; };
	int GetOverallGroundTruthRegionPixelCountPerType(int regionType);
	int GetOverallSegResultRegionPixelCountPerType(int regionType);
	void SetOverallGroundTruthRegionPixelCountPerType(int type, int count);
	void SetOverallSegResultRegionPixelCountPerType(int type, int count);

	inline std::map<int,int> * GetRecallAreaPerType() { return &m_RecallAreaPerType; };
	inline std::map<int,int> * GetRecallPixelCountPerType() { return &m_RecallPixelCountPerType; };
	void SetRecallAreaPerType(int type, int area);
	void SetRecallPixelCountPerType(int type, int count);

	/*** Error values ***/
	inline std::map<int, double> * GetOverallWeightedAreaErrorPerErrorType() { return &m_OverallWeightedAreaErrorPerErrorType; };
	inline std::map<int, double> * GetInfluenceOverallWeightedAreaErrorPerErrorType() { return &m_WeightedAreaInfluencePerType; };
	inline std::map<int, double> * GetOverallWeightedAreaErrorPerRegionType() { return &m_OverallWeightedAreaErrorPerRegionType; };
	double GetOverallWeightedAreaErrorPerErrorType(int errorType);
	double GetOverallWeightedAreaErrorPerRegionType(int regionType);
	void SetOverallWeightedAreaErrorPerErrorType(int type, double area);
	void SetOverallWeightedAreaErrorPerRegionType(int type, double area);

	double GetRelativeWeightedAreaErrorPerRegionType(int regionType);

	inline double GetOverallWeightedAreaError() { return m_OverallWeightedAreaError; };
	inline void SetOverallWeightedAreaError(double error) { m_OverallWeightedAreaError = error; };

	inline std::map<int, double> * GetOverallWeightedCountErrorPerErrorType() { return &m_OverallWeightedCountErrorPerErrorType; };
	inline std::map<int, double> * GetInfluenceOverallWeightedCountErrorPerErrorType() { return &m_WeightedCountInfluencePerType; };
	inline std::map<int, double> * GetOverallWeightedCountErrorPerRegionType() { return &m_OverallWeightedCountErrorPerRegionType; };
	double GetOverallWeightedCountErrorPerErrorType(int errorType);
	double GetOverallWeightedCountErrorPerRegionType(int regionType);
	void SetOverallWeightedCountErrorPerErrorType(int type, double error);
	void SetOverallWeightedCountErrorPerRegionType(int type, double error);

	double GetRelativeWeightedCountErrorPerRegionType(int regionType);

	inline double GetOverallWeightedCountError() { return m_OverallWeightedCountError; };
	inline void SetOverallWeightedCountError(double error ) { m_OverallWeightedCountError = error; };

	inline std::map<int, double> * GetWeightedAreaSuccessRatePerType() { return &m_WeightedAreaSuccessRatePerType; };
	double GetWeightedAreaSuccessRatePerType(int errorType);
	void SetWeightedAreaSuccessRatePerType(int errorType, double rate);
	double GetWeightedAreaInfluencePerType(int errorType);

	inline std::map<int, double> * GetWeightedCountSuccessRatePerType() { return &m_WeightedCountSuccessRatePerType; };
	double GetWeightedCountSuccessRatePerType(int errorType);
	void SetWeightedCountSuccessRatePerType(int errorType, double rate);
	double GetWeightedCountInfluencePerType(int errorType);

	inline double GetReadingOrderError() { return m_ReadingOrderError; };
	inline double GetReadingOrderSuccessRate() { return m_ReadingOrderSuccessRate; };
	inline void SetReadingOrderError(double error) { m_ReadingOrderError = error; };
	inline void SetReadingOrderSuccessRate(double rate) { m_ReadingOrderSuccessRate = rate; };

	inline std::map<int, double> * GetErrorRatePerTypeBasedOnCount() { return &m_ErrorRatePerTypeBasedOnSimpleCount; };
	double GetErrorRatePerTypeBasedOnCount(int errorType);
	void SetErrorRatePerTypeBasedOnCount(int errorType, double rate);

	inline double GetErrorRateBasedOnCount() { return m_ErrorRateBasedOnSimpleCount; };
	inline void SetErrorRateBasedOnCount(double rate) { m_ErrorRateBasedOnSimpleCount = rate; };

	inline double GetOverallWeightedAreaSuccessRate() { return m_OverallWeightedAreaSuccessRate; };		//Arithmetic mean
	inline double GetOverallWeightedCountSuccessRate() { return m_OverallWeightedCountSuccessRate; };	//Arithmetic mean
	inline double GetHarmonicWeightedAreaSuccessRate() { return m_HarmonicWeightedAreaSuccessRate; };	//Harmonic mean
	inline double GetHarmonicWeightedCountSuccessRate() { return m_HarmonicWeightedCountSuccessRate; };	//Harmonic mean
	inline void SetOverallWeightedAreaSuccessRate(double rate) { m_OverallWeightedAreaSuccessRate = rate; };
	inline void SetOverallWeightedCountSuccessRate(double rate) { m_OverallWeightedCountSuccessRate = rate; };
	inline void SetHarmonicWeightedAreaSuccessRate(double rate) { m_HarmonicWeightedAreaSuccessRate = rate; };
	inline void SetHarmonicWeightedCountSuccessRate(double rate) { m_HarmonicWeightedCountSuccessRate = rate; };

	inline double GetRecall(bool strict) { return strict ? m_RecallStrict : m_RecallNonStrict; };
	inline double GetPrecision(bool strict) { return strict ? m_PrecisionStrict : m_PrecisionNonStrict; };				
	inline void SetRecall(bool strict, double recall) { if (strict) m_RecallStrict = recall; else m_RecallNonStrict = recall; }
	inline void SetPrecision(bool strict, double precision) { if (strict) m_PrecisionStrict = precision; else m_PrecisionNonStrict = precision; }

	double GetRecall(int regionType);									//strict
	double GetPrecision(int regionType);								//strict
	inline std::map<int, double> * GetRecallPerType() { return &m_RecallPerType; };
	inline std::map<int, double> * GetPrecisionPerType() { return &m_PrecisionPerType; };
	void SetRecall(int regionType, double recall);
	void SetPrecision(int regionType, double precision);

	inline double GetFMeasure(bool strict) { return strict ? m_FMeasureStrict : m_FMeasureNonStrict; };				
	inline void SetFMeasure(bool strict, double measure) { if (strict) m_FMeasureStrict = measure; else m_FMeasureNonStrict = measure; }

	// | (#ground-truth regions) - (#seg. result regions) |
	inline int		GetRegionCountDeviation() { return abs(m_NumberOfGroundTruthRegions - m_NumberOfSegResultRegions); };
	double			GetRelativeRegionCountDeviation();

	inline double	GetOCRSuccessRate() { return m_OCRSuccessRate; };
	inline void		SetOCRSuccessRate(double rate) { m_OCRSuccessRate = rate; };
	inline double	GetOCRSuccessRateExclReplacementChar() { return m_OCRSuccessRateExclReplacementChar; };
	inline void		SetOCRSuccessRateExclReplacementChar(double rate) { m_OCRSuccessRateExclReplacementChar = rate; };
	inline double	GetOCRSuccessRateForDigits() { return m_OCRSuccessRateForDigits; };
	inline void		SetOCRSuccessRateForDigits(double rate) { m_OCRSuccessRateForDigits = rate; };
	inline double	GetOCRSuccessRateForNumericalChars() { return m_OCRSuccessRateForNumericalChars; };
	inline void		SetOCRSuccessRateForNumericalChars(double rate) { m_OCRSuccessRateForNumericalChars = rate; };

	inline CGlyphStatistics * GetGlyphStatistics() { return m_GlyphStatistics; };
	inline void SetGlyphStatistics(CGlyphStatistics * statistics) { delete m_GlyphStatistics; m_GlyphStatistics = statistics; };

private:
	void CalculateGeneralFigures();
	void CalculateWeightedErrors();
	void CalculateReadingOrderError();
	void CalculateSimpleCountBasedErrorRates();
	void CalculateOverallSuccessRate();
	void CalculateRecallAndPrecision();
	void CalculateOCRSuccessRate();

	CFunction * GetAreaSuccessFunction(int errorType);
	CFunction * GetCountSuccessFunction(int errorType);

	void GetRelativeAreaAndCountPerErrorType(int errorType, double & relativeCount, double & relativeArea);

	void SetWeightedAreaInfluencePerType(int errorType, double influence);
	void SetWeightedCountInfluencePerType(int errorType, double influence);

	bool IsDigitCharacter(CUniString c);
	bool IsNumericalCharacter(CUniString c);

private:
	bool				 m_UsePixelArea;
	int					 m_LayoutRegionType;	//TEXT, IMAGE, TABLE, ... (only for evaluation on block level, not for text lines, words or glyphs)
	CLayoutObjectEvaluationMetrics * m_MetricsForAllTypes;

	//*** General figures ***//
	int m_NumberOfGroundTruthRegions;
	int m_NumberOfSegResultRegions;

	std::map<int,int> m_NumberOfGroundTruthRegionsPerType;	//map [layout region type, count]
	std::map<int,int> m_NumberOfSegResultRegionPerType;		//map [layout region type, count]

	int m_ImageArea;			//image width * height
	int m_ForeGroundPixelCount;	//number of black pixels within the whole image

	std::map<int,int> m_GroundTruthRegionAreaPerType;		//map [region type, area]
	std::map<int,int> m_GroundTruthRegionPixelCountPerType;	//map [region type, foreground pixel count]
	int m_OverallGroundTruthRegionArea;					//Combined area of all ground-truth regions
	int m_OverallGroundTruthRegionPixelCount;			//Combined foreground pixel count of all ground-truth regions

	std::map<int,int> m_SegResultRegionAreaPerType;			//map [region type, area]
	std::map<int,int> m_SegResultRegionPixelCountPerType;	//map [region type, foreground pixel count]
	int m_OverallSegResultRegionArea;					//Combined area of all segmentation result regions
	int m_OverallSegResultRegionPixelCount;				//Combined foreground pixel count of all segmentation result regions

	std::map<int,int> m_RecallAreaPerType;			//map [region type, recall area]  (recall area = ground-truth region area that is oerlapped with a segmentation result region of the same type)
	std::map<int,int> m_RecallPixelCountPerType;		//map [region type, recall pixel count]

	/*** Error values ***/
	std::map<int, double> m_OverallWeightedAreaErrorPerErrorType;	//map [region error type, area error value]
	std::map<int, double> m_OverallWeightedAreaErrorPerRegionType;	//map [region type, area error value]
	double m_OverallWeightedAreaError;							//Overall region area error
	std::map<int, double> m_WeightedAreaSuccessRatePerType;			//map [region error type, success rate]
	std::map<int, double> m_WeightedAreaInfluencePerType;			//map [region error type, influence on overall success rate]

	std::map<int, double> m_OverallWeightedCountErrorPerErrorType;	//map [region error type, count error value]
	std::map<int, double> m_OverallWeightedCountErrorPerRegionType;	//map [region type, count error value]
	double m_OverallWeightedCountError;							//Overall weighted count error
	std::map<int, double> m_WeightedCountSuccessRatePerType;			//map [region error type, success rate]
	std::map<int, double> m_WeightedCountInfluencePerType;			//map [region error type, influence on overall success rate]

	double m_ReadingOrderError;
	double m_ReadingOrderSuccessRate;

	double m_OverallWeightedAreaSuccessRate;		//Arithmetic mean
	double m_OverallWeightedCountSuccessRate;		//Arithmetic mean
	double m_HarmonicWeightedAreaSuccessRate;		//Harmonic mean
	double m_HarmonicWeightedCountSuccessRate;		//Harmonic mean

	std::map<int, double> m_ErrorRatePerTypeBasedOnSimpleCount;	//map [region error type, value 0..1] (0=no error)
	double m_ErrorRateBasedOnSimpleCount;					//value 0..1 based on the error count

	std::map<int, double> m_RecallPerType;		// map [region type, recall]		(strict)
	std::map<int, double> m_PrecisionPerType;	// map [region type, precision]		(strict)

	double m_RecallNonStrict;		//non-strict recall			non-strict means, that if a ground-truth region
	double m_RecallStrict;			//							is overlapped by a segmentation result region
	double m_PrecisionNonStrict;	//non-strict precision		of another type, it is still a recall.
	double m_PrecisionStrict;		//							(strict = wich classification, non-strict = w/o classification)

	double m_FMeasureStrict;		//2 * prec * recall / (prec + recall)
	double m_FMeasureNonStrict;		//2 * prec * recall / (prec + recall)

	double m_OCRSuccessRate;
	double m_OCRSuccessRateExclReplacementChar;
	double m_OCRSuccessRateForDigits;
	double m_OCRSuccessRateForNumericalChars;

	CGlyphStatistics * m_GlyphStatistics;
};


/*
 * Class CFunction
 *
 * CC 29.06.2010 - created
 */
class CFunction
{
public:
	virtual double GetY(double x) = 0;
};


/*
 * Class CLogSuccessRate
 *
 * Function that returns a success rate (as percentage 0-100) for a given x.
 * Uses a logarithmic function.
 *
 * CC 29.06.2010 - created
 */
class CLogSuccessRate : public CFunction
{
public:
	CLogSuccessRate(double fiftyPercentX);
	~CLogSuccessRate();

	double GetY(double x);

private:
	//Formula: y = a - b*log(c*(x+d))

	double a;	//translation in y direction
	double b;	//scaling in y direction
	double c;	//scaling in x direction
	double d;	//translation in x direction
};


/*
 * Class CInverseSuccessRate
 *
 * Function that returns a success rate (as percentage 0-100) for a given x.
 * Uses a inverse function (a/x).
 *
 * CC 29.06.2010 - created
 */
class CInverseSuccessRate : public CFunction
{
public:
	CInverseSuccessRate(double fiftyPercentX);
	~CInverseSuccessRate();

	double GetY(double x);

private:
	//Formula: y = 100 / (a*x+1)

	double a;
};


}//end namespace