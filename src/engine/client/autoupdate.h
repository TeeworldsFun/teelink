/*
    unsigned char*
*/
#ifndef ENGINE_CLIENT_AUTOUPDATE_H
#define ENGINE_CLIENT_AUTOUPDATE_H

#include <base/system.h>
#include <engine/autoupdate.h>
#include <string>
#include <vector>

class CAutoUpdate : public IAutoUpdate
{
public:
	CAutoUpdate();

	void Reset();

	void CheckUpdates(CMenus *pMenus);
	void DoUpdates(CMenus *pMenus);
	bool Updated() { return m_Updated; }
	bool NeedResetClient() { return m_NeedUpdateClient; }
	const char* GetNewVersion() const { return m_NewVersion; }
	std::vector<std::string>& GetFilesToRemove() { return m_vToRemove; }
	std::vector<std::string>& GetFilesToDownload() { return m_vToDownload; }

	void ExecuteExit();

private:
    std::vector<std::string> m_vToDownload;
    std::vector<std::string> m_vToRemove;
	bool m_Updated;
	bool m_NeedUpdateClient;
	bool m_NeedUpdateServer;
	int m_CurrentVersionCode;
	char m_NewVersion[6];

protected:
	bool SelfDelete();
	bool GetFile(const char *pToDownload, const char *pToPath);
	bool CanUpdate(const char *pFile);
	void AddFileToDownload(const char *pFile);
	void AddFileToRemove(const char *pFile);
};

#endif
