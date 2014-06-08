#pragma once

#include <cstdio>
#include <stdarg.h>

#include <string>
#include <boost/chrono.hpp>

using std::string;
namespace chrono = boost::chrono;

#include "CLog.h"


CLog *CLog::m_Instance = NULL;


void CLog::ProcessLog() 
{
	bool 
		IsCallbackActive = false,
		IsCallbackUsed = false;
	string CallbackMsg;
	
	FILE *LogFile = fopen(m_LogFileName, "w");

	char StartLogTime[32];
	time_t StartLogTimeRaw;
	time(&StartLogTimeRaw);
	const tm * StartLogTimeInfo = localtime(&StartLogTimeRaw);
	strftime(StartLogTime, sizeof(StartLogTime), "%H:%M, %d.%m.%Y", StartLogTimeInfo);

	fprintf(LogFile, "<html><head><title>MySQL Plugin log</title><style>table {border: 1px solid black; border-collapse: collapse; line-height: 23px; table-layout: fixed; width: 863px;}th, td {border: 1px solid black; word-wrap: break-word;}thead {background-color: #C0C0C0;}		tbody {text-align: center;}		table.left1 {position: relative; left: 36px;}		table.left2 {position: relative; left: 72px;}		.time {width: 80px;}		.func {width: 200px;}		.stat {width: 75px;}		.msg {width: 400px;}	</style>	<script>		var 			LOG_ERROR = 1,			LOG_WARNING = 2,			LOG_DEBUG = 4;				var			FirstRun = true,			IsCallbackActive = false,			IsTableOpen = false,			IsThreadActive = false;				function StartCB(cbname) {			StartTable(1, 0, cbname);		}		function EndCB() {			EndTable();			IsCallbackActive = false;		}		function StartTable(iscallback, isthreaded, cbname) {			if(IsTableOpen == true || isthreaded != IsThreadActive)				EndTable();						if(iscallback == true) {				document.write(					\"<table class=left2>\" +						\"<th bgcolor=#C0C0C0 >In callback \\\"\"+cbname+\"\\\"</th>\" +					\"</table>\"				);			}						document.write(\"<table\");			if(iscallback == true || (isthreaded != IsThreadActive && isthreaded == false && IsCallbackActive == true) ) {				document.write(\" class=left2\");				IsCallbackActive = true;			}			else if(isthreaded == true) 				document.write(\" class=left1\");						IsThreadActive = isthreaded;			document.write(\">\");						if(FirstRun == true) {				FirstRun = false;				document.write(\"<thead><th class=time>Time</th><th class=func>Function</th><th class=stat>Status</th><th class=msg>Message</th></thead>\");			}			document.write(\"<tbody>\");			IsTableOpen = true;		}				function EndTable() {			document.write(\"</tbody></table>\");			IsTableOpen = false;		}						function Log(time, func, status, msg, isthreaded) {			isthreaded = typeof isthreaded !== 'undefined' ? isthreaded : 0;			if(IsTableOpen == false || isthreaded != IsThreadActive)				StartTable(false, isthreaded, \"\");			var StatColor, StatText;			switch(status) {			case LOG_ERROR:				StatColor = \"RED\";				StatText = \"ERROR\";				break;			case LOG_WARNING:				StatColor = \"#FF9900\";				StatText = \"WARNING\";				break;			case LOG_DEBUG:				StatColor = \"#00DD00\";				StatText = \"OK\";				break;			}			document.write(				\"<tr bgcolor=\"+StatColor+\">\" + 					\"<td class=time>\"+time+\"</td>\" + 					\"<td class=func>\"+func+\"</td>\" + 					\"<td class=stat>\"+StatText+\"</td>\" + 					\"<td class=msg>\"+msg+\"</td>\" + 				\"</tr>\"			);		}	</script></head><body bgcolor=grey>	<h2>Logging started at %s</h2><script>\n", StartLogTime);
	fflush(LogFile);

	while(m_LogThreadAlive) 
	{
		m_SLogData *LogData = NULL;
		while(m_LogQueue.pop(LogData)) 
		{
			if(LogData->Info == LOG_INFO_CALLBACK_BEGIN)
			{
				IsCallbackActive = true;
				IsCallbackUsed = false;
				CallbackMsg = LogData->Msg;
			}
			else if(LogData->Info == LOG_INFO_CALLBACK_END)
			{
				if(IsCallbackUsed == true)
					fputs("EndCB();", LogFile);
				IsCallbackActive = false;
				IsCallbackUsed = false;
			}
			else 
			{
				if(IsCallbackActive == true && IsCallbackUsed == false)
				{
					fputs(CallbackMsg.c_str(), LogFile);
					IsCallbackUsed = true;
				}
				
				char timeform[16];
				time_t rawtime;
				time(&rawtime);
				strftime(timeform, sizeof(timeform), "%X", localtime(&rawtime));

				//escape "'s in Msg
				string LogMsg(LogData->Msg);
				for(size_t s = 0; s < LogMsg.length(); ++s) 
				{
					char Char = LogMsg.at(s);
					if(Char == '\\')
						LogMsg.replace(s, 1, "\\\\"), s++;
					else if(Char == '"')
						LogMsg.replace(s, 1, "\\\""), s++;
					
				}

				fprintf(LogFile, "Log(\"%s\",\"%s\",%d,\"%s\",%d);\n", timeform, LogData->Name, LogData->Status, LogMsg.c_str(), LogData->Info == LOG_INFO_THREADED ? 1 : 0);//LogData->IsThreaded == false ? 0 : 1);
			}
			fputs("</script>", LogFile); //append this tag, or else the JS functions won't work
			fflush(LogFile);
			fseek(LogFile, ftell(LogFile)-9, SEEK_SET); //set position before </script>-tag to overwrite it next time

			delete LogData;
		}
		this_thread::sleep_for(chrono::milliseconds(10));
		
	}
	fputs("</script></body></html>", LogFile);
	fclose(LogFile);
}

