#include <base/math.h>

#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/servermanager.h> //H-Client
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/components/sounds.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/localization.h>

#include "binds.h"
#include "menus.h"

void CMenus::RenderServerManager(CUIRect MainView)
{
    static int s_Section = 0;

	// render background
	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);

	CUIRect rContent, rGeneralOptions, rLPanel, rRPanel;
	MainView.HSplitTop(40.0f, &rGeneralOptions, 0x0);

	MainView.VSplitRight(180.0f, &rLPanel, &rRPanel);
    rLPanel.HSplitTop(40.0f, 0x0, &rContent);

    /** LEFT **/
    //General Options
    CUIRect rLButton, rRButton;
    rGeneralOptions.HSplitTop(10.0f, 0x0, &rGeneralOptions);

    rGeneralOptions.VSplitRight(400.0f, 0x0, &rLButton);
    rLButton.VSplitMid(&rLButton, &rRButton);

    static int s_ButtonSectionA;
    if (DoButton_MenuTab(&s_ButtonSectionA, Localize("Advance Mode"), (s_Section==0?1:0), &rLButton, CUI::CORNER_L))
        s_Section = 0;

    static int s_ButtonSectionB;
    if (DoButton_MenuTab(&s_ButtonSectionB, Localize("Easy Mode"), (s_Section==1?1:0), &rRButton, 0))
        s_Section = 1;

    rGeneralOptions.VSplitLeft(220.0f, &rLButton, 0x0);
    rLButton.VSplitLeft(10.0f, 0x0, &rLButton);
    static int s_ButtonLoadCfg = 0;
    if (DoButton_Menu((void*)&s_ButtonLoadCfg, Localize("Load Config File"), 0, &rLButton))
        InvokeFileDialog(IStorageTW::TYPE_ALL, FILETYPE_CFG, "Load Server Config", "Load", "server_conf", "", CallbackOpenConfig, this);
    //Content Options
    rContent.Margin(10.0f, &rContent);
    CUIRect rContentOptions;
    rContent.HSplitBottom(30.0f, &rContent, &rContentOptions);
    rContentOptions.HSplitTop(5.0f, 0x0, &rContentOptions);
    rContentOptions.VSplitLeft(150.0f, &rContentOptions, 0x0);
    static int s_ButtonRunServer = 0;
    if (DoButton_Menu((void*)&s_ButtonRunServer, Localize("Save & Run"), 0, &rContentOptions))
    {
        char aExec[255];
        str_format(aExec, sizeof(aExec), "%s_srv.exe",  ServerManager()->GetExec(m_SelectedLocalExecServer).c_str());
        ServerManager()->CreateServer("server.cfg", aExec);
    }

    //Executable Dir
    char aBuf[255] = {0};
    CUIRect rContentExecDir, rButton;
    rContent.HSplitBottom(25.0f, &rContent, &rContentExecDir);
    rContentExecDir.HSplitTop(5.0f, 0x0, &rContentExecDir);
    static int s_ButtonsID[8];

    if (ServerManager()->NumBinExecs() > 0)
    {
        float sizeBox = 90.0f;
        static int s_ButtonsID[8];

        vec4 orgColor = ms_ColorTabbarActive;
        ms_ColorTabbarActive = vec4(1.0f, 0.39f, 0.0f, 1.0f);
        for (int i=0; i<ServerManager()->NumBinExecs(); i++)
        {
            rContentExecDir.VSplitRight(sizeBox, &rContentExecDir, &rButton);
            if (DoButton_MenuTab((void*)&s_ButtonsID[i], ServerManager()->GetExec(i).c_str(), (m_SelectedLocalExecServer==i?1:0), &rButton, 0))
            {
                m_SelectedLocalExecServer = i;
            }
        }
        ms_ColorTabbarActive = orgColor;
    }
    else
    {
        TextRender()->TextColor(1.0f, 0.0f, 0.0f, 1.0f);
        UI()->DoLabelScaled(&rContentExecDir, "Ooops!! Any Server Founded!  You can't start a server :\\", 16.0f, 0);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    }

    //Content
    RenderTools()->DrawUIRect(&rContent, ms_ColorTabbarInactive, CUI::CORNER_ALL, 10.0f);
    rContent.Margin(5.0f, &rContent);
    if (s_Section == 0)
    {
        //RenderTools()->DrawUIRect(&rContent, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);
        static float s_OffsetLocalServerConf = 0.0f;
        static int s_LocalServerConf;
        static char aConfBuff[2048];
        if(DoEditBox(&s_LocalServerConf, &rContent, aConfBuff, sizeof(aConfBuff), 14.0f, &s_OffsetLocalServerConf))
        {

        }
    }
    else if (s_Section == 1)
    {
        CUIRect rLabel, rControl, rExtended;
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

        //Name
        rContent.HSplitTop(20.0f, &rLabel, &rContent);
        rLabel.VSplitLeft(65.0f, &rLabel, &rControl);
        UI()->DoLabelScaled(&rLabel, Localize("Name:"), 16.0f, -1);
        static float s_OffsetNameServer = 0.0f;
        static int s_NameServer;
        static char aNameServer[125];
        DoEditBox(&s_NameServer, &rControl, aNameServer, sizeof(aNameServer), 14.0f, &s_OffsetNameServer);

        rContent.HSplitTop(10.0f, 0x0, &rContent);

        //Port
        rContent.HSplitTop(20.0f, &rLabel, &rContent);
        rLabel.VSplitLeft(65.0f, &rLabel, &rControl);
        rControl.VSplitLeft(50.0f, &rControl, &rExtended);
        UI()->DoLabelScaled(&rLabel, Localize("Port:"), 16.0f, -1);
        static float s_OffsetPortServer = 0.0f;
        static int s_PortServer;
        static char aPortServer[5];
        DoEditBox(&s_PortServer, &rControl, aPortServer, sizeof(aPortServer), 14.0f, &s_OffsetPortServer);

        //Register
        rExtended.VSplitLeft(15.0f, 0x0, &rExtended);
        static int s_ServerRegister = 0;
        if(DoButton_CheckBox(&s_ServerRegister, Localize("Private LAN Server"), s_ServerRegister, &rExtended))
            s_ServerRegister ^= 1;

        //GameType
        rContent.HSplitTop(20.0f, &rLabel, &rContent);
        rLabel.VSplitLeft(65.0f, &rLabel, &rControl);
        rControl.VSplitLeft(50.0f, &rControl, &rExtended);
        UI()->DoLabelScaled(&rLabel, Localize("Game Type:"), 16.0f, -1);
        static float s_OffsetGameType = 0.0f;
        static int s_GameType;
        static char aGameType[15];
        DoEditBox(&s_GameType, &rControl, aGameType, sizeof(aGameType), 14.0f, &s_OffsetGameType);

    }

    /** RIGHT **/
    CUIRect rList, rListOptions;
    rRPanel.Margin(10.0f, &rRPanel);
    //Server List
    rRPanel.HSplitBottom(30.0f, &rList, &rListOptions);
    rList.HSplitTop(40.0f, 0x0, &rList);
    RenderTools()->DrawUIRect(&rList, ms_ColorTabbarInactive, CUI::CORNER_ALL, 10.0f);
    rList.Margin(5.0f, &rList);
    //RenderTools()->DrawUIRect(&rList, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);
    RenderServersList(rList);

    //Server List Options
    rListOptions.HSplitTop(5.0f, 0x0, &rListOptions);
    static int s_ButtonNewServer = 0;
    if (DoButton_Menu((void*)&s_ButtonNewServer, Localize("Add Server"), 0, &rListOptions))
    {
    }

	if(m_Dialog == DIALOG_FILE)
	{
		static int s_NullUiTarget = 0;
		UI()->SetHotItem(&s_NullUiTarget);
		RenderFileDialog();
	}
}

