#include "PE2.h"
#include "Define.h"
#include <stddef.h>
#include <stdio.h> 
HANDLE g_hIn;
HANDLE g_hOut;
const int ROW = 22; //列數
const int COL = 80; //欄數
int g_CommandCursorX = 0; //命令列游標x軸
int g_CommandCursorY = 22; //命令列游標y軸
char *g_szToggleMode = "Insert"; //Insert or Remove
int g_Esc = 1; //Esc mode
int g_iCapsLock = 1; //大小寫狀態切換
int g_iShift = -1; //Shift按下狀態
int g_iInsert = 1; //大小寫狀態切換
char g_Command[100]; //輸入指令
bool g_Quit = false;
int g_iFileIndex = 0;
int g_iOpenFileNum = 1;
int g_lfilesize; //檔案大小
char g_SearchData[100]; //尋找內容
char g_ReplaceData[100]; //取代內容
int g_Mark[2][2] = { { -1, -1 }, { -1, -1 } }; //Mark選取範圍
char g_Markcase; //Mark的方式
int g_MarkFileNum = -1;

struct FileStruct
{
	FILE* g_pFile; //檔案
	char **g_ptr;
	int *g_arrayLen; //陣列每行長度
	int *g_stringLen; //陣列每行字串長度
	int g_iCurrentRowNum; //目前的的列數
	int g_iNewRowNum; //新的列數
	int g_iNewColNum; //新的行數
	int g_iMaxstringLen; //計算最後有值列數
	char g_FileName[100]; //檔名
	char g_NewFileName[100]; //新檔名
	int g_UIX; //視窗游標x軸
	int g_UIY; //視窗游標y軸
	int g_FileX; //檔案游標x軸
	int g_FileY; //檔案游標y軸
	int g_iPageCOL; //目前顯示之所在頁數(左右)
	int g_iPageROW;//目前顯示之所在頁數(上下)
};
struct FileStruct list[101]; //預設可開啟100個檔案

void ResetStruct()
{
	list[g_iFileIndex].g_iCurrentRowNum = 22; //目前的的列數
	list[g_iFileIndex].g_iNewRowNum = 22; //新的列數
	list[g_iFileIndex].g_iNewColNum = 0; //新的行數
	list[g_iFileIndex].g_iMaxstringLen = -1;
	list[g_iFileIndex].g_UIX = 0; //視窗游標x軸
	list[g_iFileIndex].g_UIY = 0; //視窗游標y軸
	list[g_iFileIndex].g_FileX = 0; //檔案游標x軸
	list[g_iFileIndex].g_FileY = 0; //檔案游標y軸
	list[g_iFileIndex].g_iPageCOL = 0; //目前顯示之所在頁數(左右)
	list[g_iFileIndex].g_iPageROW = 0;//目前顯示之所在頁數(上下)
}

void main(int argv, char *argc[])
{
	AddArray(AddROWxCOL, 0);
	ResetStruct();
	g_hIn = GetStdHandle(STD_INPUT_HANDLE);
	g_hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	DrewGreenBar();
	WindowSize();
	PrintfInformation();
	//PrintfStartMenu();
	MoveCursor(0, LINES_PER_PAGE);
	
	if (NULL != argc[1])
	{
		strncpy_s(g_Command, 100, argc[1], strlen(argc[1]));
		strncpy_s(list[g_iFileIndex].g_FileName, 100, argc[1], strlen(argc[1]));
		Openfile(g_Command);
		FileToArray();
		//FileToArrayTwo();
		perr(g_Command, WHITE, FileNamebar);
		memset(g_Command, '\0', strlen(g_Command));
		MoveCursor(g_CommandCursorX, g_CommandCursorY);
	}

	while (1)
	{
		KeyDown();
		if (true == g_Quit)
		{
			break;
		}
	}
	//system("Pause");
}



void DrewGreenBar()
{
	COORD coPos = { 0, LINES_PER_PAGE };
	DWORD dwTT;
	//顏色
	FillConsoleOutputAttribute(g_hOut, BACKGROUND_GREEN | 0, BYTES_PER_LINE, coPos, &dwTT);
}

void MoveCursor(int iX, int iY)	//游標移動
{
	COORD coPos = { iX, iY };
	SetConsoleCursorPosition(g_hOut, coPos);
}

void HideCursor(int status) //游標隱藏與顯示
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 10;
	if (1 == status)
	{
		info.bVisible = TRUE;
	}
	else
	{
		info.bVisible = FALSE;
	}
	SetConsoleCursorInfo(consoleHandle, &info);
}

void WindowSize()
{
	DWORD fdwMode;
	COORD dwSize = { BYTES_PER_LINE, 25 }; //set the windows without scroll bar
	SMALL_RECT consoleSize = { 0, 0, BYTES_PER_LINE - 1, 26 - 1 };

	SetConsoleScreenBufferSize(g_hOut, dwSize);
	SetConsoleWindowInfo(g_hOut, 1, &consoleSize);
	fdwMode = ENABLE_WINDOW_INPUT;
	SetConsoleMode(g_hIn, fdwMode);
	SetConsoleTitle("PE2_2020");
}

int KeyDown()  //ReadConsoleInput  讀取按下的按鈕
{
	static DWORD dwCount;
	static INPUT_RECORD input;
	char cKeyin = 0x0f;
	ReadConsoleInput(g_hIn, &input, 1, &dwCount);
	if (!input.Event.KeyEvent.bKeyDown)
	{
		return FALSE;
	}
	else
	{
		if (input.EventType != KEY_EVENT)
		{
			return FALSE;
		}
		cKeyin = input.Event.KeyEvent.uChar.AsciiChar;

		EscKey(input.Event.KeyEvent.wVirtualKeyCode);
		
		if ((LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED) & input.Event.KeyEvent.dwControlKeyState) // 按下Ctrl後功能
		{
			if (UNLOCK == g_Esc)
			{
				CtrlKey(input.Event.KeyEvent.wVirtualKeyCode);
			}
		}
		else if (SHIFT_PRESSED & input.Event.KeyEvent.dwControlKeyState) // 按下Shift後功能
		{
			ShiftKey(input.Event.KeyEvent.wVirtualKeyCode);
		}
		else if ((LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED) & input.Event.KeyEvent.dwControlKeyState) // 按下Alt後功能
		{
			if (UNLOCK == g_Esc)
			{
				AltKey(input.Event.KeyEvent.wVirtualKeyCode);
			}
		}
		else
		{
			if (UNLOCK == g_Esc)
			{
				SingleKey(input.Event.KeyEvent.wVirtualKeyCode);
			}
			else
			{
				EscSingleKey(input.Event.KeyEvent.wVirtualKeyCode);
			}
		}

		return TRUE;
	}
}

void EscKey(WORD wKeyIn)
{
	if ((VK_ESCAPE == wKeyIn)/* && (NULL != list[g_iFileIndex].g_pFile)*/)
	{
		g_Esc = g_Esc * (-1);
		if (LOCK == g_Esc)
		{
			MoveCursor(g_CommandCursorX, g_CommandCursorY); //游標移動到命令列
		}
		else
		{
			MoveCursor(list[g_iFileIndex].g_UIX, list[g_iFileIndex].g_UIY); //游標移動到文章
		}
	}
}

void PrintfStartMenu()
{
	MoveCursor(15,10);
	printf("Open entire file:     e [drive:][path][filename]");
}

void EscSingleKey(WORD wKeyIn)
{
	switch (wKeyIn)
	{
	case VK_LEFT://左
	{
		--g_CommandCursorX;
		break;
	}
	case VK_RIGHT://右
	{
		if (g_CommandCursorX < (strlen(g_Command)))
		{
			++g_CommandCursorX;
		}
		break;
	}
	case VK_BACK: //Back
	{
		if (g_CommandCursorX > 0)
		{
			memmove(g_Command + g_CommandCursorX - 1, g_Command + g_CommandCursorX, strlen(g_Command) - g_CommandCursorX + 1); //+1包括結束字元
			DisplayCommand();
			--g_CommandCursorX;
		}
		break;
	}
	case VK_DELETE: //Del
	{
		memmove(g_Command + g_CommandCursorX, g_Command + g_CommandCursorX + 1, strlen(g_Command) - g_CommandCursorX); //+1包括結束字元
		DisplayCommand();
		break;
	}
	case VK_HOME: //Home
	{
		g_CommandCursorX = 0;
		break;
	}
	case VK_END: //End
	{
		g_CommandCursorX = strlen(g_Command);
		break;
	}
	case VK_RETURN: //Enter
	{
		CommandFeatures();
		break;
	}
	case VK_CAPITAL: //Caps Lock 
	{
		g_iCapsLock = (g_iCapsLock * (-1));
		break;
	}
	case VK_F4: //F4 Quits current file
	{
		strcpy_s(g_Command, strlen("Quit") + 1, "Quit");
		DisplayCommand();
		g_CommandCursorX = strlen(g_Command);
		break;
	}
	case VK_F2: //F2 Saves current file
	{
		strcpy_s(g_Command, strlen("Save") + 1, "Save");
		DisplayCommand();
		g_CommandCursorX = strlen(g_Command);
		break;
	}
	case VK_F3: //F3 Saves and quits file
	{
		strcpy_s(g_Command, strlen("Save and Quit") + 1, "Save and Quit");
		DisplayCommand();
		g_CommandCursorX = strlen(g_Command);
		break;
	}
	case VK_F1: //偷懶用
	{
		strcpy_s(g_Command, strlen("e C:\\Users\\F62R\\Desktop\\test.txt") + 1, "e C:\\Users\\F62R\\Desktop\\test.txt");
		DisplayCommand();
		g_CommandCursorX = strlen(g_Command);
		break;
	}
	case VK_F7: //偷懶用
	{
		strcpy_s(g_Command, strlen("e G:\\test.txt") + 1, "e G:\\test.txt");
		DisplayCommand();
		g_CommandCursorX = strlen(g_Command);
		break;
	}
	default:
	{
		if (strlen(g_Command) < (COL - 1))
		{
			DrawKeyDown(wKeyIn);
		}
		break;
	}
	}
	if (g_CommandCursorX < 0)
	{
		g_CommandCursorX = 0;
	}
	if (VK_RETURN != wKeyIn)
	{
		MoveCursor(g_CommandCursorX, g_CommandCursorY);
	}
}

bool StringSearch(int iIndexX, int iIndexY)
{
	int iFind = 1;
	while ((iFind != 0) && (iIndexY <= list[g_iFileIndex].g_iMaxstringLen))
	{
		if (list[g_iFileIndex].g_stringLen[iIndexY] == iIndexX)
		{
			iIndexY++;
			iIndexX = 0;
		}
		else
		{
			iFind = memcmp(list[g_iFileIndex].g_ptr[iIndexY] + iIndexX, g_SearchData, strlen(g_SearchData));
			if (0 == iFind)
			{
				list[g_iFileIndex].g_FileX = iIndexX;
				list[g_iFileIndex].g_FileY = iIndexY;
				perr("Found", RED, Messagebar);
				return true;
			}
			else
			{
				iIndexX++;
			}
		}
	}
	if (list[g_iFileIndex].g_iMaxstringLen < iIndexY)
	{
		perr("Search pattern not found", RED, Messagebar);
		return false;
	}
}

