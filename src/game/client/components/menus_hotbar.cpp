#include <base/math.h>

#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/textrender.h>
#include <engine/storage.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/animstate.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>

#include "menus.h"
#include "skins.h"
#include "identity.h"

void CMenus::ConKeyShortcut(IConsole::IResult *pResult, void *pUserData)
{
	CMenus *pSelf = (CMenus *)pUserData;
	if(pSelf->Client()->State() == IClient::STATE_ONLINE)
	{
		if(pResult->GetInteger(0) != 0)
			pSelf->m_HotbarActive ^= 1;
	}
}

void CMenus::OnConsoleInit()
{
	Console()->Register("+hotbar", "", CFGFLAG_CLIENT, ConKeyShortcut, this, "Access the hotbar");
}

void CMenus::RenderIdents(CUIRect MainView)
{
	const int NumIdentities = m_pClient->m_pIdentity->NumIdents();
	CUIRect Button, Label, Temp;
	static float s_Scrollbar = 0;
	static float s_ScrollValue = 0;

	RenderTools()->DrawUIRect(&MainView, vec4(0.0f, 0.5f, 0.0f, 0.64f), CUI::CORNER_T, 10.0f);

	MainView.HSplitTop(8.0f, &Button, &Temp);
	Button.VSplitMid(&Button, &Label);

	// scrollbar
	Temp.HSplitTop(16.0f, &Button, &Temp);
	Button.VMargin(4.0f, &Button);
	s_ScrollValue = DoScrollbarH(&s_Scrollbar, &Button, s_ScrollValue);

	Temp.HSplitTop(68.0f, &Button, &Temp);
	UI()->ClipEnable(&Button);
	Button.HMargin(5.0f, &Button);
	Button.VSplitLeft(7.5f, 0, &Button);

	int Offset = 30.0f + 80 * (NumIdentities - 4);
	if(Offset > 0)
		Button.x -= Offset * s_ScrollValue;

	for(int i = 0; i < NumIdentities; i++)
	{
		CIdentity::CIdentEntry *pIdent = m_pClient->m_pIdentity->GetIdent(i);
		if(pIdent == NULL)
			continue;

		const CSkins::CSkin *pSkin = NULL;
		CTeeRenderInfo SkinInfo;

		pSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(pIdent->m_aSkin));

		Button.VSplitLeft(80.0f, &Label, &Button);
		if(DoButton_Menu(pIdent, "", 0, &Label, 0, CUI::CORNER_ALL, vec4(pSkin->m_BloodColor.r, pSkin->m_BloodColor.g, pSkin->m_BloodColor.b, 1.0f)))
		{
			CIdentity::CIdentEntry *pIdent = m_pClient->m_pIdentity->GetIdent(i);
			str_format(g_Config.m_PlayerName, sizeof(g_Config.m_PlayerName), pIdent->m_aName);
			str_format(g_Config.m_PlayerClan, sizeof(g_Config.m_PlayerClan), pIdent->m_aClan);
			str_format(g_Config.m_ClPlayerSkin, sizeof(g_Config.m_ClPlayerSkin), pIdent->m_aSkin);
			g_Config.m_ClPlayerUseCustomColor = pIdent->m_UseCustomColor;
			g_Config.m_ClPlayerColorBody = pIdent->m_ColorBody;
			g_Config.m_ClPlayerColorFeet = pIdent->m_ColorFeet;
			m_pClient->SendInfo(false);
		}

		if(pIdent->m_UseCustomColor)
		{
			SkinInfo.m_Texture = pSkin->m_ColorTexture;
			SkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(pIdent->m_ColorBody);
			SkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(pIdent->m_ColorFeet);
		}
		else
		{
			SkinInfo.m_Texture = pSkin->m_OrgTexture;
			SkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			SkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		SkinInfo.m_Size = 50.0f*UI()->Scale();
		RenderTools()->RenderTee(CAnimState::GetIdle(), &SkinInfo, 0, vec2(1, 0), vec2(Label.x + Label.w * 0.5f, Label.y + Label.h * 0.5f + 8.0f));

		Label.HSplitBottom(12.0f, 0, &Label);
		Label.y -= 48.0f;
		RenderTools()->DrawUIRect(&Label, vec4(0.0f, 0.0f, 0.0f, 0.95f), CUI::CORNER_T, 4.0f);
		UI()->DoLabelScaled(&Label, pIdent->m_aName, 10.0f, 0);
		Button.VSplitLeft(15.0f, 0, &Button);
	}

	UI()->ClipDisable();
}

