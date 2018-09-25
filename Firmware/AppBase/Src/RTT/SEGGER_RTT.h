/*********************************************************************
*              SEGGER MICROCONTROLLER SYSTEME GmbH                   *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996-2014 SEGGER Microcontroller Systeme GmbH           *
*                                                                    *
* Internet: www.segger.com Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_RTT.h
Date    : 17 Dec 2014
Purpose : Implementation of SEGGER real-time terminal which allows
          real-time terminal communication on targets which support
          debugger memory accesses while the CPU is running.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef SEGGER_RTT_H
#define SEGGER_RTT_H
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SEGGER_RTT_MODE_MASK                  (3 << 0)

#define SEGGER_RTT_MODE_NO_BLOCK_SKIP         (0)
#define SEGGER_RTT_MODE_NO_BLOCK_TRIM         (1 << 0)
#define SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL    (1 << 1)

#define RTT_CTRL_RESET                "\x1B[0m"

#define RTT_CTRL_CLEAR                "\x1B[2J"

#define RTT_CTRL_TEXT_BLACK           "\x1B[2;30m"
#define RTT_CTRL_TEXT_RED             "\x1B[2;31m"
#define RTT_CTRL_TEXT_GREEN           "\x1B[2;32m"
#define RTT_CTRL_TEXT_YELLOW          "\x1B[2;33m"
#define RTT_CTRL_TEXT_BLUE            "\x1B[2;34m"
#define RTT_CTRL_TEXT_MAGENTA         "\x1B[2;35m"
#define RTT_CTRL_TEXT_CYAN            "\x1B[2;36m"
#define RTT_CTRL_TEXT_WHITE           "\x1B[2;37m"

#define RTT_CTRL_TEXT_BRIGHT_BLACK    "\x1B[1;30m"
#define RTT_CTRL_TEXT_BRIGHT_RED      "\x1B[1;31m"
#define RTT_CTRL_TEXT_BRIGHT_GREEN    "\x1B[1;32m"
#define RTT_CTRL_TEXT_BRIGHT_YELLOW   "\x1B[1;33m"
#define RTT_CTRL_TEXT_BRIGHT_BLUE     "\x1B[1;34m"
#define RTT_CTRL_TEXT_BRIGHT_MAGENTA  "\x1B[1;35m"
#define RTT_CTRL_TEXT_BRIGHT_CYAN     "\x1B[1;36m"
#define RTT_CTRL_TEXT_BRIGHT_WHITE    "\x1B[1;37m"

#define RTT_CTRL_BG_BLACK             "\x1B[24;40m"
#define RTT_CTRL_BG_RED               "\x1B[24;41m"
#define RTT_CTRL_BG_GREEN             "\x1B[24;42m"
#define RTT_CTRL_BG_YELLOW            "\x1B[24;43m"
#define RTT_CTRL_BG_BLUE              "\x1B[24;44m"
#define RTT_CTRL_BG_MAGENTA           "\x1B[24;45m"
#define RTT_CTRL_BG_CYAN              "\x1B[24;46m"
#define RTT_CTRL_BG_WHITE             "\x1B[24;47m"

#define RTT_CTRL_BG_BRIGHT_BLACK      "\x1B[4;40m"
#define RTT_CTRL_BG_BRIGHT_RED        "\x1B[4;41m"
#define RTT_CTRL_BG_BRIGHT_GREEN      "\x1B[4;42m"
#define RTT_CTRL_BG_BRIGHT_YELLOW     "\x1B[4;43m"
#define RTT_CTRL_BG_BRIGHT_BLUE       "\x1B[4;44m"
#define RTT_CTRL_BG_BRIGHT_MAGENTA    "\x1B[4;45m"
#define RTT_CTRL_BG_BRIGHT_CYAN       "\x1B[4;46m"
#define RTT_CTRL_BG_BRIGHT_WHITE      "\x1B[4;47m"


/*********************************************************************
*
*       RTT API functions
*
**********************************************************************
*/

int     SEGGER_RTT_Read             (unsigned BufferIndex,       char* pBuffer, unsigned BufferSize);
int     SEGGER_RTT_Write            (unsigned BufferIndex, const char* pBuffer, unsigned NumBytes);
int     SEGGER_RTT_WriteString      (unsigned BufferIndex, const char* s);

int     SEGGER_RTT_GetKey           (void);
int     SEGGER_RTT_WaitKey          (void);
int     SEGGER_RTT_HasKey           (void);

int     SEGGER_RTT_ConfigUpBuffer   (unsigned BufferIndex, const char* sName, char* pBuffer, int BufferSize, int Flags);
int     SEGGER_RTT_ConfigDownBuffer (unsigned BufferIndex, const char* sName, char* pBuffer, int BufferSize, int Flags);

void    SEGGER_RTT_Init             (void);

/*********************************************************************
*
*       RTT "Terminal" API functions
*
**********************************************************************
*/
void    SEGGER_RTT_SetTerminal        (char TerminalId);
int     SEGGER_RTT_TerminalOut        (char TerminalId, const char* s);

/*********************************************************************
*
*       RTT printf functions (require SEGGER_RTT_printf.c)
*
**********************************************************************
*/
int SEGGER_RTT_printf(unsigned BufferIndex, const char * sFormat, ...);

/*************************** End of file ****************************/
#endif 