void CMenus::RenderServersList(CUIRect MainView)
{
	int NumOptions = 0;
	int Selected = -1;
	static int aServersIDs[MAX_LOCAL_SERVERS];
	for(int i = 0; i < MAX_LOCAL_SERVERS; i++)
	{
	    IServerManager::CMServerEntry iServer = ServerManager()->GetInfoServer(i);
		if(!iServer.m_Used)
			continue;

		if(m_SelectedLocalServer == i)
			Selected = NumOptions;

		aServersIDs[NumOptions++] = i;
	}

	static int s_ServersList = 0;
	static float s_ScrollValue = 0;
	CUIRect List = MainView;
	UiDoListboxStart(&s_ServersList, &List, 24.0f, "", "", NumOptions, 1, Selected, s_ScrollValue);

	for(int i = 0; i < NumOptions; i++)
	{
	    IServerManager::CMServerEntry iServer = ServerManager()->GetInfoServer(aServersIDs[i]);
		CListboxItem Item = UiDoListboxNextItem(&aServersIDs[i]);

		if(Item.m_Visible)
		{
            char aBuf[255];
            str_format(aBuf, sizeof(aBuf), "%s [%s]", iServer.m_Type, iServer.m_Port);
			UI()->DoLabelScaled(&Item.m_Rect, aBuf, 16.0f, -1);
		}
	}

	Selected = UiDoListboxEnd(&s_ScrollValue, 0);
	m_SelectedLocalServer = Selected != -1 ? aServersIDs[Selected] : -1;
}

