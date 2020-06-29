#include "../../framework.h"

//The most convenient way to store a link to a file.
std::ofstream GLoggerFile (std::string (getenv("USERPROFILE")) + "\\Documents\\otc_latest.txt");

void Logger::Setup () {

	if (m_Type == CONSOLE) {
		if (!m_Game.m_IsInterfacesAvailable) PanicUtils::Release (PanicUtils::Layers::INIT, "GameInterfaces is null. Can't initialize the logger.");
		m_Game.AtConsoleInterface () (NULL, "showconsole; log_color General 0xFFFFFFFF; clear");
	}

}

void Logger::Space (int count) {

	switch (m_Type) {

	    case CONSOLE:
		   m_Game.PrintNewLineToConsole (count);
		  break;

		case FILE:
			GLoggerFile << "\n" << std::endl;
		   break;

	}

}

void Logger::Info (const char* message...) {

	switch (m_Type) {

		case CONSOLE:
		    m_Game.PrintToConsole () (message);
		   break;

		case FILE:
			GLoggerFile << message << std::endl;
		   break;

	}

}

void Logger::SwitchedInfo (const char* message...) {
	
	switch (m_Type) {

	   case CONSOLE:
		   GLoggerFile << message << std::endl;
		  break;

	   case FILE:
		   m_Game.PrintToConsole () (message);
		  break;

	}

}

void Logger::Free () {

	if (m_Type == FILE) GLoggerFile.close();

}
