#include "stdafx.h"
#include <INITGUID.h>
#include "TsfReconverter.h"


//=============================================================================


//! コンストラクタ
TsfReconverter::TsfReconverter()
:	m_thread_mgr_cp(NULL),
	m_document_mgr_cp(NULL),
	m_context_cp(NULL),
	m_function_provider_cp(NULL),
	m_reconversion_cp(NULL),
	m_text_store_cp(NULL),
	m_edit_cookie(0)
{
	HRESULT		hr = E_FAIL;
	
	do
	{
		// スレッドマネージャーの生成
		hr = CoCreateInstance(
					 CLSID_TF_ThreadMgr,
					 NULL,
					 CLSCTX_INPROC_SERVER,
					 IID_ITfThreadMgr,
					 (void**)&m_thread_mgr_cp
				 );
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "スレッドマネージャーの生成に失敗しました。エラーコード:0x%08X\n", hr
						);
			break;
		}
		
		// ドキュメントマネージャーの生成
		hr = m_thread_mgr_cp->CreateDocumentMgr(&m_document_mgr_cp);
		if( FAILED(hr) || m_document_mgr_cp == NULL )
		{
			_RPTF1(_CRT_WARN,
						 "ドキュメントマネージャーの生成に失敗しました。エラーコード:0x%08X\n", hr
						);
			break;
		}
		
		// スレッドマネージャーのアクティブ化
		TfClientId	client_id = 0;
		hr = m_thread_mgr_cp->Activate(&client_id);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "スレッドマネージャーのアクティブ化に失敗しました。エラーコード:0x%08X\n", hr
						);
			break;
		}
		
		// テキストストアの生成
		ReconvTextStore::CreateInstance(m_text_store_cp);
		
		// コンテキストの生成
		hr = m_document_mgr_cp->CreateContext(
					 client_id,
					 0,			// reserved
					 (ITextStoreACP*)m_text_store_cp,
					 &m_context_cp,
					 &m_edit_cookie
				 );
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "コンテキストの生成に失敗しました。エラーコード:0x%08X\n", hr
						);
			break;
		}
		
		// コンテキストの push
		hr = m_document_mgr_cp->Push(m_context_cp);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "コンテキストの push に失敗しました。エラーコード:0x%08X\n", hr
						);
			break;
		}
		
		// ファンクションプロバイダーを取得する。
		hr = m_thread_mgr_cp->GetFunctionProvider(
					 GUID_SYSTEM_FUNCTIONPROVIDER,
					 &m_function_provider_cp
				 );
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "ファンクションプロバイダーの取得に失敗しました。エラーコード:0x%08X\n", hr
						);
			break;
		}

		// ITfFnReconversion の取得
		hr = m_function_provider_cp->GetFunction(
					 GUID_NULL,
					 IID_ITfFnReconversion,
					 (IUnknown**)&m_reconversion_cp
				 );
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN, "ITfFnReconversion の取得に失敗しました。エラーコード : 0x%08X.\n", hr);
			break;
		}
		
		// フォーカス取得
		hr = m_thread_mgr_cp->SetFocus(m_document_mgr_cp);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "スレッドマネージャーの SetFocus() に失敗しました。エラーコード:0x%08X\n", hr
						);
			break;
		}

		hr = S_OK;
	}
	while( 0 );
	
	if( FAILED(hr) )
		Cleanup();
	SetLastError(hr);
}


//=============================================================================


//! デストラクタ
TsfReconverter::~TsfReconverter()
{
	Cleanup();
}


//=============================================================================


//! クリーンアップ
bool TsfReconverter::Cleanup()
{
	// テキストストアの解放
	RELEASE(m_text_store_cp);
	// リコンバージョンファンクションの解放
	RELEASE(m_reconversion_cp);
	// ファンクションプロバイダーの解放
	RELEASE(m_function_provider_cp);
	// コンテキストの解放
	RELEASE(m_context_cp);
	// ドキュメントマネージャーの解放
	if( m_document_mgr_cp )
	{
		// 全てのコンテキストを解放する。
		m_document_mgr_cp->Pop(TF_POPF_ALL);
		RELEASE(m_document_mgr_cp);
	}
	// スレッドマネージャーの解放
	if( m_thread_mgr_cp )
	{
		// デアクティブにしてから解放する。
		m_thread_mgr_cp->Deactivate();
		RELEASE(m_thread_mgr_cp);
	}
	
	return true;
}


