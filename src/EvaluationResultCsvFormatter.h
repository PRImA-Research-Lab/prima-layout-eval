#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "ExtraString.h"
#include "LayoutEvaluation.h"

namespace PRImA
{


/*
 * Class CEvaluationResultCsvFormatter
 *
 * CSV output for layout evaluation results.
 *
 * CC 24/09/2017 - created
 */
class CEvaluationResultCsvFormatter
{
public:
	CEvaluationResultCsvFormatter(CUniString lineBreak);
	~CEvaluationResultCsvFormatter();

	void OutputCsv(CLayoutEvaluation * eval, CUniString groundTruthFile, CUniString segResultFile,
		bool printHeaders, CUniString profile);

	bool CreateCsv(CUniString & headers, CUniString & values, int layoutObjectLevel, CLayoutEvaluation * eval, CUniString groundTruthFile, CUniString segResultFile,
		 CUniString profile);

	void OutputBorderEvalResults(CEvaluationResults * results, CUniString & headers, CUniString & values,
		std::vector<int> & errorTypes, std::vector<int> & regionTypes);

	void OutputLayoutObjectEvalResults(CEvaluationResults * results, CUniString & headers, CUniString & values,
		std::vector<int> & errorTypes, std::vector<int> & regionTypes);

	void OutputGlyphStatistics(CLayoutEvaluation * eval, CUniString groundTruthFile, CUniString segResultFile,
		bool printHeaders);

	void OutputGlyphStatisticsRow(CUniString groundTruthFile, CUniString segResultFile,
		CUniString groundTruthCharacter, CUniString groundTruthUnicode,
		CUniString ocrResultCharacter, CUniString ocrResultUnicode,
		CUniString ocrError, int count);

	void CharsToHexString(CUniString str);

	void GetAllErrorTypes(std::vector<int> & errorTypes);
	void GetAllRegionTypes(std::vector<int> & regionTypes);

private:
	CUniString m_LineBreak;

};

} //end namespace