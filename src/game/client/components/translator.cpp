#include <string>
#include <base/system.h>
#include <engine/shared/config.h>
#include <engine/external/json-parser/json.hpp>
#include <engine/client/curlwrapper.h>

#include "translator.h"


CTranslator::CTranslator()
{
	m_pHandle = NULL;
}

CTranslator::~CTranslator()
{
	// clean up
	if(m_pHandle)
		curl_easy_cleanup(m_pHandle);
	m_pHandle = NULL;
	m_Queue.clear();

	thread_wait(m_pThread);
}

bool CTranslator::Init()
{
	if((m_pHandle = curl_easy_init()))
	{
		m_pThread = thread_init_named(TranslationWorker, this, "transl. worker");
		return true;
	}
	return false;
}

void CTranslator::TranslationWorker(void *pUser)
{
	CTranslator *pTrans = (CTranslator *)pUser;

	while(pTrans->m_pHandle != NULL)
	{
		CALLSTACK_ADD();

		if(pTrans->m_Queue.size() > 0)
		{
			CTransEntry Entry = pTrans->m_Queue.front();
			pTrans->m_Queue.erase(pTrans->m_Queue.begin());

			char aPost[2048*8];
			char aTranslated[1024*8];
			std::string Response;

			curl_easy_setopt(pTrans->m_pHandle, CURLOPT_URL, "http://api.mymemory.translated.net/get");
			str_format(aPost, sizeof(aPost), "q=%s&langpair=%s|%s&de=associatingblog@gmail.com", Entry.m_Text, Entry.m_SrcLang, Entry.m_DstLang);
			curl_easy_setopt(pTrans->m_pHandle, CURLOPT_POSTFIELDS, aPost);

			curl_easy_setopt(pTrans->m_pHandle, CURLOPT_WRITEFUNCTION, &CCurlWrapper::CurlCallback_WriteToStdString);
			curl_easy_setopt(pTrans->m_pHandle, CURLOPT_WRITEDATA, &Response);
			curl_easy_perform(pTrans->m_pHandle);

			// parse response
			json_value &jsonValue = *json_parse(Response.c_str(), Response.length());
			const char *pResult = jsonValue["responseData"]["translatedText"];
			if(str_length(pResult) == 0)
			{
				dbg_msg("trans/warn", "failed to parse response\n%s", Response.c_str());
				continue;
			}
			str_copy(aTranslated, pResult, sizeof(aTranslated));
			if(str_comp_nocase(Entry.m_Text, aTranslated) != 0)
			{
				if(g_Config.m_Debug)
					dbg_msg("trans", "translated '%s' from '%s' to '%s', result: '%s'", Entry.m_Text, Entry.m_SrcLang, Entry.m_DstLang, aTranslated);

				// put the result to the queue
				str_copy(Entry.m_Text, aTranslated, sizeof(Entry.m_Text));
				pTrans->m_Results.push_back(Entry);
			}
			else
				dbg_msg("trans/warn", "translating '%s' from '%s' to '%s' failed", Entry.m_Text, Entry.m_SrcLang, Entry.m_DstLang);
		}

		thread_sleep(50);
	}
}

void CTranslator::RequestTranslation(const char *pSrcLang, const char *pDstLang, const char *pText, bool In)
{
	CALLSTACK_ADD();

	if(!str_utf8_check(pText))
	{
		dbg_msg("trans", "Invalid UTF-8 string");
		return;
	}

	// prepare the entry
	CTransEntry Entry;
	str_copy(Entry.m_Text, pText, sizeof(Entry.m_Text));
	str_copy(Entry.m_SrcLang, pSrcLang, sizeof(Entry.m_SrcLang));
	str_copy(Entry.m_DstLang, pDstLang, sizeof(Entry.m_DstLang));
	Entry.m_In = In;

	// insert the entry
	m_Queue.push_back(Entry);
}
