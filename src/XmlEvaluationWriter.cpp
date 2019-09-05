
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "stdafx.h"
#include "XmlEvaluationWriter.h"


using namespace PRImA;
using namespace std;

/*
 * Class CXmlEvaluationWriter
 *
 * Writer for evaluation profile and results.
 *
 * CC
 */

CXmlEvaluationWriter::CXmlEvaluationWriter(void)
{
	m_XmlValidator = NULL;
}

CXmlEvaluationWriter::CXmlEvaluationWriter(CValidator * validator)
{
	m_XmlValidator = validator;
}

CXmlEvaluationWriter::~CXmlEvaluationWriter(void)
{
}

/*xmlNsPtr CXmlEvaluationWriter::SetNameSpace(xmlNodePtr node)
{
	xmlNsPtr ns = xmlNewNs (node, 
							(const xmlChar *) "http://schema.primaresearch.org/EVALUATION/evaluation/2010-01-15", 
							NULL);
	node->ns = ns;
	return ns;
}*/

void CXmlEvaluationWriter::WriteLayoutEvaluation(CLayoutEvaluation * layoutEval, 
												 CMetaData * metaData, CUniString fileName)
{
	Write(layoutEval, NULL, metaData, fileName);
}

void CXmlEvaluationWriter::WriteEvaluationProfile(CEvaluationProfile * profile, 
												  CMetaData * metaData, CUniString fileName)
{
	Write(NULL, profile, metaData, fileName);
}

/*
 * Writes the evaluation objects to the specified XML file.
 */
void CXmlEvaluationWriter::Write(	CLayoutEvaluation * layoutEval, 
									CEvaluationProfile * profile, 
									CMetaData * metaData,
									CUniString fileName)
{
	char * tempString;

	CMsXmlNode * baseElement;

	CMsXmlWriter * xmlWriter = new CMsXmlWriter(fileName);

	IXMLDOMDocument2 * xmlDoc = xmlWriter->CreateXmlDoc();

	//Root node
	baseElement = xmlWriter->CreateRoot(CXmlEvaluationReader::ELEMENT_Eval,
							_T("http://schema.primaresearch.org/PAGE/eval/layout/2019-07-15"),
							_T("http://www.w3.org/2001/XMLSchema-instance"),
							_T("http://schema.primaresearch.org/PAGE/eval/layout/2019-07-15 http://schema.primaresearch.org/PAGE/eval/layout/2019-07-15/layouteval.xsd"));

	//EvalID
	if(!metaData->GetEvalID().IsEmpty())
		baseElement->AddAttribute(CXmlEvaluationReader::ATTR_evalId, metaData->GetEvalID());

	//*** MetaData ***
	CMsXmlNode * metadataNode;
	metadataNode = baseElement->AddChildNode(CXmlEvaluationReader::ELEMENT_Metadata);

	//Creator
	metadataNode->AddTextNode(CXmlEvaluationReader::ELEMENT_Creator, metaData->GetCreator());

	//Created
	if(metaData->GetCreated() == -1)
		metaData->SetCreated(time(NULL));

	if(metaData->GetCreated() == -1)
		tempString = NULL;
	else
	{
		time_t temp = metaData->GetCreated();
		struct tm * time = gmtime(&temp);
		tempString = new char[20];
		strftime(tempString, 20, "%Y-%m-%dT%H:%M:%S", time);
	}
	metadataNode->AddTextNode(CXmlEvaluationReader::ELEMENT_Created, CUniString(tempString));
	delete [] tempString;

	//Last Change
	metaData->SetLastChange(time(NULL));

	if(metaData->GetLastModified() == -1)
		tempString = NULL;
	else
	{
		time_t temp = metaData->GetLastModified();
		struct tm * time = gmtime(&temp);
		tempString = new char[20];
		strftime(tempString, 20, "%Y-%m-%dT%H:%M:%S", time);
	}
	metadataNode->AddTextNode(CXmlEvaluationReader::ELEMENT_LastChange, CUniString(tempString));
	delete [] tempString;

	//Comments
	if (metaData->IsCommentsSet())
		metadataNode->AddTextNode(CXmlEvaluationReader::ELEMENT_Comments, metaData->GetComments());

	//Software
	if (metaData->IsSoftwareNameSet() || metaData->IsSoftwareVersionSet() || metaData->IsSoftwareProducerSet())
	{
		CMsXmlNode * softwareNode = metadataNode->AddChildNode(CXmlEvaluationReader::ELEMENT_Software);
		softwareNode->AddAttribute(CXmlEvaluationReader::ATTR_name, metaData->GetSoftwareName());
		softwareNode->AddAttribute(CXmlEvaluationReader::ATTR_version, metaData->GetSoftwareVersion());
		if (metaData->IsSoftwareProducerSet() && !metaData->GetSoftwareProducer().IsEmpty())
			softwareNode->AddAttribute(CXmlEvaluationReader::ATTR_producer, metaData->GetSoftwareProducer());
	}

	//*** Profile ***
	if (profile != NULL)
	{
		CMsXmlNode * profileNode;
		profileNode = baseElement->AddChildNode(CXmlEvaluationReader::ELEMENT_Profile);
		WriteProfile(profile, profileNode);
	}

	//*** Evaluation Result ***
	if (layoutEval != NULL)
	{
		CMsXmlNode * result;
		result = baseElement->AddChildNode(CXmlEvaluationReader::ELEMENT_EvalData);

		//Ground-truth file name
		CUniString fileNameOnly = layoutEval->GetGroundTruthLocation();
		if (fileNameOnly.Find(_T("\\")) >= 0)
		{
			//Extract file name from path
			CString fullPath = layoutEval->GetGroundTruthLocation(); 
			CString pathOnly; 
			CExtraFileHelper::SplitPath(fullPath, pathOnly, fileNameOnly);
		}
		result->AddAttribute(CXmlEvaluationReader::ATTR_groundTruthFilename, fileNameOnly);

		//Segmentation result file name
		fileNameOnly = layoutEval->GetSegResultLocation();
		if (fileNameOnly.Find(_T("\\")) >= 0)
		{
			//Extract file name from path
			CString fullPath = layoutEval->GetSegResultLocation(); 
			CString pathOnly; 
			CExtraFileHelper::SplitPath(fullPath, pathOnly, fileNameOnly);
		}
		result->AddAttribute(CXmlEvaluationReader::ATTR_segmentationResultFilename, fileNameOnly);

		//Image file name
		fileNameOnly = layoutEval->GetBilevelImageLocation();
		if (fileNameOnly.Find(_T("\\")) >= 0)
		{
			//Extract file name from path
			CString fullPath = layoutEval->GetBilevelImageLocation(); 
			CString pathOnly; 
			CExtraFileHelper::SplitPath(fullPath, pathOnly, fileNameOnly);
		}
		result->AddAttribute(CXmlEvaluationReader::ATTR_imageFilename, fileNameOnly);

		//Width / height
		result->AddAttribute(CXmlEvaluationReader::ATTR_imageWidth, layoutEval->GetWidth());
		result->AddAttribute(CXmlEvaluationReader::ATTR_imageHeight, layoutEval->GetHeight());

		//Raw data and metrics
		WriteRawDataAndMetrics(layoutEval, result);
	}

	//Validate
	if (/*validate &&*/ m_XmlValidator != NULL)
	{
		int valRes = m_XmlValidator->validate(xmlDoc);
		if (valRes != 0)
		{
			fprintf(stderr, "XML schema not valid!\n");

			CUniString msg(_T("Invalid XML: "));
			msg.Append(m_XmlValidator->GetErrorMsg());

			delete xmlWriter;

			throw CXmlParseException(msg);
			return;
		}
	}

	// Save XML File
	xmlWriter->SaveDocument();
	delete xmlWriter;
}


/*
 * Writes the evaluation profile (weights, ...)
 */
