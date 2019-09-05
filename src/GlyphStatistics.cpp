
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "stdafx.h"
#include "GlyphStatistics.h"

using namespace std;

namespace PRImA
{
/*
 * Class CGlyphStatistics
 *
 * Provides a lookup table for what characters have been misrecognised as what other characters and how often
 *
 * CC 01/02/2016 - created
 */

/*
 * Constructor
 */
CGlyphStatistics::CGlyphStatistics(void)
{
}


/*
 * Destructor
 */
CGlyphStatistics::~CGlyphStatistics(void)
{
	for (map<CUniString, CGlyphStatisticsItem*>::iterator it = m_Items.begin(); it != m_Items.end(); it++)
	{
		delete (*it).second;
	}
}

/*
 * Adds an entry for one ground truth glyph
 */
void CGlyphStatistics::AddEntry(CGlyph * groundTruthGlyph, CGlyph * ocrResultGlyph)
{
	if (groundTruthGlyph == NULL)
		return;

	CGlyphStatisticsItem * statisticsItem = NULL;

	//Already in map?
	map<CUniString, CGlyphStatisticsItem*>::iterator it = m_Items.find(groundTruthGlyph->GetTextUnicode());
	if (it != m_Items.end()) //Yes -> Retrieve the item
		statisticsItem = (*it).second;
	else //No -> Create and add an item
	{
		statisticsItem = new CGlyphStatisticsItem(groundTruthGlyph);
		m_Items.insert(pair<CUniString, CGlyphStatisticsItem*>(groundTruthGlyph->GetTextUnicode(), statisticsItem));
	}

	//Update the statistics item
	statisticsItem->Update(ocrResultGlyph);
}

/*
 * Returns all statistics items (map with character as key and corresponding statistics as value)
 */
map<CUniString, CGlyphStatisticsItem*> * CGlyphStatistics::GetItems()
{
	return &m_Items;
}


/*
 * Class CGlyphStatisticsItem
 *
 * Represents one item of the glyph statistics
 */

/*
 * Constructor
 */
CGlyphStatisticsItem::CGlyphStatisticsItem(CGlyph * groundTruthGlyph)
{
	//m_GroundTruthGlyphId = groundTruthGlyph->GetId();
	m_GroundTruthCharacter = groundTruthGlyph->GetTextUnicode();
	m_Misses = 0;
	m_Matches = 0;
	m_Mismatches = 0;
	;
}

/*
 * Updates this item with the given glyph (increase relevant counts, add entries etc)
 */
void CGlyphStatisticsItem::Update(CGlyph * ocrResultGlyph)
{
	//No result glyph -> 'Miss' error
	if (ocrResultGlyph == NULL)
		m_Misses++;
	else
	{
		//Compare text content
		if (m_GroundTruthCharacter == ocrResultGlyph->GetTextUnicode()) //Match
		{
			m_Matches++;
		}
		else //Mismatch
		{
			m_Mismatches++;
			//Add to map
			map<CUniString, int>::iterator it = m_OcrErrors.find(ocrResultGlyph->GetTextUnicode());
			if (it == m_OcrErrors.end()) //Not found -> add new
			{
				m_OcrErrors.insert(pair<CUniString,int>(ocrResultGlyph->GetTextUnicode(), 1));
			}
			else //Already there -> Increase count
			{
				(*it).second = (*it).second + 1;
			}
		}
	}
}

} //end namespace