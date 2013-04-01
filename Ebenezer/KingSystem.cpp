#include "StdAfx.h"
#include "KingSystem.h"
#include "EbenezerDlg.h"
#include "User.h"
#include "resource.h"

CKingSystem::CKingSystem()
{
	m_byNation = 0;

	m_byType = 0;
	m_sYear = 0;
	m_byMonth = m_byDay = m_byHour = m_byMinute = 0;

	m_byImType = 0;
	m_sImYear = 0;
	m_byImMonth = m_byImDay = m_byImHour = m_byImMinute = 0;

	m_byNoahEvent = m_byNoahEvent_Day = m_byNoahEvent_Hour = m_byNoahEvent_Minute = 0;
	m_sNoahEvent_Duration = 0;

	m_byExpEvent = m_byExpEvent_Day = m_byExpEvent_Hour = m_byExpEvent_Minute;
	m_sExpEvent_Duration = 0;

	m_nTribute = 0;
	m_byTerritoryTariff = 0;
	m_nTerritoryTax = m_nNationalTreasury = 0;

	m_bSentFirstMessage = false;
}

// Handles timed events.
void CKingSystem::CheckKingTimer()
{
	// Get the current time.
	uint8	bCurDay = g_localTime.tm_mday,
			bCurHour = g_localTime.tm_hour,
			bCurMinute = g_localTime.tm_min;

	// If there's an ongoing coin or XP event...
	if (m_byNoahEvent || m_byExpEvent)
		CheckSpecialEvent();

	switch (m_byType)
	{
	case 0:
	case 7:
		{
			if (g_localTime.tm_mon + 1 == m_byMonth
				&& bCurDay == m_byDay
				&& bCurHour == m_byHour
				&& bCurMinute == m_byMinute)
			{
				m_byType = 1;
				g_pMain->SendFormattedResource(IDS_KING_RECOMMEND_TIME, m_byNation, false);

				// KingNotifyMessage(1, m_byNation, WAR_SYSTEM_CHAT);
				// SendUDP_ElectionStatus(m_byType);
				// LoadRecommendList(m_byNation);
			}
		} break;

	case 1:
		{
			if (g_localTime.tm_mon + 1 == m_byMonth
					&& bCurDay == m_byDay
					&& bCurHour == m_byHour
					&& bCurMinute == m_byMinute)
			{
				m_byType = 2;
				g_pMain->SendFormattedResource(IDS_KING_RECOMMEND_FINISH_TIME, m_byNation, false);

				// CheckRecommendList();
				// KingNotifyMessage(2, m_byNation, WAR_SYSTEM_CHAT);
				// SendUDP_ElectionStatus(m_byType);
			}

			if ((bCurMinute % 30) && !m_bSentFirstMessage)
			{
				m_bSentFirstMessage = true;
				g_pMain->SendFormattedResource(IDS_KING_PERIOD_OF_RECOMMEND_MESSAGE, m_byNation, true);

				// KingNotifyMessage(28, m_byNation, PUBLIC_CHAT);
				break; // awkward, but official behaviour.
			}

			m_bSentFirstMessage = false;
		} break;

	case 2:
		{
			if (g_localTime.tm_mon + 1 == m_byMonth
					&& bCurDay == m_byDay
					&& bCurHour == m_byHour
					&& bCurMinute == m_byMinute)
			{
				m_byType = 3;
				g_pMain->SendFormattedResource(IDS_KING_ELECTION_TIME, m_byNation, false);

				// KingNotifyMessage(3, m_byNation, WAR_SYSTEM_CHAT);
				// SendUDP_ElectionStatus(m_byType);
			}
		} break;

	case 3:
		{
			if (g_localTime.tm_mon + 1 == m_byMonth
					&& bCurDay == m_byDay
					&& bCurHour == m_byHour
					&& bCurMinute == m_byMinute)
			{
				m_byType = 6;
				// GetElectionResult();
				return;
			}

			if ((bCurMinute % 30) && !m_bSentFirstMessage)
			{
				m_bSentFirstMessage = true;
				g_pMain->SendFormattedResource(IDS_KING_PERIOD_OF_ELECTION_MESSAGE, m_byNation, true);

				// KingNotifyMessage(29, m_byNation, PUBLIC_CHAT);
				break; // awkward, but official behaviour.
			}

			m_bSentFirstMessage = false;
		} break;
	}

	switch (m_byImType)
	{
	case 1: // 47 hours after the impeachment time, call GetImpeachmentRequestResult()
	case 2: // 2 days (48 hours) after the impeachment time, set the impeachment type to 3 and send IDS_KING_IMPEACHMENT_ELECTION_MESSAGE as WAR_SYSTEM_CHAT
	case 3: // 3 days (72 hours) after the impeachment time, set the impeachment type to 4 and call GetImpeachmentElectionResult()
		break;
	}
}