void CommandFeatures()
{
	char token[100];
	int len = 0;
	char ch;
	bool bOK;
	len = strlen(g_Command);
	
	if (0 == len)
	{
		perr("Error: no such command", RED, Messagebar);
	}
	else if (0 == strcmp("Save and Quit", g_Command)) //Save and Quit
	{
		if (0 == strlen(list[g_iFileIndex].g_FileName))
		{
			perr("Error: please new file", RED, Messagebar);
			ClearCommand();
			return;
		}
		ArrayToFile();
		Quit();
	}
	else if (0 == strcmp("Save", g_Command)) //Save
	{
		if (0 == strlen(list[g_iFileIndex].g_FileName))
		{
			perr("Error: please new file", RED, Messagebar);
			ClearCommand();
			return;
		}
		ArrayToFile();
		perr("Save succeed", RED, Messagebar);
	}
	else if (0 == strcmp("Quit", g_Command)) //Quit
	{
		Quit();
	}
	else if (0 == strcmp("e", g_Command)) //File Next Page
	{
		g_iFileIndex++;
		//if (NULL == list[g_iFileIndex].g_pFile)
		if (g_iOpenFileNum == g_iFileIndex)
		{
			g_iFileIndex = 0;
		}
		perr(list[g_iFileIndex].g_NewFileName, WHITE, FileNamebar);
		perr("Next Page", RED, Messagebar);
		DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
		PrintfInformation();
	}
	else if ('/' == g_Command[0])
	{
		memset(g_SearchData, '\0', strlen(g_SearchData));
		memset(g_ReplaceData, '\0', strlen(g_ReplaceData));
		strncpy_s(g_SearchData, 81, g_Command + 1, strlen(g_Command) - 1);
		g_SearchData[strlen(g_Command) - 1] = '\0';
		StringSearch(list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_FileY);
		TurnPage((list[g_iFileIndex].g_FileX / COL) * COL, (list[g_iFileIndex].g_FileY / ROW) * ROW, 0, Search);
		ClearCommand();
		g_Esc = UNLOCK;
		PrintfInformation();
		return;
	}
	else if (len > 0)
	{
		len = PopToken(token, sizeof(token), g_Command, " ");
		if (len == 1)
		{
			ch = token[0];

			if ('e' == ch) //Edit new or exist file
			{
				if (0 == strcmp(g_Command, list[g_iFileIndex].g_NewFileName)) //一樣的檔案目前已經被開啟
				{
					perr("File already exist", RED, Messagebar);
				}
				else
				{
					int i = 0;
					while (1) //判斷檔案是否曾經被開啟
					{
						if (0 == strcmp(g_Command, list[i].g_NewFileName))
						{
							g_iFileIndex = i;
							DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
							perr("File already exist", RED, Messagebar);
							perr(list[g_iFileIndex].g_NewFileName, WHITE, FileNamebar);
							PrintfInformation();
							break;
						}
						if (NULL == list[i].g_ptr)
						{
							break;
						}
						i++;
					}
				}
				//else if ((0 != list[g_iFileIndex].g_iMaxstringLen) && (0 != list[g_iFileIndex].g_stringLen)) //當已有檔案正被開啟
				//{
				//	ResetArray(); //陣列重新初始化
				//	ResetMarkCoordinate(); //Mark重新初始化
				//}

				if (0 != strcmp(g_Command, list[g_iFileIndex].g_FileName))
				{
					if ((NULL != list[g_iFileIndex].g_pFile) || (-1 != list[g_iFileIndex].g_iMaxstringLen))
					{
						g_iFileIndex = g_iOpenFileNum;
						if (100 == g_iFileIndex)
						{
							perr("Already open maximum number of files", RED, Messagebar);
							ClearCommand();
							return;
						}
						g_iOpenFileNum++;
					}
					ResetStruct();
					if (NULL == list[g_iFileIndex].g_ptr)
					{
						AddArray(AddROWxCOL, 0);
					}

					bOK = Openfile(g_Command);
					strncpy_s(list[g_iFileIndex].g_FileName, 81, g_Command, strlen(g_Command));
					strncpy_s(list[g_iFileIndex].g_NewFileName, 81, g_Command, strlen(g_Command));
					if (true == bOK) //檔案存在時，檔案存陣列
					{
						FileToArray(); //檔案存陣列
						PrintfInformation();
					}
					else
					{
						DisplayArray(0, 0, DisplayAll);
						PrintfInformation();
					}
					perr(g_Command, WHITE, FileNamebar);
				}
			}
			else if ('n' == ch) //Assign file name for current edit
			{
				strncpy_s(list[g_iFileIndex].g_NewFileName, 81, g_Command, strlen(g_Command));
				perr(g_Command, WHITE, FileNamebar);

				if (0 == strlen(list[g_iFileIndex].g_FileName))
				{
					strncpy_s(list[g_iFileIndex].g_FileName, 81, g_Command, strlen(g_Command));
					perr("Name file succeed", RED, Messagebar);
				}
				else
				{
					perr("Rename succeed", RED, Messagebar);
				}
			}
			else if ('c' == ch)
			{
				if ('/' == g_Command[strlen(g_Command) - 1])
				{
					memset(g_SearchData, '\0', strlen(g_SearchData));
					memset(g_ReplaceData, '\0', strlen(g_ReplaceData));
					len = PopToken(g_SearchData, sizeof(g_SearchData), g_Command, "/");
					len = PopToken(g_ReplaceData, sizeof(g_ReplaceData), g_Command, "/");
					if ((0 == strlen(g_SearchData)) || (0 == strlen(g_ReplaceData)))
					{
						perr("Error: syntax error", RED, Messagebar);
					}
					else
					{
						perr("Pattern has been set", RED, Messagebar);
					}
					ClearCommand();
					g_Esc = UNLOCK;
					//PrintfInformation();
					CtrlKey(VK_RETURN);
					return;
				}
				else
				{
					perr("Error: syntax error", RED, Messagebar);
				}
			}
			else
			{
				perr("Error: no such command", RED, Messagebar);
			}
		}
		else
		{
			perr("Error: no such command", RED, Messagebar);
		}
	}
	else
	{
		perr("Error: no such command", RED, Messagebar);
	}
	ClearCommand();
}

void ClearCommand()
{
	memset(g_Command, '\0', strlen(g_Command)); //清除g_Command內容
	DisplayCommand();
	g_CommandCursorX = 0;
}

void Quit()
{
	if (NULL != list[g_iFileIndex].g_pFile)
	{
		fclose(list[g_iFileIndex].g_pFile);
	}
	if (g_iFileIndex == g_MarkFileNum)
	{
		FileMoveTo101();
	}
	else
	{
		for (int i = 0; i < list[g_iFileIndex].g_iCurrentRowNum; i++)
		{
			free(list[g_iFileIndex].g_ptr[i]);
		}
		free(list[g_iFileIndex].g_ptr);
		list[g_iFileIndex].g_ptr = NULL;
		free(list[g_iFileIndex].g_stringLen);
		free(list[g_iFileIndex].g_arrayLen);
	}
	memset(list[g_iFileIndex].g_FileName, '\0', 100);
	if (g_iOpenFileNum == g_iFileIndex + 1)
	{
		g_iFileIndex = 0;
		perr(list[g_iFileIndex].g_FileName, WHITE, FileNamebar);
		DisplayArray(0, 0, DisplayAll);
		PrintfInformation();
	}
	else
	{
		FileMove();
	}
	
	if (1 == g_iOpenFileNum)
	{
		g_Quit = true;
	}
	g_iOpenFileNum--;
}
void FileMoveTo101()
{
	list[100].g_ptr = list[g_MarkFileNum].g_ptr;
	list[100].g_arrayLen = list[g_MarkFileNum].g_arrayLen;
	list[100].g_stringLen = list[g_MarkFileNum].g_stringLen;
	list[100].g_iCurrentRowNum = list[g_MarkFileNum].g_iCurrentRowNum;
	list[100].g_iNewRowNum = list[g_MarkFileNum].g_iNewRowNum;
	list[100].g_iNewColNum = list[g_MarkFileNum].g_iNewColNum;
	list[100].g_iMaxstringLen = list[g_MarkFileNum].g_iMaxstringLen;
	list[100].g_iPageCOL = list[g_MarkFileNum].g_iPageCOL;
	list[100].g_iPageROW = list[g_MarkFileNum].g_iPageROW;
	g_MarkFileNum = 100;
}

void FileMove()
{
	for (int i = g_iFileIndex; i < g_iOpenFileNum; i++)
	{
		list[i].g_pFile = list[i + 1].g_pFile;
		list[i].g_ptr = list[i + 1].g_ptr;
		list[i].g_arrayLen = list[i + 1].g_arrayLen;
		list[i].g_stringLen = list[i + 1].g_stringLen;
		list[i].g_iCurrentRowNum = list[i + 1].g_iCurrentRowNum;
		list[i].g_iNewRowNum = list[i + 1].g_iNewRowNum;
		list[i].g_iNewColNum = list[i + 1].g_iNewColNum;
		list[i].g_iMaxstringLen = list[i + 1].g_iMaxstringLen;
		list[i].g_UIX = list[i + 1].g_UIX;
		list[i].g_UIY = list[i + 1].g_UIY;
		list[i].g_FileX = list[i + 1].g_FileX;
		list[i].g_FileY = list[i + 1].g_FileY;
		list[i].g_iPageCOL = list[i + 1].g_iPageCOL;
		list[i].g_iPageROW = list[i + 1].g_iPageROW;
		strncpy_s(list[i].g_FileName, 100, list[i + 1].g_FileName, 100);
	}
	perr(list[g_iFileIndex].g_FileName, WHITE, FileNamebar);
	DisplayArray(0, 0, DisplayAll);
	PrintfInformation();
}

void ResetArray()
{
	fclose(list[g_iFileIndex].g_pFile);
	for (int i = 0; i < list[g_iFileIndex].g_iCurrentRowNum; i++)
	{
		free(list[g_iFileIndex].g_ptr[i]);
	}
	free(list[g_iFileIndex].g_ptr);
	free(list[g_iFileIndex].g_stringLen);
	free(list[g_iFileIndex].g_arrayLen);
	AddArray(AddROWxCOL, 0);
	list[g_iFileIndex].g_iCurrentRowNum = 22; //目前的的列數
	list[g_iFileIndex].g_iNewRowNum = 22; //新的列數
	list[g_iFileIndex].g_iNewColNum = 0; //新的行數
	list[g_iFileIndex].g_iMaxstringLen = 0; //計算最後有值列數
}

void FileToArray() //從stringLen長度開始
{
	list[g_iFileIndex].g_FileY = 0;
	int index = 0;
	int FinalStringLen = 0;
	char* PerLineString;
	char* PerLineStringLast;
	char* ptrNext = NULL;
	PerLineString = (char*)malloc(COL);

	while (1)
	{
		memset(PerLineString, '\0', COL);
		fread(PerLineString, sizeof(char), COL, list[g_iFileIndex].g_pFile);
		ptrNext = PerLineString - 1;//
		PerLineStringLast = PerLineString - 1;
		while (1)
		{
			ptrNext = strchr(ptrNext + 1, '\n');
			if (NULL != ptrNext)
			{
				list[g_iFileIndex].g_FileX = index + ptrNext - PerLineStringLast - 1;
				
				if (list[g_iFileIndex].g_iCurrentRowNum <= list[g_iFileIndex].g_FileY)
				{
					AddArray(AddROW, 0);
				}
				if (list[g_iFileIndex].g_FileX > COL - 1)
				{
					AddArray(AddCOL, 0);
				}
			}
			if (NULL != ptrNext)
			{
				memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + index, PerLineStringLast + 1, ptrNext - PerLineStringLast - 1);
				list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = index + ptrNext - PerLineStringLast - 1;

				PerLineStringLast = ptrNext;
				list[g_iFileIndex].g_FileY++;
				index = 0;
			}
			if (NULL == ptrNext)
			{
				int iFinalLen = 0;
				int iCopyLen = 0;
				iFinalLen = index;
				for (int i = 1; i <= PerLineString + COL - PerLineStringLast - 1; i++)
				{
					if (((*(PerLineStringLast + i) > 19) && (*(PerLineStringLast + i) < 127)) || (9 == *(PerLineStringLast + i)))
					{
						iFinalLen++;
						iCopyLen++;
					}
				}
				list[g_iFileIndex].g_FileX = iFinalLen;
				if (list[g_iFileIndex].g_iCurrentRowNum <= list[g_iFileIndex].g_FileY)
				{
					AddArray(AddROW, 0);
				}
				if (list[g_iFileIndex].g_FileX > COL - 1)
				{
					AddArray(AddCOL, 0);
				}
				memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + index, PerLineStringLast + 1, iCopyLen);
				list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = iFinalLen;
				index = list[g_iFileIndex].g_FileX;
				break;
			}
		}
		if (feof(list[g_iFileIndex].g_pFile))
		{
			list[g_iFileIndex].g_iMaxstringLen = list[g_iFileIndex].g_FileY;
			break;
		}
	}
	
	free(PerLineString);
	fseek(list[g_iFileIndex].g_pFile, 0, SEEK_SET);
	Tab();
	list[g_iFileIndex].g_FileX = 0;
	list[g_iFileIndex].g_FileY = 0;	
	DisplayArray(list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_FileY, DisplayAll);
}

void Tab()
{
	for (list[g_iFileIndex].g_FileY = 0; list[g_iFileIndex].g_FileY <= list[g_iFileIndex].g_iMaxstringLen; list[g_iFileIndex].g_FileY++)
	{
		char* pszTab = NULL;
		char* pszStart = NULL;
		pszTab = list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] - 1;
		pszStart = list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY];

		while (1)
		{
			pszTab = strchr(pszTab + 1, '\t');
			if (NULL != pszTab)
			{
				int len = pszTab - pszStart;
				//更新字串長度
				list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] + 4;

				//陣列增加
				if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY])
				{
					list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] / COL) * COL;
					if (0 != (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] % COL))
					{
						list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] + COL;
					}
					AddArray(AddCOL, Alt);
					pszStart = list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY];
					pszTab = pszStart + len;
				}

				//字串搬移
				memmove(pszTab + 1 + 4, pszTab + 1, list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - 4) - (pszTab + 1));

				//貼上空白
				memcpy(pszTab, "					", strlen("					"));
				//list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][(pszTab - pszStart) + 4] = '\t';
				pszTab = pszTab + 4;
			}

			if (NULL == pszTab)
			{
				break;
			}
		}
	}
}

