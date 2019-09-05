#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "DocumentLayout.h"
#include <map>
#include <set>
#include <deque>
#include "RegionOverlap.h"
#include "EvaluationProfile.h"
#include "opencvimage.h"
#include "GlyphStatistics.h"
#include "EvaluationResults.h"


namespace PRImA
{

class CEvaluationResults;
class CEvaluationProfile;

/*
 * Class CLayoutEvaluation
 *
 * Container for evaluation results and settings
 *
 * CC 13.01.2010 - created
 */

class CLayoutEvaluation
{
public:
	CLayoutEvaluation(bool takeResonsibiltyForDocumentsAndImages = true);
	~CLayoutEvaluation(void);

	void InitialiseFrom(CLayoutEvaluation * other);

	inline CString	GetGroundTruthLocation() { return m_GroundTruthLocation; };
	inline CString	GetSegResultLocation() { return m_SegResultLocation; };
	inline CString	GetBilevelImageLocation() { return m_BilevelImageLocation; };
	inline CString	GetColourImageLocation() { return m_ColourImageLocation; };

	inline void		SetGroundTruthLocation(CString path) { m_GroundTruthLocation = path; };
	inline void		SetSegResultLocation(CString path) { m_SegResultLocation = path; };
	inline void		SetBilevelImageLocation(CString path) { m_BilevelImageLocation = path; };
	inline void		SetColourImageLocation(CString path) { m_ColourImageLocation = path; };

	inline CPageLayout	*	GetGroundTruth() { return m_GrountTruth; };
	inline CPageLayout	*	GetSegResult() { return m_SegResult; };
	void						SetGroundTruth(CPageLayout * groundTruth);
	void						SetSegResult(CPageLayout * segResult);

	inline COpenCvBiLevelImage	*	GetBilevelImage() { return m_BilevelImage; };
	inline COpenCvImage			*	GetColourImage() { return m_ColourImage; };
	void						SetBilevelImage(COpenCvBiLevelImage * img);
	void						SetColourImage(COpenCvImage * img);

	int							GetWidth();
	int							GetHeight();
	void						SetWidth(int w);
	void						SetHeight(int h);

	CEvaluationResults		*	GetResults(int layoutObjectType, bool createIfNotExists = false);

	void						DeleteResults(int layoutObjectType);

	inline CEvaluationProfile * GetProfile() { return m_Profile; };
	inline void					SetProfile(CEvaluationProfile * profile) { m_Profile = profile; };

	inline bool					IsEmpty() { return m_GrountTruth == NULL && m_SegResult == NULL; };

	CSingleLock				*	Lock();
	void						Unlock(CSingleLock * lockObject);

	static CUniString GetLayoutRegionSubtype(CLayoutRegion * reg, CEvaluationProfile * profile);

private:
	CString	m_GroundTruthLocation;	//Full path to the ground truth XML file
	CString	m_SegResultLocation;	//Full path to the segmentation result XML file
	CString	m_BilevelImageLocation;	//Full path to the bi-level image file
	CString	m_ColourImageLocation;	//Full path to the colour image file

	CPageLayout		*	m_GrountTruth;		//Ground-truth document layout
	CPageLayout		*	m_SegResult;		//Segmentation result document layout

	COpenCvBiLevelImage		*	m_BilevelImage;		//Black-and-white image
	COpenCvImage			*	m_ColourImage;		//Colour or grey level image

	int						m_Width;			//Document
	int						m_Height;			//dimensions

	CEvaluationProfile	*	m_Profile;			//Weights and settings for the evaluation

	std::map<int, CEvaluationResults *> m_Results;	//Map [layoutObjectType, EvaluationResults]

	bool m_HasResonsibiltyForDocumentsAndImages;

	CCriticalSection m_CriticalSect;			//For synchronization

};



} //end namespace