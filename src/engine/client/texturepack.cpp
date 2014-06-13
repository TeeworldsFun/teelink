#include "texturepack.h"

#include <engine/storage.h>
#include <engine/shared/config.h>
#include <game/generated/client_data.h>
#include <string>

void CTexturePack::Init()
{
    m_pStorage = Kernel()->RequestInterface<IStorage>();
    m_pGraphics = Kernel()->RequestInterface<IGraphics>();
    m_pTextRender = Kernel()->RequestInterface<ITextRender>();
    m_pSound = Kernel()->RequestInterface<ISound>();
    SearchThemes();
}

bool CTexturePack::Load(const char *rawpackname)
{
    if (!m_pStorage)
        return false;

    str_copy(m_RawPackName, rawpackname, sizeof(m_RawPackName));

    // Load Colors
    LoadStyle();

    // Load Font
    LoadFont();

    // Load Images
    LoadImages();

	// Load Sounds
	//LoadSounds();

    return true;
}

void CTexturePack::SearchThemes()
{
    m_pStorage->ListDirectory(1, "themes", ThemeScan, this);
}

void CTexturePack::LoadTheme(const char *themeName)
{
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "themes/%s/info.tws", themeName);
	IOHANDLE InfoFile = m_pStorage->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
	if (!InfoFile)
        return;

    CTheme theme;
	char buff[255];
	while (io_read_line(InfoFile, buff, sizeof(buff)) > 0)
    {
        std::string line(buff);
        std::string::size_type dpos = line.find_first_of('=');

        if (line.empty() || dpos == std::string::npos || dpos == line.length() || line.at(0) == '#')
            continue;

        std::string paramName(line.substr(0, dpos));
        std::string paramValue(line.substr(dpos+1));

        if (paramName.compare("packname") == 0)
            str_copy(theme.m_Name, paramValue.c_str(), sizeof(theme.m_Name));
        else if (paramName.compare("version") == 0)
            str_copy(theme.m_Version, paramValue.c_str(), sizeof(theme.m_Version));
        else if (paramName.compare("author") == 0)
            str_copy(theme.m_Author, paramValue.c_str(), sizeof(theme.m_Author));
        else if (paramName.compare("mail") == 0)
            str_copy(theme.m_Mail, paramValue.c_str(), sizeof(theme.m_Mail));
        else if (paramName.compare("website") == 0)
            str_copy(theme.m_Web, paramValue.c_str(), sizeof(theme.m_Web));
    }
	io_close(InfoFile);

    str_copy(theme.m_FolderName, themeName, sizeof(theme.m_FolderName));
    AddTheme(theme);
}