//=============================================================================


//! 初期化に成功したかどうか調べる。
bool TsfReconverter::IsOpened()
{
	return m_thread_mgr_cp != NULL;
}


//=============================================================================


//! フォーカスの取得
bool TsfReconverter::SetFocus()
{
	bool		retval = false;
	
	if( IsOpened() )
	{
		// フォーカス
		HRESULT hr = m_thread_mgr_cp->SetFocus(m_document_mgr_cp);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN,
						 "スレッドマネージャーの SetFocus() に失敗しました。エラーコード:0x%08X\n", hr
						);
		}
		else
		{
			retval = true;
		}
	}
	
	return retval;
}


//=============================================================================


//! 変換候補リストを列挙する。
bool TsfReconverter::EnumCandidates(
	const wchar_t*						i_yomi,
	EnumCandidatesCallbackP		i_callback,
	void*											io_param
)
{
	bool								retval					= false;
	HRESULT							hr							= E_FAIL;
	TF_SELECTION				selections[10]	= { 0 };
	ULONG								fetched_count		= 0;
	ITfRange*						range_cp				= NULL;
	BOOL								is_converted		= FALSE;
	ITfCandidateList*		cand_list_cp		= NULL;
	ULONG								cand_length			= 0;
	
	do
	{
		if( IsOpened() == false )
		{
			_RPTF0(_CRT_WARN, "初期化に失敗したのに EnumCandidates() が呼ばれました。\n");
			break;
		}
	
		// 文字列をセット
		m_text_store_cp->SetString(i_yomi);
		
		// ドキュメントのロックを行う。
		if( m_text_store_cp->LockDocument(TS_LF_READ) )
		{
			// 選択範囲の取得
			hr = m_context_cp->GetSelection(
						 m_edit_cookie,
						 TF_DEFAULT_SELECTION,
						 numberof(selections),
						 selections,
						 &fetched_count
					 );
			
			// ドキュメントのアンロック
			m_text_store_cp->UnlockDocument();
			
			if( FAILED(hr) )
			{
				_RPTF1(_CRT_WARN, "選択範囲の取得に失敗しました。エラーコード : 0x%08X.\n", hr);
				break;
			}
		}
		
		// 変換範囲を取得する。
		hr = m_reconversion_cp->QueryRange(selections[0].range, &range_cp, &is_converted);
		if( FAILED(hr) || range_cp == NULL )
		{
			_RPTF1(_CRT_WARN, "変換範囲の取得に失敗しました。エラーコード : 0x%08X.\n", hr);
			break;
		}

		// 再変換を行う
		hr = m_reconversion_cp->GetReconversion(selections[0].range, &cand_list_cp);
		if( FAILED(hr) || cand_list_cp == NULL )
		{
			_RPTF1(_CRT_WARN, "再変換に失敗しました。エラーコード : 0x%08X.\n", hr);
			break;
		}
		
		// 候補数を取得する。
		hr = cand_list_cp->GetCandidateNum(&cand_length);
		if( FAILED(hr) )
		{
			_RPTF1(_CRT_WARN, "候補数の取得に失敗しました。エラーコード : 0x%08X.\n", hr);
			break;
		}
		
		// 候補を戻り値にセットする。
		for(ULONG i = 0; i < cand_length; i++)
		{
			ITfCandidateString*		string_cp = NULL;
			BSTR									bstr			= NULL;

			if( SUCCEEDED(cand_list_cp->GetCandidate(i, &string_cp))
			&&	SUCCEEDED(string_cp->GetString(&bstr)) )
			{
				if( i_callback(i, cand_length, bstr, io_param) == false )
					i = cand_length;

				SysFreeString(bstr);
			}
			
			RELEASE(string_cp);
		}
		
		retval = true;
	}
	while( 0 );
	
	// クリーンアップ
	RELEASE(cand_list_cp);
	RELEASE(range_cp);
	for(int i = 0; i < numberof(selections); i++)
	{
		RELEASE(selections[i].range);
	}
	
	return retval;
}


//=============================================================================