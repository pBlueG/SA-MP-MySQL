#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <stdarg.h>

#include <string>
using std::string;

#include "CLog.h"


CLog *CLog::m_Instance = NULL;


void CLog::ProcessLog() {
	FILE *LogFile = fopen(m_LogFileName, "w");

	char StartLogTime[32];
	time_t StartLogTimeRaw;
	time(&StartLogTimeRaw);
	const tm * StartLogTimeInfo = localtime(&StartLogTimeRaw);
	strftime(StartLogTime, sizeof(StartLogTime), "%H:%M, %d.%m.%Y", StartLogTimeInfo);

	fprintf(LogFile, "<html><head><title>MySQL Plugin log</title><style>table {border: 1px solid black; border-collapse: collapse; line-height: 23px; table-layout: fixed; width: 863px;}th, td {border: 1px solid black; word-wrap: break-word;}thead {background-color: #C0C0C0;}		tbody {text-align: center;}		table.left1 {position: relative; left: 36px;}		table.left2 {position: relative; left: 72px;}		.time {width: 80px;}		.func {width: 200px;}		.stat {width: 75px;}		.msg {width: 400px;}	</style>	<script>		var 			LOG_ERROR = 1,			LOG_WARNING = 2,			LOG_DEBUG = 4;				var			FirstRun = true,			IsCallbackActive = false,			IsTableOpen = false,			IsThreadActive = false;				function StartCB(cbname) {			StartTable(1, 0, cbname);		}		function EndCB() {			EndTable();			IsCallbackActive = false;		}		function StartTable(iscallback, isthreaded, cbname) {			if(IsTableOpen == true || isthreaded != IsThreadActive)				EndTable();						if(iscallback == true) {				document.write(					\"<table class=left2>\" +						\"<th bgcolor=#C0C0C0 >In callback \\\"\"+cbname+\"\\\"</th>\" +					\"</table>\"				);			}						document.write(\"<table\");			if(iscallback == true || (isthreaded != IsThreadActive && isthreaded == false && IsCallbackActive == true) ) {				document.write(\" class=left2\");				IsCallbackActive = true;			}			else if(isthreaded == true) 				document.write(\" class=left1\");						IsThreadActive = isthreaded;			document.write(\">\");						if(FirstRun == true) {				FirstRun = false;				document.write(\"<thead><th class=time>Time</th><th class=func>Function</th><th class=stat>Status</th><th class=msg>Message</th></thead>\");			}			document.write(\"<tbody>\");			IsTableOpen = true;		}				function EndTable() {			document.write(\"</tbody></table>\");			IsTableOpen = false;		}						function Log(time, func, status, msg, isthreaded) {			isthreaded = typeof isthreaded !== 'undefined' ? isthreaded : 0;			if(IsTableOpen == false || isthreaded != IsThreadActive)				StartTable(false, isthreaded, \"\");			var StatColor, StatText;			switch(status) {			case LOG_ERROR:				StatColor = \"RED\";				StatText = \"ERROR\";				break;			case LOG_WARNING:				StatColor = \"#FF9900\";				StatText = \"WARNING\";				break;			case LOG_DEBUG:				StatColor = \"#00DD00\";				StatText = \"OK\";				break;			}			document.write(				\"<tr bgcolor=\"+StatColor+\">\" + 					\"<td class=time>\"+time+\"</td>\" + 					\"<td class=func>\"+func+\"</td>\" + 					\"<td class=stat>\"+StatText+\"</td>\" + 					\"<td class=msg>\"+msg+\"</td>\" + 				\"</tr>\"			);		}	</script></head><body bgcolor=grey>	<h2>Logging started at %s</h2><script>\n", StartLogTime);
	fflush(LogFile);

	while(m_LogThreadAlive) {
		
		m_SLogData *LogData = NULL;
		while(m_LogQueue.pop(LogData)) {
			
			if(LogData->IsCallback == true) 
				fputs(LogData->Msg, LogFile);
			else {
				char timeform[16];
				time_t rawtime;
				time(&rawtime);
				struct tm * timeinfo;
				timeinfo = localtime(&rawtime);
				strftime(timeform, sizeof(timeform), "%X", timeinfo);

				//escape "'s in Msg
				string LogMsg(LogData->Msg);
				for(size_t s = 0; s < LogMsg.length(); ++s) {
					char Char = LogMsg.at(s);
					if(Char == '\\')
						LogMsg.replace(s, 1, "\\\\"), s++;
					else if(Char == '"')
						LogMsg.replace(s, 1, "\\\""), s++;
					
				}

				fprintf(LogFile, "Log(\"%s\",\"%s\",%d,\"%s\",%d);\n", timeform, LogData->Name, LogData->Status, LogMsg.c_str(), LogData->IsThreaded == false ? 0 : 1);
			}
			fputs("</script>", LogFile); //append this tag, or else the JS functions won't work
			fflush(LogFile);
			fseek(LogFile, ftell(LogFile)-9, SEEK_SET); //set position before </script>-tag to overwrite it next time

			delete LogData;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		
	}
	fputs("</script></body></html>", LogFile);
	fclose(LogFile);
	m_LogThreadAlive = true;
}

void CLog::Initialize(const char *logfile) {
	strcpy(m_LogFileName, logfile);
	SetLogType(m_LogType);
	m_MainThreadID = boost::this_thread::get_id();
}

void CLog::SetLogType(unsigned int logtype)  {
	if(logtype != LOG_TYPE_HTML && logtype != LOG_TYPE_TEXT)
		return ;
	if(logtype == m_LogType)
		return ;
	m_LogType = logtype;
	if(logtype == LOG_TYPE_HTML) {
		if(m_LogThread == NULL)
			m_LogThread = new boost::thread(&CLog::ProcessLog, this);
		m_LogThread->detach();

		string FileName(m_LogFileName);
		int Pos = FileName.find_first_of(".");
		FileName.erase(Pos, FileName.size() - Pos);
		FileName.append(".html");
		strcpy(m_LogFileName, FileName.c_str());
	}
	else if(logtype == LOG_TYPE_TEXT) {
		string FileName(m_LogFileName);
		int Pos = FileName.find_first_of(".");
		FileName.erase(Pos, FileName.size() - Pos);
		FileName.append(".txt");
		strcpy(m_LogFileName, FileName.c_str());
	}
}


void CLog::LogFunction(unsigned int status, char *funcname, char *msg, ...) {
	if(m_LogLevel == LOG_NONE)
		return ;
	switch(m_LogType) {
		case LOG_TYPE_HTML: {
			if (m_LogLevel & status) {
		
				m_SLogData *LogData = new m_SLogData;
					LogData->IsThreaded = (boost::this_thread::get_id() != m_MainThreadID);

				LogData->Status = status;

				LogData->Msg = (char *)malloc(2048 * sizeof(char));
				va_list args;
				va_start(args, msg);
				vsprintf(LogData->Msg, msg, args);
				va_end (args);

				LogData->Name = (char *)malloc((strlen(funcname)+1) * sizeof(char));
				strcpy(LogData->Name, funcname);

				m_LogQueue.push(LogData);
			}
		} break;

		case LOG_TYPE_TEXT: {
			char MsgBuf[2048];
			int RealMsgLen=0;
			va_list args;
			va_start(args, msg);
			RealMsgLen = vsprintf(MsgBuf, msg, args);
			va_end (args);
			
			char *LogText = (char *)malloc((strlen(funcname) + RealMsgLen + 8) * sizeof(char));
			sprintf(LogText, "%s - %s", funcname, MsgBuf);
			TextLog(status, LogText);
			free(LogText);
		} break;
	}
}


void CLog::StartCallback(const char *cbname) {
	if(m_LogLevel == LOG_NONE)
		return ;
	if(m_LogType == LOG_TYPE_HTML) {
		m_SLogData *LogData = new m_SLogData;

		LogData->IsCallback = true;
		LogData->Msg = (char *)malloc((strlen(cbname)+20) * sizeof(char));
		sprintf(LogData->Msg, "StartCB(\"%s\");", cbname);

		m_LogQueue.push(LogData);
	}
	else if(m_LogType == LOG_TYPE_TEXT) {
		char LogText[64];
		sprintf(LogText, "Calling callback \"%s\"..", cbname);
		TextLog(LOG_DEBUG, LogText);
	}
}

void CLog::EndCallback() {
	if(m_LogType != LOG_TYPE_HTML)
		return ;

	if(m_LogLevel == LOG_NONE)
		return ;
	
	m_SLogData *LogData = new m_SLogData;

	LogData->IsCallback = true;
	LogData->Msg = (char *)malloc(9 * sizeof(char));
	strcpy(LogData->Msg, "EndCB();");

	m_LogQueue.push(LogData);
}


CLog::~CLog() {
	if(m_LogThread != NULL) {
		m_LogThreadAlive = false;
		
		while(m_LogThreadAlive == false) 
			boost::this_thread::sleep(boost::posix_time::milliseconds(5));

		delete m_LogThread;
	}
}

void CLog::TextLog(unsigned int level, char* text) {
    if (m_LogLevel & level) {
        char prefix[16];
        switch(level) {
        case LOG_ERROR:
            sprintf(prefix, "ERROR");
            break;
        case LOG_WARNING:
            sprintf(prefix, "WARNING");
            break;
        case LOG_DEBUG:
            sprintf(prefix, "DEBUG");
            break;
        }
        char timeform[16];
        time_t rawtime;
        time(&rawtime);
        struct tm * timeinfo;
        timeinfo = localtime(&rawtime);
        strftime(timeform, sizeof(timeform), "%X", timeinfo);

        FILE *file = fopen(m_LogFileName, "a");
        if(file != NULL) {
            fprintf(file, "[%s] [%s] %s\n", timeform, prefix, text);
            fclose(file);
        }
                
    }
}