void CMenus::AddFileDialogEntry(int Index, CUIRect *pView)
{
	m_FilesCur++;
	if(m_FilesCur-1 < m_FilesStartAt || m_FilesCur >= m_FilesStopAt)
		return;

	CUIRect Button, FileIcon;
	pView->HSplitTop(15.0f, &Button, pView);
	pView->HSplitTop(2.0f, 0, pView);
	Button.VSplitLeft(Button.h, &FileIcon, &Button);
	Button.VSplitLeft(5.0f, 0, &Button);

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FILEICONS].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(m_FileList[Index].m_IsDir?SPRITE_FILE_FOLDER:SPRITE_FILE_MAP2);
	IGraphics::CQuadItem QuadItem(FileIcon.x, FileIcon.y, FileIcon.w, FileIcon.h);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	if(DoButton_File(&m_FileList[Index], m_FileList[Index].m_aName, m_FilesSelectedIndex == Index, &Button, 0, 0))
	{
		if(!m_FileList[Index].m_IsDir)
			str_copy(m_aFileDialogFileName, m_FileList[Index].m_aFilename, sizeof(m_aFileDialogFileName));
		else
			m_aFileDialogFileName[0] = 0;
		m_FilesSelectedIndex = Index;

		if(Input()->MouseDoubleClick())
			m_aFileDialogActivate = true;
	}
}

void CMenus::CallbackOpenConfig(const char *pFileName, int StorageType, void *pUser)
{
	/*CMenus *pMenus = (CMenus*)pUser;
	if(pMenus->Load(pFileName, StorageType))
	{
		str_copy(pMenus->m_aFileName, pFileName, 512);
		pMenus->m_ValidSaveFilename = StorageType == IStorageTW::TYPE_SAVE && pMenus->m_pFileDialogPath == pMenus->m_aFileDialogCurrentFolder;
		pMenus->SortImages();
		pMenus->m_Dialog = DIALOG_NONE;
	}*/
}