void FileToArrayTwo()
{
	unsigned char* PerLineString;
	list[g_iFileIndex].g_FileY = 0;
	fseek(list[g_iFileIndex].g_pFile, 0, SEEK_END);
	fputc('\n', list[g_iFileIndex].g_pFile);
	fflush(list[g_iFileIndex].g_pFile);
	g_lfilesize = ftell(list[g_iFileIndex].g_pFile);
	PerLineString = (char*)malloc(g_lfilesize); //創動態記憶體存檔案每行字串
	fseek(list[g_iFileIndex].g_pFile, 0, SEEK_SET);
	
	while (NULL != fgets(PerLineString, g_lfilesize, list[g_iFileIndex].g_pFile)) //檔案逐行複製字串至PerLineStringString
	{
		list[g_iFileIndex].g_FileX = strlen(PerLineString) - 1;
		if (list[g_iFileIndex].g_iCurrentRowNum <= list[g_iFileIndex].g_FileY)
		{
			AddArray(AddROW, 0);
		}
		if (list[g_iFileIndex].g_FileX > COL/* - 1*/)
		{
			AddArray(AddCOL, 0);
		}
		memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY], PerLineString, strlen(PerLineString)-1);
		list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = strlen(PerLineString) - 1;
		list[g_iFileIndex].g_iMaxstringLen = list[g_iFileIndex].g_FileY;
		list[g_iFileIndex].g_FileY++;
	}

	//list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_iMaxstringLen]++;
	ArrayToFile();
	free(PerLineString);
	fseek(list[g_iFileIndex].g_pFile, 0, SEEK_SET);
	list[g_iFileIndex].g_FileX = 0;
	list[g_iFileIndex].g_FileY = 0;
	DisplayArray(list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_FileY, DisplayAll);
}

void ArrayToFile()
{
	if (NULL != list[g_iFileIndex].g_pFile)
	{
		fclose(list[g_iFileIndex].g_pFile);
	}
	errno_t iRet;
	iRet = fopen_s(&list[g_iFileIndex].g_pFile, list[g_iFileIndex].g_FileName, "w");
	if (0 == iRet)
	{
		/*for (int i = 0; i < list[g_iFileIndex].g_iMaxstringLen; i++)
		{
			fwrite(list[g_iFileIndex].g_ptr[i], 1, list[g_iFileIndex].g_stringLen[i], list[g_iFileIndex].g_pFile);
			fputc('\n', list[g_iFileIndex].g_pFile);
		}*/

		for (int y = 0; y < list[g_iFileIndex].g_iMaxstringLen; y++)
		{
			for (int x = 0; x < list[g_iFileIndex].g_stringLen[y]; x++)
			{
				fputc(list[g_iFileIndex].g_ptr[y][x], list[g_iFileIndex].g_pFile);
				if ('\t' == list[g_iFileIndex].g_ptr[y][x])
				{
					x = x + 4;
				}
			}
			fputc('\n', list[g_iFileIndex].g_pFile);
		}
		for (int x = 0; x < list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_iMaxstringLen]; x++)
		{
			fputc(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_iMaxstringLen][x], list[g_iFileIndex].g_pFile);
			if ('\t' == list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_iMaxstringLen][x])
			{
				x = x + 4;
			}
		}

		//fwrite(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_iMaxstringLen], 1, list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_iMaxstringLen], list[g_iFileIndex].g_pFile);
		if (0 != strcmp(list[g_iFileIndex].g_FileName, list[g_iFileIndex].g_NewFileName))
		{
			fclose(list[g_iFileIndex].g_pFile);
			if (0 == rename(list[g_iFileIndex].g_FileName, list[g_iFileIndex].g_NewFileName))
			{
				strncpy_s(list[g_iFileIndex].g_FileName, 81, g_Command, strlen(g_Command));
			}
		}
		fflush(list[g_iFileIndex].g_pFile);
		return;
	}
	else
	{
		perr("Error: save failure", RED, Messagebar);
		return false;
	}
}

bool Openfile(char* filename)
{
	errno_t iRet;
	iRet = fopen_s(&list[g_iFileIndex].g_pFile, filename, "r+");
	if (0 == iRet)
	{
		perr("Read file succeed", RED, Messagebar);
		return true;
	}
	else
	{
		iRet = fopen_s(&list[g_iFileIndex].g_pFile, filename, "w+");
		if (0 == iRet)
		{
			fclose(list[g_iFileIndex].g_pFile);
			iRet = remove(filename);
			if (0 == iRet)
			{
				perr("New file succeed", RED, Messagebar);
			}
			//DisplayArray(list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_FileY, DisplayAll);
			return false;
		}
		else
		{
			perr("Error: Read file error", RED, Messagebar);
			return false;
		}
	}
}

int PopToken(char* pszToken, int iTokenBufLen, char* pszData, char* pszKeyword)
{
	char* pszTokenCut;
	char* pszNext;

	pszTokenCut = strtok_s(pszData, pszKeyword, &pszNext);
	if (NULL == pszTokenCut)
	{
		return -1;
	}
	else
	{
		strcpy_s(pszToken, iTokenBufLen, pszTokenCut);
		memmove(pszData, pszNext, strlen(pszNext) + 1); //因分段字元所以+1
		return strlen(pszToken);
	}
}

void UpdatePage(int status)
{
	//頁數更新
	if ((Right == status) || (Keydown == status)) //向右翻頁1	//輸入過程中向右翻頁2
	{
		list[g_iFileIndex].g_iPageCOL++;
	}
	else if (Left == status) //向左翻頁
	{
		list[g_iFileIndex].g_iPageCOL--;
	}
	else if (Down == status) //向下翻頁
	{
		list[g_iFileIndex].g_iPageROW++;
	}
	else if (Up == status) //向上翻頁
	{
		list[g_iFileIndex].g_iPageROW--;
	}
	else if (Home == status) //Home鍵
	{
		list[g_iFileIndex].g_iPageCOL = 0;
	}
	else if (Search == status) //Search
	{
		list[g_iFileIndex].g_iPageCOL = (list[g_iFileIndex].g_FileX / COL) * 2;
		list[g_iFileIndex].g_iPageROW = (list[g_iFileIndex].g_FileY / ROW);
	}
}

void DisplayArray(int iFirst, int iHight, char status) //iFirst橫向開始列印位置(左側開始位置)  //iHight縱向開始列印位置(高度開始位置)
{
	HideCursor(0);
	int x = 0;
	int y = 0;
	int index = 0;

	switch (status)
	{
	case DisplayAll: //整頁更新
	{
		for (y = 0; y < ROW; y++)
		{
			index = 0;
			MoveCursor(0, y);
			for (x = iFirst; x < iFirst + COL; x++)
			{
				if (y + iHight >= list[g_iFileIndex].g_iCurrentRowNum)
				{
					printf(" ");
				}
				else if (list[g_iFileIndex].g_stringLen[y + iHight] > (list[g_iFileIndex].g_iPageCOL * (COL / 2)) + index)
				{
					if ((list[g_iFileIndex].g_ptr[y + iHight][x] > 32) && (list[g_iFileIndex].g_ptr[y + iHight][x] < 127))
					{
						printf("%c", list[g_iFileIndex].g_ptr[y + iHight][x]);
					}
					else
					{
						printf(" ");
					}
				}
				else
				{
					printf(" ");
				}

				if (g_iFileIndex == g_MarkFileNum)
				{
					DisplayMark(x, index, y, iHight, DisplayAll);
				}

				index++;
			}
		}
		break;
	}
	case DisplayLine: //單列更新
	{
		MoveCursor(0, list[g_iFileIndex].g_UIY);
		for (x = iFirst; x < iFirst + COL; x++)
		{
			if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > (list[g_iFileIndex].g_iPageCOL * (COL / 2)) + index)
			{
				if ((list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][x] > 32) && (list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][x] < 127))
				{
					printf("%c", list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][x]);
				}
				else
				{
					printf(" ");
				}
			}
			else
			{
				printf(" ");
			}

			if (g_iFileIndex == g_MarkFileNum)
			{
				DisplayMark(x, index, y, iHight, DisplayLine);
			}

			index++;
		}
		break;
	}
	}
	HideCursor(1);
}

void DisplayMark(x, index, y, iHight, status)
{
	switch (status)
	{
	case DisplayAll:
	{
		if (Alt_C == g_Markcase)
		{
			if (((y + iHight >= g_Mark[0][1]) && (y + iHight <= g_Mark[1][1])) && (x < list[g_iFileIndex].g_stringLen[y + iHight]))
			{
				COORD coPos = { index, y };
				DWORD dwTT;
				//顏色
				if (g_Mark[0][1] == g_Mark[1][1])
				{
					if ((x >= g_Mark[0][0]) && (x <= g_Mark[1][0]))
					{
						FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
					}
				}
				else if ((y + iHight == g_Mark[0][1]) && (x >= g_Mark[0][0]))
				{
					FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
				}
				else if ((y + iHight == g_Mark[1][1]) && (x < g_Mark[1][0]))
				{
					FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
				}
				else if ((y + iHight > g_Mark[0][1]) && (y + iHight < g_Mark[1][1]))
				{
					FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
				}
			}
		}
		else
		{
			if (((x >= g_Mark[0][0]) && (x <= g_Mark[1][0])) && ((y + iHight >= g_Mark[0][1]) && (y + iHight <= g_Mark[1][1])))
			{
				COORD coPos = { index, y };
				DWORD dwTT;
				//顏色
				FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
			}
		}
		break;
	}
	case DisplayLine:
	{
		if (Alt_C == g_Markcase)
		{
			if (((list[g_iFileIndex].g_FileY >= g_Mark[0][1]) && (list[g_iFileIndex].g_FileY <= g_Mark[1][1])) && (x < list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY]))
			{
				COORD coPos = { index, list[g_iFileIndex].g_FileY };
				DWORD dwTT;
				//顏色
				if (g_Mark[0][1] == g_Mark[1][1])
				{
					if ((x >= g_Mark[0][0]) && (x <= g_Mark[1][0]))
					{
						FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
					}
				}
				else if ((list[g_iFileIndex].g_FileY == g_Mark[0][1]) && (x >= g_Mark[0][0]))
				{
					FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
				}
				else if ((list[g_iFileIndex].g_FileY == g_Mark[1][1]) && (x < g_Mark[1][0]))
				{
					FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
				}
				else if ((list[g_iFileIndex].g_FileY > g_Mark[0][1]) && (list[g_iFileIndex].g_FileY < g_Mark[1][1]))
				{
					FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
				}
			}
		}
		else
		{
			if (((x >= g_Mark[0][0]) && (x <= g_Mark[1][0])) && ((list[g_iFileIndex].g_FileY >= g_Mark[0][1]) && (list[g_iFileIndex].g_FileY <= g_Mark[1][1])))
			{
				COORD coPos = { index, list[g_iFileIndex].g_FileY };
				DWORD dwTT;
				//顏色
				FillConsoleOutputAttribute(g_hOut, BACKGROUND_RED | BACKGROUND_INTENSITY | 0, 1, coPos, &dwTT);
			}
		}
	}
	}
	
}

void TurnPage(int iFirst, int iHight, int stringLeniCursorY, char status) //換頁
{
	UpdatePage(status); //更新頁數

	DisplayArray(iFirst, iHight, DisplayAll); //顯示檔案內容
}

