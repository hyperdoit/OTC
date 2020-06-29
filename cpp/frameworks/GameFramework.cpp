#include "../../framework.h"

void GameFramework::PrintNewLineToConsole (int newLineCount) {
	for (int lineIterator = 0; lineIterator < newLineCount; lineIterator++) m_PrintInterface ("\n");
}

GameFramework::oPrintConsoleInterface GameFramework::PrintToConsole( ) {
    //New line for formating.
    PrintNewLineToConsole ();
    //"Timer" stonks.
    Sleep (0x90);
    return m_PrintInterface;
}