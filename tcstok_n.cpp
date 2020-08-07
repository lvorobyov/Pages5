/**
 * @Author: Lev Vorobjev
 * @Date:   26.04.2018
 * @Email:  lev.vorobjev@rambler.ru
 * @Filename: tcstok_n.cpp
 * @Last modified by:   Lev Vorobjev
 * @Last modified time: 26.04.2018
 * @License: MIT
 * @Copyright: Copyright (c) 2018 Lev Vorobjev
 */

#include "tcstok_n.h"

#ifdef _UNICODE
  #define SCAS_TCHAR    "scasw\n\t"
  #define MOVS_TCHAR    "movsw\n\t"
  #define STOS_TCHAR    "stosw\n\t"
  #define LODS_TCHAR    "lodsw\n\t"
  #define CMP_AX_TCHAR  "cmp ax, word ptr [rsi]\n\t"
  #define MOV_TCHAR_AX  "mov word ptr [rsi], ax\n\t"
  #define MOV_TCHAR_Z   "mov word ptr [rsi-2], 0\n\t"
  #define TCHAR_PTR     "word ptr"
  #define INC_ESI_TCHAR "add rsi, 2\n\t"
  #define DEC_ESI_TCHAR "sub rsi, 2\n\t"
#else
  #define SCAS_TCHAR    "scasb\n\t"
  #define MOVS_TCHAR    "movsb\n\t"
  #define STOS_TCHAR    "stosb\n\t"
  #define LODS_TCHAR    "lodsb\n\t"
  #define CMP_AX_TCHAR  "cmp al, byte ptr [rsi]\n\t"
  #define MOV_TCHAR_AX  "mov byte ptr [rsi], al\n\t"
  #define MOV_TCHAR_Z   "mov byte ptr [rsi-1], 0\n\t"
  #define TCHAR_PTR     "byte ptr"
  #define INC_ESI_TCHAR "inc rsi\n\t"
  #define DEC_ESI_TCHAR "dec rsi\n\t"
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
LPTSTR _tcstok_n(LPTSTR tcs, LPCTSTR delim, __int64 count) {
    static LPTSTR ptr = NULL;
    LPTSTR result;
    if (tcs == NULL) {
        if (ptr == NULL) {
            return NULL;
        } else {
            tcs = ptr;
        }
    }
	int flag = 0;
	do {
		// Пропуск ведущих разделителей
		if (_tcschr(delim, *tcs) != nullptr) {
			if (*tcs++ == 0) {
				ptr = nullptr;
				return nullptr;
			}
			continue;
		}
		// Найден токен
		if (flag == 0) {
			// Запомнить начало токена
			result = tcs++;
			flag++;
		}
		// Найти конец токена
		while (*tcs != 0 && _tcschr(delim, *tcs) == nullptr)
			tcs++;
        // Достигнут конец токена
    } while (--count);
	// Найдено N токенов
	*tcs++ = 0;
	ptr = tcs;
    return result;
}