// Checks to see if a special (coin/XP) event should end.
void CKingSystem::CheckSpecialEvent()
{
	// Get the current time.
	uint8	bCurDay = g_localTime.tm_mday,
			bCurHour = g_localTime.tm_hour,
			bCurMinute = g_localTime.tm_min;

	int16 sEventExpiry;

	// If there's an exp event ongoing...
	if (m_byExpEvent)
	{
		if (bCurDay == m_byExpEvent_Day)
			sEventExpiry = bCurMinute + 60 * (bCurHour - m_byExpEvent_Hour) - m_byExpEvent_Minute;
		else
			sEventExpiry = bCurMinute + 60 * (bCurHour - m_byExpEvent_Hour + 24) - m_byExpEvent_Minute;

		if (sEventExpiry > m_sExpEvent_Duration)
		{
			m_byExpEvent = 0;
			m_byExpEvent_Day = 0;
			m_byExpEvent_Hour = 0;
			m_byExpEvent_Minute = 0;
			m_sExpEvent_Duration = 0;

			// TO-DO: Tell other servers that the event expired (i.e. via UDP)
			// TO-DO: Update the bonuses on the AI server's side (which we don't have implemented). 
			// TO-DO: Update the KING_SYSTEM table to reset the stored event data there too.

			g_pMain->SendFormattedResource(IDS_KING_EXP_BONUS_EVENT_STOP, m_byNation, false);

			// KingNotifyMessage(IDS_KING_EXP_BONUS_EVENT_STOP, m_byNation, WAR_SYSTEM_CHAT); 
			// 31 translates to a resource ID of 230, other args: 0, 0, 0, 0
		}
	}

	// If there's a coin event ongoing...
	if (m_byNoahEvent)
	{
		if (bCurDay == m_byNoahEvent_Day)
			sEventExpiry = bCurMinute + 60 * (bCurHour - m_byNoahEvent_Hour) - m_byNoahEvent_Minute;
		else
			sEventExpiry = bCurMinute + 60 * (bCurHour - m_byNoahEvent_Hour + 24) - m_byNoahEvent_Minute;

		if (sEventExpiry > m_sNoahEvent_Duration)
		{
			m_byNoahEvent = 0;
			m_byNoahEvent_Day = 0;
			m_byNoahEvent_Hour = 0;
			m_byNoahEvent_Minute = 0;
			m_sNoahEvent_Duration = 0;

			// TO-DO: Tell other servers that the event expired (i.e. via UDP)
			// TO-DO: Update the bonuses on the AI server's side (which we don't have implemented). 
			// TO-DO: Update the KING_SYSTEM table to reset the stored event data there too.
			g_pMain->SendFormattedResource(IDS_KING_NOAH_BONUS_EVENT_STOP, m_byNation, false);

			// KingNotifyMessage(IDS_KING_NOAH_BONUS_EVENT_STOP, m_byNation, WAR_SYSTEM_CHAT);
			// 32 translates to a resource ID of 231, other args: 0, 0, 0, 0
		}
	}
}

