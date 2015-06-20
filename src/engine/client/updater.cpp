#include <base/math.h>
#include <base/system.h>
#include <game/version.h>
#include <cstring>
#include <cstdio>
#include <algorithm>
#if defined(CONF_FAMILY_UNIX)
    #include <unistd.h>
#elif defined(CONF_FAMILY_WINDOWS)
    #include <windows.h>
#endif
#include <engine/external/json-parser/json.h>
#include <game/client/components/menus.h>
#include <engine/client/updater.h>

#define UPDATES_HOST            "hclient-updater.redneboa.es"
#define UPDATES_BASE_PATH       "/"
#define UPDATES_MANIFEST_FILE   "updates.json"

static NETSOCKET invalid_socket = {NETTYPE_INVALID, -1, -1};
static LOCK m_UpdatesLock = 0;

CUpdater::CUpdater()
{
    m_UpdatesLock = lock_create();
	Reset();
}

void CUpdater::Reset()
{
	m_NeedUpdateClient = false;
	m_NeedUpdateServer = false;
	m_Updated = false;
	mem_zero(m_NewVersion, sizeof(m_NewVersion));
	mem_zero(m_CurrentDownloadFileName, sizeof(m_CurrentDownloadFileName));
	m_CurrentDownloadProgress = 0;
}

void CUpdater::AddFileToDownload(const char *pFile)
{
    // Remove it from remove list
    for (std::vector<std::string>::iterator it=m_vToRemove.begin(); it!=m_vToRemove.end();)
    {
        if (str_comp(it->c_str(), pFile) == 0) it = m_vToRemove.erase(it++);
        else ++it;
    }

    // Check if already in the list
    for (std::vector<std::string>::iterator it=m_vToDownload.begin(); it!=m_vToDownload.end(); ++it)
        if (str_comp(it->c_str(), pFile) == 0) return;

    m_vToDownload.push_back(pFile);
}

void CUpdater::AddFileToRemove(const char *pFile)
{
    // Remove it from download list
    for (std::vector<std::string>::iterator it=m_vToDownload.begin(); it!=m_vToDownload.end();)
    {
        if (str_comp(it->c_str(), pFile) == 0) it = m_vToDownload.erase(it++);
        else ++it;
    }

    // Check if already in the list
    for (std::vector<std::string>::iterator it=m_vToRemove.begin(); it!=m_vToRemove.end(); ++it)
        if (str_comp(it->c_str(), pFile) == 0) return;

    m_vToRemove.push_back(pFile);
}

void CUpdater::ExecuteExit()
{
	if (!NeedResetClient() || !m_Updated)
		return;

    SelfDelete();

    #ifdef CONF_FAMILY_WINDOWS
        ShellExecuteA(0,0,"du.bat",0,0,SW_HIDE);
    #else
        if (fs_rename("tw_tmp","teeworlds"))
            dbg_msg("autoupdate", "Error renaming binary file");
        if (system("chmod +x teeworlds"))
            dbg_msg("autoupdate", "Error setting executable bit");

        pid_t pid;
        pid = fork();
        if (pid == 0)
        {
            char* argv[1];
            argv[0] = NULL;
            execv("teeworlds", argv);
        }
        else
            return;
    #endif
}

