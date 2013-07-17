/*
    unsigned char*
*/
#ifndef ENGINE_CLIENT_SERVERMANAGER_H
#define ENGINE_CLIENT_SERVERMANAGER_H

#ifdef CONF_FAMILY_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#include <engine/servermanager.h>
#include <base/tl/array.h>
#include <string>

class CServerManager : public IServerManager
{
public:
    /** H-Client **/
    struct PipeInfo
    {
    #if defined(CONF_FAMILY_WINDOWS)
        HANDLE m_PipeIn;
        HANDLE m_NewPipeIn;
        HANDLE m_PipeOut;
        HANDLE m_NewPipeOut;
        PROCESS_INFORMATION m_ProcessInformation;
    #endif
    };

	CServerManager();

	IServerManager::CMServerEntry m_aServers[MAX_LOCAL_SERVERS];
	array<std::string> m_lLocalServersExecs;

	void Init();
	void ShutdownAll();

	void SearchServersBin();

    IServerManager::CMServerEntry GetInfoServer(int index) const { return m_aServers[index]; }
    bool CreateServer(char *fileconfg, char *exec);
	bool DestroyServer(int index);

	std::string GetExec(int index) { return m_lLocalServersExecs[index]; }
	int NumBinExecs() const { return m_lLocalServersExecs.size(); }

	bool SaveConfigFile(const char *filename);
	void GetConfigParam(const char *param, const char *filename, char *aDest, unsigned int sizeDest, unsigned int offset = 0);

private:
    PipeInfo m_aPipes[MAX_LOCAL_SERVERS];

    class IConsole *m_pConsole;
    class IStorageTW *m_pStorage;

    class IStorageTW *Storage() { return m_pStorage; }

    bool CreatePipeProcess(const char *filename, PipeInfo *pPipeInfo, void *pUser = 0x0);
    int ReadPipe(PipeInfo *pPipeInfo, char *buff, unsigned int sizeBuff);
    void ClosePipe(PipeInfo *pPipeInfo);
};

int SearchServersBinCallback(const char *pName, int IsDir, int StorageType, void *pUser);

#endif