void SingleKey(WORD wKeyIn)
{
	switch (wKeyIn)
	{
	case VK_UP://上
	{
		list[g_iFileIndex].g_FileY--;
		list[g_iFileIndex].g_UIY--;
		if (-1 != list[g_iFileIndex].g_UIY)
		{
			if ('\t' == list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX])
			{
				while (1)
				{
					if ('\t' != list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX - 1])
					{
						break;
					}
					if ('\t' != list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX])
					{
						break;
					}
					list[g_iFileIndex].g_FileX++;
					list[g_iFileIndex].g_UIX++;
					if (COL == list[g_iFileIndex].g_UIX)
					{
						TurnPage(list[g_iFileIndex].g_FileX - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Right);
					}
				}
			}
		}
		
		if ((-1 == list[g_iFileIndex].g_UIY) && (0 != list[g_iFileIndex].g_iPageROW))
		{
			TurnPage(list[g_iFileIndex].g_iPageCOL * (COL / 2), (list[g_iFileIndex].g_FileY + 1) - ROW, 0, Up);
		}
		break;
	}
	case VK_DOWN://下
	{
		if (list[g_iFileIndex].g_iMaxstringLen != list[g_iFileIndex].g_FileY)
		{
			list[g_iFileIndex].g_FileY++;
			list[g_iFileIndex].g_UIY++;
			if ('\t' == list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX])
			{
				while (1)
				{
					if ('\t' != list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX - 1])
					{
						break;
					}
					if ('\t' != list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX])
					{
						break;
					}
					list[g_iFileIndex].g_FileX++;
					list[g_iFileIndex].g_UIX++;
					if (COL == list[g_iFileIndex].g_UIX)
					{
						TurnPage(list[g_iFileIndex].g_FileX - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Right);
					}
				}
			}
			if (ROW == list[g_iFileIndex].g_UIY)
			{
				TurnPage(list[g_iFileIndex].g_iPageCOL * (COL / 2), list[g_iFileIndex].g_FileY, 0, Down);
			}
		}
		break;
	}
	case VK_LEFT://左
	{
		list[g_iFileIndex].g_FileX--;
		list[g_iFileIndex].g_UIX--;
		if ('\t' == list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX])
		{
			for (int i = 1; i < TabSpaceLen; i++)
			{
				list[g_iFileIndex].g_FileX--;
				list[g_iFileIndex].g_UIX--;
				if ((-1 == list[g_iFileIndex].g_UIX) && (0 != list[g_iFileIndex].g_iPageCOL))
				{
					TurnPage((list[g_iFileIndex].g_FileX + 1) - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Left);
				}
			}
		}
		
		if ((-1 == list[g_iFileIndex].g_UIX) && (0 != list[g_iFileIndex].g_iPageCOL))
		{
			TurnPage((list[g_iFileIndex].g_FileX + 1) - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Left);
		}
		break;
	}
	case VK_RIGHT://右
	{
		if ('\t' == list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX])
		{
			for (int i = 1; i < TabSpaceLen; i++)
			{
				list[g_iFileIndex].g_FileX++;
				list[g_iFileIndex].g_UIX++;
				if (COL == list[g_iFileIndex].g_UIX)
				{
					TurnPage(list[g_iFileIndex].g_FileX - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Right);
				}
			}
		}

		list[g_iFileIndex].g_FileX++;
		list[g_iFileIndex].g_UIX++;
		if (COL == list[g_iFileIndex].g_UIX)
		{
			TurnPage(list[g_iFileIndex].g_FileX - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Right);
		}
		break;
	}
	case VK_HOME: //Home
	{
		TurnPage(0, (list[g_iFileIndex].g_iPageROW * ROW), 0, Home);
		list[g_iFileIndex].g_FileX = 0;
		break;
	}
	case VK_END: //End
	{
		int istringLen;
		if (list[g_iFileIndex].g_FileY > list[g_iFileIndex].g_iMaxstringLen)
		{
			istringLen = 0;
		}
		else
		{
			istringLen = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY];
		}
		list[g_iFileIndex].g_iPageCOL = (istringLen / COL) * 2;
		TurnPage(list[g_iFileIndex].g_iPageCOL * (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, 0);
		list[g_iFileIndex].g_FileX = istringLen;
		break;
	}
	case VK_PRIOR: //page up
	{
		if (0 != list[g_iFileIndex].g_iPageROW)
		{
			TurnPage((list[g_iFileIndex].g_iPageCOL * (COL / 2)), (list[g_iFileIndex].g_iPageROW * ROW) - ROW, 0, Up);
			list[g_iFileIndex].g_FileY = list[g_iFileIndex].g_FileY - ROW;
		}
		break;
	}
	case VK_NEXT: //page down
	{
		if ((list[g_iFileIndex].g_iPageROW + 1) * ROW <= list[g_iFileIndex].g_iMaxstringLen)
		{
			TurnPage((list[g_iFileIndex].g_iPageCOL * (COL / 2)), (list[g_iFileIndex].g_iPageROW + 1) * ROW, 0, Down);
			list[g_iFileIndex].g_FileY = list[g_iFileIndex].g_FileY + ROW;
		}
		if (list[g_iFileIndex].g_FileY > list[g_iFileIndex].g_iMaxstringLen)
		{
			list[g_iFileIndex].g_FileY = list[g_iFileIndex].g_iMaxstringLen;
		}
		break;
	}
	case VK_CAPITAL: //Caps Lock 
	{
		g_iCapsLock = (g_iCapsLock * (-1));
		break;
	}
	case VK_INSERT: //Insert
	{
		g_iInsert = g_iInsert * (-1);
		if (LOCK == g_iInsert)
		{
			g_szToggleMode = "Insert";
		}
		else
		{
			g_szToggleMode = "Replace";
		}
		break;
	}
	case VK_F5: //F5 Erases contents of line
	{
		free(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY]);
		TurnPage(0, list[g_iFileIndex].g_iPageROW * ROW, 0, Home);
		list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] = (char*)malloc(sizeof(char) * COL);
		list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = 0;
		list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = COL;
		list[g_iFileIndex].g_FileX = 0;
		break;
	}
	case VK_F6: //F6 Erases to end of line
	{
		for (int i = list[g_iFileIndex].g_FileX; i <= list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY]; i++)
		{
			list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][i] = '0x00';
		}

		list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX;
		DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageCOL * ROW, DisplayLine);
		break;
	}
	case VK_BACK: //Back
	{
		if ('\t' == list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX - 1])
		{
			for (int i = 0; i < TabSpaceLen; i++)
			{
				list[g_iFileIndex].g_UIX--;
				if ((-1 == list[g_iFileIndex].g_UIX) && (0 != list[g_iFileIndex].g_iPageCOL))
				{
					TurnPage((list[g_iFileIndex].g_FileX + 1) - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Left);
				}
				StringMove(Back);
			}
		}
		else
		{
			StringMove(Back);
		}
		DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
		break;
	}
	case VK_DELETE: //Delete
	{
		if ('\t' == list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX])
		{
			for (int i = 0; i < TabSpaceLen; i++)
			{
				StringMove(Delete);
			}
		}
		else
		{
			StringMove(Delete);
		}
		
		DisplayArray((list[g_iFileIndex].g_iPageCOL* (COL / 2)), list[g_iFileIndex].g_iPageROW* ROW, DisplayAll);
		break;
	}
	case VK_RETURN: //Enter
	{
		StringMove(Enter);
		DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
		break;
	}
	default:
		if (TAB == wKeyIn)
		{
			for (int i = 0; i < TabSpaceLen; i++)
			{
				DrawKeyDown(wKeyIn);
			}
			break;
		}
		DrawKeyDown(wKeyIn);
		break;
	}
	if (list[g_iFileIndex].g_FileX < 0)
	{
		list[g_iFileIndex].g_FileX = 0;
	}
	if (list[g_iFileIndex].g_FileY < 0)
	{
		list[g_iFileIndex].g_FileY = 0;
	}
	PrintfInformation();
}

void AddArray(char addmode, char status)
{
	int i = 0;
	switch (addmode)
	{
	case AddROWxCOL: //初始化
	{
		list[g_iFileIndex].g_ptr = (char **)malloc(sizeof(char *) * ROW);
		list[g_iFileIndex].g_arrayLen = (int*)calloc(ROW, sizeof(int)); //長度初始化為0
		list[g_iFileIndex].g_stringLen = (int*)calloc(ROW, sizeof(int)); //陣列每行字串長度

		for (int i = 0; i < ROW; i++)
		{
			list[g_iFileIndex].g_ptr[i] = (char *)malloc(sizeof(char) * COL);
			list[g_iFileIndex].g_arrayLen[i] = COL;
		}
		break;
	}
	case AddROW: //向下增加
	{
		switch (status)
		{
		case Alt:
		{
			break;
		}
		case Enter:
		{
			list[g_iFileIndex].g_iNewRowNum = ROW + (list[g_iFileIndex].g_FileY / ROW) * ROW;
			break;
		}
		default:
		{
			list[g_iFileIndex].g_iNewRowNum = list[g_iFileIndex].g_iNewRowNum + ROW;
			break;
		}
		}
		list[g_iFileIndex].g_arrayLen = (int *)realloc(list[g_iFileIndex].g_arrayLen, sizeof(int) * list[g_iFileIndex].g_iNewRowNum); //每列陣列長度(頁數)
		list[g_iFileIndex].g_stringLen = (int *)realloc(list[g_iFileIndex].g_stringLen, sizeof(int) * list[g_iFileIndex].g_iNewRowNum); //每列陣列字串長度

		list[g_iFileIndex].g_ptr = (char **)realloc(list[g_iFileIndex].g_ptr, sizeof(char *) * list[g_iFileIndex].g_iNewRowNum);

		for (i = list[g_iFileIndex].g_iCurrentRowNum; i < list[g_iFileIndex].g_iNewRowNum; i++) // 長度初始化為0;
		{
			list[g_iFileIndex].g_arrayLen[i] = COL;
			list[g_iFileIndex].g_stringLen[i] = 0;
			list[g_iFileIndex].g_ptr[i] = (char *)malloc(sizeof(char) * COL);
		}

		list[g_iFileIndex].g_iCurrentRowNum = list[g_iFileIndex].g_iNewRowNum;
		OutputDebugString("A");
		break;
	}
	case AddCOL: //向右增加
	{
		switch (status)
		{
		case Alt:
		{
			list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] = (char *)realloc(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY], sizeof(char) * list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY]);
			break;
		}
		default:
		{
			if (list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] < list[g_iFileIndex].g_FileX + 1)
			{
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = COL + (list[g_iFileIndex].g_FileX / COL) * COL;
			}
			else
			{
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] + COL;
			}

			list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] = (char *)realloc(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY], sizeof(char) * list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY]);
			OutputDebugString("B");
			break;
		}
		}
		break;
	}
	}
}

void StringMove(char status)
{
	switch (status)
	{
	case Insert: //Insert
	{
		memmove(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX + 1, list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - list[g_iFileIndex].g_FileX);
		list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY]++;
		break;
	}
	case Back: //Back
	{
		if ((0 == list[g_iFileIndex].g_FileX) && (0 == list[g_iFileIndex].g_FileY)) //於原點時直接return
		{
			return;
		}
		if ((0 == list[g_iFileIndex].g_UIX) && (0 != list[g_iFileIndex].g_iPageCOL)) //iCursorX為0時向左翻頁
		{
			TurnPage(list[g_iFileIndex].g_FileX - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Left);
		}
		if (((0 == list[g_iFileIndex].g_UIY) && (0 == list[g_iFileIndex].g_UIX)) && (0 != list[g_iFileIndex].g_iPageROW)) //iCursorY為0時向上翻頁
		{
			SingleKey(VK_UP);
			SingleKey(VK_END);
			if (list[g_iFileIndex].g_iMaxstringLen > list[g_iFileIndex].g_FileY)
			{
				BackToFront(); //陣列由後往前移動
			}
			return;
		}
		if ((0 != list[g_iFileIndex].g_FileX) && (list[g_iFileIndex].g_FileX <= list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY])) //BACK功能
		{
			memmove(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX - 1, list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - list[g_iFileIndex].g_FileX + 1);
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY]--;
			list[g_iFileIndex].g_FileX--;
		}
		else if ((0 == list[g_iFileIndex].g_FileX) && (list[g_iFileIndex].g_iMaxstringLen >= list[g_iFileIndex].g_FileY)) //iCursorX為起點且後面列還有字串
		{
			list[g_iFileIndex].g_FileY--;
			SingleKey(VK_END);
			BackToFront(); //陣列由後往前移動
		}
		else if (0 == list[g_iFileIndex].g_FileX) //iCursorX為起點時，移動到上一行結尾
		{
			list[g_iFileIndex].g_FileY--;
			SingleKey(VK_END);
		}
		else
		{
			list[g_iFileIndex].g_FileX--;
		}
		break;
	}
	case Delete: //Delete
	{
		if ((list[g_iFileIndex].g_iMaxstringLen > list[g_iFileIndex].g_FileY) && (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] <= list[g_iFileIndex].g_FileX))
		{
			BackToFront(); //陣列由後往前移動
		}
		else if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > list[g_iFileIndex].g_FileX)
		{
			memmove(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX + 1, list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - list[g_iFileIndex].g_FileX);
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY]--;
		}
		break;
	}
	case Enter: //Enter
	{
		int iInitialg_FileY = list[g_iFileIndex].g_FileY;
		if (list[g_iFileIndex].g_FileY <= list[g_iFileIndex].g_iMaxstringLen)
		{
			if (list[g_iFileIndex].g_iMaxstringLen + 1 >= list[g_iFileIndex].g_iCurrentRowNum)
			{
				AddArray(AddROW, 0);
			}

			//游標後一行全部往後一行
			for (int i = list[g_iFileIndex].g_iMaxstringLen + 1; i > list[g_iFileIndex].g_FileY + 1; i--)
			{
				list[g_iFileIndex].g_ptr[i] = list[g_iFileIndex].g_ptr[i - 1];
				list[g_iFileIndex].g_stringLen[i] = list[g_iFileIndex].g_stringLen[i - 1];
				list[g_iFileIndex].g_arrayLen[i] = list[g_iFileIndex].g_arrayLen[i - 1];
			}
			//處理list[g_iFileIndex].g_FileY後一行
			list[g_iFileIndex].g_FileY++;
			list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] = COL;
			char* newptr = (char *)malloc(sizeof(char) * COL);
			list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] = newptr;
			int len = list[g_iFileIndex].g_stringLen[iInitialg_FileY] - list[g_iFileIndex].g_FileX;
			if (len < 0)
			{
				len = 0;
			}
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = len;
			if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > COL)
			{
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] / COL) * COL;
				if (0 != (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] % COL))
				{
					list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] + COL;
				}
				AddArray(AddCOL, Alt);
			}
			memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY], list[g_iFileIndex].g_ptr[iInitialg_FileY] + list[g_iFileIndex].g_FileX, len);
			//處理初始list[g_iFileIndex].g_FileY那行
			list[g_iFileIndex].g_FileY = iInitialg_FileY;
			memset(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, ' ', len);
			if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] >= list[g_iFileIndex].g_FileX)
			{
				list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX;
			}
		}
		if (ROW == ((list[g_iFileIndex].g_FileY % ROW) + 1))
		{
			TurnPage((list[g_iFileIndex].g_iPageCOL * (COL / 2)), ++list[g_iFileIndex].g_FileY, 0, Down);
		}
		else
		{
			list[g_iFileIndex].g_FileY++;
		}
		TurnPage(0, list[g_iFileIndex].g_iPageROW * ROW, 0, Home);
		list[g_iFileIndex].g_FileX = 0;
		list[g_iFileIndex].g_iMaxstringLen++;
		break;
	}
	}
}

