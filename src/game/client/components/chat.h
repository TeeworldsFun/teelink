/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_CHAT_H
#define GAME_CLIENT_COMPONENTS_CHAT_H
#include <engine/textrender.h>
#include <engine/shared/ringbuffer.h>
#include <game/client/component.h>
#include <game/client/lineinput.h>
#include <list> //H-Client

class CChat : public CComponent
{
	CLineInput m_Input;

	enum
	{
		MAX_LINES = 25,
	};

	struct CLine
	{
		int64 m_Time;
		float m_YOffset[2];
		int m_ClientID;
		int m_Team;
		int m_NameColor;
		int m_Type; //H-Client
		char m_aName[64];
		char m_aText[512];
		char m_aChan[64]; //H-Client
		bool m_Highlighted;
	};

	CLine m_aLines[MAX_LINES];
	int m_CurrentLine;

    //H-Client: Chat Emotes
	struct CChatEmote
	{
	    const char *m_Emote;
	    int m_SpriteID;

        CChatEmote() {}
	    CChatEmote(const char *emote, int scid): m_Emote(emote), m_SpriteID(scid) {}
	};
    std::list<CChat::CChatEmote> m_vChatEmotes;

	void AssignChatEmote(const char *emote, int sc_id);
	int m_ChatRoom;
	//

	// chat
	enum
	{
		MODE_NONE=0,
		MODE_ALL,
		MODE_TEAM,
		MODE_IRC,
	};

	int m_Mode;
	bool m_Show;
	bool m_InputUpdate;
	int m_ChatStringOffset;
	int m_OldChatStringLength;
	int m_CompletionChosen;
	char m_aCompletionBuffer[256];
	int m_PlaceholderOffset;
	int m_PlaceholderLength;
	char *m_pHistoryEntry;
	TStaticRingBuffer<char, 64*1024, CRingBufferBase::FLAG_RECYCLE> m_History;

	static void ConSay(IConsole::IResult *pResult, void *pUserData);
	static void ConSayTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConSayIrc(IConsole::IResult *pResult, void *pUserData); //H-Client
	static void ConChat(IConsole::IResult *pResult, void *pUserData);
	static void ConShowChat(IConsole::IResult *pResult, void *pUserData);

	void HighlightText(const char *pText, vec4 dfcolor, CTextCursor *pCursor); //H-Client

public:
	CChat();

	bool IsActive() const { return m_Mode != MODE_NONE; }

	void AddLine(int ClientID, int Team, const char *pLine);
	void AddLine(const char *pFrom, const char *pUser, const char *pLine);

	void EnableMode(int Team);

	void Say(int Team, const char *pLine);

	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual void OnStateChange(int NewState, int OldState);
	virtual void OnRender();
	virtual void OnRelease();
	virtual void OnMessage(int MsgType, void *pRawMsg);
	virtual void OnMessageIrc(const char *pFrom, const char *pUser, const char *pText);
	virtual bool OnInput(IInput::CEvent Event);
};
#endif