void CXmlEvaluationWriter::WriteProfile(CEvaluationProfile * profile, CMsXmlNode * profileNode)
{
	if (profile == NULL)
		return;

	//Name
	CUniString name;
	if (!profile->GetName().IsEmpty())
		name = profile->GetName();
	else //Use file path
	{
		if (!profile->GetFilePath().IsEmpty())
		{
			CUniString path;
			CExtraFileHelper::SplitPath(profile->GetFilePath(), path, name);
		}
	}
	if (!name.IsEmpty())
		profileNode->AddAttribute(CXmlEvaluationReader::ATTR_name, name);

	//General settings (use pixel area, ...)
	CParameterMap * generalSettings = profile->GetGeneralSettings();
	if (generalSettings != NULL)
	{
		CMsXmlNode * generalSettingsNode;
		generalSettingsNode = profileNode->AddChildNode(CXmlEvaluationReader::ELEMENT_GeneralSettings);

		CMsXmlParameterWriter paramWriter(generalSettingsNode);
		paramWriter.AppendParameters(generalSettings);
	}

	//Error Type Weights
	map<int,CErrorTypeWeight*> * errorTypeWeights = profile->GetErrorTypeWeights();
	if (errorTypeWeights != NULL)
	{
		CMsXmlNode * errorTypeWeightsNode;
		errorTypeWeightsNode = profileNode->AddChildNode(CXmlEvaluationReader::ELEMENT_ErrorTypeWeights);
		map<int,CErrorTypeWeight*>::iterator it = errorTypeWeights->begin();
		while (it != errorTypeWeights->end())
		{
			WriteWeightNode((*it).second, errorTypeWeightsNode, NULL);
			it++;
		}
	}
	//Region Type Weights
	map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = profile->GetRegionTypeWeights();
	if (regionTypeWeights != NULL)
	{
		CMsXmlNode * regionTypeWeightsNode;
		regionTypeWeightsNode = profileNode->AddChildNode(CXmlEvaluationReader::ELEMENT_RegionTypeWeights);
		map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
		while (it != regionTypeWeights->end())
		{
			WriteWeightNode((*it).second, regionTypeWeightsNode, NULL);
			it++;
		}
	}
	//Reading Order Weight
	if (profile->GetReadingOrderWeightObject() != NULL)
	{
		WriteWeightNode(profile->GetReadingOrderWeightObject(), profileNode, NULL);
	}

	vector<int> errTypes;
	errTypes.push_back(CLayoutObjectEvaluationError::TYPE_MERGE);
	errTypes.push_back(CLayoutObjectEvaluationError::TYPE_SPLIT);
	errTypes.push_back(CLayoutObjectEvaluationError::TYPE_MISS);
	errTypes.push_back(CLayoutObjectEvaluationError::TYPE_PART_MISS);
	errTypes.push_back(CLayoutObjectEvaluationError::TYPE_INVENT);

	//Text Line weights
	CMsXmlNode * textLineWeightsNode;
	textLineWeightsNode = profileNode->AddChildNode(CXmlEvaluationReader::ELEMENT_TextLineWeights);
	for (unsigned int i=0; i<errTypes.size(); i++)
	{
		CWeight * weight = profile->GetErrorTypeWeightObjectForTextSubStructure(errTypes[i],
																				CLayoutObject::TYPE_TEXT_LINE);
		if (weight != NULL)
		{
			CMsXmlNode * weightNode = WriteWeightNode(weight, textLineWeightsNode, NULL);
			weightNode->AddAttribute(CXmlEvaluationReader::ATTR_type, ErrorTypeIntToString(errTypes[i]));
		}
	}

	//Word weights
	CMsXmlNode * wordWeightsNode;
	wordWeightsNode = profileNode->AddChildNode(CXmlEvaluationReader::ELEMENT_WordWeights);
	for (unsigned int i=0; i<errTypes.size(); i++)
	{
		CWeight * weight = profile->GetErrorTypeWeightObjectForTextSubStructure(errTypes[i],
																				CLayoutObject::TYPE_WORD);
		if (weight != NULL)
		{
			CMsXmlNode * weightNode = WriteWeightNode(weight, wordWeightsNode, NULL);
			weightNode->AddAttribute(CXmlEvaluationReader::ATTR_type, ErrorTypeIntToString(errTypes[i]));
		}
	}

	//Glyph weights
	CMsXmlNode * glyphWeightsNode;
	glyphWeightsNode = profileNode->AddChildNode(CXmlEvaluationReader::ELEMENT_GlyphWeights);
	for (unsigned int i=0; i<errTypes.size(); i++)
	{
		CWeight * weight = profile->GetErrorTypeWeightObjectForTextSubStructure(errTypes[i],
																				CLayoutObject::TYPE_GLYPH);
		if (weight != NULL)
		{
			CMsXmlNode * weightNode = WriteWeightNode(weight, glyphWeightsNode, NULL);
			weightNode->AddAttribute(CXmlEvaluationReader::ATTR_type, ErrorTypeIntToString(errTypes[i]));
		}
	}

	//Reading order group weights
	errTypes.push_back(CLayoutObjectEvaluationError::TYPE_MISCLASS);
	CMsXmlNode * groupWeightsNode;
	groupWeightsNode = profileNode->AddChildNode(CXmlEvaluationReader::ELEMENT_ReadingOrderGroupWeights);
	for (unsigned int i = 0; i<errTypes.size(); i++)
	{
		CWeight * weight = profile->GetErrorTypeWeightObjectForReadingOrderGroup(errTypes[i]);
		if (weight != NULL)
		{
			CMsXmlNode * weightNode = WriteWeightNode(weight, groupWeightsNode, NULL);
			weightNode->AddAttribute(CXmlEvaluationReader::ATTR_type, ErrorTypeIntToString(errTypes[i]));
		}
	}

	//Border weights
	CMsXmlNode * borderWeightsNode;
	borderWeightsNode = profileNode->AddChildNode(CXmlEvaluationReader::ELEMENT_BorderWeights);
	if (profile->GetIncludedBackgroundBorderWeightObject() != NULL)
		WriteWeightNode(profile->GetIncludedBackgroundBorderWeightObject(), borderWeightsNode, NULL, CXmlEvaluationReader::ELEMENT_IncludedBackgroundWeight);
	if (profile->GetExcludedForegroundBorderWeightObject() != NULL)
		WriteWeightNode(profile->GetExcludedForegroundBorderWeightObject(), borderWeightsNode, NULL, CXmlEvaluationReader::ELEMENT_ExcludedForegroundWeight);
	map<int,CLayoutObjectTypeWeight*> * missingRegionAreaWeights = profile->GetMissingRegionAreaBorderWeights();
	if (missingRegionAreaWeights != NULL)
	{
		CMsXmlNode * missingRegionAreaWeightsNode;
		missingRegionAreaWeightsNode = borderWeightsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_MissingRegionAreaWeights);
		map<int,CLayoutObjectTypeWeight*>::iterator it = missingRegionAreaWeights->begin();
		while (it != missingRegionAreaWeights->end())
		{
			WriteWeightNode((*it).second, missingRegionAreaWeightsNode, NULL);
			it++;
		}
	}

	//Reading order penalty matrix
	WriteReadingOrderPenalties(profile, profileNode);
}

/*
 * Writes the penalty matrix (if not default).
 */