void BackToFront()
{
	list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX + list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY + 1];
	list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = COL + (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] / COL) * COL;
	list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] = (char*)realloc(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY], sizeof(char) * list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY]);
	//strncpy_s(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY], list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY + 1], list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY + 1]);
	memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY + 1], list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY + 1]);

	for (int i = list[g_iFileIndex].g_FileY + 1; i < list[g_iFileIndex].g_iMaxstringLen; i++)
	{
		list[g_iFileIndex].g_ptr[i] = list[g_iFileIndex].g_ptr[i + 1];
		list[g_iFileIndex].g_stringLen[i] = list[g_iFileIndex].g_stringLen[i + 1];
		list[g_iFileIndex].g_arrayLen[i] = list[g_iFileIndex].g_arrayLen[i + 1];
	}
	list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_iMaxstringLen] = NULL;
	list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_iMaxstringLen] = (char*)malloc(sizeof(char) * COL);
	list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_iMaxstringLen] = 0;
	list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_iMaxstringLen] = COL;
	list[g_iFileIndex].g_iMaxstringLen--;
}

void AltB()
{
	if (-1 == g_Mark[0][0])
	{
		for (int y = 0; y < 2; y++)
		{
			for (int x = 0; x < 2; x++)
			{
				if (0 == x)
				{
					g_Mark[y][x] = list[g_iFileIndex].g_FileX;
				}
				else
				{
					g_Mark[y][x] = list[g_iFileIndex].g_FileY;
				}
			}
		}
	}
	else
	{	
		perr("", RED, Messagebar);
		int preg_Mark[2][2] = { { g_Mark[0][0], g_Mark[0][1] }, { g_Mark[1][0], g_Mark[1][1] } };
		if (list[g_iFileIndex].g_FileX < g_Mark[0][0])
		{
			g_Mark[1][0] = g_Mark[0][0];
			g_Mark[0][0] = list[g_iFileIndex].g_FileX;
		}
		else
		{
			g_Mark[1][0] = list[g_iFileIndex].g_FileX;
		}

		if ((list[g_iFileIndex].g_FileY < g_Mark[0][1]))
		{
			g_Mark[1][1] = g_Mark[0][1];
			g_Mark[0][1] = list[g_iFileIndex].g_FileY;
		}
		else
		{
			g_Mark[1][1] = list[g_iFileIndex].g_FileY;
		}
		for (int y = g_Mark[0][1]; y <= g_Mark[1][1]; y++)
		{
			if ('\t' == list[g_iFileIndex].g_ptr[y][g_Mark[0][0]])
			{
				if ('\t' == list[g_iFileIndex].g_ptr[y][g_Mark[0][0] - 1])
				{
					int i = g_Mark[0][0] - 1;
					int len = 0;
					while (1)
					{
						len++;
						if ('\t' != list[g_iFileIndex].g_ptr[y][i])
						{
							break;
						}
						i--;
					}
					if (0 != len % (TabSpaceLen + 1))
					{
						for (int y = 0; y < 2; y++)
						{
							for (int x = 0; x < 2; x++)
							{
								g_Mark[y][x] = preg_Mark[y][x];
							}
						}
						perr("Error: incomplete block (TAB)", RED, Messagebar);
					}
				}
			}

			if ('\t' == list[g_iFileIndex].g_ptr[y][g_Mark[1][0]])
			{
				if ('\t' == list[g_iFileIndex].g_ptr[y][g_Mark[1][0] + 1])
				{
					int i = g_Mark[1][0] + 1;
					int len = 0;
					while (1)
					{
						len++;
						if ('\t' != list[g_iFileIndex].g_ptr[y][i])
						{
							break;
						}
						i++;
					}
					if (0 != len % (TabSpaceLen + 1))
					{
						for (int y = 0; y < 2; y++)
						{
							for (int x = 0; x < 2; x++)
							{
								g_Mark[y][x] = preg_Mark[y][x];
							}
						}
						perr("Error: incomplete block (TAB)", RED, Messagebar);
					}
				}
			}
		}
	}
}

void AltL()
{
	if (-1 == g_Mark[0][0])
	{
		g_Mark[0][0] = 0;
		g_Mark[0][1] = list[g_iFileIndex].g_FileY;
		g_Mark[1][0] = MaxOfInt;
		g_Mark[1][1] = list[g_iFileIndex].g_FileY;
	}
	else
	{
		if ((list[g_iFileIndex].g_FileY < g_Mark[0][1]))
		{
			g_Mark[1][1] = g_Mark[0][1];
			g_Mark[0][1] = list[g_iFileIndex].g_FileY;
			g_Mark[0][0] = 0;
			g_Mark[1][0] = MaxOfInt;
		}
		else
		{
			g_Mark[1][1] = list[g_iFileIndex].g_FileY;
			g_Mark[0][0] = 0;
			g_Mark[1][0] = MaxOfInt;
		}
	}
}

void AltC()
{
	if (-1 == g_Mark[0][0])
	{
		g_Mark[0][0] = list[g_iFileIndex].g_FileX;
		g_Mark[0][1] = list[g_iFileIndex].g_FileY;
		g_Mark[1][0] = list[g_iFileIndex].g_FileX;
		g_Mark[1][1] = list[g_iFileIndex].g_FileY;
	}
	else
	{
		if (((list[g_iFileIndex].g_FileX < g_Mark[0][0]) && (g_Mark[0][1] == list[g_iFileIndex].g_FileY)) || (list[g_iFileIndex].g_FileY < g_Mark[0][1]))
		{
			g_Mark[1][0] = g_Mark[0][0];
			g_Mark[1][1] = g_Mark[0][1];
			g_Mark[0][0] = list[g_iFileIndex].g_FileX;
			g_Mark[0][1] = list[g_iFileIndex].g_FileY;
		}
		else if ((list[g_iFileIndex].g_FileX > g_Mark[0][0]) || (list[g_iFileIndex].g_FileY > g_Mark[0][1]))
		{
			g_Mark[1][0] = list[g_iFileIndex].g_FileX/* + 1*/;
			g_Mark[1][1] = list[g_iFileIndex].g_FileY;
		}
	}
}

void FillCoordinate(char status)
{
	if (g_iFileIndex != g_MarkFileNum)
	{
		ResetMarkCoordinate();
	}
	g_MarkFileNum = g_iFileIndex;
	switch (status)
	{
	case Alt_B:
	{
		AltB();
		break;
	}
	case Alt_L:
	{
		AltL();
		break;
	}
	case Alt_C:
	{
		AltC();
		break;
	}
	}
}

void Blockmark(char status)
{
	FillCoordinate(status);
	DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
}

void ResetMarkCoordinate()
{
	if (100 == g_MarkFileNum)
	{
		for (int i = 0; i < list[g_MarkFileNum].g_iCurrentRowNum; i++)
		{
			free(list[g_MarkFileNum].g_ptr[i]);
		}
		free(list[g_MarkFileNum].g_ptr);
		list[g_MarkFileNum].g_ptr = NULL;
		free(list[g_MarkFileNum].g_stringLen);
		free(list[g_MarkFileNum].g_arrayLen);
	}
	g_MarkFileNum = -1;
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < 2; x++)
		{
			g_Mark[y][x] = -1;
		}
	}
}

