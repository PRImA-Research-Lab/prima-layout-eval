
/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "stdafx.h"
#include "EvalXmlValidatorProvider.h"

using namespace PRImA;
using namespace std;

/*
 * Class CEvalXmlValidatorProvider
 *
 * Implementation of CXmlValidatorProvider
 *
 * CC 11.06.2010 - created
 */

CEvalXmlValidatorProvider::CEvalXmlValidatorProvider(CUniString & rootPath)
{
	m_SchemaPath = rootPath;
	m_SchemaPath.Append("schema\\");
	InitVersionList();
}

CEvalXmlValidatorProvider::~CEvalXmlValidatorProvider(void)
{
	//Delete the validators
	map<CUniString, CValidator *>::iterator it = m_Validators.begin();
	while (it != m_Validators.end())
	{
		delete (*it).second;
		it++;
	}
}

/*
 * Initializes the list with the schema version history.
 * Add new versions to the end of the list!
 */
void CEvalXmlValidatorProvider::InitVersionList()
{
	m_SchemaVersions.push_back(CUniString(_T("2010-01-15")));
	m_SchemaVersions.push_back(CUniString(_T("2010-08-06")));
	m_SchemaVersions.push_back(CUniString(_T("2011-02-09")));
	m_SchemaVersions.push_back(CUniString(_T("2011-08-22")));
	m_SchemaVersions.push_back(CUniString(_T("2013-03-11")));
	m_SchemaVersions.push_back(CUniString(_T("2013-07-15")));
	m_SchemaVersions.push_back(CUniString(_T("2018-07-15")));
	m_SchemaVersions.push_back(CUniString(_T("2019-07-15")));

	m_NameSpaces.push_back(_T("http://schema.primaresearch.org/PAGE/eval/layout/2010-01-15"));
	m_NameSpaces.push_back(_T("http://schema.primaresearch.org/PAGE/eval/layout/2010-08-06"));
	m_NameSpaces.push_back(_T("http://schema.primaresearch.org/PAGE/eval/layout/2011-02-09"));
	m_NameSpaces.push_back(_T("http://schema.primaresearch.org/PAGE/eval/layout/2011-08-22"));
	m_NameSpaces.push_back(_T("http://schema.primaresearch.org/PAGE/eval/layout/2013-03-11"));
	m_NameSpaces.push_back(_T("http://schema.primaresearch.org/PAGE/eval/layout/2013-07-15"));
	m_NameSpaces.push_back(_T("http://schema.primaresearch.org/PAGE/eval/layout/2018-07-15"));
	m_NameSpaces.push_back(_T("http://schema.primaresearch.org/PAGE/eval/layout/2019-07-15"));
}

/*
 * Returns the validator for the given schema version.
 */
CValidator * CEvalXmlValidatorProvider::GetValidator(CUniString schemaVersion)
{
	CUniString errs;
	return GetValidator(schemaVersion, errs);
}

/*
 * Returns the validator for the given schema version.
 */
CValidator	* CEvalXmlValidatorProvider::GetValidator(CUniString schemaVersion, CUniString & errMsg)
{
	map<CUniString, CValidator *>::iterator it = m_Validators.find(schemaVersion);

	CValidator * validator = NULL;
	m_ErrorMessage.Clear();
	if (it == m_Validators.end()) //Validator not found -> create it
	{
		CUniString path(m_SchemaPath);
		path.Append(schemaVersion);
		path.Append(_T("\\layouteval.xsd"));

		if (CExtraFileHelper::FileExists(path))
		{
			const wchar_t * nameSpace = m_NameSpaces[GetVersionNumber(schemaVersion)];

			validator = new CMsXmlValidator(path.GetBuffer(), nameSpace, NULL); //TODO:  CLayoutObject::GetDocumentationMap());		

			m_Validators.insert(pair<CUniString, CValidator*>(schemaVersion, validator));
		}
		else
		{
			CUniString msg(_T("Schema file missing for version "));
			msg.Append(schemaVersion);
			m_ErrorMessage = msg;
		}
	}
	else
		validator = (*it).second;

	return validator;
}

/*
 * Returns the validator for the latest schema.
 */
CValidator * CEvalXmlValidatorProvider::GetLatestValidator()
{
	return GetValidator(GetLatestVersion());
}

/*
 * If a schema with the given version string exists, a number >= 0 will be returned.
 * Otherwise it returns -1.
 */
int CEvalXmlValidatorProvider::GetVersionNumber(CUniString versionString)
{
	//Simply return the index of the string in the version list.
	for (unsigned int i=0; i<m_SchemaVersions.size(); i++)
	{
		if (m_SchemaVersions[i] == versionString)
			return i;
	}
	return -1;
}

CUniString CEvalXmlValidatorProvider::GetVersionString(int versionNumber)
{
	if (versionNumber < 0 || (unsigned int)versionNumber >= m_SchemaVersions.size())
		return CUniString();
	return m_SchemaVersions[m_SchemaVersions.size()-1];
}

CUniString CEvalXmlValidatorProvider::GetLatestVersion()
{
	return m_SchemaVersions[m_SchemaVersions.size()-1];
}
