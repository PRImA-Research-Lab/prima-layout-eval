#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "glyph.h"
#include <map>

namespace PRImA
{

/*
 * Class CGlyphStatisticsItem
 *
 * Represents one item of the glyph statistics
 */
class CGlyphStatisticsItem
{
public:
	CGlyphStatisticsItem(CGlyph * groundTruthGlyph);

	void Update(CGlyph * ocrResultGlyph);

public:
	CUniString m_GroundTruthCharacter;
	int m_Misses;
	int m_Matches;
	int m_Mismatches;
	std::map<CUniString,int> m_OcrErrors; //Map [ocr result character, error count]
};

/*
 * Class CGlyphStatistics
 *
 * Provides a lookup table for what characters have been misrecognised as what other characters and how often
 *
 * CC 01/02/2016 - created
 */
class CGlyphStatistics
{
public:
	CGlyphStatistics(void);
	~CGlyphStatistics(void);

	void AddEntry(CGlyph * groundTruthGlyph, CGlyph * ocrResultGlyph);

	std::map<CUniString, CGlyphStatisticsItem*> * GetItems();

private:
	std::map<CUniString, CGlyphStatisticsItem*> m_Items;
};

} //end namespace