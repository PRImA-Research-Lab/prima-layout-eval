#pragma once

/*
 * University of Salford
 * Pattern Recognition and Image Analysis Research Lab
 * Author: Christian Clausner
 */

#include <time.h>
#include "Parameter.h"


/*
 * Class CMetaData
 *
 * Meta data for evaluation.
 *
 * CC 15.01.2010 - created
 */

namespace PRImA
{

class CMetaData
{
public:
	CMetaData(void);
	~CMetaData(void);

	inline CUniString		GetComments() { return m_Comments.GetValue(false); };
	inline bool				IsCommentsSet() { return m_Comments.IsSet(); };
	inline time_t			GetCreated() { return m_Created; };
	inline CUniString		GetCreator() { return m_Creator.GetValue(false); };
	CUniString				GetEvalID();
	inline time_t			GetLastModified() { return m_LastChange; };
	inline CParameterMap *	GetParameters() { return &m_Params; };
	inline CUniString		GetSoftwareName() { return IsSoftwareNameSet() ? m_SoftwareName.GetValue() : CUniString(); };
	inline bool				IsSoftwareNameSet() { return m_SoftwareName.IsSet(); };
	inline CUniString		GetSoftwareVersion() { return IsSoftwareVersionSet() ? m_SoftwareVersion.GetValue() : CUniString(); };
	inline bool				IsSoftwareVersionSet() { return m_SoftwareVersion.IsSet(); };
	inline CUniString		GetSoftwareProducer() { return IsSoftwareProducerSet() ? m_SoftwareProducer.GetValue() : CUniString(); };
	inline bool				IsSoftwareProducerSet() { return m_SoftwareProducer.IsSet(); };

	void				SetEvalId(CUniString newEvalId);
	inline void			SetCreator(CUniString creator) { m_Creator.SetValue(creator); };
	void				SetCreated(time_t time);
	void				SetLastChange(time_t time);
	inline void			SetComments(CUniString comments) { m_Comments.SetValue(comments); };
	inline void			SetSoftware(CUniString name, CUniString version) { m_SoftwareName.SetValue(name); m_SoftwareVersion.SetValue(version); };
	inline void			SetSoftware(CUniString name, CUniString version, CUniString producer) { m_SoftwareName.SetValue(name); m_SoftwareVersion.SetValue(version); m_SoftwareProducer.SetValue(producer);};

private:
	CStringParameter		m_Comments;
	CStringParameter		m_Creator;
	CStringParameter		m_EvalId;		//Evaluation ID
	time_t					m_Created;		// File Modification Time, Stored In UNIX Time. It is signed but should only use -1 to represent no value
	time_t					m_LastChange;	// File Modification Time, Stored In UNIX Time. It is signed but should only use -1 to represent no value
	CStringParameter		m_CreatedStr;
	CStringParameter		m_LastChangeStr;
	CParameterMap			m_Params;
	CStringParameter		m_SoftwareName;
	CStringParameter		m_SoftwareVersion;
	CStringParameter		m_SoftwareProducer;

};

}