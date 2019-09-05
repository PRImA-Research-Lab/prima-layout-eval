#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "LayoutEvaluation.h"
#include "EvaluationProfile.h"
#include "MetaData.h"
#include "Crypto.h"

namespace PRImA
{

class CEvaluationReaderListener;

/*
 * DocReader base class.
 *
 * CC 15.01.2010 - created
 */
class CEvaluationReader
{
public:
	CEvaluationReader(void);
	~CEvaluationReader(void);

	virtual bool	ReadLayoutEvaluation(	CUniString fileName, CLayoutEvaluation * layoutEval, 
											CMetaData * metaData) = 0;

	virtual bool	ReadEvaluationProfile(CUniString fileName, CEvaluationProfile * profile,
											CMetaData * metaData, CCrypto * crypto = NULL) = 0;

	virtual bool	Read(CUniString fileName,
							CLayoutEvaluation * layoutEvaluation,
							CEvaluationProfile * evaluationProfile,
							CMetaData * metaData) = 0;

	virtual CUniString		GetErrorMsg() = 0;

	virtual void SetListener(CEvaluationReaderListener * listener) = 0;

};

/*
 * Class CEvaluationReaderListener
 *
 * Interface for document file listener
 *
 * CC 12.07.2010 - created
 */
class CEvaluationReaderListener
{
public:
	virtual void GroundTruthFileNameLoaded(CLayoutEvaluation * layoutEval, CUniString fileName) = 0;
	virtual void SegmentationResultFileNameLoaded(CLayoutEvaluation * layoutEval, CUniString fileName) = 0;
	virtual void ImageFileNameLoaded(CLayoutEvaluation * layoutEval, CUniString fileName) = 0;
};

}