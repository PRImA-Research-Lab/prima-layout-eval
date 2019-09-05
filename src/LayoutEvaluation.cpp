
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "stdafx.h"
#include "LayoutEvaluation.h"
#include "GraphicRegionInfo.h"
#include "chartregioninfo.h"

using namespace PRImA;
using namespace std;

/*
 * Class CLayoutEvaluation
 *
 * Container for evaluation results
 *
 * CC 13.01.2010 - created
 * CC 02.11.2010 - Changed all keys from CLayoutObject to CUniString (region ID)
 */

/*
 * Constructor
 *
 * 'takeResonsibiltyForDocumentsAndImage' - If true, the layout evaluation will release (delete) 
 *                                          the documents and the images on destruction.
 */
CLayoutEvaluation::CLayoutEvaluation(bool takeResonsibiltyForDocumentsAndImages /*= true*/)
{
	m_GrountTruth = NULL;
	m_SegResult = NULL;
	m_BilevelImage = NULL;
	m_ColourImage = NULL;
	m_Width = -1;
	m_Height = -1;
	m_Profile = NULL;
	m_HasResonsibiltyForDocumentsAndImages = takeResonsibiltyForDocumentsAndImages;
}

CLayoutEvaluation::~CLayoutEvaluation(void)
{
	map<int, CEvaluationResults *>::iterator it = m_Results.begin();
	while (it != m_Results.end())
	{
		delete (*it).second;
		it++;
	}

	if (m_HasResonsibiltyForDocumentsAndImages)
	{
		delete m_GrountTruth;
		delete m_SegResult;
		delete m_BilevelImage;
		delete m_ColourImage;
	}
}

/*
 * Copies all fields (except the evaluation results) from the given other layout evaluation object.
 * Sets hasResonsibiltyForDocumentsAndImages to FALSE.
 */
void CLayoutEvaluation::InitialiseFrom(CLayoutEvaluation * other)
{
	m_HasResonsibiltyForDocumentsAndImages = false;

	m_GrountTruth = other->m_GrountTruth;
	m_SegResult = other->m_SegResult;
	m_BilevelImage = other->m_BilevelImage;
	m_ColourImage = other->m_ColourImage;

	m_GroundTruthLocation = other->m_GroundTruthLocation;
	m_SegResultLocation = other->m_SegResultLocation;
	m_BilevelImageLocation = other->m_BilevelImageLocation;
	m_ColourImageLocation = other->m_ColourImageLocation;

	m_Width = other->m_Width;
	m_Height = other->m_Height;

	m_Profile = other->m_Profile;
}

int CLayoutEvaluation::GetWidth()
{
	if (m_Width < 0 && m_GrountTruth != NULL)
		m_Width = m_GrountTruth->GetWidth();
	if (m_Width < 0 && m_SegResult != NULL)
		m_Width = m_SegResult->GetWidth();
	return m_Width;
}

int CLayoutEvaluation::GetHeight()
{
	if (m_Height < 0 && m_GrountTruth != NULL)
		m_Height = m_GrountTruth->GetHeight();
	if (m_Height < 0 && m_SegResult != NULL)
		m_Height = m_SegResult->GetHeight();
	return m_Height;
}

void CLayoutEvaluation::SetWidth(int w)
{
	m_Width = w;
	if (m_GrountTruth != NULL)
		m_GrountTruth->SetWidth(w);
	if (m_SegResult != NULL)
		m_SegResult->SetWidth(w);
}

void CLayoutEvaluation::SetHeight(int h)
{
	m_Height = h;
	if (m_GrountTruth != NULL)
		m_GrountTruth->SetHeight(h);
	if (m_SegResult != NULL)
		m_SegResult->SetHeight(h);
}

void CLayoutEvaluation::SetGroundTruth(CPageLayout * groundTruth) 
{ 
	if (groundTruth != m_GrountTruth)
	{
		delete m_GrountTruth;
		m_GrountTruth = groundTruth;
		if (m_GrountTruth != NULL)
		{
			m_Width = m_GrountTruth->GetWidth();
			m_Height = m_GrountTruth->GetHeight();
		}
	}
}