void ArrayCopies()
{
	switch (g_Markcase)
	{
	case Alt_B:
	{
		int iStringLen = 0; 
		int iHigh = 0;
		int iCopylen = 0;
		int iInitialg_FileY = list[g_iFileIndex].g_FileY;
		int prestringLen;
		iStringLen = g_Mark[1][0] - g_Mark[0][0] + 1;
		iHigh = g_Mark[1][1] - g_Mark[0][1] + 1;

		//判斷複製地方是否與被複製字串位置衝突
		if (g_MarkFileNum == g_iFileIndex)
		{
			if (((list[g_iFileIndex].g_FileY > g_Mark[0][1] - iHigh) && (list[g_iFileIndex].g_FileY <= g_Mark[1][1])) && (list[g_iFileIndex].g_FileX <= g_Mark[1][0]))
			{
				perr("Error: source and target conflict", RED, Messagebar);
				return;
			}
		}
		//ROW是否做增加
		if (list[g_iFileIndex].g_iCurrentRowNum <= list[g_iFileIndex].g_FileY + iHigh)
		{
			list[g_iFileIndex].g_iNewRowNum = COL + (((list[g_iFileIndex].g_FileY + iHigh) / COL) * COL);
			AddArray(AddROW, Alt);
			list[g_iFileIndex].g_iMaxstringLen = list[g_iFileIndex].g_FileY + iHigh - 1;
		}
		//原本字串往後位移並於空白處貼上新字串
		for (int index = 0; index < iHigh; index++)
		{
			prestringLen = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY];
			
			if (list[g_iFileIndex].g_FileX >= list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY])
			{
				list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX + iStringLen;
			}
			else
			{
				list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] + iStringLen;
			}

			if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY])
			{
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] / COL) * COL;
				if (0 != (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] % COL))
				{
					list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] + COL;
				}
				AddArray(AddCOL, Alt);
			}
			if (prestringLen < list[g_iFileIndex].g_FileX)
			{
				memset(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + prestringLen, ' ', list[g_iFileIndex].g_FileX - prestringLen);
			}
			if (list[g_iFileIndex].g_FileX < list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - iStringLen)
			{
				memmove(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX + iStringLen, list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - iStringLen - list[g_iFileIndex].g_FileX);
			}

			if (list[g_MarkFileNum].g_stringLen[g_Mark[0][1] + index] < g_Mark[1][0])
			{
				iCopylen = list[g_MarkFileNum].g_stringLen[g_Mark[0][1] + index] - g_Mark[0][0]/* + 1*/;
				if (iCopylen < 0)
				{
					iCopylen = 0;
				}
			}
			else
			{
				iCopylen = iStringLen;
			}
			memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_MarkFileNum].g_ptr[g_Mark[0][1] + index] + g_Mark[0][0], iCopylen);
			memset(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX + iCopylen, ' ', (g_Mark[1][0] - g_Mark[0][0] + 1) - iCopylen);
			
			list[g_iFileIndex].g_FileY++;
		}
		list[g_iFileIndex].g_FileY = iInitialg_FileY;

		if (list[g_iFileIndex].g_iMaxstringLen <  list[g_iFileIndex].g_FileY + iHigh - 1)
		{
			list[g_iFileIndex].g_iMaxstringLen = list[g_iFileIndex].g_FileY + iHigh - 1;
		}

		break;
	}
	case Alt_C:
	{
		int iHigh = 0;
		int iIndex = 0;
		//int iStringLen = 0;
		int prestringLen = 0;
		iHigh = g_Mark[1][1] - g_Mark[0][1] + 1;

		if (1 == iHigh)
		{
			g_Markcase = Alt_B;
			ArrayCopies();
			g_Markcase = Alt_C;
			return;
		}

		//Mark位置判斷
		if (g_MarkFileNum == g_iFileIndex)
		{
			if ((list[g_iFileIndex].g_FileY == g_Mark[0][1]) && (list[g_iFileIndex].g_FileX <= g_Mark[0][0]))
			{
				g_Mark[0][0] = g_Mark[0][0] - list[g_iFileIndex].g_FileX + g_Mark[1][0];
				g_Mark[0][1] = g_Mark[0][1] + iHigh - 1;
				g_Mark[1][1] = g_Mark[1][1] + iHigh - 1;
			}
			else if (list[g_iFileIndex].g_FileY < g_Mark[0][1])
			{
				g_Mark[0][1] = g_Mark[0][1] + iHigh - 1;
				g_Mark[1][1] = g_Mark[1][1] + iHigh - 1;
			}
			else if (((list[g_iFileIndex].g_FileY >= g_Mark[0][1]) && (list[g_iFileIndex].g_FileY <= g_Mark[1][1] - 1)) || ((g_Mark[1][1] == list[g_iFileIndex].g_FileY) && (list[g_iFileIndex].g_FileX < g_Mark[1][0])))
			{
				perr("Error: source and target conflict", RED, Messagebar);
				return;
			}
		}

		if (list[g_iFileIndex].g_iCurrentRowNum < (list[g_iFileIndex].g_iMaxstringLen + iHigh))
		{
			list[g_iFileIndex].g_iNewRowNum = list[g_iFileIndex].g_iCurrentRowNum + (ROW + (iHigh / ROW) * ROW);
			AddArray(AddROW, Alt);
		}

		for (int i = list[g_iFileIndex].g_iMaxstringLen; i > list[g_iFileIndex].g_FileY; i--)
		{
			list[g_iFileIndex].g_ptr[i + iHigh - 1] = list[g_iFileIndex].g_ptr[i];
			list[g_iFileIndex].g_stringLen[i + iHigh - 1] = list[g_iFileIndex].g_stringLen[i];
			list[g_iFileIndex].g_arrayLen[i + iHigh - 1] = list[g_iFileIndex].g_arrayLen[i];
		}

		//中間完整行部分進行清空及複製
		iIndex = 1;
		for (int i = list[g_iFileIndex].g_FileY + 1; i < list[g_iFileIndex].g_FileY + iHigh - 1; i++) //清出空間
		{
			list[g_iFileIndex].g_ptr[i] = NULL;
			list[g_iFileIndex].g_ptr[i] = (char*)malloc(sizeof(char) * list[g_MarkFileNum].g_arrayLen[g_Mark[0][1] + iIndex]);
			memcpy(list[g_iFileIndex].g_ptr[i], list[g_MarkFileNum].g_ptr[g_Mark[0][1] + iIndex], list[g_MarkFileNum].g_stringLen[g_Mark[0][1] + iIndex]);
			list[g_iFileIndex].g_stringLen[i] = list[g_MarkFileNum].g_stringLen[g_Mark[0][1] + iIndex];
			list[g_iFileIndex].g_arrayLen[i] = list[g_MarkFileNum].g_arrayLen[g_Mark[0][1] + iIndex];
			iIndex++;
		}

		//清出空間的最後一行進行字串複製
		list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY + iHigh - 1] = NULL;	
		if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > list[g_iFileIndex].g_FileX)
		{
			list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY + iHigh - 1] = COL + ((g_Mark[1][0] + (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - list[g_iFileIndex].g_FileX + 1)) / COL) * COL;
			list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY + iHigh - 1] = (char*)malloc(sizeof(char) * list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY + iHigh - 1]);
			memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY + iHigh - 1] + g_Mark[1][0], list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - list[g_iFileIndex].g_FileX);
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY + iHigh - 1] = g_Mark[1][0] + (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - list[g_iFileIndex].g_FileX);
		}
		else
		{
			list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY + iHigh - 1] = COL + (g_Mark[1][0] / COL) * COL;
			list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY + iHigh - 1] = (char*)malloc(sizeof(char) * list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY + iHigh - 1]);
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY + iHigh - 1] = g_Mark[1][0];
		}
		memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY + iHigh - 1], list[g_MarkFileNum].g_ptr[g_Mark[1][1]], g_Mark[1][0]);
		
		//清出空間的第一行進行字串複製
		prestringLen = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY];
		if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > list[g_iFileIndex].g_FileX)
		{
			memset(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, ' ', list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - list[g_iFileIndex].g_FileX);
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX;
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] + list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] - g_Mark[0][0];
		}
		else
		{
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX + list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] - g_Mark[0][0];
		}
		
		if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY])
		{
			list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] / COL) * COL;
			if (0 != (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] % COL))
			{
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] + COL;
			}
			AddArray(AddCOL, Alt);
		}
		if (prestringLen < list[g_iFileIndex].g_FileX)
		{
			memset(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + prestringLen, ' ', list[g_iFileIndex].g_FileX - prestringLen);
		}
		memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_MarkFileNum].g_ptr[g_Mark[0][1]] + g_Mark[0][0], list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] - g_Mark[0][0]);

		list[g_iFileIndex].g_iMaxstringLen = list[g_iFileIndex].g_iMaxstringLen + iHigh - 1;
		break;
	}
	case Alt_L:
	{
		int iHigh = 0;
		int iIndex = 0;
		iHigh = g_Mark[1][1] - g_Mark[0][1] + 1;

		//Mark位置判斷
		if (g_MarkFileNum == g_iFileIndex)
		{
			if (list[g_iFileIndex].g_FileY < g_Mark[0][1])
			{
				g_Mark[0][1] = g_Mark[0][1] + iHigh;
				g_Mark[1][1] = g_Mark[1][1] + iHigh;
			}
			else if ((list[g_iFileIndex].g_FileY >= g_Mark[0][1]) && (list[g_iFileIndex].g_FileY <= g_Mark[1][1]))
			{
				perr("Error: source and target conflict", RED, Messagebar);
				return;
			}
		}

		//陣列往下增加
		if (list[g_iFileIndex].g_iCurrentRowNum < (list[g_iFileIndex].g_iMaxstringLen + iHigh))
		{
			list[g_iFileIndex].g_iNewRowNum = list[g_iFileIndex].g_iCurrentRowNum + (ROW + (iHigh / ROW) * ROW);
			AddArray(AddROW, Alt);
		}

		for (int i = list[g_iFileIndex].g_iMaxstringLen; i >= list[g_iFileIndex].g_FileY; i--)
		{
			list[g_iFileIndex].g_ptr[i + iHigh] = list[g_iFileIndex].g_ptr[i];
			list[g_iFileIndex].g_stringLen[i + iHigh] = list[g_iFileIndex].g_stringLen[i];
			list[g_iFileIndex].g_arrayLen[i + iHigh] = list[g_iFileIndex].g_arrayLen[i];
		}

		iIndex = 0;
		for (int i = list[g_iFileIndex].g_FileY; i < list[g_iFileIndex].g_FileY + iHigh; i++)
		{
			list[g_iFileIndex].g_ptr[i] = NULL;
			list[g_iFileIndex].g_ptr[i] = (char*)malloc(sizeof(char) * list[g_MarkFileNum].g_arrayLen[g_Mark[0][1] + iIndex]);
			memcpy(list[g_iFileIndex].g_ptr[i], list[g_MarkFileNum].g_ptr[g_Mark[0][1] + iIndex], list[g_MarkFileNum].g_stringLen[g_Mark[0][1] + iIndex]);
			list[g_iFileIndex].g_stringLen[i] = list[g_MarkFileNum].g_stringLen[g_Mark[0][1] + iIndex];
			list[g_iFileIndex].g_arrayLen[i] = list[g_MarkFileNum].g_arrayLen[g_Mark[0][1] + iIndex];
			iIndex++;
		}

		list[g_iFileIndex].g_iMaxstringLen = list[g_iFileIndex].g_iMaxstringLen + iHigh;
		break;
	}
	}
}

void ArrayMove()
{
	switch (g_Markcase)
	{
	case Alt_B:
	{
		int iStringLen = 0;
		for (int i = g_Mark[0][1]; i <= g_Mark[1][1]; i++)
		{
			iStringLen = list[g_MarkFileNum].g_stringLen[i] - g_Mark[1][0];
			if (list[g_MarkFileNum].g_stringLen[i] < g_Mark[0][0])
			{
				break;
			}
			else if (iStringLen <= 0)
			{
				memset(list[g_MarkFileNum].g_ptr[i] + g_Mark[0][0], ' ', list[g_MarkFileNum].g_stringLen[i] - g_Mark[0][0]);
				list[g_MarkFileNum].g_stringLen[i] = g_Mark[0][0];
			}
			else
			{
				memmove(list[g_MarkFileNum].g_ptr[i] + g_Mark[0][0], list[g_MarkFileNum].g_ptr[i] + g_Mark[1][0] + 1, list[g_MarkFileNum].g_stringLen[i] - g_Mark[1][0]);
				list[g_MarkFileNum].g_stringLen[i] = list[g_MarkFileNum].g_stringLen[i] - (g_Mark[1][0] - g_Mark[0][0] + 1);
			}
		}
		break;
	}
	case Alt_C:
	{
		list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] = g_Mark[0][0] + (list[g_MarkFileNum].g_stringLen[g_Mark[1][1]] - g_Mark[1][0]);
		if (list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] > list[g_MarkFileNum].g_arrayLen[g_Mark[0][1]])
		{
			list[g_MarkFileNum].g_arrayLen[g_Mark[0][1]] = (list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] / COL) * COL;
			if (0 != (list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] % COL))
			{
				list[g_MarkFileNum].g_arrayLen[g_Mark[0][1]] = list[g_MarkFileNum].g_arrayLen[g_Mark[0][1]] + COL;
			}
			int iInitialg_FileY = list[g_MarkFileNum].g_FileY;
			list[g_MarkFileNum].g_FileY = g_Mark[0][1];
			AddArray(AddCOL, Alt);
			list[g_MarkFileNum].g_FileY = iInitialg_FileY;
		}
		memcpy(list[g_MarkFileNum].g_ptr[g_Mark[0][1]] + g_Mark[0][0], list[g_MarkFileNum].g_ptr[g_Mark[1][1]] + g_Mark[1][0], list[g_MarkFileNum].g_stringLen[g_Mark[1][1]] - g_Mark[1][0]);

		//字串往前
		for (int i = g_Mark[0][1] + 1; i <= list[g_MarkFileNum].g_iMaxstringLen - (g_Mark[1][1] - g_Mark[0][1]); i++)
		{
			list[g_MarkFileNum].g_ptr[i] = list[g_MarkFileNum].g_ptr[i + (g_Mark[1][1] - g_Mark[0][1])];
			list[g_MarkFileNum].g_stringLen[i] = list[g_MarkFileNum].g_stringLen[i + (g_Mark[1][1] - g_Mark[0][1])];
			list[g_MarkFileNum].g_arrayLen[i] = list[g_MarkFileNum].g_arrayLen[i + (g_Mark[1][1] - g_Mark[0][1])];
		}

		//後面多餘的字串刪除
		for (int i = list[g_MarkFileNum].g_iMaxstringLen - (g_Mark[1][1] - g_Mark[0][1]) + 1; i <= list[g_MarkFileNum].g_iMaxstringLen; i++)
		{
			list[g_MarkFileNum].g_ptr[i] = NULL;
			list[g_MarkFileNum].g_ptr[i] = (char*)malloc(sizeof(char) * COL);
			list[g_MarkFileNum].g_stringLen[i] = 0;
			list[g_MarkFileNum].g_arrayLen[i] = COL;
		}
		list[g_MarkFileNum].g_iMaxstringLen = list[g_MarkFileNum].g_iMaxstringLen - (g_Mark[1][1] - g_Mark[0][1]);

		break;
	}
	case Alt_L:
	{
		for (int i = g_Mark[0][1]; i <= list[g_MarkFileNum].g_iMaxstringLen - (g_Mark[1][1] - g_Mark[0][1] + 1); i++)
		{
			list[g_MarkFileNum].g_ptr[i] = list[g_MarkFileNum].g_ptr[i + (g_Mark[1][1] - g_Mark[0][1] + 1)];
			list[g_MarkFileNum].g_stringLen[i] = list[g_MarkFileNum].g_stringLen[i + (g_Mark[1][1] - g_Mark[0][1] + 1)];
			list[g_MarkFileNum].g_arrayLen[i] = list[g_MarkFileNum].g_arrayLen[i + (g_Mark[1][1] - g_Mark[0][1] + 1)];
		}

		//後面多餘的字串刪除
		for (int i = list[g_MarkFileNum].g_iMaxstringLen - (g_Mark[1][1] - g_Mark[0][1] + 1) + 1; i <= list[g_MarkFileNum].g_iMaxstringLen; i++)
		{
			list[g_MarkFileNum].g_ptr[i] = NULL;
			list[g_MarkFileNum].g_ptr[i] = (char*)malloc(sizeof(char) * COL);
			list[g_MarkFileNum].g_stringLen[i] = 0;
			list[g_MarkFileNum].g_arrayLen[i] = COL;
		}
		list[g_MarkFileNum].g_iMaxstringLen = list[g_MarkFileNum].g_iMaxstringLen - (g_Mark[1][1] - g_Mark[0][1] + 1);
		break;
	}
	}
}