void CMenus::RenderFileDialog()
{
	// GUI coordsys
	Graphics()->MapScreen(UI()->Screen()->x, UI()->Screen()->y, UI()->Screen()->w, UI()->Screen()->h);
	CUIRect View = *UI()->Screen();
	float Width = View.w, Height = View.h;

	RenderTools()->DrawUIRect(&View, vec4(0,0,0,0.25f), 0, 0);
	View.VMargin(150.0f, &View);
	View.HMargin(50.0f, &View);
	RenderTools()->DrawUIRect(&View, vec4(0,0,0,0.75f), CUI::CORNER_ALL, 5.0f);
	View.Margin(10.0f, &View);

	CUIRect Title, FileBox, FileBoxLabel, ButtonBar, Scroll;
	View.HSplitTop(18.0f, &Title, &View);
	View.HSplitTop(5.0f, 0, &View); // some spacing
	View.HSplitBottom(14.0f, &View, &ButtonBar);
	View.HSplitBottom(10.0f, &View, 0); // some spacing
	View.HSplitBottom(14.0f, &View, &FileBox);
	FileBox.VSplitLeft(55.0f, &FileBoxLabel, &FileBox);
	View.HSplitBottom(10.0f, &View, 0); // some spacing
	View.VSplitRight(15.0f, &View, &Scroll);

	// title
	RenderTools()->DrawUIRect(&Title, vec4(1, 1, 1, 0.25f), CUI::CORNER_ALL, 4.0f);
	Title.VMargin(10.0f, &Title);
	UI()->DoLabel(&Title, m_pFileDialogTitle, 12.0f, -1, -1);

	// filebox
	if(m_FileDialogStorageType == IStorageTW::TYPE_SAVE)
	{
		static float s_FileBoxID = 0;
		UI()->DoLabel(&FileBoxLabel, "Filename:", 10.0f, -1, -1);
		if(DoEditBox(&s_FileBoxID, &FileBox, m_aFileDialogFileName, sizeof(m_aFileDialogFileName), 10.0f, &s_FileBoxID))
		{
			// remove '/' and '\'
			for(int i = 0; m_aFileDialogFileName[i]; ++i)
				if(m_aFileDialogFileName[i] == '/' || m_aFileDialogFileName[i] == '\\')
					str_copy(&m_aFileDialogFileName[i], &m_aFileDialogFileName[i+1], (int)(sizeof(m_aFileDialogFileName))-i);
			m_FilesSelectedIndex = -1;
		}
	}

	int Num = (int)(View.h/17.0f)+1;
	static int ScrollBar = 0;
	Scroll.HMargin(5.0f, &Scroll);
	m_FileDialogScrollValue = DoScrollbarV(&ScrollBar, &Scroll, m_FileDialogScrollValue);

	int ScrollNum = m_FileList.size()-Num+1;
	if(ScrollNum > 0)
	{
		if(Input()->KeyPresses(KEY_MOUSE_WHEEL_UP))
			m_FileDialogScrollValue -= 3.0f/ScrollNum;
		if(Input()->KeyPresses(KEY_MOUSE_WHEEL_DOWN))
			m_FileDialogScrollValue += 3.0f/ScrollNum;
	}
	else
		ScrollNum = 0;

	if(m_FilesSelectedIndex > -1)
	{
		for(int i = 0; i < Input()->NumEvents(); i++)
		{
			int NewIndex = -1;
			if(Input()->GetEvent(i).m_Flags&IInput::FLAG_PRESS)
			{
				if(Input()->GetEvent(i).m_Key == KEY_DOWN) NewIndex = m_FilesSelectedIndex + 1;
				if(Input()->GetEvent(i).m_Key == KEY_UP) NewIndex = m_FilesSelectedIndex - 1;
			}
			if(NewIndex > -1 && NewIndex < m_FileList.size())
			{
				//scroll
				float IndexY = View.y - m_FileDialogScrollValue*ScrollNum*17.0f + NewIndex*17.0f;
				int Scroll = View.y > IndexY ? -1 : View.y+View.h < IndexY+17.0f ? 1 : 0;
				if(Scroll)
				{
					if(Scroll < 0)
						m_FileDialogScrollValue = ((float)(NewIndex)+0.5f)/ScrollNum;
					else
						m_FileDialogScrollValue = ((float)(NewIndex-Num)+2.5f)/ScrollNum;
				}

				if(!m_FileList[NewIndex].m_IsDir)
					str_copy(m_aFileDialogFileName, m_FileList[NewIndex].m_aFilename, sizeof(m_aFileDialogFileName));
				else
					m_aFileDialogFileName[0] = 0;
				m_FilesSelectedIndex = NewIndex;
			}
		}
	}

	for(int i = 0; i < Input()->NumEvents(); i++)
	{
		if(Input()->GetEvent(i).m_Flags&IInput::FLAG_PRESS)
		{
			if(Input()->GetEvent(i).m_Key == KEY_RETURN || Input()->GetEvent(i).m_Key == KEY_KP_ENTER)
				m_aFileDialogActivate = true;
		}
	}

	if(m_FileDialogScrollValue < 0) m_FileDialogScrollValue = 0;
	if(m_FileDialogScrollValue > 1) m_FileDialogScrollValue = 1;

	m_FilesStartAt = (int)(ScrollNum*m_FileDialogScrollValue);
	if(m_FilesStartAt < 0)
		m_FilesStartAt = 0;

	m_FilesStopAt = m_FilesStartAt+Num;

	m_FilesCur = 0;

	// set clipping
	UI()->ClipEnable(&View);

	for(int i = 0; i < m_FileList.size(); i++)
		AddFileDialogEntry(i, &View);

	// disable clipping again
	UI()->ClipDisable();

	// the buttons
	static int s_OkButton = 0;
	static int s_CancelButton = 0;
	static int s_NewFolderButton = 0;

	CUIRect Button;
	ButtonBar.VSplitRight(50.0f, &ButtonBar, &Button);
	bool IsDir = m_FilesSelectedIndex >= 0 && m_FileList[m_FilesSelectedIndex].m_IsDir;
	if(DoButton_Menu(&s_OkButton, IsDir ? "Open" : m_pFileDialogButtonText, 0, &Button) || m_aFileDialogActivate)
	{
		m_aFileDialogActivate = false;
		if(IsDir)	// folder
		{
			if(str_comp(m_FileList[m_FilesSelectedIndex].m_aFilename, "..") == 0)	// parent folder
			{
				if(fs_parent_dir(m_pFileDialogPath))
					m_pFileDialogPath = m_aFileDialogCurrentFolder;	// leave the link
			}
			else	// sub folder
			{
				if(m_FileList[m_FilesSelectedIndex].m_IsLink)
				{
					m_pFileDialogPath = m_aFileDialogCurrentLink;	// follow the link
					str_copy(m_aFileDialogCurrentLink, m_FileList[m_FilesSelectedIndex].m_aFilename, sizeof(m_aFileDialogCurrentLink));
				}
				else
				{
					char aTemp[MAX_PATH_LENGTH];
					str_copy(aTemp, m_pFileDialogPath, sizeof(aTemp));
					str_format(m_pFileDialogPath, MAX_PATH_LENGTH, "%s/%s", aTemp, m_FileList[m_FilesSelectedIndex].m_aFilename);
				}
			}
			FilelistPopulate(!str_comp(m_pFileDialogPath, "server_conf") ? m_FileDialogStorageType :
				m_FileList[m_FilesSelectedIndex].m_StorageType);
			if(m_FilesSelectedIndex >= 0 && !m_FileList[m_FilesSelectedIndex].m_IsDir)
				str_copy(m_aFileDialogFileName, m_FileList[m_FilesSelectedIndex].m_aFilename, sizeof(m_aFileDialogFileName));
			else
				m_aFileDialogFileName[0] = 0;
		}
		else // file
		{
			str_format(m_aFileSaveName, sizeof(m_aFileSaveName), "%s/%s", m_pFileDialogPath, m_aFileDialogFileName);
			if(!str_comp(m_pFileDialogButtonText, "Save"))
			{
				IOHANDLE File = Storage()->OpenFile(m_aFileSaveName, IOFLAG_READ, IStorageTW::TYPE_SAVE);
				if(File)
				{
					io_close(File);
					//m_PopupEventType = POPEVENT_SAVE;
					//m_PopupEventActivated = true;
				}
				else
					if(m_pfnFileDialogFunc)
						m_pfnFileDialogFunc(m_aFileSaveName, m_FilesSelectedIndex >= 0 ? m_FileList[m_FilesSelectedIndex].m_StorageType : m_FileDialogStorageType, m_pFileDialogUser);
			}
			else
				if(m_pfnFileDialogFunc)
					m_pfnFileDialogFunc(m_aFileSaveName, m_FilesSelectedIndex >= 0 ? m_FileList[m_FilesSelectedIndex].m_StorageType : m_FileDialogStorageType, m_pFileDialogUser);
		}
	}

	ButtonBar.VSplitRight(40.0f, &ButtonBar, &Button);
	ButtonBar.VSplitRight(50.0f, &ButtonBar, &Button);
	if(DoButton_Menu(&s_CancelButton, "Cancel", 0, &Button) || Input()->KeyPressed(KEY_ESCAPE))
		m_Dialog = DIALOG_NONE;

	/*if(m_FileDialogStorageType == IStorageTW::TYPE_SAVE)
	{
		ButtonBar.VSplitLeft(40.0f, 0, &ButtonBar);
		ButtonBar.VSplitLeft(70.0f, &Button, &ButtonBar);
		if(DoButton_Editor(&s_NewFolderButton, "New folder", 0, &Button, 0, 0))
		{
			m_FileDialogNewFolderName[0] = 0;
			m_FileDialogErrString[0] = 0;
			static int s_NewFolderPopupID = 0;
			UiInvokePopupMenu(&s_NewFolderPopupID, 0, Width/2.0f-200.0f, Height/2.0f-100.0f, 400.0f, 200.0f, PopupNewFolder);
			UI()->SetActiveItem(0);
		}
	}*/
}