void CMenus::RenderTrans(CUIRect MainView)
{
	CUIRect Button, Label, Temp;

	RenderTools()->DrawUIRect(&MainView, vec4(0.0f, 0.5f, 0.0f, 0.64f), CUI::CORNER_L, 10.0f);

	MainView.HSplitTop(8.0f, &Button, &Temp);
	Button.VSplitMid(&Button, &Label);

	MainView.HSplitTop(6.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(230.0f, &Button, 0);
	Button.x += 4.0f;
	if(DoButton_CheckBox(&g_Config.m_ClTransIn, Localize("Translate incoming messages"), g_Config.m_ClTransIn, &Button))
		g_Config.m_ClTransIn ^= 1;

	MainView.HSplitTop(1.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(230.0f, &Button, 0);
	Button.x += 4.0f;
	if(DoButton_CheckBox(&g_Config.m_ClTransOut, Localize("Translate outgoing messages"), g_Config.m_ClTransOut, &Button))
		g_Config.m_ClTransOut ^= 1;

	MainView.HSplitTop(14.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(40.0f, &Button, 0);
	Label.x += 4.0f;
	Button.x += 10.0f;
	UI()->DoLabelScaled(&Label, "In  Source:", 14.0, -1);
	static float s_OffsetInSrc = 0.0f;
	DoEditBox(&g_Config.m_ClTransInSrc, &Button, g_Config.m_ClTransInSrc, sizeof(g_Config.m_ClTransInSrc), 14.0f, &s_OffsetInSrc);

	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(40.0f, &Button, 0);
	Button.x += 102.0f;
	Label.x += 64.0f;
	UI()->DoLabelScaled(&Label, "In  Destination:", 14.0, -1);
	static float s_OffsetInDst = 0.0f;
	DoEditBox(&g_Config.m_ClTransInDst, &Button, g_Config.m_ClTransInDst, sizeof(g_Config.m_ClTransInDst), 14.0f, &s_OffsetInDst);

	MainView.HSplitTop(3.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(40.0f, &Button, 0);
	Label.x += 4.0f;
	Button.x += 10.0f;
	UI()->DoLabelScaled(&Label, "Out Source:", 14.0, -1);
	static float s_OffsetOutSrc = 0.0f;
	DoEditBox(&g_Config.m_ClTransOutSrc, &Button, g_Config.m_ClTransOutSrc, sizeof(g_Config.m_ClTransOutSrc), 14.0f, &s_OffsetOutSrc);

	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(40.0f, &Button, 0);
	Button.x += 102.0f;
	Label.x += 64.0f;
	UI()->DoLabelScaled(&Label, "Out Destination:", 14.0, -1);
	static float s_OffsetOutDst = 0.0f;
	DoEditBox(&g_Config.m_ClTransOutDst, &Button, g_Config.m_ClTransOutDst, sizeof(g_Config.m_ClTransOutDst), 14.0f, &s_OffsetOutDst);
}

void CMenus::RenderHotbar(CUIRect MainView)
{
	if(!m_HotbarActive)
	{
		m_HotbarWasActive = false;
		return;
	}

	m_HotbarWasActive = true;

	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	Graphics()->BlendNormal();

	CUIRect r;
	MainView.HSplitTop(115.0f, &r, 0);
	r.w = 380.0f;
	r.x = MainView.w / 2.0f - r.w / 2.0f;
	r.y = MainView.h - r.h;
	RenderIdents(r);

	CUIRect t;
	MainView.HSplitTop(115.0f, &t, 0);
	t.w = 340.0f;
	t.x = MainView.w - t.w;
	t.y = MainView.h / 2.0f - t.h / 2.0f;
	RenderTrans(t);
}

bool CMenus::HotbarLockInput(IInput::CEvent e)
{
	if(m_HotbarActive)
	{
		if(e.m_Flags&IInput::FLAG_PRESS && (e.m_Key == KEY_MOUSE_1 || e.m_Key == KEY_MOUSE_2))
		{
			return true;
		}
	}
	return false;
}