void CXmlEvaluationWriter::WriteReadingOrderPenalties(CEvaluationProfile * profile, CMsXmlNode * profileNode)
{
	if (profile == NULL || profile->GetReadingOrderPenalties() == NULL
		|| profile->GetReadingOrderPenalties()->IsDefaultMatrix())
		return;

	CReadingOrderPenalties * matrix = profile->GetReadingOrderPenalties();
	CMsXmlNode * penaltiesNode;
	penaltiesNode = profileNode->AddChildNode(CXmlEvaluationReader::ELEMENT_ReadingOrderPenalties);

	for (int i=1; i<9; i++)
	{
		for (int j=1; j<9; j++)
		{
			CMsXmlNode * penaltyNode;
			penaltyNode = penaltiesNode->AddChildNode(CXmlEvaluationReader::ELEMENT_Penalty);
			penaltyNode->AddAttribute(CXmlEvaluationReader::ATTR_row, i);
			penaltyNode->AddAttribute(CXmlEvaluationReader::ATTR_col, j);
			penaltyNode->AddAttribute(CXmlEvaluationReader::ATTR_value, matrix->GetPenalty(i, j));
		}
	}
}

/*
 * Adds a single weight node to the specified parent.
 */
CMsXmlNode * CXmlEvaluationWriter::WriteWeightNode(CWeight * weight, CMsXmlNode * parentNode,
												   CWeightSetting * defaultSetting,
												   const wchar_t * elementName /*= NULL*/)
{
	//Leaf node?
	if (defaultSetting != NULL)
	{
		if (weight->GetParam()->GetType() != CParameter::TYPE_MULTI)
		{
			//Compare aginst default setting
			CWeightSetting currSetting(weight->GetValue(), weight->GetAllowableValue(), weight->IsAllowableWeightEnabled(), weight->IsUseAllowableWeight());

			if (currSetting == (*defaultSetting))
				return NULL; //Nothing to write
		}
		else //It's a parent node
		{
			//Compare all child nodes against the default settings
			if (CompareAllchildWeightsAgainstSetting(weight, defaultSetting))
				return NULL;
		}
	}
	bool deleteDefaultSetting = false;

	CMsXmlNode * weightNode = NULL;
	if (elementName == NULL)
	{
		if (typeid(*weight) == typeid(CErrorTypeWeight))
			weightNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_ErrorTypeWeight);
		else if (typeid(*weight) == typeid(CLayoutObjectTypeWeight))
			weightNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_RegionTypeWeight);
		else if (typeid(*weight) == typeid(CSubTypeWeight))
			weightNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_SubTypeWeight);
		else if (typeid(*weight) == typeid(CReadingOrderWeight))
			weightNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_ReadingOrderWeight);
		else if (typeid(*weight) == typeid(CTextSubStructureWeight))
			weightNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_ErrorTypeWeight);
		else
		{
			//TODO ?
		}
	}
	else
	{
		weightNode = parentNode->AddChildNode(elementName);
	}

	//Name
	weightNode->AddAttribute(CXmlEvaluationReader::ATTR_name, weight->GetName());

	//Description
	if (!weight->GetDescription().IsEmpty())
		weightNode->AddTextNode(CXmlEvaluationReader::ELEMENT_Description, weight->GetDescription());

	//Value (allowable / non allowable) and 'use allowable' flag
	if (weight->GetParam()->GetType() != CParameter::TYPE_MULTI) //Child weight
	{
		//Value (non-allowable)
		if (weight->GetValue() >= 0.0)
			weightNode->AddAttribute(CXmlEvaluationReader::ATTR_weight, weight->GetValue());
		//Allowable value
		if (weight->IsAllowableWeightEnabled() && weight->GetAllowableValue() >= 0.0)
			weightNode->AddAttribute(CXmlEvaluationReader::ATTR_allowableWeight, weight->GetAllowableValue());
		//'Use allowable' flag
		if (weight->IsAllowableWeightEnabled())
			weightNode->AddAttribute(CXmlEvaluationReader::ATTR_useAllowable, weight->IsUseAllowableWeight());
	}
	else //It's a parent weight
	{
		if (defaultSetting == NULL)
		{
			//Find the most common weight setting (value, allowable value, use allowable) 
			//from the child weights and use it as default
			defaultSetting = FindMostCommonSetting(weight);
			deleteDefaultSetting = true;

			if (defaultSetting != NULL)
			{
				//Value (non-allowable)
				if (defaultSetting->GetValue() >= 0.0)
					weightNode->AddAttribute(CXmlEvaluationReader::ATTR_weight, defaultSetting->GetValue());
				//Allowable value
				if (weight->IsAllowableWeightEnabled() && defaultSetting->GetAllowableValue() >= 0.0)
					weightNode->AddAttribute(CXmlEvaluationReader::ATTR_allowableWeight, defaultSetting->GetAllowableValue());
				//'Use allowable' flag
				if (weight->IsAllowableWeightEnabled())
					weightNode->AddAttribute(CXmlEvaluationReader::ATTR_useAllowable, defaultSetting->IsUseAllowableWeight());
			}
		}
	}

	//'Enable allowable' flag
	weightNode->AddAttribute(CXmlEvaluationReader::ATTR_enableAllowable, weight->IsAllowableWeightEnabled());

	//Error Type Weight specific things
	if (typeid(*weight) == typeid(CErrorTypeWeight))
	{
		CErrorTypeWeight * errTypeWeight = (CErrorTypeWeight*)weight;

		//Error type
		weightNode->AddAttribute(CXmlEvaluationReader::ATTR_type, ErrorTypeIntToString(errTypeWeight->GetType()));
		//Child Weights (Region Type Weights)
		map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = errTypeWeight->GetRegionTypeWeights();
		if (regionTypeWeights != NULL)
		{
			map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
			while (it != regionTypeWeights->end())
			{
				WriteWeightNode((*it).second, weightNode, defaultSetting);
				it++;
			}
		}
	}
	//Region Type Weight specific things
	else if (typeid(*weight) == typeid(CLayoutObjectTypeWeight))
	{
		CLayoutObjectTypeWeight * regionTypeWeight = (CLayoutObjectTypeWeight*)weight;

		//Region type
		weightNode->AddAttribute(CXmlEvaluationReader::ATTR_type, RegionTypeIntToString(regionTypeWeight->GetType()));

		//Child Weights (Region Type Weights)
		map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = regionTypeWeight->GetRegionTypeWeights();
		if (regionTypeWeights != NULL)
		{
			map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
			while (it != regionTypeWeights->end())
			{
				WriteWeightNode((*it).second, weightNode, defaultSetting);
				it++;
			}
		}
		//Child Weights (Sub Type Weights)
		map<CUniString,CSubTypeWeight*> * subTypeWeights = regionTypeWeight->GetSubTypeWeights();
		if (subTypeWeights != NULL)
		{
			map<CUniString,CSubTypeWeight*>::iterator it = subTypeWeights->begin();
			while (it != subTypeWeights->end())
			{
				WriteWeightNode((*it).second, weightNode, defaultSetting);
				it++;
			}
		}
	}
	//Sub Type Weight specific things
	else if (typeid(*weight) == typeid(CSubTypeWeight))
	{
		CSubTypeWeight * subTypeWeight = (CSubTypeWeight*)weight;

		//Subtype
		weightNode->AddAttribute(CXmlEvaluationReader::ATTR_subtype, subTypeWeight->GetType());
		//Child Weights (Region Type Weights)
		map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = subTypeWeight->GetRegionTypeWeights();
		if (regionTypeWeights != NULL)
		{
			map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
			while (it != regionTypeWeights->end())
			{
				WriteWeightNode((*it).second, weightNode, defaultSetting);
				it++;
			}
		}
	}

	if (deleteDefaultSetting)
		delete defaultSetting;

	return weightNode;
}

/*
 * Recursively looks for the most common setting of
 *   - Weight value
 *   - Allowable weight value
 *   - Enable allowable
 *   - Use allowable
 */