void CUpdater::CheckUpdates(CMenus *pMenus)
{
	dbg_msg("autoupdate", "Checking for updates...");
	if (!GetFile(UPDATES_MANIFEST_FILE, UPDATES_MANIFEST_FILE))
	{
		dbg_msg("autoupdate", "Error downloading updates manifest :/");
		return;
	}

    Reset();

    IOHANDLE fileAutoUpdate = io_open(UPDATES_MANIFEST_FILE, IOFLAG_READ);
    io_seek(fileAutoUpdate, 0, IOSEEK_END);
    std::streamsize size = io_tell(fileAutoUpdate);
    io_seek(fileAutoUpdate, 0, IOSEEK_START);

    std::vector<char> buffer(size);
    if (io_read(fileAutoUpdate, buffer.data(), size))
    {
        bool needUpdate = false;

        json_settings JsonSettings;
        mem_zero(&JsonSettings, sizeof(JsonSettings));
        char aError[256];
        json_value *pJsonNodeMain = json_parse_ex(&JsonSettings, buffer.data(), size, aError);
        if (pJsonNodeMain == 0)
        {
            dbg_msg("autoupdate", "Error: %s", aError);
            return;
        }

        int verCode = -1;
        for(int j=pJsonNodeMain->u.object.length-1; j>=0; j--) // Ascendent Search: Manifest has descendant order
        {
            sscanf((const char *)pJsonNodeMain->u.object.values[j].name, "%d", &verCode);
            json_value *pNodeCode = pJsonNodeMain->u.object.values[j].value;

            if (verCode <= HCLIENT_VERSION_CODE)
                continue;

            needUpdate = true;

            const json_value &rVersion = (*pNodeCode)["version"];
            str_copy(m_NewVersion, (const char *)rVersion, sizeof(m_NewVersion));

            // Need update client?
            const json_value &rClient = (*pNodeCode)["client"];
            m_NeedUpdateClient = (rClient.u.boolean);

            // Need update server?
            const json_value &rServer = (*pNodeCode)["server"];
            m_NeedUpdateServer = (rServer.u.boolean);

            // Get files to download
            const json_value &rDownload = (*pNodeCode)["download"];
            for(unsigned k = 0; k < rDownload.u.array.length; k++)
                AddFileToDownload((const char *)rDownload[k]);
            // Get files to remove
            const json_value &rRemove = (*pNodeCode)["remove"];
            for(unsigned k = 0; k < rRemove.u.array.length; k++)
                AddFileToRemove((const char *)rRemove[k]);
        }

        if (needUpdate) pMenus->SetPopup(CMenus::POPUP_AUTOUPDATE);
        else m_Updated = true;
    }

    io_close(fileAutoUpdate);
    fs_remove(UPDATES_MANIFEST_FILE);
}

void CUpdater::DoUpdates(CMenus *pMenus)
{
    bool noErrors = true;

    // Remove Files
    for (std::vector<std::string>::iterator it=m_vToRemove.begin(); it!=m_vToRemove.end(); ++it)
        if (fs_is_file(it->c_str()) && fs_remove(it->c_str()) != 0) noErrors = false;
    m_vToRemove.clear();

    // Download Files
    for (std::vector<std::string>::iterator it=m_vToDownload.begin(); it!=m_vToDownload.end(); ++it)
        if (!GetFile(it->c_str(), it->c_str())) noErrors = false;
    m_vToDownload.clear();

    if (m_NeedUpdateClient)
    {
        #ifdef CONF_FAMILY_WINDOWS
            #ifdef CONF_PLATFORM_WIN64
                if (!GetFile("teeworlds64.exe", "tw_tmp"))
            #else
                if (!GetFile("teeworlds.exe", "tw_tmp"))
            #endif
        #elif defined(CONF_FAMILY_UNIX)
            #ifdef CONF_PLATFORM_MACOSX
                if (!GetFile("teeworlds_mac", "tw_tmp"))
            #elif defined(CONF_ARCH_IA64) || defined(CONF_ARCH_AMD64)
                if (!GetFile("teeworlds64", "tw_tmp"))
            #else
                if (!GetFile("teeworlds", "tw_tmp"))
            #endif
        #endif
        {
            noErrors = false;
        }
    }

    if (m_NeedUpdateServer)
    {
        #ifdef CONF_FAMILY_WINDOWS
            #ifdef CONF_PLATFORM_WIN64
                if (!GetFile("teeworlds_srv64.exe", "teeworlds_srv.exe"))
            #else
                if (!GetFile("teeworlds_srv.exe", "teeworlds_srv.exe"))
            #endif
        #elif defined(CONF_FAMILY_UNIX)
            #ifdef CONF_PLATFORM_MACOSX
                if (!GetFile("teeworlds_srv_mac", "teeworlds_srv"))
            #elif defined(CONF_ARCH_IA64) || defined(CONF_ARCH_AMD64)
                if (!GetFile("teeworlds_srv64", "teeworlds_srv"))
            #else
                if (!GetFile("teeworlds_srv", "teeworlds_srv"))
            #endif
        #endif
        {
            noErrors = false;
        }
    }

    if (noErrors)
        m_Updated = true;

    pMenus->SetPopup(CMenus::POPUP_AUTOUPDATE_RESULT);
}