// This sends the appropriate resource as a notice to the server (or to a particular user)
// Beyond initial reversing, this doesn't need to exist -- in fact, not even going to use it.
// It's just a temporary point of reference.
void CKingSystem::KingNotifyMessage(uint32 nResourceID, int byNation, ChatType chatType)
{
	std::string result;
	switch (nResourceID)
	{
	//	Resource ID (SERVER_RESOURCE)						// ID used internally (officially)
	case IDS_KING_RECOMMEND_TIME:							// 1 (none)
	case IDS_KING_RECOMMEND_FINISH_TIME:					// 2 (none)
	case IDS_KING_ELECTION_TIME:							// 3 (none)
	case IDS_KING_IMPEACHMENT_REQUEST_TIME:					// 4 (none)
	case IDS_KING_IMPEACHMENT_ELECTION_TIME:				// 5 (none)
	case IDS_KING_REIGN_TIME:								// 7 (none)
	case IDS_KING_KARUS_PRIZE_EVENT_MESSAGE:				// 11 (awarded %s %d coins)
	case IDS_KING_ELMO_PRIZE_EVENT_MESSAGE:					// 12 (awarded %s %d coins)
	case IDS_KING_KARUS_FUGITIVE_EVENT_MESSAGE_1:			// 13 (awarded %s %d coins -- probably inaccurate though, see below)
	case IDS_KING_ELMO_FUGITIVE_EVENT_MESSAGE_1:			// 14 (awarded %s %d coins -- probably inaccurate though, see below)
	case IDS_KING_FUGITIVE_EVENT_MESSAGE_2:					// 15 (%s killed %s and received %d coins as a reward)
	case IDS_KING_KARUS_WEATHER_FINE_EVENT:					// 16 (none)
	case IDS_KING_KARUS_WEATHER_RAIN_EVENT:					// 17 (none)
	case IDS_KING_KARUS_WEATHER_SNOW_EVENT:					// 18 (none)
	case IDS_KING_ELMO_WEATHER_FINE_EVENT:					// 19 (none)
	case IDS_KING_ELMO_WEATHER_RAIN_EVENT:					// 20 (none)
	case IDS_KING_ELMO_WEATHER_SNOW_EVENT:					// 21 (none)
	case IDS_KING_KARUS_NOAH_BONUS_EVENT:					// 22 (%d%% increased coin rate)
	case IDS_KING_KARUS_EXP_BONUS_EVENT:					// 23 (%d%% increased XP rate)
	case IDS_KING_ELMO_NOAH_BONUS_EVENT:					// 24 (%d%% increased coin rate)
	case IDS_KING_ELMO_EXP_BONUS_EVENT:						// 25 (%d%% increased XP rate)
	case IDS_KING_RECOMMEND_REQUEST_MESSAGE:				// 26 (%s can nominate a King)
	case IDS_KING_CANDIDACY_RECOMMEND_MESSAGE:				// 27 (%s has nominated %s as a King)
	case IDS_KING_PERIOD_OF_RECOMMEND_MESSAGE:				// 28 (none)
	case IDS_KING_PERIOD_OF_ELECTION_MESSAGE:				// 29 (none)
	case IDS_KING_ELECTION_RESULT_MESSAGE:					// 30 (%d%% of the vote was won by %s)
	case IDS_KING_EXP_BONUS_EVENT_STOP:						// 31 (none)
	case IDS_KING_NOAH_BONUS_EVENT_STOP:					// 32 (none)
	case IDS_KING_IMPEACHMENT_REQUEST_MESSAGE:				// 33 (none)
	case IDS_KING_IMPEACHMENT_PASS_MESSAGE:					// 34 (none)
	case IDS_KING_IMPEACHMENT_REJECT_MESSAGE:				// 35 (none)
	case IDS_KING_IMPEACHMENT_ELECTION_MESSAGE:				// 36 (none)
	case IDS_KING_IMPEACHMENT_ELECTION_YES_RESULT_MESSAGE:	// 37 (none)
	case IDS_KING_IMPEACHMENT_ELECTION_NO_RESULT_MESSAGE:	// 38 (none)
		break;
	}
}

// Wrapper to lookup the appropriate King system instance.
void CKingSystem::PacketProcess(CUser * pUser, Packet & pkt)
{
	if (pUser == NULL)
		return;

	// ... onwards, to official-like code.
	CKingSystem * pKingSystem = g_pMain->m_KingSystemArray.GetData(pUser->GetNation());
	if (pKingSystem != NULL)
		pKingSystem->KingPacketProcess(pUser, pkt);
}

// The real method to handle packets from the client.
void CKingSystem::KingPacketProcess(CUser * pUser, Packet & pkt)
{
	switch (pkt.read<uint8>())
	{
	case KING_ELECTION:
		ElectionSystem(pUser, pkt);
		break;

	case KING_IMPEACHMENT:
		ImpeachmentSystem(pUser, pkt);
		break;

	case KING_TAX:
		KingTaxSystem(pUser, pkt);
		break;

	case KING_EVENT:
		KingSpecialEvent(pUser, pkt);
		break;

	case KING_NATION_INTRO:
		break;
	}
}

void CKingSystem::ElectionSystem(CUser * pUser, Packet & pkt)
{
	switch (pkt.read<uint8>())
	{
	case KING_ELECTION_SCHEDULE:
		ElectionScheduleConfirmation(pUser, pkt);
		break;

	case KING_ELECTION_NOMINATE:
		CandidacyRecommend(pUser, pkt);
		break;

	case KING_ELECTION_NOTICE_BOARD:
		CandidacyNoticeBoard(pUser, pkt);
		break;

	case KING_ELECTION_POLL:
		ElectionPoll(pUser, pkt);
		break;

	case KING_ELECTION_RESIGN:
		CandidacyResign(pUser, pkt);
		break;
	}
}