CWeightSetting * CXmlEvaluationWriter::FindMostCommonSetting(CWeight * weight)
{
	vector<CWeightSetting> weightSettings;

	//Recursion
	FindMostCommonSetting(weight, &weightSettings);

	//Find setting with highest count
	int maxCount = 0;
	int maxCountIndex = -1;
	for (unsigned int i=0; i<weightSettings.size(); i++)
	{
		if (weightSettings[i].GetCount() > maxCount)
		{
			maxCount = weightSettings[i].GetCount();
			maxCountIndex = i;
		}
	}

	if (maxCount >= 2)
	{
		return new CWeightSetting(&(weightSettings[maxCountIndex]));
	}
	return NULL;
}

/*
 * Recursively looks for the most common setting of
 *   - Weight value
 *   - Allowable weight value
 *   - Enable allowable
 *   - Use allowable
 */
void CXmlEvaluationWriter::FindMostCommonSetting(CWeight * weight, vector<CWeightSetting> * weightSettings)
{
	//Parent?
	if (weight->GetParam()->GetType() == CParameter::TYPE_MULTI)
	{
		//Error Type Weight specific things
		if (typeid(*weight) == typeid(CErrorTypeWeight))
		{
			CErrorTypeWeight * errTypeWeight = (CErrorTypeWeight*)weight;
			//Child Weights (Region Type Weights)
			map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = errTypeWeight->GetRegionTypeWeights();
			if (regionTypeWeights != NULL)
			{
				map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
				while (it != regionTypeWeights->end())
				{
					FindMostCommonSetting((*it).second, weightSettings);
					it++;
				}
			}
		}
		//Region Type Weight specific things
		else if (typeid(*weight) == typeid(CLayoutObjectTypeWeight))
		{
			CLayoutObjectTypeWeight * regionTypeWeight = (CLayoutObjectTypeWeight*)weight;
			//Child Weights (Region Type Weights)
			map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = regionTypeWeight->GetRegionTypeWeights();
			if (regionTypeWeights != NULL)
			{
				map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
				while (it != regionTypeWeights->end())
				{
					FindMostCommonSetting((*it).second, weightSettings);
					it++;
				}
			}
			//Child Weights (Sub Type Weights)
			map<CUniString,CSubTypeWeight*> * subTypeWeights = regionTypeWeight->GetSubTypeWeights();
			if (subTypeWeights != NULL)
			{
				map<CUniString,CSubTypeWeight*>::iterator it = subTypeWeights->begin();
				while (it != subTypeWeights->end())
				{
					FindMostCommonSetting((*it).second, weightSettings);
					it++;
				}
			}
		}
		//Sub Type Weight specific things
		else if (typeid(*weight) == typeid(CSubTypeWeight))
		{
			CSubTypeWeight * subTypeWeight = (CSubTypeWeight*)weight;
			//Child Weights (Region Type Weights)
			map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = subTypeWeight->GetRegionTypeWeights();
			if (regionTypeWeights != NULL)
			{
				map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
				while (it != regionTypeWeights->end())
				{
					FindMostCommonSetting((*it).second, weightSettings);
					it++;
				}
			}
		}

	}
	else //Child
	{
		CWeightSetting childSetting(weight->GetValue(), weight->GetAllowableValue(), weight->IsAllowableWeightEnabled(), weight->IsUseAllowableWeight());

		//Check if the same setting exists already and if yes, increse the count
		bool found = false;
		for (unsigned int i=0; i<weightSettings->size(); i++)
		{
			if (weightSettings->at(i) == childSetting)
			{
				weightSettings->at(i).IncrementCount();
				found = true;
				break;
			}
		}
		//Add if not there yet
		if (!found)
			weightSettings->push_back(childSetting);
	}
}

/*
 * Recursively checks if all child weights are equal to the given weight setting.
 * Returns true, if all children are equal to the setting, otherwise false.
 */
bool CXmlEvaluationWriter::CompareAllchildWeightsAgainstSetting(CWeight * weight, CWeightSetting * weightSetting)
{
	//Child?
	if (weight->GetParam()->GetType() != CParameter::TYPE_MULTI)
	{
		CWeightSetting currSetting(weight->GetValue(), weight->GetAllowableValue(), weight->IsAllowableWeightEnabled(), weight->IsUseAllowableWeight());
		return currSetting == (*weightSetting);
	}
	else //Parent
	{
		//Error Type Weight specific things
		if (typeid(*weight) == typeid(CErrorTypeWeight))
		{
			CErrorTypeWeight * errTypeWeight = (CErrorTypeWeight*)weight;
			//Child Weights (Region Type Weights)
			map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = errTypeWeight->GetRegionTypeWeights();
			if (regionTypeWeights != NULL)
			{
				map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
				while (it != regionTypeWeights->end())
				{
					if (!CompareAllchildWeightsAgainstSetting((*it).second, weightSetting))
						return false;
					it++;
				}
			}
		}
		//Region Type Weight specific things
		else if (typeid(*weight) == typeid(CLayoutObjectTypeWeight))
		{
			CLayoutObjectTypeWeight * regionTypeWeight = (CLayoutObjectTypeWeight*)weight;
			//Child Weights (Region Type Weights)
			map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = regionTypeWeight->GetRegionTypeWeights();
			if (regionTypeWeights != NULL)
			{
				map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
				while (it != regionTypeWeights->end())
				{
					if (!CompareAllchildWeightsAgainstSetting((*it).second, weightSetting))
						return false;
					it++;
				}
			}
			//Child Weights (Sub Type Weights)
			map<CUniString,CSubTypeWeight*> * subTypeWeights = regionTypeWeight->GetSubTypeWeights();
			if (subTypeWeights != NULL)
			{
				map<CUniString,CSubTypeWeight*>::iterator it = subTypeWeights->begin();
				while (it != subTypeWeights->end())
				{
					if (!CompareAllchildWeightsAgainstSetting((*it).second, weightSetting))
						return false;
					it++;
				}
			}
		}
		//Sub Type Weight specific things
		else if (typeid(*weight) == typeid(CSubTypeWeight))
		{
			CSubTypeWeight * subTypeWeight = (CSubTypeWeight*)weight;
			//Child Weights (Region Type Weights)
			map<int,CLayoutObjectTypeWeight*> * regionTypeWeights = subTypeWeight->GetRegionTypeWeights();
			if (regionTypeWeights != NULL)
			{
				map<int,CLayoutObjectTypeWeight*>::iterator it = regionTypeWeights->begin();
				while (it != regionTypeWeights->end())
				{
					if (!CompareAllchildWeightsAgainstSetting((*it).second, weightSetting))
						return false;
					it++;
				}
			}
		}
	}

	return true;
}

/*
 * Writes raw data (merge, split, ... errors) and metrics (success / error rates, ...)
 */
void CXmlEvaluationWriter::WriteRawDataAndMetrics(CLayoutEvaluation * layoutEval, CMsXmlNode * parentNode)
{
	//Regions, text lines, words, glyphs, groups
	vector<int> regionTypes;
	regionTypes.push_back(CLayoutObject::TYPE_LAYOUT_REGION);
	regionTypes.push_back(CLayoutObject::TYPE_TEXT_LINE);
	regionTypes.push_back(CLayoutObject::TYPE_WORD);
	regionTypes.push_back(CLayoutObject::TYPE_GLYPH);
	regionTypes.push_back(CLayoutObject::TYPE_READING_ORDER_GROUP);

	for (unsigned int i=0; i<regionTypes.size(); i++)
	{
		CEvaluationResults * results = layoutEval->GetResults(regionTypes[i]);
		if (results == NULL)
			continue;

		CMsXmlNode * resultsNode;
		resultsNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_PageObjectResults);
		resultsNode->AddAttribute(CXmlEvaluationReader::ATTR_type, PageObjectLevelIntToString(results->GetLayoutObjectType()));

		CMsXmlNode * rawDataNode;
		rawDataNode = resultsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_RawData);
		WriteRawData(results, rawDataNode);

		//Metrics and statistics
		WriteMetricResults(results, resultsNode);
	}

	//Border
	CEvaluationResults * results = layoutEval->GetResults(CLayoutObject::TYPE_BORDER);
	if (results != NULL)
	{
		CBorderEvaluationMetrics * metrics = (CBorderEvaluationMetrics*)results->GetMetrics();
		if (metrics != NULL)
		{
			CMsXmlNode * resultsNode;
			resultsNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_BorderResults);

			resultsNode->AddAttribute(CXmlEvaluationReader::ATTR_includedBackgroundSuccessRate, metrics->GetIncludedBackgroundSuccessRate());
			resultsNode->AddAttribute(CXmlEvaluationReader::ATTR_excludedForegroundSuccessRate, metrics->GetExcludedForegroundSuccessRate());
			resultsNode->AddAttribute(CXmlEvaluationReader::ATTR_missingRegionAreaSuccessRate, metrics->GetMissingRegionAreaSuccessRate());
			resultsNode->AddAttribute(CXmlEvaluationReader::ATTR_overallSuccessRate, metrics->GetOverallSuccessRate());
		}
	}
}

