/*
    unsigned char*
*/
#ifndef ENGINE_CLIENT_UPDATER_H
#define ENGINE_CLIENT_UPDATER_H

#include <base/system.h>
#include <engine/updater.h>

class CUpdater : public IUpdater
{
public:
	CUpdater();

	void Reset();

	void CheckUpdates(CMenus *pMenus);
	void DoUpdates(CMenus *pMenus);

	bool Updated() { return m_Updated; }
	bool NeedResetClient() const { return m_NeedUpdateClient; }
	bool NeedUpdateClient() const { return m_NeedUpdateClient; }
	bool NeedUpdateServer() const { return m_NeedUpdateServer; }
	const char* GetNewVersion() const { return m_NewVersion; }
	array<std::string>& GetFilesToRemove() { return m_vToRemove; }
	array<std::string>& GetFilesToDownload() { return m_vToDownload; }

	const char* GetCurrentDownloadFileName() const { return m_CurrentDownloadFileName; }
	float GetCurrentDownloadProgress() { return m_CurrentDownloadProgress; }

	void ExecuteExit();

private:
    array<std::string> m_vToDownload;
    array<std::string> m_vToRemove;
	bool m_Updated;
	bool m_NeedUpdateClient;
	bool m_NeedUpdateServer;
	char m_NewVersion[6];

    char m_CurrentDownloadFileName[128];
	float m_CurrentDownloadProgress;

protected:
	bool SelfDelete();
	bool GetFile(const char *url, const char *path);
	bool CanUpdate(const char *pFile);
	void AddFileToDownload(const char *pFile);
	void AddFileToRemove(const char *pFile);
};

#endif