void CTexturePack::LoadStyle()
{
    char path[255];
    str_format(path, sizeof(path), "themes/%s/style.tws", m_RawPackName);
    IOHANDLE styleFile = m_pStorage->OpenFile(path, IOFLAG_READ, IStorage::TYPE_ALL);

    if (styleFile)
    {
        char buff[255];
        while (io_read_line(styleFile, buff, sizeof(buff)))
        {
            std::string line(buff);
            std::string::size_type dpos = line.find_first_of('=');

            if (line.empty() || dpos == std::string::npos || dpos == line.length() || line.at(0) == '#')
                continue;

            std::string paramName(line.substr(0, dpos));
            std::string paramValue(line.substr(dpos+1));

            if (paramName.compare("list_header_background_color") == 0)
                SET_THEME_VALUE(hcListHeaderBackgroundColor, paramValue.c_str())
            else if (paramName.compare("list_header_text_color") == 0)
                SET_THEME_VALUE(hcListHeaderTextColor, paramValue.c_str())
            else if (paramName.compare("list_footer_background_color") == 0)
                SET_THEME_VALUE(hcListFooterBackgroundColor, paramValue.c_str())
            else if (paramName.compare("list_footer_text_color") == 0)
                SET_THEME_VALUE(hcListFooterTextColor, paramValue.c_str())
            else if (paramName.compare("list_background_color") == 0)
                SET_THEME_VALUE(hcListBackgroundColor, paramValue.c_str())
            else if (paramName.compare("list_text_color") == 0)
                SET_THEME_VALUE(hcListTextColor, paramValue.c_str())
            else if (paramName.compare("list_item_selected_color") == 0)
                SET_THEME_VALUE(hcListItemSelectedColor, paramValue.c_str())
            else if (paramName.compare("list_item_odd_color") == 0)
                SET_THEME_VALUE(hcListItemOddColor, paramValue.c_str())
            else if (paramName.compare("list_column_selected_color") == 0)
                SET_THEME_VALUE(hcListColumnSelectedColor, paramValue.c_str())
            else if (paramName.compare("serverbrowser_list_group_header_background_color") == 0)
                SET_THEME_VALUE(hcServerbrowserListGroupHeaderBackgroundColor, paramValue.c_str())
            else if (paramName.compare("serverbrowser_list_group_header_text_color") == 0)
                SET_THEME_VALUE(hcServerbrowserListGroupHeaderTextColor, paramValue.c_str())
            else if (paramName.compare("serverbrowser_list_extra_info_background_color") == 0)
                SET_THEME_VALUE(hcServerbrowserListExtraInfoBackgroundColor, paramValue.c_str())
            else if (paramName.compare("serverbrowser_list_extra_info_text_color") == 0)
                SET_THEME_VALUE(hcServerbrowserListExtraInfoTextColor, paramValue.c_str())
            else if (paramName.compare("container_header_background_color") == 0)
                SET_THEME_VALUE(hcContainerHeaderBackgroundColor, paramValue.c_str())
            else if (paramName.compare("container_header_text_color") == 0)
                SET_THEME_VALUE(hcContainerHeaderTextColor, paramValue.c_str())
            else if (paramName.compare("container_background_color") == 0)
                SET_THEME_VALUE(hcContainerBackgroundColor, paramValue.c_str())
            else if (paramName.compare("container_text_color") == 0)
                SET_THEME_VALUE(hcContainerTextColor, paramValue.c_str())
            else if (paramName.compare("subcontainer_header_background_color") == 0)
                SET_THEME_VALUE(hcSubcontainerHeaderBackgroundColor, paramValue.c_str())
            else if (paramName.compare("subcontainer_header_text_color") == 0)
                SET_THEME_VALUE(hcSubcontainerHeaderTextColor, paramValue.c_str())
            else if (paramName.compare("subcontainer_background_color") == 0)
                SET_THEME_VALUE(hcSubcontainerBackgroundColor, paramValue.c_str())
            else if (paramName.compare("subcontainer_text_color") == 0)
                SET_THEME_VALUE(hcSubcontainerTextColor, paramValue.c_str())
            else if (paramName.compare("popup_header_background_color") == 0)
                SET_THEME_VALUE(hcPopupHeaderBackgroundColor, paramValue.c_str())
            else if (paramName.compare("popup_header_text_color") == 0)
                SET_THEME_VALUE(hcPopupHeaderTextColor, paramValue.c_str())
            else if (paramName.compare("popup_background_color") == 0)
                SET_THEME_VALUE(hcPopupBackgroundColor, paramValue.c_str())
            else if (paramName.compare("popup_text_color") == 0)
                SET_THEME_VALUE(hcPopupTextColor, paramValue.c_str())
            else if (paramName.compare("editbox_background_color") == 0)
                SET_THEME_VALUE(hcEditboxBackgroundColor, paramValue.c_str())
            else if (paramName.compare("editbox_text_color") == 0)
                SET_THEME_VALUE(hcEditboxTextColor, paramValue.c_str())
            else if (paramName.compare("button_background_color") == 0)
                SET_THEME_VALUE(hcButtonBackgroundColor, paramValue.c_str())
            else if (paramName.compare("button_text_color") == 0)
                SET_THEME_VALUE(hcButtonTextColor, paramValue.c_str())
            else if (paramName.compare("trackbar_background_color") == 0)
                SET_THEME_VALUE(hcTrackbarBackgroundColor, paramValue.c_str())
            else if (paramName.compare("trackbar_slider_background_color") == 0)
                SET_THEME_VALUE(hcTrackbarSliderBackgroundColor, paramValue.c_str())
            else if (paramName.compare("progressbar_background_color") == 0)
                SET_THEME_VALUE(hcProgressbarBackgroundColor, paramValue.c_str())
            else if (paramName.compare("progressbar_slider_background_color") == 0)
                SET_THEME_VALUE(hcProgressbarSliderBackgroundColor, paramValue.c_str())
            else if (paramName.compare("mainmenu_text_color") == 0)
                SET_THEME_VALUE(hcMainmenuTextColor, paramValue.c_str())
            else if (paramName.compare("mainmenu_background_top_color") == 0)
                SET_THEME_VALUE(hcMainmenuBackgroundTopColor, paramValue.c_str())
            else if (paramName.compare("mainmenu_background_bottom_color") == 0)
                SET_THEME_VALUE(hcMainmenuBackgroundBottomColor, paramValue.c_str())
            else if (paramName.compare("paneltab_selected_background_color") == 0)
                SET_THEME_VALUE(hcPaneltabSelectedBackgroundColor, paramValue.c_str())
            else if (paramName.compare("paneltab_selected_text_color") == 0)
                SET_THEME_VALUE(hcPaneltabSelectedTextColor, paramValue.c_str())
            else if (paramName.compare("paneltab_background_color") == 0)
                SET_THEME_VALUE(hcPaneltabBackgroundColor, paramValue.c_str())
            else if (paramName.compare("paneltab_text_color") == 0)
                SET_THEME_VALUE(hcPaneltabTextColor, paramValue.c_str())
            else if (paramName.compare("settings_paneltab_selected_background_color") == 0)
                SET_THEME_VALUE(hcSettingsPaneltabSelectedBackgroundColor, paramValue.c_str())
            else if (paramName.compare("settings_paneltab_selected_text_color") == 0)
                SET_THEME_VALUE(hcSettingsPaneltabSelectedTextColor, paramValue.c_str())
            else if (paramName.compare("settings_paneltab_background_color") == 0)
                SET_THEME_VALUE(hcSettingsPaneltabBackgroundColor, paramValue.c_str())
            else if (paramName.compare("settings_paneltab_text_color") == 0)
                SET_THEME_VALUE(hcSettingsPaneltabTextColor, paramValue.c_str())
        }

        io_close(styleFile);
    }
}

