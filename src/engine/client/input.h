/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_INPUT_H
#define ENGINE_CLIENT_INPUT_H

class CInput : public IEngineInput
{
	IEngineGraphics *m_pGraphics;

	int m_InputGrabbed;
	char *m_pClipboardText;

	int64 m_LastRelease;
	int64 m_ReleaseDelta;
	int64 m_LastReleaseNative;
	int64 m_ReleaseDeltaNative;

	bool m_MouseFocus;
	int m_VideoRestartNeeded;

	void AddEvent(char *pText, int Key, int Flags);
	void Clear();
	bool IsEventValid(CEvent *pEvent) const { return pEvent->m_InputCount == m_InputCounter; };

	//quick access to input
	unsigned short m_aInputCount[g_MaxKeys];	// tw-KEY
	unsigned char m_aInputState[g_MaxKeys];	// SDL_SCANCODE
	int m_InputCounter;

	//ime support
	int m_CountEditingText;
	char m_aEditingText[32];
	int m_EditingCursor;

	bool KeyState(int Key) const;

	IEngineGraphics *Graphics() { return m_pGraphics; }

public:
	CInput();

	virtual void Init();

	bool KeyIsPressed(int Key) const { return KeyState(Key); }
	bool KeyPress(int Key, bool CheckCounter) const { return CheckCounter ? (m_aInputCount[Key] == m_InputCounter) : m_aInputCount[Key] != 0; }

	virtual void MouseRelative(float *x, float *y);
	virtual void MouseModeAbsolute();
	virtual void MouseModeRelative();
	virtual bool InputGrabbed() const { return m_InputGrabbed != 0; }
	virtual void NativeMousePos(int *x, int *y);
	virtual bool NativeMousePressed(int index);
	virtual int MouseDoubleClick();
	virtual int MouseDoubleClickNative();
	virtual const char* GetClipboardText();
	virtual void SetClipboardText(const char *Text);

	virtual void SimulateKeyPress(int Key) { AddEvent(0, Key, FLAG_PRESS); }
	virtual void SimulateKeyPressSTD(std::string Key) { int id = GetKeyID(Key); if(id < 0) return; AddEvent(0, id, FLAG_PRESS); }
	virtual void SimulateKeyRelease(int Key) { AddEvent(0, Key, FLAG_RELEASE); }
	virtual void SimulateKeyReleaseSTD(std::string Key) { int id = GetKeyID(Key); if(id < 0) return; AddEvent(0, id, FLAG_RELEASE); }

	virtual int Update();
	virtual void NextFrame();

	virtual int VideoRestartNeeded();

	virtual bool GetIMEState();
	virtual void SetIMEState(bool Activate);
	virtual const char* GetIMECandidate();
	virtual int GetEditingCursor();
};

#endif
