#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "LayoutEvaluation.h"
#include "EvaluationProfile.h"
#include "MetaData.h"
#include "ExtraString.h"

namespace PRImA
{

/*
 * Class CEvaluationWriter
 *
 * Evaluation writer base class.
 *
 * CC 15.01.2010 - created
 */

class CEvaluationWriter
{
public:
	CEvaluationWriter(void);
	virtual ~CEvaluationWriter(void) = 0;

	virtual void WriteLayoutEvaluation(	CLayoutEvaluation * layoutEval, 
										CMetaData * metaData, CUniString fileName) = 0;

	virtual void WriteEvaluationProfile(CEvaluationProfile * profile, 
										CMetaData * metaData, CUniString fileName) = 0;

	virtual void Write(	CLayoutEvaluation * layoutEval, 
						CEvaluationProfile * profile, 
						CMetaData * metaData,
						CUniString fileName) = 0;

};

}