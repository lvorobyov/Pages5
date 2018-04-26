/**
 * @Author: Lev Vorobjev
 * @Date:   26.04.2018
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: tcstok_n.h
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 26.04.2018
 * @License: MIT
 * @Copyright: Copyright (c) 2018 Lev Vorobjev
 */

#ifndef TCSTOK_N_H
#define TCSTOK_N_H

#include <tchar.h>
#include <wtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * _tcstok_n
 * Разделение строки на группы по несколько токенов
 * @see _tcstok(tcs,delim)
 * @param tcs обрабатываемая строка
 * @param delim строка разделителей
 * @param count количество токенов в одной группе
 * @return указатель на строку токенов или NULL, если токенов нет
 */
LPTSTR _tcstok_n(LPTSTR tcs, LPCTSTR delim, __int64 count);

#ifdef __cplusplus
}
#endif

#endif
