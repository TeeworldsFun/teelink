/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SKINS_H
#define GAME_CLIENT_COMPONENTS_SKINS_H
#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <game/client/component.h>
#include <map>
#include <string>

class CSkins : public CComponent
{
public:
	// do this better and nicer
	struct CSkin
	{
		int m_OrgTexture;
		int m_ColorTexture;
		char m_aName[24];
		vec3 m_BloodColor;

		bool operator<(const CSkin &Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};

	struct InfoDownloadSkinThread
	{
        CSkins *m_pSkins;
        char m_SkinName[64];
    };

	void OnInit();

	vec3 GetColorV3(int v);
	vec4 GetColorV4(int v);
	int Num();
	const CSkin *Get(int Index);
	int Find(const char *pName, bool tryDownload = false); // H-Client

    // H-Client
	static int SkinScan(const char *pName, int IsDir, int DirType, void *pUser);
	int LoadSkinFromFile(const char *pPath, const char *pName, int DirType);
	void DownloadSkin(const char *pName);
	bool IsDownloding(std::string name)
	{
		std::map<std::string, bool>::iterator it = m_DownloadedSkinsSet.find(name);
		return (it != m_DownloadedSkinsSet.end() && !it->second);
	}
	//

private:
	sorted_array<CSkin> m_aSkins;
	std::map<std::string, bool> m_DownloadedSkinsSet;
};
void ThreadDownloadSkin(void *params);
#endif
