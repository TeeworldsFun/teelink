/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_TEXTUREPACK_H
#define ENGINE_CLIENT_TEXTUREPACK_H

#include <engine/texturepack.h>
#include <engine/storage.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/sound.h>

#define SET_THEME_VALUE(a,b) str_copy(g_Config.m_##a, b, sizeof(g_Config.m_##a));

class CTexturePack : public ITexturePack
{
    IStorage *m_pStorage;
	IGraphics *m_pGraphics;
	ITextRender *m_pTextRender;
	ISound *m_pSound;

	char m_RawPackName[128];
	std::list<CTheme> m_Themes;

public:
	IStorage* Storage() const { return m_pStorage; }
	IGraphics* Graphics() const { return m_pGraphics; }
	ITextRender* TextRender() const { return m_pTextRender; }
	ISound* Sound() const { return m_pSound; }

    virtual void Init();
    virtual bool Load(const char *rawpackname);
    virtual std::list<CTheme> GetThemes() { return m_Themes; }

    void LoadTheme(const char *theme);

private:
    void LoadStyle();
    void LoadFont();
    void LoadImages();
    void LoadSounds();
    void SearchThemes();
    void AddTheme(CTheme theme) { m_Themes.push_back(theme); }

    static int ThemeScan(const char *pName, int IsDir, int DirType, void *pUser);
};
#endif
