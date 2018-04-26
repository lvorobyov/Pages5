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
    __asm__ (
        ".intel_syntax noprefix\n\t"
        "cld\n\t"
        "mov rdi, %[delim]\n\t"
        "mov rcx, -1\n\t"
        "xor ax, ax\n\t"
        "repne " SCAS_TCHAR
        "sub rcx, -1\n\t"
        "neg rcx\n\t"
        "mov r8, rcx\n\t"
        "xor rdx, rdx\n\t"
        // Пропуск ведущих разделителей
        ".for1:\n\t"
        LODS_TCHAR
        "mov rdi, %[delim]\n\t"
        "repne " SCAS_TCHAR
        "jne .end1\n\t"
        "jrcxz .exit\n\t"
        "mov rcx, r8\n\t"
        "jmp .for1\n\t"
        ".end1:\n\t"
        // Найден токен
        "test rdx, rdx\n\t"
        "jnz .end2\n\t"
        // Запомнить начало токена
        DEC_ESI_TCHAR
        "mov %0, rsi\n\t"
		INC_ESI_TCHAR
        "inc rdx\n\t"
        ".end2:\n\t"
        // Найти конец токена
        ".for2:\n\t"
        LODS_TCHAR
        "mov rcx, r8\n\t"
        "mov rdi, %[delim]\n\t"
        "repne " SCAS_TCHAR
        "jne .for2\n\t"
        "jrcxz .exit\n\t"
        // Достигнут конец токена
        "dec rbx\n\t"
        "test rbx, rbx\n\t"
        "jz .end3\n\t"
        "mov rcx, r8\n\t"
        "jmp .for1\n\t"
        ".end3:\n\t"
        // Найдено N токенов
        MOV_TCHAR_Z
        "mov %[ptr], rsi\n\t"
        "jmp .exit2\n\t"
        ".exit:\n\t"
        // Достигнут конец строки
        "mov %[ptr], 0\n\t"
        ".exit2:\n\t"
        : "=m" (result), [ptr]"=m"(ptr)
        : "S" (tcs), [delim]"r" (delim), "b" (count)
        : "rax", "rcx", "rdx", "rdi", "r8", "cc", "memory"
    );
    return result;
}