void CTexturePack::LoadFont()
{
    char path[255];
	char aFilename[512];
	str_format(path, sizeof(path), "themes/%s/font.ttf", m_RawPackName); // FIXME: Not use generic name and add option for select font size
	IOHANDLE File = m_pStorage->OpenFile(path, IOFLAG_READ, IStorage::TYPE_ALL, aFilename, sizeof(aFilename));
	if(File)
	{
		io_close(File);
        m_pTextRender->SetFont(m_pTextRender->LoadFont(aFilename));
	} else
	{
	    File = m_pStorage->OpenFile("fonts/DejaVuSans.ttf", IOFLAG_READ, IStorage::TYPE_ALL, aFilename, sizeof(aFilename));
	    if (File)
        {
            io_close(File);
            m_pTextRender->SetFont(m_pTextRender->LoadFont(aFilename));
        }
	}
}

void CTexturePack::LoadImages()
{
    char aBuf[255];
    for (int i=0; i<g_pData->m_NumImages; i++)
    {
        mem_zero(aBuf, sizeof(aBuf));
        str_format(aBuf, sizeof(aBuf), "themes/%s/%s", g_Config.m_hcTheme, g_pData->m_aImages[i].m_pFilename);
        IOHANDLE File = m_pStorage->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
        if (!File)
            str_copy(aBuf, g_pData->m_aImages[i].m_pFilename, sizeof(aBuf));
        else
            io_close(File);

        if (g_pData->m_aImages[i].m_Id != -1)
            Graphics()->UnloadTexture(g_pData->m_aImages[i].m_Id);
        g_pData->m_aImages[i].m_Id = Graphics()->LoadTexture(aBuf, IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
    }
}

void CTexturePack::LoadSounds()
{
    char aBuf[255];
    m_pSound->StopAll();
    for (int s=0; s<g_pData->m_NumSounds; s++)
    {
		for(int i = 0; i < g_pData->m_aSounds[s].m_NumSounds; i++)
		{
            str_format(aBuf, sizeof(aBuf), "themes/%s/%s", g_Config.m_hcTheme, g_pData->m_aSounds[s].m_aSounds[i].m_pFilename);
            IOHANDLE file = m_pStorage->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
            if (!file)
                str_copy(aBuf, g_pData->m_aSounds[s].m_aSounds[i].m_pFilename, sizeof(aBuf));
            else
                io_close(file);

			g_pData->m_aSounds[s].m_aSounds[i].m_Id = m_pSound->LoadWV(aBuf);
		}
    }
}

int CTexturePack::ThemeScan(const char *pName, int IsDir, int DirType, void *pUser)
{
    CTexturePack *pSelf = (CTexturePack*)pUser;
	if(!IsDir)
		return 0;

    pSelf->LoadTheme(pName);
	return 0;
}