/*
 * Writes success rates, error rates, figures, numbers, ...
 */
void CXmlEvaluationWriter::WriteMetricResults(CEvaluationResults * results, CMsXmlNode * resultsNode)
{
	//For all types
	CLayoutObjectEvaluationMetrics * metricResult = (CLayoutObjectEvaluationMetrics*)results->GetMetrics();
	if (metricResult != NULL)
	{
		CMsXmlNode * metricsNode;
		metricsNode = resultsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_Metrics);
		WriteMetricResult(metricResult, metricsNode);
	}
	//Per region type
	map<int,CLayoutObjectEvaluationMetrics *> * met = results->GetMetricsPerType();
	map<int,CLayoutObjectEvaluationMetrics *>::iterator it = met->begin();
	while (it != met->end())
	{
		metricResult = (*it).second;
		if (metricResult != NULL)
		{
			CMsXmlNode * metricsNode;
			metricsNode = resultsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_Metrics);
			WriteMetricResult(metricResult, metricsNode);
		}
		it++;
	}
}

/*
 * Writes success rates, error rates, figures, numbers, ... for a region type
 */
void CXmlEvaluationWriter::WriteMetricResult(CLayoutObjectEvaluationMetrics * metricResult, 
											 CMsXmlNode * metricsNode)
{
	//Region type (if defined)
	int regionType = metricResult->GetRegionType();
	if (regionType == CLayoutRegion::TYPE_ALL)
		metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_type, CUniString(L"all")); 
	else
		metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_type, RegionTypeIntToString(regionType));

	//Attributes
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_numberOfGroundTruthRegions			,
							metricResult->GetNumberOfGroundTruthRegions());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_numberOfSegResultRegions			,
							metricResult->GetNumberOfSegResultRegions());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_imageArea							,
							metricResult->GetImageArea());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_foregroundPixelCount				,
							metricResult->GetImageForegroundPixelCount());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_overallGroundTruthRegionArea		,
							metricResult->GetOverallGroundTruthRegionArea());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_overallGroundTruthRegionPixelCount	,
							metricResult->GetOverallGroundTruthRegionPixelCount());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_overallSegResultRegionArea			,
							metricResult->GetOverallSegResultRegionArea());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_overallSegResultRegionPixelCount	,
							metricResult->GetOverallSegResultRegionPixelCount());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_overallWeightedAreaError			,
							metricResult->GetOverallWeightedAreaError());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_overallWeightedCountError			,
							metricResult->GetOverallWeightedCountError());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_readingOrderError					,
							metricResult->GetReadingOrderError());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_readingOrderSuccessRate			,
							metricResult->GetReadingOrderSuccessRate());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_overallWeightedAreaSuccessRate		,
							metricResult->GetOverallWeightedAreaSuccessRate());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_overallWeightedCountSuccessRate	,
							metricResult->GetOverallWeightedCountSuccessRate());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_harmonicWeightedAreaSuccessRate	,
							metricResult->GetHarmonicWeightedAreaSuccessRate());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_harmonicWeightedCountSuccessRate	,
							metricResult->GetHarmonicWeightedCountSuccessRate());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_errorRateBasedOnSimpleCount		,
							metricResult->GetErrorRateBasedOnCount());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_recallNonStrict					,
							metricResult->GetRecall(false));
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_recallStrict						,
							metricResult->GetRecall(true));
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_precisionNonStrict					,
							metricResult->GetPrecision(false));
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_precisionStrict					,
							metricResult->GetPrecision(true));
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_fMeasureNonStrict					,
							metricResult->GetFMeasure(false));
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_fMeasureStrict						,
							metricResult->GetFMeasure(true));
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_regionCountDeviation				,
							metricResult->GetRegionCountDeviation());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_relativeRegionCountDeviation		,
							metricResult->GetRelativeRegionCountDeviation());
	metricsNode->AddAttribute(CXmlEvaluationReader::ATTR_OCRSuccessRate						,
							metricResult->GetOCRSuccessRate());

	//Elements
	WriteIntPerRegionTypeNode(metricResult->GetNumberOfGroundTruthRegionsPerType(), 
						CXmlEvaluationReader::ELEMENT_NumberOfGroundTruthRegions, 
						metricsNode);
	WriteIntPerRegionTypeNode(metricResult->GetNumberOfSegResultRegionsPerType(), 
						CXmlEvaluationReader::ELEMENT_NumberOfSegResultRegions, 
						metricsNode);
	WriteIntPerRegionTypeNode(metricResult->GetOverallGroundTruthRegionAreaPerType(), 
						CXmlEvaluationReader::ELEMENT_GroundTruthRegionArea, 
						metricsNode);
	WriteIntPerRegionTypeNode(metricResult->GetOverallGroundTruthRegionPixelCountPerType(), 
						CXmlEvaluationReader::ELEMENT_GroundTruthRegionPixelCount, 
						metricsNode);
	WriteIntPerRegionTypeNode(metricResult->GetOverallSegResultRegionAreaPerType(), 
						CXmlEvaluationReader::ELEMENT_SegResultRegionArea, 
						metricsNode);
	WriteIntPerRegionTypeNode(metricResult->GetOverallSegResultRegionPixelCountPerType(), 
						CXmlEvaluationReader::ELEMENT_SegResultRegionPixelCount, 
						metricsNode);
	WriteIntPerRegionTypeNode(metricResult->GetRecallAreaPerType(), 
						CXmlEvaluationReader::ELEMENT_RecallArea, 
						metricsNode);
	WriteIntPerRegionTypeNode(metricResult->GetRecallPixelCountPerType(), 
						CXmlEvaluationReader::ELEMENT_RecallPixelCount, 
						metricsNode);
	WriteDoublePerErrorTypeNode(metricResult->GetOverallWeightedAreaErrorPerErrorType(), 
						CXmlEvaluationReader::ELEMENT_OverallWeightedAreaErrorPerErrorType, 
						metricsNode);
	WriteDoublePerErrorTypeNode(metricResult->GetInfluenceOverallWeightedAreaErrorPerErrorType(), 
						CXmlEvaluationReader::ELEMENT_InfluenceWeightedAreaErrorPerErrorType, 
						metricsNode);
	WriteDoublePerRegionTypeNode(metricResult->GetOverallWeightedAreaErrorPerRegionType(), 
						CXmlEvaluationReader::ELEMENT_OverallWeightedAreaErrorPerRegionType, 
						metricsNode);
	WriteDoublePerErrorTypeNode(metricResult->GetWeightedAreaSuccessRatePerType(), 
						CXmlEvaluationReader::ELEMENT_WeightedAreaSuccessRate, 
						metricsNode);
	WriteDoublePerErrorTypeNode(metricResult->GetOverallWeightedCountErrorPerErrorType(), 
						CXmlEvaluationReader::ELEMENT_OverallWeightedCountErrorPerErrorType, 
						metricsNode);
	WriteDoublePerErrorTypeNode(metricResult->GetInfluenceOverallWeightedCountErrorPerErrorType(), 
						CXmlEvaluationReader::ELEMENT_InfluenceWeightedCountErrorPerErrorType, 
						metricsNode);
	WriteDoublePerRegionTypeNode(metricResult->GetOverallWeightedCountErrorPerRegionType(), 
						CXmlEvaluationReader::ELEMENT_OverallWeightedCountErrorPerRegionType, 
						metricsNode);
	WriteDoublePerErrorTypeNode(metricResult->GetWeightedCountSuccessRatePerType(), 
						CXmlEvaluationReader::ELEMENT_WeightedCountSuccessRate, 
						metricsNode);
	WriteDoublePerErrorTypeNode(metricResult->GetErrorRatePerTypeBasedOnCount(), 
						CXmlEvaluationReader::ELEMENT_ErrorRateBasedOnSimpleCount, 
						metricsNode);
	WriteDoublePerRegionTypeNode(metricResult->GetRecallPerType(), 
						CXmlEvaluationReader::ELEMENT_Recall, 
						metricsNode);
	WriteDoublePerRegionTypeNode(metricResult->GetPrecisionPerType(), 
						CXmlEvaluationReader::ELEMENT_Precision, 
						metricsNode);
}

