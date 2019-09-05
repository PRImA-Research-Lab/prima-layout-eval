#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include "XmlValidator.h"
#include "MsXmlValidator.h"
#include <vector>
#include <map>
#include "ExtraFileHelper.h"

namespace PRImA
{

/*
 * Class CEvalXmlValidatorProvider
 *
 * Implementation of CXmlValidatorProvider
 *
 * CC 11.06.2010 - created
 */
class CEvalXmlValidatorProvider :
	public CXmlValidatorProvider
{
public:
	CEvalXmlValidatorProvider(CUniString & rootPath);
	~CEvalXmlValidatorProvider(void);

	virtual CValidator		*	GetValidator(CUniString schemaVersion);
	virtual CValidator		*	GetValidator(CUniString schemaVersion, CUniString & errMsg);

	virtual CValidator		*	GetLatestValidator();

	virtual int					GetVersionNumber(CUniString versionString);

	virtual CUniString			GetVersionString(int versionNumber);

	virtual CUniString			GetLatestVersion();


private:
	void		InitVersionList();

private:
	CUniString				m_SchemaPath;
	std::vector<CUniString>		m_SchemaVersions;
	std::vector<const wchar_t *>	m_NameSpaces;

	std::map<CUniString, CValidator *>	m_Validators;

};

}