void CLog::Initialize(const char *logfile) 
{
	strcpy(m_LogFileName, logfile);
	SetLogType(m_LogType);
	m_MainThreadID = this_thread::get_id();
}

void CLog::SetLogType(unsigned int logtype)  
{
	if(logtype != LOG_TYPE_HTML && logtype != LOG_TYPE_TEXT)
		return ;
	if(logtype == m_LogType)
		return ;

	m_LogType = logtype;

	string filename(m_LogFileName);
	int Pos = filename.find_first_of(".");
	filename.erase(Pos, filename.size() - Pos);

	if(logtype == LOG_TYPE_HTML) 
	{
		if(m_LogThread == NULL)
			m_LogThread = new thread(&CLog::ProcessLog, this);

		filename.append(".html");
	}
	else if(logtype == LOG_TYPE_TEXT) 
		filename.append(".txt");

	strcpy(m_LogFileName, filename.c_str());
}


int CLog::LogFunction(unsigned int loglevel, char *funcname, char *msg, ...) 
{
	if(m_LogLevel != LOG_NONE)
	{
		switch(m_LogType) 
		{
			case LOG_TYPE_HTML: 
			{
				if (m_LogLevel & loglevel) 
				{
					m_SLogData *log_data = new m_SLogData;

					log_data->Info = (this_thread::get_id() != m_MainThreadID) ? LOG_INFO_THREADED : LOG_INFO_NONE;
					log_data->Status = loglevel;

					log_data->Msg = (char *)malloc(2048 * sizeof(char));
					va_list args;
					va_start(args, msg);
					vsprintf(log_data->Msg, msg, args);
					va_end (args);

					log_data->Name = (char *)malloc((strlen(funcname)+1) * sizeof(char));
					strcpy(log_data->Name, funcname);

					m_LogQueue.push(log_data);
				}
			} 
			break;
			case LOG_TYPE_TEXT: 
			{
				char msg_buf[2048];

				va_list args;
				va_start(args, msg);
				const int real_msg_len = vsprintf(msg_buf, msg, args);
				va_end (args);
			
				char *log_text = (char *)malloc((strlen(funcname) + real_msg_len + 8) * sizeof(char));
				sprintf(log_text, "%s - %s", funcname, msg_buf);
				LogText(loglevel, log_text);
				free(log_text);
			} 
			break;
		}
	}
	return 0;
}

int CLog::LogText(unsigned int loglevel, char* text) 
{
    if (m_LogLevel & loglevel) 
	{
        char prefix[16];
        switch(loglevel) {
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

        FILE *log_file = fopen(m_LogFileName, "a");
        if(log_file != NULL) 
		{
            fprintf(log_file, "[%s] [%s] %s\n", timeform, prefix, text);
            fclose(log_file);
        }
                
    }
	return 0;
}

void CLog::StartCallback(const char *cbname) 
{
	if(m_LogLevel == LOG_NONE)
		return ;
	if(m_LogType == LOG_TYPE_HTML) 
	{
		m_SLogData *log_data = new m_SLogData;

		log_data->Info = LOG_INFO_CALLBACK_BEGIN;
		log_data->Msg = (char *)malloc((strlen(cbname)+20) * sizeof(char));
		sprintf(log_data->Msg, "StartCB(\"%s\");", cbname);

		m_LogQueue.push(log_data);
	}
	else if(m_LogType == LOG_TYPE_TEXT) 
	{
		char log_text[64];
		sprintf(log_text, "Calling callback \"%s\"..", cbname);
		LogText(LOG_DEBUG, log_text);
	}
}

void CLog::EndCallback() 
{
	if(m_LogType != LOG_TYPE_HTML)
		return ;

	if(m_LogLevel == LOG_NONE)
		return ;
	
	m_SLogData *log_data = new m_SLogData;
	log_data->Info = LOG_INFO_CALLBACK_END;
	m_LogQueue.push(log_data);
}


CLog::~CLog() 
{
	if(m_LogThread != NULL) 
	{
		m_LogThreadAlive = false;

		m_LogThread->join();
		delete m_LogThread;
	}
}