/*
 * Writes map entries for a map [type, int value]
 */
void CXmlEvaluationWriter::WriteIntPerRegionTypeNode(map<int, int> * values, const wchar_t * elementName,
											 CMsXmlNode * parentNode)
{
	map<int, int>::iterator it = values->begin();
	while (it != values->end())
	{
		CMsXmlNode * node;
		node = parentNode->AddChildNode(elementName);
		node->AddAttribute(CXmlEvaluationReader::ATTR_type, RegionTypeIntToString((*it).first));
		node->AddAttribute(CXmlEvaluationReader::ATTR_value, (*it).second);
		it++;
	}
}

/*
 * Writes map entries for a map [type, double value]
 */
void CXmlEvaluationWriter::WriteDoublePerRegionTypeNode(map<int, double> * values, const wchar_t * elementName,
											 CMsXmlNode * parentNode)
{
	map<int, double>::iterator it = values->begin();
	while (it != values->end())
	{
		CMsXmlNode * node;
		node = parentNode->AddChildNode(elementName);
		node->AddAttribute(CXmlEvaluationReader::ATTR_type, RegionTypeIntToString((*it).first));
		node->AddAttribute(CXmlEvaluationReader::ATTR_value, (*it).second);
		it++;
	}
}

/*
 * Writes map entries for a map [type, int value]
 */
void CXmlEvaluationWriter::WriteIntPerErrorTypeNode(map<int, int> * values, const wchar_t * elementName,
											 CMsXmlNode * parentNode)
{
	map<int, int>::iterator it = values->begin();
	while (it != values->end())
	{
		CMsXmlNode * node;
		node = parentNode->AddChildNode(elementName);
		node->AddAttribute(CXmlEvaluationReader::ATTR_type, ErrorTypeIntToString((*it).first));
		node->AddAttribute(CXmlEvaluationReader::ATTR_value, (*it).second);
		it++;
	}
}

/*
 * Writes map entries for a map [type, double value]
 */
void CXmlEvaluationWriter::WriteDoublePerErrorTypeNode(map<int, double> * values, const wchar_t * elementName,
											 CMsXmlNode * parentNode)
{
	map<int, double>::iterator it = values->begin();
	while (it != values->end())
	{
		CMsXmlNode * node;
		node = parentNode->AddChildNode(elementName);
		node->AddAttribute(CXmlEvaluationReader::ATTR_type, ErrorTypeIntToString((*it).first));
		node->AddAttribute(CXmlEvaluationReader::ATTR_value, (*it).second);
		it++;
	}
}

/*
 * Writes overlap lookup tables, region errors (merge, ...) and reading order errors.
 */
void CXmlEvaluationWriter::WriteRawData(CEvaluationResults * results, CMsXmlNode * parentNode)
{
	//Ground-truth overlaps
	map<CUniString, set<CUniString>*> * overlaps = results->GetGroundTruthOverlaps();
	map<CUniString, set<CUniString>*>::iterator itReg1 = overlaps->begin();
	while (itReg1 != overlaps->end())
	{
		CMsXmlNode * overlapNode;
		overlapNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_GroundTruthOverlap);
		WriteOverlapEntries((*itReg1).first, (*itReg1).second, overlapNode);
		itReg1++;
	}

	//Seg result overlaps
	overlaps = results->GetSegResultOverlaps();
	itReg1 = overlaps->begin();
	while (itReg1 != overlaps->end())
	{
		CMsXmlNode * overlapNode;
		overlapNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_SegResultOverlap);
		WriteOverlapEntries((*itReg1).first, (*itReg1).second, overlapNode);
		itReg1++;
	}

	//Region results (merge, split, ...)
	map<CUniString, CLayoutObjectEvaluationResult*> * regionResults = results->GetGroundTruthObjectResults();
	map<CUniString, CLayoutObjectEvaluationResult*>::iterator itRegRes = regionResults->begin();
	while (itRegRes != regionResults->end())
	{
		CMsXmlNode * regionResultsNode;
		regionResultsNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_RegionResults);
		WriteRegionResults((*itRegRes).first, (*itRegRes).second, regionResultsNode);
		itRegRes++;
	}
	// False detection
	regionResults = results->GetSegResultObjectResults();
	itRegRes = regionResults->begin();
	while (itRegRes != regionResults->end())
	{
		CMsXmlNode * regionResultsNode;
		regionResultsNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_RegionResults);
		WriteRegionResults((*itRegRes).first, (*itRegRes).second, regionResultsNode);
		itRegRes++;
	}

	//Reading Order result
	CReadingOrderEvaluationResult * readingOrderResults = results->GetReadingOrderResults();
	if (readingOrderResults != NULL)
	{
		CMsXmlNode * readingOrderResultsNode;
		readingOrderResultsNode = parentNode->AddChildNode(CXmlEvaluationReader::ELEMENT_ReadingOrderResults);

		for (int i=0; i<readingOrderResults->GetErrorCount(); i++)
		{
			CReadingOrderError * error = readingOrderResults->GetError(i);

			CMsXmlNode * readingOrderErrorNode;
			readingOrderErrorNode = readingOrderResultsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_ReadingOrderError);

			WriteReadingOrderError(error, readingOrderErrorNode);
		}
	}
}

/*
 * Writes a single reading order error (for a region pair)
 */
void CXmlEvaluationWriter::WriteReadingOrderError(CReadingOrderError * error, 
												  CMsXmlNode * readingOrderErrorNode)
{
	//Seg result region IDs
	readingOrderErrorNode->AddAttribute(CXmlEvaluationReader::ATTR_regionId1, error->GetRegion1());
	readingOrderErrorNode->AddAttribute(CXmlEvaluationReader::ATTR_regionId2, error->GetRegion2());
	//Penalty
	readingOrderErrorNode->AddAttribute(CXmlEvaluationReader::ATTR_penalty, error->GetPenalty());

	//Seg result relation
	CMsXmlNode * segResultRelNode;
	segResultRelNode = readingOrderErrorNode->AddChildNode(CXmlEvaluationReader::ELEMENT_SegResultRelation);
	WriteRelation(error->GetSegResultRelation(), segResultRelNode);

	//Fuzzy ground-truth relations
	vector<CFuzzyReadingOrderRelation> * groundTruthRelations = error->GetGroundTruthRelations();
	for (unsigned int i=0; i<groundTruthRelations->size(); i++)
	{
		CFuzzyReadingOrderRelation fuzzyRel = groundTruthRelations->at(i);
		CMsXmlNode * groundTruthRelNode;
		groundTruthRelNode = readingOrderErrorNode->AddChildNode(CXmlEvaluationReader::ELEMENT_GroundTruthRelation);
		//Penalty
		groundTruthRelNode->AddAttribute(CXmlEvaluationReader::ATTR_penalty, fuzzyRel.GetPenalty());
		//Weight
		groundTruthRelNode->AddAttribute(CXmlEvaluationReader::ATTR_weight, fuzzyRel.GetWeight());
		//Ground-truth region IDs
		groundTruthRelNode->AddAttribute(CXmlEvaluationReader::ATTR_regionId1, fuzzyRel.GetRegion1());
		groundTruthRelNode->AddAttribute(CXmlEvaluationReader::ATTR_regionId2, fuzzyRel.GetRegion2());
		//Ground-truth base relation used for the error
		groundTruthRelNode->AddAttribute(CXmlEvaluationReader::ATTR_groundTruthRelation, fuzzyRel.GetCausingGroundTruthRelation());
		//Seg result base relation used for the error
		groundTruthRelNode->AddAttribute(CXmlEvaluationReader::ATTR_segResultRelation, fuzzyRel.GetCausingSegResultRelation());
		//Relation set
		WriteRelation(&fuzzyRel.GetRelation(), groundTruthRelNode);
	}
}

