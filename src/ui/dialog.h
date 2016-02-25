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

#ifndef TAIGA_UI_DIALOG_H
#define TAIGA_UI_DIALOG_H

#include <windows.h>

namespace ui {

enum Dialog {
  kDialogNone,
  kDialogAbout,
  kDialogAnimeInformation,
  kDialogMain,
  kDialogSettings,
  kDialogTorrents,
  kDialogUpdate,
  kDialogScore
};

void DestroyDialog(Dialog dialog);
void EnableDialogInput(Dialog dialog, bool enable);
HWND GetWindowHandle(Dialog dialog);
void ShowDialog(Dialog dialog);

void ShowDlgAnimeEdit(int anime_id);
void ShowDlgAnimeInfo(int anime_id);
void ShowDlgSettings(int section, int page);
void showDlgScore(int anime_id);

}  // namespace ui

#endif  // TAIGA_UI_DIALOG_H