void ArrayOverlays()
{
	//陣列是否往下擴展
	int iHigh = g_Mark[1][1] - g_Mark[0][1];
	if (list[g_iFileIndex].g_FileY + iHigh > list[g_iFileIndex].g_iCurrentRowNum)
	{
		list[g_iFileIndex].g_iNewRowNum = list[g_iFileIndex].g_iCurrentRowNum + (ROW + (iHigh / ROW) * ROW);
		AddArray(AddROW, Alt);
	}
	if (list[g_iFileIndex].g_FileY + iHigh > list[g_iFileIndex].g_iMaxstringLen)
	{
		list[g_iFileIndex].g_iMaxstringLen = list[g_iFileIndex].g_iMaxstringLen + iHigh;
	}

	switch (g_Markcase)
	{
	case Alt_B:
	{
		int iCopylen = 0;
		int iStringLen = g_Mark[1][0] - g_Mark[0][0] + 1;
		int iInitialg_FileY = list[g_iFileIndex].g_FileY;
		int prestringLen = 0;

		//判斷複製地方是否與被複製字串位置衝突
		if (g_MarkFileNum == g_iFileIndex)
		{
			if (((list[g_iFileIndex].g_FileY > g_Mark[0][1] - (g_Mark[1][1] - g_Mark[0][1] + 1)) && (list[g_iFileIndex].g_FileY <= g_Mark[1][1])) && (list[g_iFileIndex].g_FileX <= g_Mark[1][0]))
			{
				perr("Error: source and target conflict", RED, Messagebar);
				return;
			}
		}

		for (int i = g_Mark[0][1]; i <= g_Mark[1][1]; i++)
		{
			prestringLen = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY];
			if ((list[g_iFileIndex].g_FileX + (g_Mark[1][0] - g_Mark[0][0]) - 1) > list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY])
			{
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = COL + ((list[g_iFileIndex].g_FileX + (g_Mark[1][0] - g_Mark[0][0]) - 1) / COL) * COL;
				AddArray(AddCOL, Alt);
			}
			//
			if (list[g_MarkFileNum].g_stringLen[i] < g_Mark[1][0])
			{
				iCopylen = list[g_MarkFileNum].g_stringLen[i] - g_Mark[0][0]/* + 1*/;
				if (iCopylen < 0)
				{
					iCopylen = 0;
				}
			}
			else
			{
				iCopylen = iStringLen;
			}
			if (prestringLen < list[g_iFileIndex].g_FileX)
			{
				memset(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + prestringLen, ' ', list[g_iFileIndex].g_FileX - prestringLen);
			}
			memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_MarkFileNum].g_ptr[i] + g_Mark[0][0], iCopylen);
			memset(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX + iCopylen, ' ', (g_Mark[1][0] - g_Mark[0][0] + 1) - iCopylen);

			//覆蓋後長度大於原本長度
			if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] < list[g_iFileIndex].g_FileX + (g_Mark[1][0] - g_Mark[0][0] + 1))
			{
				list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX + (g_Mark[1][0] - g_Mark[0][0] + 1);
			}
			list[g_iFileIndex].g_FileY++;
		}
		list[g_iFileIndex].g_FileY = iInitialg_FileY;

		break;
	}
	case Alt_C:
	{
		int iHigh = 0; //Mark區塊第一行與最後一行高度差
		iHigh = g_Mark[1][1] - g_Mark[0][1];

		//Mark位置判斷
		if (g_MarkFileNum == g_iFileIndex)
		{
			if (((list[g_iFileIndex].g_FileY >= g_Mark[0][1] - iHigh) && (list[g_iFileIndex].g_FileY <= g_Mark[1][1] - 1)) || ((g_Mark[1][1] == list[g_iFileIndex].g_FileY) && (list[g_iFileIndex].g_FileX < g_Mark[1][0])))
			{
				perr("Error: source and target conflict", RED, Messagebar);
				return;
			}
		}

		if (0 == iHigh)
		{
			g_Markcase = Alt_B;
			ArrayOverlays();
			g_Markcase = Alt_C;
			return;
		}

		//複製Mark區塊第一行
		if (list[g_iFileIndex].g_FileX + (list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] - g_Mark[0][0]) > list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY])
		{
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX + (list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] - g_Mark[0][0]);
			//字串長度大於陣列長度，增加陣列長度
			if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY])
			{
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] / COL) * COL;
				if (0 != (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] % COL))
				{
					list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] + COL;
				}
				AddArray(AddCOL, Alt);
			}
		}
		memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, list[g_MarkFileNum].g_ptr[g_Mark[0][1]] + g_Mark[0][0], list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] - g_Mark[0][0] + 1);
		list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX + (list[g_MarkFileNum].g_stringLen[g_Mark[0][1]] - g_Mark[0][0]);


		int iInitialg_FileY = list[g_iFileIndex].g_FileY;
		list[g_iFileIndex].g_FileY = list[g_iFileIndex].g_FileY + 1;
		for (int i = g_Mark[0][1] + 1; i < g_Mark[1][1]; i++)
		{
			if (list[g_MarkFileNum].g_stringLen[i] > list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY])
			{
				list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_MarkFileNum].g_stringLen[i];
			}
			if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY])
			{
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] / COL) * COL;
				if (0 != (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] % COL))
				{
					list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] + COL;
				}
				AddArray(AddCOL, Alt);
			}
			memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY], list[g_MarkFileNum].g_ptr[i], list[g_MarkFileNum].g_stringLen[i]);
			list[g_iFileIndex].g_FileY++;
		}
		list[g_iFileIndex].g_FileY = iInitialg_FileY;


		//複製Mark區塊最後一行
		if (g_Mark[1][0] > list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY + iHigh])
		{
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY + iHigh] = g_Mark[1][0];
			//字串長度大於陣列長度，增加陣列長度
			if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY + iHigh] > list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY + iHigh]) 
			{
				list[g_iFileIndex].g_FileY = list[g_iFileIndex].g_FileY + iHigh;
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] / COL) * COL;
				if (0 != (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] % COL))
				{
					list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] + COL;
				}
				AddArray(AddCOL, Alt);
				list[g_iFileIndex].g_FileY = list[g_iFileIndex].g_FileY - iHigh;
			}
		}
		memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY + iHigh], list[g_MarkFileNum].g_ptr[g_Mark[1][1]], g_Mark[1][0]);

		break;
	}
	case Alt_L:
	{
		if (g_MarkFileNum == g_iFileIndex)
		{
			if ((list[g_iFileIndex].g_FileY >= g_Mark[0][1] - iHigh) && (list[g_iFileIndex].g_FileY <= g_Mark[1][1]))
			{
				perr("Error: source and target conflict", RED, Messagebar);
				return;
			}
		}

		int iInitialg_FileY = list[g_iFileIndex].g_FileY;
		for (int i = g_Mark[0][1]; i <= g_Mark[1][1]; i++)
		{
			if (list[g_MarkFileNum].g_stringLen[i] > list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY])
			{
				list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_MarkFileNum].g_arrayLen[i];
				AddArray(AddCOL, Alt);
			}
			memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY], list[g_MarkFileNum].g_ptr[i], list[g_MarkFileNum].g_stringLen[i]);
			list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_MarkFileNum].g_stringLen[i];
			list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_MarkFileNum].g_arrayLen[i];
			list[g_iFileIndex].g_FileY++;
		}
		list[g_iFileIndex].g_FileY = iInitialg_FileY;

		break;
	}
	}
}

void AltKey(WORD wKeyIn)
{
	switch (wKeyIn)
	{
	case Alt_B: //B鍵 Block mark for rectangles, vertical, and horizontal lines
	{
		g_Markcase = Alt_B;
		Blockmark(Alt_B);
		break;
	}
	case Alt_C: //C鍵 Character mark for characters, words, and	sentences.
	{
		g_Markcase = Alt_C;
		Blockmark(Alt_C);
		break;
	}
	case Alt_L: //L鍵 Line mark for one line or paragraph
	{
		g_Markcase = Alt_L;
		Blockmark(Alt_L);
		break;
	}
	case Alt_U: //U鍵 Unmarks all marks
	{
		g_Markcase = NULL;
		ResetMarkCoordinate(); //Mark陣列重設
		DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
		break;
	}
	case Alt_Z: //Z鍵 Copies marked text, leaving original text
	{
		if (NULL == g_Markcase)
		{
			perr("Error: no mark text", RED, Messagebar);
		}
		else
		{
			ArrayCopies(g_Markcase);
			DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
		}
		
		break;
	}
	case Alt_M: //M鍵 Moves marked text, deleting original text
	{
		if (NULL == g_Markcase)
		{
			perr("Error: no mark text", RED, Messagebar);
		}
		else
		{
			if ((0 == (g_Mark[1][1] - g_Mark[0][1])) && (Alt_C == g_Markcase))
			{
				g_Markcase = Alt_B;
				ArrayCopies();
				ArrayMove();
				g_Markcase = Alt_C;
			}
			else
			{
				ArrayCopies();
				ArrayMove();
			}
			g_Markcase = NULL;
			ResetMarkCoordinate();
			DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
		}
		
		break;
	}
	case Alt_O: //O鍵 Overlays marked text, leaving original text
	{
		if (NULL == g_Markcase)
		{
			perr("Error: no mark text", RED, Messagebar);
		}
		else
		{
			ArrayOverlays();
			DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
		}

		break;
	}
	case Alt_D: //D鍵 Deletes marked text
	{
		if (g_MarkFileNum == g_iFileIndex)
		{
			if (NULL == g_Markcase)
			{
				perr("Error: no mark text", RED, Messagebar);
			}
			else
			{
				ArrayMove();
				list[g_iFileIndex].g_FileX = g_Mark[0][0];
				list[g_iFileIndex].g_FileY = g_Mark[0][1];
				g_Markcase = NULL;
				ResetMarkCoordinate();

				list[g_iFileIndex].g_iPageROW = list[g_iFileIndex].g_FileY / ROW;
				TurnPage(0, (list[g_iFileIndex].g_iPageROW * ROW), 0, Home);
				//DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
			}
		}

		break;
	}
	}

	if (list[g_iFileIndex].g_FileX < 0)
	{
		list[g_iFileIndex].g_FileX = 0;
	}
	if (list[g_iFileIndex].g_FileY < 0)
	{
		list[g_iFileIndex].g_FileY = 0;
	}
	PrintfInformation();
}

void DrawKeyDown(char cKeyIn) //字元儲存陣列, 輸入時是否超出陣列, 字串長度計算, 文章最後字串位置, Insert或Replace模式
{
	cKeyIn = cKeyInTransform(cKeyIn); //字元判斷

	if (-1 != cKeyIn)
	{
		if (UNLOCK == g_Esc) //文章輸入模式
		{
			COORD coCharpos = { list[g_iFileIndex].g_UIX, list[g_iFileIndex].g_UIY };
			DWORD dwTT;
			//輸出字元
			FillConsoleOutputCharacter(g_hOut, cKeyIn, 1, coCharpos, &dwTT);

			//欄(ROW長度向下延展)
			if (list[g_iFileIndex].g_iCurrentRowNum <= list[g_iFileIndex].g_FileY)
			{
				OutputDebugString("ROW+");
				AddArray(AddROW, 0);
			}
			//列(COL長度向右增加)
			if ((list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] < (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] + 1)) || (list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] < (list[g_iFileIndex].g_FileX + 1)))/*(目前記憶體配置長度 < 輸入後字串長度) || (目前記憶體配置長度 < 目前輸入位置)*/
			{
				AddArray(AddCOL, 0);
				OutputDebugString("COL+");
			}

			if ((list[g_iFileIndex].g_FileX + 1 <= list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY]) && (LOCK == g_iInsert)) //所有字元向後位移
			{
				StringMove(Insert);
				list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX] = cKeyIn;
				DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), (list[g_iFileIndex].g_iPageROW * ROW), DisplayLine);
			}

			list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX] = cKeyIn;

			if (list[g_iFileIndex].g_FileX + 1 > list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY]) //每列字串長度計算
			{
				for (int i = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY]; i < list[g_iFileIndex].g_FileX; i++)
				{
					list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][i] = ' ';
				}
				list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_FileX + 1;
			}
			list[g_iFileIndex].g_FileX++;
			list[g_iFileIndex].g_UIX++;

			if ((0 == (list[g_iFileIndex].g_UIX % COL)) && (list[g_iFileIndex].g_UIX > (COL / 2))) //目前頁數*(COL / 2)+游標位置+1 && 頁數不等於0
			{
				TurnPage(list[g_iFileIndex].g_FileX - (COL / 2), list[g_iFileIndex].g_iPageROW * ROW, 0, Keydown);
			}

			for (int i = 0; i < list[g_iFileIndex].g_iCurrentRowNum; i++)
			{
				if (0 != list[g_iFileIndex].g_stringLen[i])
				{
					list[g_iFileIndex].g_iMaxstringLen = i;
				}
			}
		}
		else //Esc輸入模式
		{
			COORD coCharpos = { g_CommandCursorX, g_CommandCursorY };
			DWORD dwTT;
			if (g_CommandCursorX < strlen(g_Command)) //所有字元向後位移
			{
				if ((strlen(g_Command) + 1) <= COL)
				{
					memmove(g_Command + g_CommandCursorX + 1, g_Command + g_CommandCursorX, strlen(g_Command) - g_CommandCursorX + 1);
					FillConsoleOutputCharacter(g_hOut, cKeyIn, 1, coCharpos, &dwTT);
					g_Command[g_CommandCursorX] = cKeyIn;
					MoveCursor(0, g_CommandCursorY);
					printf("%s", g_Command);
					DrewGreenBar();
				}
			}
			else
			{
				FillConsoleOutputCharacter(g_hOut, cKeyIn, 1, coCharpos, &dwTT);
				g_Command[g_CommandCursorX] = cKeyIn;
				g_Command[g_CommandCursorX + 1] = '\0';
			}
			if ((g_CommandCursorX < COL) && (strlen(g_Command) < COL))
			{
				g_CommandCursorX++;
			}
		}
	}
}