/*
 * Writes a reading order relation set
 */
void CXmlEvaluationWriter::WriteRelation(set<int> * relation, CMsXmlNode * node)
{
	if (relation != NULL)
	{
		set<int>::iterator it = relation->begin();
		while (it != relation->end())
		{
			CMsXmlNode * baseRelationNode;
			baseRelationNode = node->AddChildNode(CXmlEvaluationReader::ELEMENT_Relation);
			//Type
			baseRelationNode->AddAttribute(CXmlEvaluationReader::ATTR_type, ReadingOrderRelationTypeIntToString((*it)));
			it++;
		}
	}
}

/*
 * Writes the overlapping regions of a specified region.
 * region1 can be either a ground-truth region or a segmentation result region.
 * regions2 is then a list of overlappint regions from the opposite document layout.
 */
void CXmlEvaluationWriter::WriteOverlapEntries(CUniString region1, set<CUniString> * regions2, 
											   CMsXmlNode * overlapNode)
{
	//ID of region 1
	overlapNode->AddAttribute(CXmlEvaluationReader::ATTR_regionId, region1);

	if (regions2 != NULL)
	{
		set<CUniString>::iterator itReg2 = regions2->begin();
		while (itReg2 != regions2->end())
		{
			CMsXmlNode * overlapRegionNode;
			overlapRegionNode = overlapNode->AddChildNode(CXmlEvaluationReader::ELEMENT_OverlapsRegion);
			//ID of region 2
			overlapRegionNode->AddAttribute(CXmlEvaluationReader::ATTR_id, (*itReg2));
			itReg2++;
		}
	}
}

/*
 * Writes merge, split, ... of a single region
 */
void CXmlEvaluationWriter::WriteRegionResults(CUniString region, 
											  CLayoutObjectEvaluationResult * results, 
											  CMsXmlNode * resultsNode)
{
	//Region ID
	resultsNode->AddAttribute(CXmlEvaluationReader::ATTR_regionId, region);

	//Errors
	map<int, CLayoutObjectEvaluationError*> * errors = results->GetErrors();
	map<int, CLayoutObjectEvaluationError*>::iterator it = errors->begin();
	while (it != errors->end())
	{
		CLayoutObjectEvaluationError * error = (*it).second;
		CMsXmlNode * errorNode;
		if (error->GetType() == CLayoutObjectEvaluationError::TYPE_MERGE)
			errorNode = resultsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_RegionErrorMerge);
		else if (error->GetType() == CLayoutObjectEvaluationError::TYPE_SPLIT)
			errorNode = resultsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_RegionErrorSplit);
		else if (error->GetType() == CLayoutObjectEvaluationError::TYPE_MISCLASS)
			errorNode = resultsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_RegionErrorMisclass);
		else
			errorNode = resultsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_RegionError);
		WriteRegionError(error, errorNode);
		it++;
	}
}

/*
 * Region Evaluation Error (merge, split, ...)
 */
void CXmlEvaluationWriter::WriteRegionError(CLayoutObjectEvaluationError * error, 
											CMsXmlNode * errorNode)
{
	//Type
	errorNode->AddAttribute(CXmlEvaluationReader::ATTR_type, ErrorTypeIntToString(error->GetType()));

	//Weighted area error
	errorNode->AddAttribute(CXmlEvaluationReader::ATTR_weightedAreaError, error->GetWeightedAreaError());
	//Weighted count error
	errorNode->AddAttribute(CXmlEvaluationReader::ATTR_weightedCountError, error->GetWeightedCountError());
	//Area
	errorNode->AddAttribute(CXmlEvaluationReader::ATTR_area, (int)error->GetArea());
	//foregroundPixelCount
	errorNode->AddAttribute(CXmlEvaluationReader::ATTR_foregroundPixelCount, (int)error->GetPixelCount());
	//Count
	errorNode->AddAttribute(CXmlEvaluationReader::ATTR_count, error->GetCount());
	//False Alarm
	errorNode->AddAttribute(CXmlEvaluationReader::ATTR_falseAlarm, error->IsFalseAlarm());

	//Rects
	list<CRect*> * rects = error->GetRects();
	if (rects != NULL && !rects->empty())
	{
		CMsXmlNode * rectsNode;
		rectsNode = errorNode->AddChildNode(CXmlEvaluationReader::ELEMENT_ErrorRects);
		WriteRects(rects, rectsNode);
	}

	//False Alarm Rects
	rects = error->GetFalseAlarmRects();
	if (rects != NULL && !rects->empty())
	{
		CMsXmlNode * rectsNode;
		rectsNode = errorNode->AddChildNode(CXmlEvaluationReader::ELEMENT_FalseAlarmRects);
		WriteRects(rects, rectsNode);
	}

	//Merge specific things
	if (error->GetType() == CLayoutObjectEvaluationError::TYPE_MERGE)
	{
		CEvaluationErrorMerge * merge = (CEvaluationErrorMerge*)error;
		//Overlaps
		map<CUniString, COverlapRects*> * mergeMap = merge->GetMergingRegions();	//map [seg result region, overlaps to ground truth regions]

		map<CUniString,COverlapRects*>::iterator it = mergeMap->begin();
		while (it != mergeMap->end())
		{
			CMsXmlNode * mergeNode;
			mergeNode = errorNode->AddChildNode(CXmlEvaluationReader::ELEMENT_Merge);

			//Seg Result Region ID
			mergeNode->AddAttribute(CXmlEvaluationReader::ATTR_regionId, (*it).first);

			WriteOverlaps((*it).second, mergeNode);
			it++;
		}

		//Allowable entries
		map<CUniString, bool> * allowableEntries = merge->GetAllowableEntries();
		map<CUniString, bool>::iterator itAllowable = allowableEntries->begin();
		while (itAllowable != allowableEntries->end())
		{
			CMsXmlNode * allowableNode;
			allowableNode = errorNode->AddChildNode(CXmlEvaluationReader::ELEMENT_AllowableEntry);
			//Region Id (ground-truth region 2)
			allowableNode->AddAttribute(CXmlEvaluationReader::ATTR_regionId, (*itAllowable).first);
			//Is Allowable
			allowableNode->AddAttribute(CXmlEvaluationReader::ATTR_allowable, (*itAllowable).second);

			itAllowable++;
		}
	}
	//Split specific things
	else if (error->GetType() == CLayoutObjectEvaluationError::TYPE_SPLIT)
	{
		CEvaluationErrorSplit * split = (CEvaluationErrorSplit*)error;
		//Is Allowable
		errorNode->AddAttribute(CXmlEvaluationReader::ATTR_allowable, split->IsAllowable());
		//Overlaps
		WriteOverlaps(split->GetSplittingRegions(), errorNode);
	}
	//Misclassification specific things
	else if (error->GetType() == CLayoutObjectEvaluationError::TYPE_MISCLASS)
	{
		CEvaluationErrorMisclass * misclass = (CEvaluationErrorMisclass*)error;
		//Overlaps
		WriteOverlaps(misclass->GetMisclassRegions(), errorNode);
	}
}