// TODO: Ugly but works
bool CUpdater::GetFile(const char *url, const char *path)
{
	NETSOCKET Socket = invalid_socket;
	NETADDR HostAddress, BindAddr;
    mem_zero(&HostAddress, sizeof(HostAddress));
    mem_zero(&BindAddr, sizeof(BindAddr));
	char aNetBuff[1024];

	//Lookup
	if(net_host_lookup(UPDATES_HOST, &HostAddress, NETTYPE_IPV4) != 0)
	{
		dbg_msg("autoupdate", "Error running host lookup");
		return false;
	}
	HostAddress.port = 80;

	//Connect
    BindAddr.type = NETTYPE_IPV4;
	Socket = net_tcp_create(BindAddr);
	if(net_tcp_connect(Socket, &HostAddress) != 0)
	{
		net_tcp_close(Socket);
		dbg_msg("autoupdate","Error connecting to host");
		return false;
	}

	//Send request
	str_format(aNetBuff, sizeof(aNetBuff), "GET "UPDATES_BASE_PATH"%s HTTP/1.0\r\nHost: "UPDATES_HOST"\r\n\r\n", url);
	net_tcp_send(Socket, aNetBuff, str_length(aNetBuff));

	//read data
	IOHANDLE dstFile = 0;

	std::string NetData;
	int TotalRecv = 0, TotalBytes = 0, CurrentRecv = 0, enterCtrl = 0;
	bool isHead = true, isStatusLine = true;
	while ((CurrentRecv = net_tcp_recv(Socket, aNetBuff, sizeof(aNetBuff))) > 0)
	{
		for (int i=0; i<CurrentRecv ; i++)
		{
			if (isHead)
			{
				if (aNetBuff[i]=='\n')
				{
					if (++enterCtrl == 2) // Go To Body Part
					{
                        dstFile = io_open(path, IOFLAG_WRITE);
                        if (!dstFile)
                        {
                            net_tcp_close(Socket);
                            dbg_msg("autoupdate","Error writing to disk");
                            return false;
                        }

                        str_copy(m_CurrentDownloadFileName, url, sizeof(m_CurrentDownloadFileName));

						isHead = false;
						NetData.clear();
						continue;
					}

                    std::transform(NetData.begin(), NetData.end(), NetData.begin(), ::tolower);

                    // Check Status
                    if (isStatusLine)
                    {
                        bool isCodeOk = NetData.find("200") != std::string::npos;
                        bool isStatusOk = NetData.find("ok") != std::string::npos;

                        if (!isCodeOk || !isStatusOk)
                        {
                            net_tcp_close(Socket);
                            dbg_msg("autoupdate","Server Host returns error code");
                            return false;
                        }
                    }
                    isStatusLine = false;

                    // Get File Size
                    std::size_t fpos = std::string::npos;
					if ((fpos=NetData.find("content-length:")) != std::string::npos)
                        sscanf(NetData.substr(fpos+15).c_str(), "%d", &TotalBytes);

					NetData.clear();
					continue;
				}
				else if (aNetBuff[i]!='\r')
					enterCtrl=0;

				NetData+=aNetBuff[i];
			}
			else // Body Part
			{
				if (TotalBytes <= 0)
				{
					io_close(dstFile);
					net_tcp_close(Socket);
					dbg_msg("autoupdate","Error receiving file");
					return false;
				}

                m_CurrentDownloadProgress = (float)TotalRecv/(float)TotalBytes;

				io_write(dstFile, &aNetBuff[i], 1);

				TotalRecv++;
				if (TotalRecv == TotalBytes)
					break;
			}
		}
	}

	//Finish
	io_close(dstFile);
	net_tcp_close(Socket);

	return true;
}

bool CUpdater::SelfDelete()
{
	#ifdef CONF_FAMILY_WINDOWS
        IOHANDLE bhFile = io_open("du.bat", IOFLAG_WRITE);
        if (!bhFile)
            return false;

        const char *fileCode = ":_R\r\ndel \"teeworlds.exe\"\r\nif exist \"teeworlds.exe\" goto _R\r\nrename \"tw_tmp.exe\" \"teeworlds.exe\"\r\n:_T\r\nif not exist \"teeworlds.exe\" goto _T\r\nstart teeworlds.exe\r\ndel \"du.bat\"\r\n\0";
        io_write(bhFile, fileCode, str_length(fileCode));
        io_close(bhFile);
	#else
		fs_remove("teeworlds");
	#endif

	return true;
}

void ThreadUpdates(void *params)
{
    InfoUpdatesThread *pInfoThread = static_cast<InfoUpdatesThread*>(params);

    lock_wait(m_UpdatesLock);
    pInfoThread->m_pAutoUpdate->DoUpdates(pInfoThread->m_pMenus);
    lock_unlock(m_UpdatesLock);
}