static int ManagerListdirCallback(const char *pName, int IsDir, int StorageType, void *pUser)
{
	CMenus *pMenus = (CMenus*)pUser;
	int Length = str_length(pName);
	if((pName[0] == '.' && (pName[1] == 0 ||
		(pName[1] == '.' && pName[2] == 0 && !str_comp(pMenus->m_pFileDialogPath, "server_conf")))) ||
		(!IsDir && (pMenus->m_FileDialogFileType == CMenus::FILETYPE_CFG && (Length < 4 || str_comp(pName+Length-4, ".cfg")))))
		return 0;

	CMenus::CFilelistItem Item;
	str_copy(Item.m_aFilename, pName, sizeof(Item.m_aFilename));
	if(IsDir)
		str_format(Item.m_aName, sizeof(Item.m_aName), "%s/", pName);
	else
		str_copy(Item.m_aName, pName, min(static_cast<int>(sizeof(Item.m_aName)), Length-3));
	Item.m_IsDir = IsDir != 0;
	Item.m_IsLink = false;
	Item.m_StorageType = StorageType;
	pMenus->m_FileList.add(Item);

	return 0;
}

void CMenus::FilelistPopulate(int StorageType)
{
	m_FileList.clear();
	if(m_FileDialogStorageType != IStorageTW::TYPE_SAVE && !str_comp(m_pFileDialogPath, "server_conf"))
	{
		CFilelistItem Item;
		str_copy(Item.m_aFilename, "server_conf", sizeof(Item.m_aFilename));
		str_copy(Item.m_aName, "server_conf/", sizeof(Item.m_aName));
		Item.m_IsDir = true;
		Item.m_IsLink = true;
		Item.m_StorageType = IStorageTW::TYPE_SAVE;
		m_FileList.add(Item);
	}
	Storage()->ListDirectory(StorageType, m_pFileDialogPath, ManagerListdirCallback, this);
	m_FilesSelectedIndex = m_FileList.size() ? 0 : -1;
	m_aFileDialogActivate = false;
}

void CMenus::InvokeFileDialog(int StorageType, int FileType, const char *pTitle, const char *pButtonText,
	const char *pBasePath, const char *pDefaultName,
	void (*pfnFunc)(const char *pFileName, int StorageType, void *pUser), void *pUser)
{
	m_FileDialogStorageType = StorageType;
	m_pFileDialogTitle = pTitle;
	m_pFileDialogButtonText = pButtonText;
	m_pfnFileDialogFunc = pfnFunc;
	m_pFileDialogUser = pUser;
	m_aFileDialogFileName[0] = 0;
	m_aFileDialogCurrentFolder[0] = 0;
	m_aFileDialogCurrentLink[0] = 0;
	m_pFileDialogPath = m_aFileDialogCurrentFolder;
	m_FileDialogFileType = FileType;
	m_FileDialogScrollValue = 0.0f;

	if(pDefaultName)
		str_copy(m_aFileDialogFileName, pDefaultName, sizeof(m_aFileDialogFileName));
	if(pBasePath)
		str_copy(m_aFileDialogCurrentFolder, pBasePath, sizeof(m_aFileDialogCurrentFolder));

	FilelistPopulate(m_FileDialogStorageType);

	m_Dialog = DIALOG_FILE;
}
