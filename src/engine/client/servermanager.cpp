#include <base/math.h>
#include <base/system.h>

#include <engine/console.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <string.h>
#include <stdio.h>	// sscanf

#include "servermanager.h"


CServerManager::CServerManager()
{
}

void CServerManager::ShutdownAll()
{
    for (int i=0; i<MAX_LOCAL_SERVERS; DestroyServer(i++));
}

void CServerManager::Init()
{
    m_pConsole = Kernel()->RequestInterface<IConsole>();
    m_pStorage = Kernel()->RequestInterface<IStorageTW>();

    //Initialize array of local servers info
    for (int i=0; i<MAX_LOCAL_SERVERS; m_aServers[i++].m_Used = false);

}

bool CServerManager::CreateServer(char *fileconfg, char *exec)
{
    char aPort[5], aType[35];
    GetConfigParam("sv_port", fileconfg, aPort, sizeof(aPort));
    GetConfigParam("sv_gametype", fileconfg, aType, sizeof(aType));

    //Set Default Options
    if (aPort[0] == 0)
        str_copy(aPort, "8303", sizeof(aPort));
    if (aType[0] == 0)
        str_copy(aType, "DM", sizeof(aType));

    //Control Port Used
    //Todo: Ugly to see, good that works :/
    if (aPort[0] == 0)
        return false;
    for (int i=0; i<MAX_LOCAL_SERVERS; i++)
    {
        if (m_aServers[i].m_Used && str_comp(m_aServers[i].m_Port, aPort) == 0)
            return false;
    }

    for (int i=0; i<MAX_LOCAL_SERVERS; i++)
    {
        if (m_aServers[i].m_Used)
            continue;

        str_copy(m_aServers[i].m_Port, aPort, sizeof(m_aServers[i].m_Port));
        str_copy(m_aServers[i].m_Type, aType, sizeof(m_aServers[i].m_Type));
        str_copy(m_aServers[i].m_FileConfg, fileconfg, sizeof(m_aServers[i].m_FileConfg));
        str_copy(m_aServers[i].m_Exec, exec, sizeof(m_aServers[i].m_Exec));

        str_to_upper(m_aServers[i].m_Type, str_length(m_aServers[i].m_Type));

        if (!CreatePipeProcess(m_aServers[i].m_Exec, &m_aPipes[i], m_aServers[i].m_FileConfg))
            return false;

        m_aServers[i].m_Used = true;

		if(g_Config.m_Debug)
			m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvmanager", "New local server created.");

        return true;
    }

    if(g_Config.m_Debug)
        m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvmanager", "ERROR: More local servers running!");

    return false;
}
bool CServerManager::DestroyServer(int index)
{
    if (index < 0 || index >= MAX_LOCAL_SERVERS-1)
        return false;

    if(m_aServers[index].m_Used)
        ClosePipe(&m_aPipes[index]);

    m_aServers[index].m_Used = false;

    if(g_Config.m_Debug)
        m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvmanager", "Local server closed.");

    return true;
}

bool CServerManager::SaveConfigFile(const char *filename)
{
    return false;
}

void CServerManager::GetConfigParam(const char *param, const char *filename, char *aDest, unsigned int sizeDest, unsigned int offset)
{
    IOHANDLE ConfigFile = io_open(filename, IOFLAG_READ);
    if(!ConfigFile)
    {
        str_copy(aDest, "", sizeof(sizeDest));
        return;
    }

    char buff[1024], aValue[1024];
    const unsigned int filesize = io_length(ConfigFile);
    unsigned int FoundOffSet = 0;
    int found = 0;

    memset(aValue, 0, sizeof(aValue));

    if (filesize == 0)
    {
        str_copy(aDest, "", sizeof(sizeDest));
        return;
    }

    for (int o=0; o<filesize;)
    {
        memset(buff, 0, sizeof(buff));

        for(int i=0; i<sizeof(buff); i++)
        {
            o += io_read(ConfigFile, &buff[i], 1);
            if (buff[i] == 0 || buff[i] == '\n' || (buff[i] == '\r' && buff[i+1] == '\n'))
            {
                char aParam[125];
                memset(aParam, 0, sizeof(aParam));
                memset(aValue, 0, sizeof(aValue));

                sscanf(buff, "%[^ ]s", aParam);
                if (str_comp_nocase(aParam, param) == 0)
                {
                    int offsetParam = str_length(aParam)+1;
                    for (int e=0; e<sizeof(aValue); e++)
                    {
                        if (offsetParam+e >= sizeof(buff) || buff[offsetParam+e] == 0)
                        {
                            aValue[e] = 0;
                            break;
                        }

                        aValue[e] = buff[offsetParam+e];
                    }

                    if (FoundOffSet == offset)
                        found ^= 1;

                    FoundOffSet++;
                }

                if (buff[i] == '\r')
                    o++;

                break;
            }
        }

        if (found)
            break;
    }

    io_close(ConfigFile);
    ConfigFile = 0;

    str_copy(aDest, aValue, sizeof(sizeDest));
}