// "Check election day" button at the election NPC
void CKingSystem::ElectionScheduleConfirmation(CUser * pUser, Packet & pkt)
{
	Packet result(WIZ_KING, uint8(KING_ELECTION));
	result << uint8(KING_ELECTION_SCHEDULE);

	switch (m_byImType)
	{
		// No impeachment, send election date.
		case 0:
		{
			// Client expects month as 1,12 (tm_mon is 0,11)
			uint8 byElectionMonth = g_localTime.tm_mon + 1;

			/* When's the next election? */
			// If we've passed the election date, we need next month's election.
			// (NOTE: this is official behaviour; it disregards the month set in the table)
			if (g_localTime.tm_mday > m_byDay)
			{
				// Next month is January? Make it so.
				++byElectionMonth;
				while (byElectionMonth > 12)
					byElectionMonth -= 12;
			}

			result	<< uint8(1) // election type
					<< byElectionMonth 
					<< m_byDay << m_byHour << m_byMinute;
		} break;

		// Last scheduled impeachment?
		case 1:
		{
			result	<< uint8(3)
					<< m_byImMonth 
					<< m_byImDay << m_byImHour << m_byImMinute;
		} break;

		// Next impeachment?
		case 3:
		{
			// This should not be necessary, but will leave.
			uint8 byImpeachmentMonth = m_byImMonth;
			while (byImpeachmentMonth > 12)
				m_byImMonth -= 12;

			result	<< uint8(2)
					<< byImpeachmentMonth
					<< m_byImDay << m_byImHour << m_byImMinute;
		} break;

		default:
			return;
	}

	pUser->Send(&result);
}

void CKingSystem::CandidacyRecommend(CUser * pUser, Packet & pkt) {}
void CKingSystem::CandidacyNoticeBoard(CUser * pUser, Packet & pkt) {}
void CKingSystem::ElectionPoll(CUser * pUser, Packet & pkt) {}
void CKingSystem::CandidacyResign(CUser * pUser, Packet & pkt) {}

void CKingSystem::ImpeachmentSystem(CUser * pUser, Packet & pkt)
{
	switch (pkt.read<uint8>())
	{
	case KING_IMPEACHMENT_REQUEST:
		ImpeachmentRequest(pUser, pkt);
		break;

	case KING_IMPEACHMENT_REQUEST_ELECT:
		ImpeachmentRequestElect(pUser, pkt);
		break;

	case KING_IMPEACHMENT_LIST:
		ImpeachmentList(pUser, pkt);
		break;

	case KING_IMPEACHMENT_ELECT:
		ImpeachmentElect(pUser, pkt);
		break;

	case KING_IMPEACHMENT_REQUEST_UI_OPEN:
		ImpeachmentRequestUiOpen(pUser, pkt);
		break;

	case KING_IMPEACHMENT_ELECTION_UI_OPEN:
		ImpeachmentElectionUiOpen(pUser, pkt);
		break;
	}
}

void CKingSystem::ImpeachmentRequest(CUser * pUser, Packet & pkt) {}
void CKingSystem::ImpeachmentRequestElect(CUser * pUser, Packet & pkt) {}
void CKingSystem::ImpeachmentList(CUser * pUser, Packet & pkt) {}
void CKingSystem::ImpeachmentElect(CUser * pUser, Packet & pkt) {}
void CKingSystem::ImpeachmentRequestUiOpen(CUser * pUser, Packet & pkt) {}
void CKingSystem::ImpeachmentElectionUiOpen(CUser * pUser, Packet & pkt) {}

void CKingSystem::KingTaxSystem(CUser * pUser, Packet & pkt)
{
	Packet result(WIZ_KING, uint8(KING_TAX));
	uint8 bOpcode = pkt.read<uint8>();
	result << bOpcode;

	// If you're not a King, you shouldn't have access to this command.
	if (!pUser->isKing())
	{
		result << int16(-1);
		pUser->Send(&result);
		return;
	}

	switch (bOpcode)
	{
		// Collect King's fund
		case 2:
			break;

		// Lookup the tariff
		case 3:
		{
			result << int16(1) << m_byTerritoryTariff;
			pUser->Send(&result);
		} break;

		// Update the tariff
		case 4:
		{
			uint8 byTerritoryTariff = pkt.read<uint8>();

			// Invalid tariff amount
			if (byTerritoryTariff > 10)
			{
				result << int16(-2);
				pUser->Send(&result);
				return;
			}

			// Update the tariff
			m_byTerritoryTariff = byTerritoryTariff;

			// Let all users in your nation know.
			result << int16(1) << byTerritoryTariff << m_byNation;
			g_pMain->Send_All(&result, NULL, m_byNation);

			// Update the database (TO-DO: Implement the request)
			DatabaseThread::AddRequest(&result);
		} break;

		// King's sceptre
		case 7:
			break;
	}

}

void CKingSystem::KingSpecialEvent(CUser * pUser, Packet & pkt)
{
}