/*
 * Overlaps
 */
void CXmlEvaluationWriter::WriteOverlaps(COverlapRects * overlapRects, 
											  CMsXmlNode * node)
{
	vector<CUniString> * mergedRegions = overlapRects->GetRegions();
	for (unsigned int i=0; i<mergedRegions->size(); i++)
	{
		CUniString groundTruthRegion = mergedRegions->at(i);
		CMsXmlNode * overlapNode;
		overlapNode = node->AddChildNode(CXmlEvaluationReader::ELEMENT_Overlap);

		//Ground-truth region ID
		overlapNode->AddAttribute(CXmlEvaluationReader::ATTR_regionId, groundTruthRegion);
		//Area
		overlapNode->AddAttribute(CXmlEvaluationReader::ATTR_area, 
									(int)overlapRects->GetOverlapArea(groundTruthRegion));
		//foregroundPixelCount
		overlapNode->AddAttribute(CXmlEvaluationReader::ATTR_foregroundPixelCount, 
									(int)overlapRects->GetOverlapPixelCount(groundTruthRegion));
	}
}

/*
 * Writes rectangles
 */
void CXmlEvaluationWriter::WriteRects(list<CRect*> * rects, CMsXmlNode * rectsNode)
{
	CUniString rectsString;
	for (list<CRect*>::iterator it = rects->begin(); it != rects->end(); it++)
	{
		/*CMsXmlNode * rectNode;
		rectNode = rectsNode->AddChildNode(CXmlEvaluationReader::ELEMENT_Rect);

		rectNode->AddAttribute(CXmlEvaluationReader::ATTR_left, (*it)->left);
		rectNode->AddAttribute(CXmlEvaluationReader::ATTR_top, (*it)->top);
		rectNode->AddAttribute(CXmlEvaluationReader::ATTR_right, (*it)->right);
		rectNode->AddAttribute(CXmlEvaluationReader::ATTR_bottom, (*it)->bottom);
		*/
		if (!rectsString.IsEmpty())
			rectsString.Append(L" ");
		rectsString.Append((int)(*it)->left);
		rectsString.Append(L",");
		rectsString.Append((int)(*it)->top);
		rectsString.Append(L",");
		rectsString.Append((int)(*it)->right);
		rectsString.Append(L",");
		rectsString.Append((int)(*it)->bottom);
	}
	rectsNode->AddAttribute(CXmlEvaluationReader::ATTR_rects, rectsString);
}

CUniString CXmlEvaluationWriter::RegionTypeIntToString(int type)
{
	if (CLayoutRegion::TYPE_ALL == type)
		return CUniString(L"all");
	if (CLayoutRegion::TYPE_TEXT == type)
		return CUniString(L"text");
	if (CLayoutRegion::TYPE_IMAGE == type)
		return CUniString(L"image");
	if (CLayoutRegion::TYPE_GRAPHIC == type)
		return CUniString(L"graphic");
	if (CLayoutRegion::TYPE_LINEART == type)
		return CUniString(L"line-drawing");
	if (CLayoutRegion::TYPE_CHART == type)
		return CUniString(L"chart");
	if (CLayoutRegion::TYPE_SEPARATOR == type)
		return CUniString(L"separator");
	if (CLayoutRegion::TYPE_MATHS == type)
		return CUniString(L"maths");
	if (CLayoutRegion::TYPE_ADVERT == type)
		return CUniString(L"advert");
	if (CLayoutRegion::TYPE_CHEM == type)
		return CUniString(L"chem");
	if (CLayoutRegion::TYPE_MUSIC == type)
		return CUniString(L"music");
	if (CLayoutRegion::TYPE_MAP == type)
		return CUniString(L"map");
	if (CLayoutRegion::TYPE_NOISE == type)
		return CUniString(L"noise");
	if (CLayoutRegion::TYPE_TABLE == type)
		return CUniString(L"table");
	if (CLayoutRegion::TYPE_CUSTOM == type)
		return CUniString(L"custom");
	return CUniString(L"unknown");
}

CUniString CXmlEvaluationWriter::PageObjectLevelIntToString(int level)
{
	if (CLayoutObject::TYPE_LAYOUT_REGION == level)
		return CUniString(L"region");
	if (CLayoutObject::TYPE_TEXT_LINE == level)
		return CUniString(L"text-line");
	if (CLayoutObject::TYPE_WORD == level)
		return CUniString(L"word");
	if (CLayoutObject::TYPE_GLYPH == level)
		return CUniString(L"glyph");
	if (CLayoutObject::TYPE_READING_ORDER_GROUP == level)
		return CUniString(L"group");
	return L"";
}

CUniString CXmlEvaluationWriter::ErrorTypeIntToString(int type)
{
	if (CLayoutObjectEvaluationError::TYPE_MERGE == type)
		return CUniString(L"merge");
	if (CLayoutObjectEvaluationError::TYPE_SPLIT == type)
		return CUniString(L"split");
	if (CLayoutObjectEvaluationError::TYPE_MISS == type)
		return CUniString(L"miss");
	if (CLayoutObjectEvaluationError::TYPE_PART_MISS == type)
		return CUniString(L"partial-miss");
	if (CLayoutObjectEvaluationError::TYPE_MISCLASS == type)
		return CUniString(L"misclassification");
	if (CLayoutObjectEvaluationError::TYPE_INVENT == type)
		return CUniString(L"false-detection");
	return L"";
}

CUniString CXmlEvaluationWriter::ReadingOrderRelationTypeIntToString(int type)
{
	if (CReadingOrder::RELATION_PREDECESSOR == type)
		return CUniString(L"before-directly");
	if (CReadingOrder::RELATION_SUCCESSOR == type)
		return CUniString(L"after-directly");
	if (CReadingOrder::RELATION_BEFORE == type)
		return CUniString(L"before-unordered");
	if (CReadingOrder::RELATION_AFTER == type)
		return CUniString(L"after-unordered");
	if (CReadingOrder::RELATION_UNORDERED == type)
		return CUniString(L"unordered");
	if (CReadingOrder::RELATION_UNRELATED == type)
		return CUniString(L"unrelated");
	if (CReadingOrder::RELATION_UNKNOWN == type)
		return CUniString(L"unknown");
	return CUniString(L"not-defined");
}



/*
 * Class CWeightSetting
 *
 * CC 11.03.2013
 */

CWeightSetting::CWeightSetting(double value, double allowableValue, bool enableAllowable, bool useAllowable)
{
	m_Count = 1;
	m_Value = value;
	m_AllowableValue = allowableValue;
	m_EnableAllowable = enableAllowable;
	m_UseAllowable = useAllowable;
}

CWeightSetting::CWeightSetting(CWeightSetting * toCopy)
{
	m_Count = toCopy->m_Count;
	m_Value = toCopy->m_Value;
	m_AllowableValue = toCopy->m_AllowableValue;
	m_EnableAllowable = toCopy->m_EnableAllowable;
	m_UseAllowable = toCopy->m_UseAllowable;
}

CWeightSetting::~CWeightSetting()
{
}

bool CWeightSetting::operator==(const CWeightSetting& other)
{
	return		abs(m_Value - other.m_Value) < 0.00001
			&&	abs(m_AllowableValue - other.m_AllowableValue) < 0.00001
			&&	m_EnableAllowable == other.m_EnableAllowable
			&&	m_UseAllowable == other.m_UseAllowable;
}