int SearchServersBinCallback(const char *pName, int IsDir, int StorageType, void *pUser)
{
	CServerManager *pSelf = (CServerManager *)pUser;
	int Length = str_length(pName);
	if(IsDir || (pName[0] == '.' && (pName[1] == 0 ||
		(pName[1] == '.' && pName[2] == 0))) ||
		(!IsDir && (Length < 8 || str_comp(pName+Length-8, "_srv.exe"))))
		return 0;

    char aName[255];
    for (int i=0; i<sizeof(aName); i++)
    {
        if (i >= Length-8)
        {
            aName[i]=0;
            break;
        }

        aName[i]=pName[i];
    }

    pSelf->m_lLocalServersExecs.add(aName);

	return 0;
}

void CServerManager::SearchServersBin()
{
    m_lLocalServersExecs.clear();
	Storage()->ListDirectory(IStorageTW::TYPE_ALL, ".", SearchServersBinCallback, this);
}



/** PIPE FUNCTIONS **/
bool CServerManager::CreatePipeProcess(const char *filename, PipeInfo *pPipeInfo, void *pUser)
{
#if defined(CONF_FAMILY_WINDOWS)
    if (CreatePipe(&pPipeInfo->m_NewPipeIn, &pPipeInfo->m_PipeIn, NULL, 0) == 0)   //create stdin pipe
    {
		if(g_Config.m_Debug)
			m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvmanager", "ERROR PIPE: CreatePipe STDIN");
        dbg_msg("H-CLIENT", "PIPE IN mal");

        pPipeInfo = 0x0;
        return false;
    }

    if (CreatePipe(&pPipeInfo->m_PipeOut, &pPipeInfo->m_NewPipeOut, NULL, 0) == 0)  //create stdout pipe
    {
		if(g_Config.m_Debug)
			m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvmanager", "ERROR PIPE: CreatePipe STDOUT");
        dbg_msg("H-CLIENT", "PIPE OUT mal");

        CloseHandle(pPipeInfo->m_NewPipeIn);
        CloseHandle(pPipeInfo->m_PipeIn);

        pPipeInfo = 0x0;
        return false;
    }

    STARTUPINFO si;
    GetStartupInfo(&si);                            //set startupinfo for the spawned process
    si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = pPipeInfo->m_NewPipeOut;
    si.hStdError = pPipeInfo->m_NewPipeOut;         //set the new handles for the child process
    si.hStdInput = pPipeInfo->m_NewPipeIn;
    //spawn the child process
    char aParams[255];
    if (pUser)
        str_format(aParams, sizeof(aParams), "%s -f %s", filename, static_cast<char*>(pUser));

    dbg_msg("h-client", "PARAMETROS: %s", aParams);

    if (CreateProcess(filename, (!pUser)?NULL:aParams, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pPipeInfo->m_ProcessInformation) == 0)
    {

            /*LPTSTR errorText = NULL;
            DWORD dw = GetLastError();
            FormatMessage(
               // use system message tables to retrieve error text
               FORMAT_MESSAGE_FROM_SYSTEM
               // allocate buffer on local heap for error text
               |FORMAT_MESSAGE_ALLOCATE_BUFFER
               // Important! will fail otherwise, since we're not
               // (and CANNOT) pass insertion parameters
               |FORMAT_MESSAGE_IGNORE_INSERTS,
               NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
               dw,
               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
               (LPTSTR)&errorText,  // output
               0, // minimum size for output buffer
               NULL);   // arguments - see note

            if ( NULL != errorText )
            {
                dbg_msg("H-CLIENT", "ERROR PIPE: %s", errorText);
               LocalFree(errorText);
               errorText = NULL;
            }*/

		if(g_Config.m_Debug)
			m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvmanager", "ERROR PIPE: CreateProcess");

        dbg_msg("H-CLIENT", "PROCESO MAL");

        CloseHandle(pPipeInfo->m_NewPipeIn);
        CloseHandle(pPipeInfo->m_NewPipeOut);
        CloseHandle(pPipeInfo->m_PipeIn);
        CloseHandle(pPipeInfo->m_PipeOut);

        pPipeInfo = 0x0;
        return false;
    }
    dbg_msg("H-CLIENT", "PROCESO CREADO");
#endif
    return true;
}

int CServerManager::ReadPipe(PipeInfo *pPipeInfo, char *buff, unsigned int sizeBuff)
{
#if defined(CONF_FAMILY_WINDOWS)
    unsigned long bread;   //bytes read
    unsigned long avail;   //bytes available

    memset(buff, 0, sizeBuff);

    PeekNamedPipe(pPipeInfo->m_PipeOut, reinterpret_cast<LPVOID>(buff), sizeBuff, &bread, &avail, NULL);

    if (avail == 0)
        return 0;

    ReadFile(pPipeInfo->m_PipeOut, reinterpret_cast<LPVOID>(buff), sizeBuff, &bread, NULL);  //read the stdout pipe

    return (avail - sizeBuff);
#endif

    return -1;
}

void CServerManager::ClosePipe(PipeInfo *pPipeInfo)
{
#if defined(CONF_FAMILY_WINDOWS)
    CloseHandle(pPipeInfo->m_ProcessInformation.hThread);
    CloseHandle(pPipeInfo->m_ProcessInformation.hProcess);
    CloseHandle(pPipeInfo->m_NewPipeOut);
    CloseHandle(pPipeInfo->m_NewPipeIn);
    CloseHandle(pPipeInfo->m_PipeOut);
    CloseHandle(pPipeInfo->m_NewPipeIn);

    pPipeInfo = 0x0;
#endif
}
