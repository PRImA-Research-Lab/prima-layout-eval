
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "stdafx.h"
#include "MetaData.h"

using namespace PRImA;
using namespace std;

/*
 * Class CMetaData
 *
 * Meta data for evaluation.
 *
 * CC 15.01.2010 - created
 */

CMetaData::CMetaData(void)
{
	//Parameters
	m_EvalId.Init("EvalID", "Evaluation ID (for constraints see the XML definition of attribute type 'ID')");
	m_Comments.Init("Comments", "");
	m_Creator.Init("Creator", "");
	m_SoftwareName.Init("Software name", "");
	m_SoftwareVersion.Init("Software version", "");
	m_SoftwareProducer.Init("Software producer", "");
	//Read only parameters:
	m_CreatedStr.Init("Created", "", CUniString(_T("?")));
	m_CreatedStr.SetReadOnly(true);
	m_LastChangeStr.Init("LastChange", "", CUniString(_T("?")));
	m_LastChangeStr.SetReadOnly(true);
	
	//Sorting
	m_EvalId.SetSortIndex(1);
	m_Creator.SetSortIndex(2);
	m_SoftwareName.SetSortIndex(3);
	m_SoftwareVersion.SetSortIndex(4);
	m_SoftwareProducer.SetSortIndex(5);
	m_Comments.SetSortIndex(6);
	m_CreatedStr.SetSortIndex(7);
	m_LastChangeStr.SetSortIndex(8);

	//Parameter Map
	m_Params.Clear();
	m_Params.Add(&m_Comments);
	m_Params.Add(&m_Creator);
	m_Params.Add(&m_SoftwareName);
	m_Params.Add(&m_SoftwareVersion);
	m_Params.Add(&m_SoftwareProducer);
	m_Params.Add(&m_CreatedStr);
	m_Params.Add(&m_LastChangeStr);
	m_Params.Add(&m_EvalId);
	m_Params.SetSchemaType(CUniString(_T("MetadataType")));

//	m_EvalId            = NULL;
	m_Created           = -1;
	m_LastChange        = -1;
}

CMetaData::~CMetaData(void)
{
}

CUniString CMetaData::GetEvalID()
{
	return m_EvalId.IsSet() ? m_EvalId.GetValue() : CUniString();
}

void CMetaData::SetCreated(time_t time) 
{ 
	m_Created = time; 

	//String value (for meta data dialog)
	struct tm * timeStruct = new struct tm;
	gmtime_s(timeStruct, &time);
	char * tempString = new char[20];
	strftime(tempString, 20, "%Y-%m-%dT%H:%M:%S", timeStruct);
	m_CreatedStr.SetValue(CUniString(tempString));
	delete [] tempString;
}

void CMetaData::SetEvalId(CUniString id)
{
	m_EvalId.SetValue(id);
}

void CMetaData::SetLastChange(time_t time) 
{ 
	m_LastChange = time; 

	//String value (for meta data dialog)
	struct tm * timeStruct = new struct tm;
	gmtime_s(timeStruct, &time);
	char * tempString = new char[20];
	strftime(tempString, 20, "%Y-%m-%dT%H:%M:%S", timeStruct);
	m_LastChangeStr.SetValue(CUniString(tempString));
	delete [] tempString;
}
