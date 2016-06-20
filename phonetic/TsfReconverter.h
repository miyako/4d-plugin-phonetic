#ifndef __TSF_RECONVERTER_H_INCLUDED__
#define __TSF_RECONVERTER_H_INCLUDED__
#include <msctf.h>
#include <msaatext.h>
#include <ctffunc.h>
#include "ReconvTextStore.h"


//! TsfReconverter::EnumCandidates() に渡すコールバック関数
/*!
 *	@param[in]			i_index
 *		何番目の候補か。
 *	@param[in]			i_number_of_candidates
 *		候補のトータル数
 *	@param[in]			i_candidate
 *		候補文字列。コールバックから抜けたら文字列は解放される。
 *	@param[in,out]	io_param
 *		EnumCandidates() に渡したパラメーター。
 *
 *	@return 列挙を続ける場合は true, 中止する場合は false を返す。
 */
typedef bool (*EnumCandidatesCallbackP)(int							i_index,
																				int							i_number_of_candidates,
																				const wchar_t*	i_candidate,
																				void*						io_param
																			 );


//! TSF(Text Service Framework) を利用して変換候補リストを取得するクラス。
class TsfReconverter
{
	//===========================================================================
	// 公開メソッド
	//===========================================================================
public:
	//! コンストラクタ
	TsfReconverter();
	//! デストラクタ
	virtual ~TsfReconverter();
	
	//! クリーンアップ
	bool Cleanup();
	
	//! 初期化に成功したかどうか調べる。
	bool IsOpened();
	
	//! フォーカスの取得。
	bool SetFocus();

	//! 変換候補リストを列挙する。
	bool EnumCandidates(
		const wchar_t*						i_yomi,
		EnumCandidatesCallbackP		i_callback,
		void*											io_param
	);
	
	//===========================================================================
	// 非公開メソッド
	//===========================================================================
private:

	//! コピーコンストラクタは使用禁止
	TsfReconverter(const TsfReconverter&);
	//! 代入演算子は使用禁止
	const TsfReconverter& operator=(const TsfReconverter&);

	//===========================================================================
	// メンバ
	//===========================================================================
public:
	ITfThreadMgr*					m_thread_mgr_cp;
	ITfDocumentMgr*				m_document_mgr_cp;
	ITfContext*						m_context_cp;
	ITfFunctionProvider*	m_function_provider_cp;
	ReconvTextStore*			m_text_store_cp;
	ITfFnReconversion*		m_reconversion_cp;
	TfEditCookie					m_edit_cookie;
};


#endif