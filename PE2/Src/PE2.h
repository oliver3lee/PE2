#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <Windows.h>
#include <stdbool.h>
const int WHITE = 15;
const int RED = 12;

void main(int argv, char *argc[]);

void DrewGreenBar();
void WindowSize();

void MoveCursor(int iX, int iY);

int KeyDown();
void OutPutKey(char szKey);
void DrawKeyDown(char cKeyIn);
void SingleKey(WORD wKeyIn);
///////////////////////////////////////////////
void ShiftKey(WORD wKeyIn);
void AltKey(WORD wKeyIn);
void CtrlKey(WORD wKeyIn);

void AddArray(char status, int ColNum);
void TurnPage(int iFirst, int iEnd, char status);
void DisplayArray(int iFirst, int iHight, char status);
void StringMove(char status);
char cKeyInTransform(char cKeyIn);
void HideCursor(int status);
void EscKey();
void EscSingleKey(WORD wKeyIn);
void CalculateUICursor();
void BackToFront();
void PrintfInformation();
void DisplayCommand();
void CommandFeatures();
//void C_DrawKeyDown(char cKeyIn);
void perr(char* msg, int color, int UIY);
int PopToken(char* pszToken, int iTokenBufLen, char* pszData, char pszKeyword);
//void DisplayCommand();
bool Openfile(char* filename);
void FileToArray();
void ArrayToFile();
void FileToArrayTwo();
void PrintfStartMenu();
void ResetArray();
void Blockmark(char status);
void FillCoordinate(char status);
void AltB();
void AltL();
void AltC();
void DisplayMark(int x,int index,int y,int iHight, char status);
void ArrayCopies();
void ArrayMove();
void ArrayOverlays();
void Quit();
void ResetMarkCoordinate();
bool StringSearch(int iIndexX, int iIndexY);
void StringReplace();
void ClearCommand();
void ResetStruct();
void FileMove();
void Tab();
void FileMoveTo101();