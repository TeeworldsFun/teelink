/* (c) unsigned char*. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at https://github.com/CytraL/HClient */
#include <base/math.h>
#include <base/system.h>
#include <game/version.h>
#include <game/client/components/menus.h>
#include <engine/external/json-parser/json.h>
#include <engine/shared/http_downloader.h>
#include <engine/client/updater.h>
#if defined(CONF_FAMILY_UNIX)
    #include <unistd.h>
#elif defined(CONF_FAMILY_WINDOWS)
    #include <windows.h>
#endif
#include <cstring>
#include <cstdio>

#define UPDATES_HOST            "hclient-updater.redneboa.es"
#define UPDATES_BASE_PATH       "/"
#define UPDATES_MANIFEST_FILE   "updates.json"

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
    for (int i=0; i<m_vToRemove.size();)
    {
        if (str_comp(m_vToRemove[i].c_str(), pFile) == 0) m_vToRemove.remove_index(i);
        else ++i;
    }

    // Check if already in the list
    for (int i=0; i<m_vToDownload.size(); i++)
        if (str_comp(m_vToDownload[i].c_str(), pFile) == 0) return;

    m_vToDownload.add(pFile);
}

void CUpdater::AddFileToRemove(const char *pFile)
{
    // Remove it from download list
    for (int i=0; i<m_vToDownload.size();)
    {
        if (str_comp(m_vToDownload[i].c_str(), pFile) == 0) m_vToDownload.remove_index(i);
        else ++i;
    }

    // Check if already in the list
    for (int i=0; i<m_vToRemove.size(); i++)
        if (str_comp(m_vToRemove[i].c_str(), pFile) == 0) return;

    m_vToRemove.add(pFile);
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
	if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH UPDATES_MANIFEST_FILE, UPDATES_MANIFEST_FILE))
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

        if (needUpdate) pMenus->SetPopup(CMenus::POPUP_UPDATER);
        else m_Updated = true;
    }

    io_close(fileAutoUpdate);
    fs_remove(UPDATES_MANIFEST_FILE);
}

void CUpdater::DoUpdates(CMenus *pMenus)
{
    bool noErrors = true;

    // Remove Files
    for (int i=0; i<m_vToRemove.size(); i++)
        if (fs_is_file(m_vToRemove[i].c_str()) && fs_remove(m_vToRemove[i].c_str()) != 0) noErrors = false;
    m_vToRemove.clear();

    // Download Files
    for (int i=0; i<m_vToDownload.size(); i++)
        if (!CHttpDownloader::GetToFile(m_vToDownload[i].c_str(), m_vToDownload[i].c_str())) noErrors = false;
    m_vToDownload.clear();

    if (m_NeedUpdateClient)
    {
        #ifdef CONF_FAMILY_WINDOWS
            #ifdef CONF_PLATFORM_WIN64
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds64.exe", "tw_tmp"))
            #else
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds.exe", "tw_tmp"))
            #endif
        #elif defined(CONF_FAMILY_UNIX)
            #ifdef CONF_PLATFORM_MACOSX
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds_mac", "tw_tmp"))
            #elif defined(CONF_ARCH_IA64) || defined(CONF_ARCH_AMD64)
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds64", "tw_tmp"))
            #else
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds", "tw_tmp"))
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
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds_srv64.exe", "teeworlds_srv.exe"))
            #else
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds_srv.exe", "teeworlds_srv.exe"))
            #endif
        #elif defined(CONF_FAMILY_UNIX)
            #ifdef CONF_PLATFORM_MACOSX
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds_srv_mac", "teeworlds_srv"))
            #elif defined(CONF_ARCH_IA64) || defined(CONF_ARCH_AMD64)
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds_srv64", "teeworlds_srv"))
            #else
                if (!CHttpDownloader::GetToFile("http://" UPDATES_HOST UPDATES_BASE_PATH "teeworlds_srv", "teeworlds_srv"))
            #endif
        #endif
        {
            noErrors = false;
        }
    }

    if (noErrors)
        m_Updated = true;

    pMenus->SetPopup(CMenus::POPUP_UPDATER_RESULT);
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
    pInfoThread->m_pUpdater->DoUpdates(pInfoThread->m_pMenus);
    lock_unlock(m_UpdatesLock);
}
