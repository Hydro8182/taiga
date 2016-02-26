/*
** Taiga
** Copyright (C) 2010-2014, Eren Okka
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <curl/curlver.h>
#include <jsoncpp/json/json.h>
#include <pugixml/pugixml.hpp>
#include <utf8proc/utf8proc.h>
#include <zlib/zlib.h>

#include "base/file.h"
#include "base/gfx.h"
#include "base/string.h"
#include "taiga/orange.h"
#include "taiga/resource.h"
#include "taiga/stats.h"
#include "taiga/taiga.h"
#include "ui/dlg/dlg_score.h"
#include "library/anime_util.h"
#include "library/anime_db.h"

namespace ui {

	enum ThirdPartyLibrary {
		kJsoncpp,
		kLibcurl,
		kPugixml,
		kUtf8proc,
		kZlib,
	};

	static std::wstring GetLibraryVersion(ThirdPartyLibrary library) {
		switch (library) {
		case kJsoncpp:
			return StrToWstr(JSONCPP_VERSION_STRING);
		case kLibcurl:
			return StrToWstr(LIBCURL_VERSION);
		case kPugixml: {
			base::SemanticVersion version((PUGIXML_VERSION / 100),
				(PUGIXML_VERSION % 100) / 10,
				(PUGIXML_VERSION % 100) % 10);
			return version;
		}
		case kUtf8proc:
			return StrToWstr(utf8proc_version());
		case kZlib:
			return StrToWstr(ZLIB_VERSION);
			break;
		}

		return std::wstring();
	}

	////////////////////////////////////////////////////////////////////////////////

	class ScoreDialog DlgScore;


	ScoreDialog::ScoreDialog() : anime_id_(anime::ID_UNKNOWN){
		RegisterDlgClass(L"TaigaScoreW");
	}

	BOOL ScoreDialog::OnDestroy() {
		taiga::orange.Stop();

		return TRUE;
	}

	BOOL ScoreDialog::OnInitDialog() {
		rich_edit_.Attach(GetDlgItem(IDC_RICHEDIT_ABOUT));
		auto schemes = L"http:https:irc:";
		rich_edit_.SendMessage(EM_AUTOURLDETECT, TRUE /*= AURL_ENABLEURL*/,
			reinterpret_cast<LPARAM>(schemes));
		rich_edit_.SetEventMask(ENM_LINK);

		std::wstring text =
			L"{\\rtf1\\ansi\\deff0\\deflang1024"
			L"{\\fonttbl"
			L"{\\f0\\fnil\\fcharset0 Segoe UI;}"
			L"}"
			L"\\fs24\\b " TAIGA_APP_NAME L"\\b0  " + std::wstring(Taiga.version) + L"\\line\\fs18\\par "
			L"\\b Author:\\b0\\line "
			L"Eren 'erengy' Okka\\line\\par "
			L"\\b Contributors:\\b0\\line "
			L"saka, Diablofan, slevir, LordGravewish, cassist, rr-, sunjayc, LordHaruto, Keelhauled,\\line "
			L"thesethwalker, Soinou, menma1234, KazukiMutou, vipirius, Kokoro-chan, snowfag,\\line "
			L"such-doge\\line\\par "
			L"\\b Third-party components:\\b0\\line "
			L"{\\field{\\*\\fldinst{HYPERLINK \"https://github.com/yusukekamiyamane/fugue-icons\"}}{\\fldrslt{Fugue Icons 3.4.5}}}, "
			L"{\\field{\\*\\fldinst{HYPERLINK \"https://github.com/open-source-parsers/jsoncpp\"}}{\\fldrslt{JsonCpp " + GetLibraryVersion(kJsoncpp) + L"}}}, "
			L"{\\field{\\*\\fldinst{HYPERLINK \"https://github.com/bagder/curl\"}}{\\fldrslt{libcurl " + GetLibraryVersion(kLibcurl) + L"}}}, "
			L"{\\field{\\*\\fldinst{HYPERLINK \"https://github.com/zeux/pugixml\"}}{\\fldrslt{pugixml " + GetLibraryVersion(kPugixml) + L"}}}, "
			L"{\\field{\\*\\fldinst{HYPERLINK \"https://github.com/JuliaLang/utf8proc\"}}{\\fldrslt{utf8proc " + GetLibraryVersion(kUtf8proc) + L"}}}, "
			L"{\\field{\\*\\fldinst{HYPERLINK \"https://github.com/madler/zlib\"}}{\\fldrslt{zlib " + GetLibraryVersion(kZlib) + L"}}}\\line\\par "
			L"\\b Links:\\b0\\line "
			L"\u2022 {\\field{\\*\\fldinst{HYPERLINK \"http://taiga.moe\"}}{\\fldrslt{Home page}}}\\line "
			L"\u2022 {\\field{\\*\\fldinst{HYPERLINK \"https://github.com/erengy/taiga\"}}{\\fldrslt{GitHub repository}}}\\line "
			L"\u2022 {\\field{\\*\\fldinst{HYPERLINK \"https://hummingbird.me/groups/taiga\"}}{\\fldrslt{Hummingbird group}}}\\line "
			L"\u2022 {\\field{\\*\\fldinst{HYPERLINK \"http://myanimelist.net/clubs.php?cid=21400\"}}{\\fldrslt{MyAnimeList club}}}\\line "
			L"\u2022 {\\field{\\*\\fldinst{HYPERLINK \"https://twitter.com/taigaapp\"}}{\\fldrslt{Twitter account}}}\\line "
			L"\u2022 {\\field{\\*\\fldinst{HYPERLINK \"irc://irc.rizon.net/taiga\"}}{\\fldrslt{IRC channel}}}"
			L"}";
		rich_edit_.SetTextEx(WstrToStr(text));
		win::ComboBox combobox = GetDlgItem(IDC_COMBO_ANIME_STATUS);
		combobox.SetWindowHandle(GetDlgItem(IDC_COMBO_ANIME_SCORE));
		if (combobox.GetCount() == 0) {
			for (int i = 10; i >= 0; i--) {
				combobox.AddString(anime::TranslateMyScoreFull(i).c_str());
			}
		}
		//combobox.SetCurSel(10 - anime_item->GetMyScore());
		combobox.SetCurSel(10);
		combobox.SetWindowHandle(nullptr);

		Refresh(anime_id_);
		return TRUE;
	}

	BOOL ScoreDialog::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_COMMAND: {
			// Icon click
			if (HIWORD(wParam) == STN_DBLCLK) {
				SetText(L"Orange");
				Stats.tigers_harmed++;
				taiga::orange.Start();
				return TRUE;
			}
			break;
		}

		case WM_NOTIFY: {
			switch (reinterpret_cast<LPNMHDR>(lParam)->code) {
				// Execute link
			case EN_LINK: {
				auto en_link = reinterpret_cast<ENLINK*>(lParam);
				if (en_link->msg == WM_LBUTTONUP) {
					ExecuteLink(rich_edit_.GetTextRange(&en_link->chrg));
					return TRUE;
				}
				break;
			}
			}
			break;
		}
		}

		return DialogProcDefault(hwnd, uMsg, wParam, lParam);
	}

	void ScoreDialog::OnPaint(HDC hdc, LPPAINTSTRUCT lpps) {
		win::Dc dc = hdc;
		win::Rect rect;

		win::Rect rect_edit;
		rich_edit_.GetWindowRect(GetWindowHandle(), &rect_edit);

		const int margin = rect_edit.top;
		const int sidebar_width = rect_edit.left - margin;

		// Paint background
		GetClientRect(&rect);
		rect.left = sidebar_width;
		dc.FillRect(rect, ::GetSysColor(COLOR_WINDOW));

		// Paint application icon
		rect.Set(margin / 2, margin, sidebar_width - (margin / 2), rect.bottom);
		DrawIconResource(IDI_MAIN, dc.Get(), rect, true, false);
		win::Window label = GetDlgItem(IDC_STATIC_APP_ICON);
		label.SetPosition(nullptr, rect,
			SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOOWNERZORDER | SWP_NOZORDER);
		label.SetWindowHandle(nullptr);
	}

	void ScoreDialog::Refresh(int anime_id) {
		if (!anime::IsValidId(anime_id))
			return;

		int anime_id_ = anime_id;
		auto anime_item = AnimeDatabase.FindItem(anime_id_);

		if (!anime_item || !anime_item->IsInList())
			return;

		// Episodes watched
		SendDlgItemMessage(IDC_SPIN_PROGRESS, UDM_SETRANGE32, 0,
			anime_item->GetEpisodeCount() > 0 ? anime_item->GetEpisodeCount() : 9999);
		SendDlgItemMessage(IDC_SPIN_PROGRESS, UDM_SETPOS32, 0, anime_item->GetMyLastWatchedEpisode());

		// Rewatching
		CheckDlgButton(IDC_CHECK_ANIME_REWATCH, anime_item->GetMyRewatching());

		// Status
		win::ComboBox combobox = GetDlgItem(IDC_COMBO_ANIME_STATUS);
		if (combobox.GetCount() == 0)
			for (int i = anime::kMyStatusFirst; i < anime::kMyStatusLast; i++)
				combobox.AddItem(anime::TranslateMyStatus(i, false).c_str(), i);
		combobox.SetCurSel(anime_item->GetMyStatus() - 1);
		combobox.SetWindowHandle(nullptr);

		// Score
		combobox.SetWindowHandle(GetDlgItem(IDC_COMBO_ANIME_SCORE));
		if (combobox.GetCount() == 0) {
			for (int i = 10; i >= 0; i--) {
				combobox.AddString(anime::TranslateMyScoreFull(i).c_str());
			}
		}
		combobox.SetCurSel(10 - anime_item->GetMyScore());
		combobox.SetWindowHandle(nullptr);

		// Tags
		win::Edit edit = GetDlgItem(IDC_EDIT_ANIME_TAGS);
		edit.SetCueBannerText(L"Enter tags here, separated by a comma (e.g. tag1, tag2)");
		edit.SetText(anime_item->GetMyTags());
		edit.SetWindowHandle(nullptr);

		// Dates
		bool start_date_changed_ = false;
		bool finish_date_changed_ = false;
		auto date_format = L"yyyy-MM-dd";
		SendDlgItemMessage(IDC_DATETIME_START, DTM_SETFORMAT, 0, (LPARAM)date_format);
		SendDlgItemMessage(IDC_DATETIME_FINISH, DTM_SETFORMAT, 0, (LPARAM)date_format);
		auto set_default_systemtime = [&](int control_id, SYSTEMTIME& st) {
			SendDlgItemMessage(control_id, DTM_SETRANGE, GDTR_MIN, (LPARAM)&st);
			SendDlgItemMessage(control_id, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
		};
		if (anime::IsValidDate(anime_item->GetDateStart())) {
			SYSTEMTIME stSeriesStart = anime_item->GetDateStart();
			set_default_systemtime(IDC_DATETIME_START, stSeriesStart);
			set_default_systemtime(IDC_DATETIME_FINISH, stSeriesStart);
		}
		if (anime::IsValidDate(anime_item->GetDateEnd())) {
			SYSTEMTIME stSeriesEnd = anime_item->GetDateEnd();
			set_default_systemtime(IDC_DATETIME_FINISH, stSeriesEnd);
		}
		auto fix_systemtime = [](SYSTEMTIME& st) {
			if (!st.wMonth) st.wMonth = 1;
			if (!st.wDay) st.wDay = 1;
		};
		if (anime::IsValidDate(anime_item->GetMyDateStart())) {
			SYSTEMTIME stMyStart = anime_item->GetMyDateStart();
			fix_systemtime(stMyStart);
			SendDlgItemMessage(IDC_DATETIME_START, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&stMyStart);
		}
		else {
			SendDlgItemMessage(IDC_DATETIME_START, DTM_SETSYSTEMTIME, GDT_NONE, 0);
		}
		if (anime::IsValidDate(anime_item->GetMyDateEnd())) {
			SYSTEMTIME stMyFinish = anime_item->GetMyDateEnd();
			fix_systemtime(stMyFinish);
			SendDlgItemMessage(IDC_DATETIME_FINISH, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&stMyFinish);
		}
		else {
			SendDlgItemMessage(IDC_DATETIME_FINISH, DTM_SETSYSTEMTIME, GDT_NONE, 0);
		}

		// Alternative titles
		edit.SetWindowHandle(GetDlgItem(IDC_EDIT_ANIME_ALT));
		edit.SetCueBannerText(L"Enter alternative titles here, separated by a semicolon (e.g. Title 1; Title 2)");
		edit.SetText(Join(anime_item->GetUserSynonyms(), L"; "));
		edit.SetWindowHandle(nullptr);
		CheckDlgButton(IDC_CHECK_ANIME_ALT, anime_item->GetUseAlternative());

		// Folder
		edit.SetWindowHandle(GetDlgItem(IDC_EDIT_ANIME_FOLDER));
		edit.SetText(anime_item->GetFolder());
		edit.SetWindowHandle(nullptr);

		// Fansub group
	//	RefreshFansubPreference();
	}

	

}  // namespace ui