void StringReplace()
{
	//需移動字串長度
	int iMoveLen = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - (list[g_iFileIndex].g_FileX + strlen(g_SearchData));
	//需移除字串長度
	int imemsetLen = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] - (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] + (strlen(g_ReplaceData) - strlen(g_SearchData)));
	
	list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] + (strlen(g_ReplaceData) - strlen(g_SearchData));
	//判斷陣列增加
	if (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] > COL)
	{
		list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] / COL) * COL;
		if (0 != (list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY] % COL))
		{
			list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] = list[g_iFileIndex].g_arrayLen[list[g_iFileIndex].g_FileY] + COL;
		}
		AddArray(AddCOL, Alt);
	}
	//列騰出可塞取代項目空間
	memmove(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX + strlen(g_ReplaceData), list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX + strlen(g_SearchData), iMoveLen);
	
	//移除多餘字串
	if (imemsetLen > 0)
	{
		memset(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_stringLen[list[g_iFileIndex].g_FileY], ' ', imemsetLen);
	}
	
	//字串複製
	memcpy(list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY] + list[g_iFileIndex].g_FileX, g_ReplaceData, strlen(g_ReplaceData));

	list[g_iFileIndex].g_FileX = list[g_iFileIndex].g_FileX + strlen(g_ReplaceData);
}

void CtrlKey(WORD wKeyIn)
{
	switch (wKeyIn)
	{
	case VK_LEFT://Ctrl + 左
	{
		list[g_iFileIndex].g_FileX = list[g_iFileIndex].g_FileX - 40;
		list[g_iFileIndex].g_UIX = list[g_iFileIndex].g_UIX - 40;
		if ((0 != list[g_iFileIndex].g_iPageCOL) && (0 > list[g_iFileIndex].g_UIX))
		{
			TurnPage(((list[g_iFileIndex].g_iPageCOL * (COL / 2)) - (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, 0, Left);
		}
		else if (0 > list[g_iFileIndex].g_UIX)
		{
			list[g_iFileIndex].g_FileX = 0;
		}
		if ('\t' == list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX])
		{
			CalculateUICursor();
			while (1)
			{
				if (('\t' != list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX - 1]) || (0 == list[g_iFileIndex].g_FileX))
				{
					break;
				}
				list[g_iFileIndex].g_FileX--;
				list[g_iFileIndex].g_UIX--;
				if ((-1 == list[g_iFileIndex].g_UIX) && (0 != list[g_iFileIndex].g_iPageCOL))
				{
					TurnPage((list[g_iFileIndex].g_FileX + 1) - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Left);
				}
				
			}
		}
		break;
	}
	case VK_RIGHT://Ctrl + 右
	{
		list[g_iFileIndex].g_FileX = list[g_iFileIndex].g_FileX + 40;
		list[g_iFileIndex].g_UIX = list[g_iFileIndex].g_UIX + 40;
		if (COL <= list[g_iFileIndex].g_UIX)
		{
			TurnPage(((list[g_iFileIndex].g_iPageCOL + 1) * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, 0, Right);
		}
		if ('\t' == list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX])
		{
			CalculateUICursor();
			while (1)
			{
				if (('\t' != list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX]) || ('\t' != list[g_iFileIndex].g_ptr[list[g_iFileIndex].g_FileY][list[g_iFileIndex].g_FileX - 1]))
				{
					break;
				}
				list[g_iFileIndex].g_FileX++;
				list[g_iFileIndex].g_UIX++;
				if (COL == list[g_iFileIndex].g_UIX)
				{
					TurnPage(list[g_iFileIndex].g_FileX - (COL / 2), (list[g_iFileIndex].g_iPageROW * ROW), 0, Right);
				}
			}
		}
		break;
	}
	case VK_HOME: //Ctrl + Home
	{
		list[g_iFileIndex].g_iPageROW = 0;
		DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), 0, DisplayAll);
		list[g_iFileIndex].g_FileY = 0;
		break;
	}
	case VK_END: //Ctrl + End
	{
		list[g_iFileIndex].g_iPageROW = list[g_iFileIndex].g_iMaxstringLen / ROW;
		DisplayArray((list[g_iFileIndex].g_iPageCOL * (COL / 2)), list[g_iFileIndex].g_iPageROW * ROW, DisplayAll);
		list[g_iFileIndex].g_FileY = list[g_iFileIndex].g_iMaxstringLen;
		break;
	}
	case VK_PRIOR: //Ctrl + Page up
	{
		list[g_iFileIndex].g_FileY = list[g_iFileIndex].g_iPageROW * ROW;
		break;
	}
	case VK_RETURN: //Ctrl + Enter
	{
		bool Find;
		Find = StringSearch(list[g_iFileIndex].g_FileX + 1, list[g_iFileIndex].g_FileY);
		if ((0 != strlen(g_ReplaceData)) && (true == Find))
		{
			StringReplace();
			perr("Replace success", RED, Messagebar);
		}
		TurnPage((list[g_iFileIndex].g_FileX / COL) * COL, (list[g_iFileIndex].g_FileY / ROW) * ROW, 0, Search);
		break;
	}
	}

	if (list[g_iFileIndex].g_FileX < 0)
	{
		list[g_iFileIndex].g_FileX = 0;
	}
	if (list[g_iFileIndex].g_FileY < 0)
	{
		list[g_iFileIndex].g_FileY = 0;
	}
	PrintfInformation();
}

void CalculateUICursor()
{
	if (0 == (list[g_iFileIndex].g_iPageCOL % 2)) //偶數頁
	{
		list[g_iFileIndex].g_UIX = list[g_iFileIndex].g_FileX % COL;
		list[g_iFileIndex].g_UIY = list[g_iFileIndex].g_FileY % ROW;
	}
	else //奇數頁
	{
		list[g_iFileIndex].g_UIX = (list[g_iFileIndex].g_FileX + (COL / 2)) % COL;
		list[g_iFileIndex].g_UIY = list[g_iFileIndex].g_FileY % ROW;
	}
}

char cKeyInTransform(char cKeyIn) //鍵盤與印出字元轉換
{
	if (UNLOCK == g_iShift)
	{
		if ((cKeyIn >= 65) && (cKeyIn <= 90))//鍵盤a-z, 數字鍵盤0-9
		{
			if (LOCK == g_iCapsLock)
			{
				cKeyIn = cKeyIn + 32;
			}
		}
		else if (((cKeyIn >= 48) && (cKeyIn <= 57)) || (cKeyIn == 32))
		{
			return cKeyIn;
		}
		else if ((cKeyIn >= VK_NUMPAD0) && (cKeyIn <= VK_NUMPAD9)) //數字鍵盤由英文轉數字
		{
			cKeyIn = cKeyIn - '0';
		}
		else if ((cKeyIn >= -68) && (cKeyIn <= -65))
		{
			cKeyIn = cKeyIn + 112;
		}
		else if ((cKeyIn >= -37) && (cKeyIn <= -35))
		{
			cKeyIn = cKeyIn + 128;
		}
		else if (((cKeyIn >= 106) && (cKeyIn <= 111)) && (cKeyIn != 108))
		{
			cKeyIn = cKeyIn - 64;
		}
		else if (-70 == cKeyIn)
		{
			cKeyIn = 59;
		}
		else if (-69 == cKeyIn)
		{
			cKeyIn = 61;
		}
		else if (-64 == cKeyIn)
		{
			cKeyIn = 96;
		}
		else if (-34 == cKeyIn)
		{
			cKeyIn = 39;
		}
		else if (9 == cKeyIn)
		{
			cKeyIn = '\t';
		}
		else
		{
			cKeyIn = -1;
		}
	}
	else if (LOCK == g_iShift)
	{
		if ((cKeyIn >= 65) && (cKeyIn <= 90))//鍵盤a-z, 數字鍵盤0-9
		{
			if (LOCK == g_iCapsLock)
			{
				cKeyIn = cKeyIn + 32;
			}
		}
		else if ((cKeyIn >= 51) && (cKeyIn <= 53))
		{
			cKeyIn = cKeyIn - 16;
		}
		else if ((cKeyIn >= -37) && (cKeyIn <= -35))
		{
			cKeyIn = cKeyIn + 160;
		}
		else if (-64 == cKeyIn)
		{
			cKeyIn = 126;
		}
		else if (49 == cKeyIn)
		{
			cKeyIn = 33;
		}
		else if (50 == cKeyIn)
		{
			cKeyIn = 64;
		}
		else if (54 == cKeyIn)
		{
			cKeyIn = 94;
		}
		else if (55 == cKeyIn)
		{
			cKeyIn = 38;
		}
		else if (56 == cKeyIn)
		{
			cKeyIn = 42;
		}
		else if (57 == cKeyIn)
		{
			cKeyIn = 40;
		}
		else if (48 == cKeyIn)
		{
			cKeyIn = 41;
		}
		else if (-67 == cKeyIn)
		{
			cKeyIn = 95;
		}
		else if (-69 == cKeyIn)
		{
			cKeyIn = 43;
		}
		else if (-34 == cKeyIn)
		{
			cKeyIn = 34;
		}
		else if ((cKeyIn >= -70) && (cKeyIn <= -65))
		{
			cKeyIn = cKeyIn + 128;
		}
		else
		{
			cKeyIn = -1;
		}
	}
	return cKeyIn;
}

void ShiftKey(WORD wKeyIn)
{
	switch (wKeyIn)
	{
	case VK_F5:
	{
		bool Find;
		if (UNLOCK == g_Esc)
		{
			if (0 == strlen(g_ReplaceData))
			{
				perr("Please set replace pattern", RED, Messagebar);
			}
			else
			{
				Find = StringSearch(list[g_iFileIndex].g_FileX/* + 1*/, list[g_iFileIndex].g_FileY);
				if (true == Find)
				{
					while (1)
					{
						static DWORD dwCount;
						static INPUT_RECORD input;
						ReadConsoleInput(g_hIn, &input, 1, &dwCount);
						perr("Do you really want to replace ? [y] or [n]", RED, Messagebar);
						if ('Y' == input.Event.KeyEvent.wVirtualKeyCode)
						{
							perr("                                          ", RED, Messagebar);
							if (0 != strlen(g_ReplaceData))
							{
								StringReplace();
							}
							break;
						}
						if ('N' == input.Event.KeyEvent.wVirtualKeyCode)
						{
							perr("                                          ", RED, Messagebar);
							break;
						}
						HideCursor(0);
					}
				}
				TurnPage((list[g_iFileIndex].g_FileX / COL) * COL, (list[g_iFileIndex].g_FileY / ROW) * ROW, 0, Search);
			}
			PrintfInformation();
		}
		break;
	}
	default:
	{
		g_iCapsLock = g_iCapsLock * (-1);
		g_iShift = LOCK;
		DrawKeyDown(wKeyIn);
		if (UNLOCK == g_Esc)
		{
			PrintfInformation();
		}
		else
		{
			MoveCursor(g_CommandCursorX, g_CommandCursorY);
		}
		g_iCapsLock = g_iCapsLock * (-1);
		g_iShift = UNLOCK;
		break;
	}
	}
}

void perr(char* msg, int color, int UIY)
{
	HideCursor(0);
	MoveCursor(0, UIY);
	printf("                                                           ");
	MoveCursor(0, UIY);
	SetConsoleTextAttribute(g_hOut, color);
	puts(msg);
	SetConsoleTextAttribute(g_hOut, 15);
	HideCursor(1);
}

void PrintfInformation()
{
	HideCursor(0);
	CalculateUICursor();//計算list[g_iFileIndex].g_UIX, list[g_iFileIndex].g_UIY
	MoveCursor(Informationbar, Messagebar);
	printf("                   ");
	MoveCursor(Informationbar, Messagebar);
	printf("%d, %d  Page(x,y)", list[g_iFileIndex].g_iPageCOL, list[g_iFileIndex].g_iPageROW);
	MoveCursor(Informationbar, FileNamebar);
	printf("                   ");
	MoveCursor(Informationbar, FileNamebar);
	printf("%d, %d  %s", list[g_iFileIndex].g_FileX, list[g_iFileIndex].g_FileY, g_szToggleMode);
	MoveCursor(list[g_iFileIndex].g_UIX, list[g_iFileIndex].g_UIY);
	HideCursor(1);
}

void DisplayCommand()
{
	HideCursor(0);
	MoveCursor(0, g_CommandCursorY);
	printf("                                                                                ");
	MoveCursor(0, g_CommandCursorY);
	printf("%s", g_Command);
	DrewGreenBar();
	HideCursor(1);
}