void CLayoutEvaluation::SetSegResult(CPageLayout * segResult) 
{ 
	if (segResult != m_SegResult)
	{
		delete m_SegResult;
		m_SegResult = segResult;
		if (m_SegResult != NULL)
		{
			m_Width = m_SegResult->GetWidth();
			m_Height = m_SegResult->GetHeight();
		}
	}
}

CSingleLock * CLayoutEvaluation::Lock()
{
	CSingleLock * lockObject = new CSingleLock(&m_CriticalSect);
	lockObject->Lock();
	return lockObject;
}

void CLayoutEvaluation::Unlock(CSingleLock * lockObject)
{
	if (lockObject != NULL)
	{
		lockObject->Unlock();
		delete lockObject;
	}
}

void CLayoutEvaluation::SetBilevelImage(COpenCvBiLevelImage * img) 
{ 
	if (img != m_BilevelImage)
	{
		delete m_BilevelImage;
		m_BilevelImage = img;
		if (m_BilevelImage != NULL)
		{
			m_Width = m_BilevelImage->GetWidth();
			m_Height = m_BilevelImage->GetHeight();
		}
	}
}

void CLayoutEvaluation::SetColourImage(COpenCvImage * img) 
{ 
	if (img != m_ColourImage)
	{
		delete m_ColourImage;
		m_ColourImage = img;
		if (m_ColourImage != NULL)
		{
			m_Width = m_ColourImage->GetWidth();
			m_Height = m_ColourImage->GetHeight();
		}
	}
}

/*
 * Returns the results per object level (layout region, text line, word, glyph, group)
 *
 * 'createIfNotExists' - If true and no object exists yet, a results object for 
 *                       the given type will be created and stored.
 */
CEvaluationResults * CLayoutEvaluation::GetResults(int layoutObjectType, bool createIfNotExists)
{
	map<int, CEvaluationResults*>::iterator it = m_Results.find(layoutObjectType);

	CEvaluationResults * ret = NULL;
	if (it != m_Results.end()) //already exists
		ret = (*it).second;
	else if (createIfNotExists)
	{
		ret = new CEvaluationResults(this, m_Profile, layoutObjectType);
		m_Results.insert(pair<int, CEvaluationResults*>(layoutObjectType, ret));
	}
	return ret;
}

void CLayoutEvaluation::DeleteResults(int layoutObjectType)
{
	map<int, CEvaluationResults*>::iterator it = m_Results.find(layoutObjectType);
	if (it != m_Results.end()) //exists
	{
		delete (*it).second;
		m_Results.erase(it);
	}
}

/*
 * Returns the subtype of the given layout region.
 * Returns the type name or NULL, if the region has no subtype.
 */
CUniString CLayoutEvaluation::GetLayoutRegionSubtype(CLayoutRegion * reg, CEvaluationProfile * profile)
{
	CUniString subType;
	int layoutRegionType = reg->GetType();
	if (layoutRegionType == CLayoutRegion::TYPE_TEXT)
	{
		CTextRegionInfo * info = (CTextRegionInfo*)reg->GetRegionInfo();
		if (info->IsTextTypeSet())
			subType = info->GetTextType();
		else if (profile != NULL)
		{
			CParameterMap * settings = profile->GetGeneralSettings();
			CStringParameter * defTextType = (CStringParameter*)settings->GetParamForId(CEvaluationProfile::PARAM_DEFAULT_TEXT_TYPE);
			if (defTextType != NULL && defTextType->IsSet())
				subType = defTextType->GetValue();
			else
				subType = CEvaluationProfile::STRING_SUBTYPE_UNDEFINED;
		}
	}
	else if (layoutRegionType == CLayoutRegion::TYPE_GRAPHIC)
	{
		CGraphicRegionInfo * info = (CGraphicRegionInfo*)reg->GetRegionInfo();
		if (info->IsTypeSet())
			subType = info->GetType();
		else
			subType = CEvaluationProfile::STRING_SUBTYPE_UNDEFINED;
	}
	else if (layoutRegionType == CLayoutRegion::TYPE_CHART)
	{
		CChartRegionInfo * info = (CChartRegionInfo*)reg->GetRegionInfo();
		if (info->IsTypeSet())
			subType = info->GetType();
		else
			subType = CEvaluationProfile::STRING_SUBTYPE_UNDEFINED;
	}

	return subType;
}

