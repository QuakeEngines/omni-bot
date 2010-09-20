////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy$
// $LastChangedDate$
// $LastChangedRevision$
//
////////////////////////////////////////////////////////////////////////////////

#include "PrecompRTCW.h"
#include "ScriptManager.h"

#include "RTCW_Client.h"
#include "RTCW_NavigationFlags.h"
#include "RTCW_VoiceMacros.h"
#include "RTCW_FilterClosest.h"
#include "RTCW_Messages.h"
#include "RTCW_BaseStates.h"


//////////////////////////////////////////////////////////////////////////
// MOVE THIS

class Limbo : public StateSimultaneous
{
public:

	obReal GetPriority() 
	{
		return GetClient()->HasEntityFlag(RTCW_ENT_FLAG_INLIMBO) ? 1.f : 0.f;		
	}

	void Exit()
	{
		GetRootState()->OnSpawn();
	}

	State::StateStatus Update(float fDt) 
	{
		// need to do something special here?
		return State_Busy; 
	}

	Limbo() : StateSimultaneous("Limbo") 
	{
	}
protected:
};

class Incapacitated : public StateChild
{
public:
	obReal GetPriority() 
	{
		return !InterfaceFuncs::IsAlive(GetClient()->GetGameEntity()) ? 1.f : 0.f; 
	}

	State::StateStatus Update(float fDt) 
	{
		if(InterfaceFuncs::GetReinforceTime(GetClient()) < 1.0f)
		{
			if(!InterfaceFuncs::IsMedicNear(GetClient())) 
			{
				InterfaceFuncs::GoToLimbo(GetClient());
			}
		}
		GetClient()->SetMovementVector(Vector3f::ZERO);
		return State_Busy; 
	}

	Incapacitated() : StateChild("Incapacitated") 
	{
	}
protected:
};

//////////////////////////////////////////////////////////////////////////

RTCW_Client::RTCW_Client() :
	m_BreakableTargetDistance(0.f)
{
	m_StepHeight = 8.0f; // subtract a small value as a buffer to jump
}

RTCW_Client::~RTCW_Client()
{
}

void RTCW_Client::Init(int _gameid)
{
	Client::Init(_gameid);

	FilterPtr filter(new RTCW_FilterClosest(this, AiState::SensoryMemory::EntEnemy));
	filter->AddCategory(ENT_CAT_SHOOTABLE);
	GetTargetingSystem()->SetDefaultTargetingFilter(filter);
}


void RTCW_Client::ProcessEvent(const MessageHelper &_message, CallbackParameters &_cb)
{
	switch(_message.GetMessageId())
	{
		HANDLER(RTCW_EVENT_DROWNING)
		{
			_cb.CallScript();
			break;
		}
		HANDLER(RTCW_EVENT_RECIEVEDAMMO)
		{
			const Event_Ammo *m = _message.Get<Event_Ammo>();
			_cb.CallScript();
			_cb.AddEntity("who", m->m_WhoDoneIt);
			break;
		}
	}
	Client::ProcessEvent(_message, _cb);
}

NavFlags RTCW_Client::GetTeamFlag()
{
	return GetTeamFlag(GetTeam());
}

NavFlags RTCW_Client::GetTeamFlag(int _team)
{
	static const NavFlags defaultTeam = 0;
	switch(_team)
	{
	case RTCW_TEAM_AXIS:
		return F_NAV_TEAM1;
	case RTCW_TEAM_ALLIES:
		return F_NAV_TEAM2;	
	default:
		return defaultTeam;
	}
}

void RTCW_Client::SendVoiceMacro(int _macroId) 
{
	RTCW_VoiceMacros::SendVoiceMacro(this, _macroId);
}

int RTCW_Client::HandleVoiceMacroEvent(const MessageHelper &_message)
{
	const Event_VoiceMacro *m = _message.Get<Event_VoiceMacro>();
	
	int iVoiceId = RTCW_VoiceMacros::GetVChatId(m->m_MacroString);
	switch(iVoiceId)
	{
		/*case VCHAT_TEAM_PATHCLEARED:
		case VCHAT_TEAM_ENEMYWEAK:
		case VCHAT_TEAM_ALLCLEAR:
		case VCHAT_TEAM_INCOMING:*/
	case VCHAT_TEAM_FIREINTHEHOLE:
		{
			break;
		}
		/*case VCHAT_TEAM_ONDEFENSE:
		case VCHAT_TEAM_ONOFFENSE:
		case VCHAT_TEAM_TAKINGFIRE:
		case VCHAT_TEAM_MINESCLEARED:
		case VCHAT_TEAM_ENEMYDISGUISED:*/
	case VCHAT_TEAM_MEDIC:
		{
			if(m->m_WhoSaidIt.IsValid() && (GetClass() == RTCW_CLASS_MEDIC))
			{
				//FIXME
				/*
				BotBrain::EvaluatorPtr eval(new RTCW_Evaluator_RequestGiveHealth(this, m->m_WhoSaidIt));
				if(GetBrain())
					GetBrain()->AddGoalEvaluator(eval);*/
			}
			break;
		}
	case VCHAT_TEAM_NEEDAMMO:
		{
			if(m->m_WhoSaidIt.IsValid() && (GetClass() == RTCW_CLASS_LIEUTENANT))
			{
				//FIXME
				/*
				BotBrain::EvaluatorPtr eval(new RTCW_Evaluator_RequestGiveAmmo(this, m->m_WhoSaidIt));
				if(GetBrain())
					GetBrain()->AddGoalEvaluator(eval);*/
			}
			break;
		}
		/*case VCHAT_TEAM_NEEDBACKUP:
		case VCHAT_TEAM_NEEDENGINEER:
		case VCHAT_TEAM_COVERME:
		case VCHAT_TEAM_HOLDFIRE:
		case VCHAT_TEAM_WHERETO:
		case VCHAT_TEAM_NEEDOPS:
		case VCHAT_TEAM_FOLLOWME:
		case VCHAT_TEAM_LETGO:
		case VCHAT_TEAM_MOVE:
		case VCHAT_TEAM_CLEARPATH:
		case VCHAT_TEAM_DEFENDOBJECTIVE:
		case VCHAT_TEAM_DISARMDYNAMITE:
		case VCHAT_TEAM_CLEARMINES:
		case VCHAT_TEAM_REINFORCE_OFF:
		case VCHAT_TEAM_REINFORCE_DEF:
		case VCHAT_TEAM_AFFIRMATIVE:
		case VCHAT_TEAM_NEGATIVE:
		case VCHAT_TEAM_THANKS:
		case VCHAT_TEAM_WELCOME:
		case VCHAT_TEAM_SORRY:
		case VCHAT_TEAM_OOPS:*/

		// Command related
		/*case VCHAT_TEAM_COMMANDACKNOWLEDGED:
		case VCHAT_TEAM_COMMANDDECLINED:
		case VCHAT_TEAM_COMMANDCOMPLETED:
		case VCHAT_TEAM_DESTROYPRIMARY:
		case VCHAT_TEAM_DESTROYSECONDARY:
		case VCHAT_TEAM_DESTROYCONSTRUCTION:
		case VCHAT_TEAM_CONSTRUCTIONCOMMENCING:
		case VCHAT_TEAM_REPAIRVEHICLE:
		case VCHAT_TEAM_DESTROYVEHICLE:
		case VCHAT_TEAM_ESCORTVEHICLE:*/

		// Global messages
		/*case VCHAT_GLOBAL_AFFIRMATIVE:
		case VCHAT_GLOBAL_NEGATIVE:
		case VCHAT_GLOBAL_ENEMYWEAK:
		case VCHAT_GLOBAL_HI:
		case VCHAT_GLOBAL_BYE:
		case VCHAT_GLOBAL_GREATSHOT:
		case VCHAT_GLOBAL_CHEER:
		case VCHAT_GLOBAL_THANKS:
		case VCHAT_GLOBAL_WELCOME:
		case VCHAT_GLOBAL_OOPS:
		case VCHAT_GLOBAL_SORRY:
		case VCHAT_GLOBAL_HOLDFIRE:
		case VCHAT_GLOBAL_GOODGAME:*/
	}
	return iVoiceId;
}

void RTCW_Client::ProcessGotoNode(const Path &_path)
{
	Path::PathPoint pt;
	_path.GetCurrentPt(pt);

	if(pt.m_NavFlags & F_RTCW_NAV_SPRINT)
	{
		PressButton(BOT_BUTTON_SPRINT);
	}

	//fix some ladder stuckages ...
	//if they miss the node, they keep pressing
	/*if(pt.m_NavFlags & F_NAV_CLIMB)
	{
		PressButton(BOT_BUTTON_FWD);
	}*/

	if(pt.m_NavFlags & F_RTCW_NAV_STRAFE_L)
	{		
		PressButton(BOT_BUTTON_LSTRAFE);
	}
	else if(pt.m_NavFlags & F_RTCW_NAV_STRAFE_R)
	{
		PressButton(BOT_BUTTON_RSTRAFE);
	}
}

float RTCW_Client::GetGameVar(GameVar _var) const
{
	switch(_var)
	{
	case JumpGapOffset:
		return 0.32f;
	}
	return 0.0f;
}

float RTCW_Client::GetAvoidRadius(int _class) const
{
	switch(_class)
	{
	//case ENT_CLASS_GENERIC_BUTTON:
	case ENT_CLASS_GENERIC_HEALTH:
	case ENT_CLASS_GENERIC_AMMO:
	case ENT_CLASS_GENERIC_ARMOR:
		return 5.0f;
	}

	switch(_class)
	{
	case RTCW_CLASS_SOLDIER:
	case RTCW_CLASS_MEDIC:
	case RTCW_CLASS_ENGINEER:
	case RTCW_CLASS_LIEUTENANT:
		return 16.0f;
	case RTCW_CLASSEX_DYNAMITE:
		return 32.0f;
	}

	return 32.0f;
}

bool RTCW_Client::DoesBotHaveFlag(MapGoalPtr _mapgoal)
{
	return InterfaceFuncs::HasFlag(this);
}

bool RTCW_Client::IsFlagGrabbable(MapGoalPtr _mapgoal)
{
	return InterfaceFuncs::ItemCanBeGrabbed(this, _mapgoal->GetEntity());
}

bool RTCW_Client::CanBotSnipe() 
{
	if(GetClass() == RTCW_CLASS_SOLDIER)
	{
		// Make sure we have a sniping weapon.
		if (GetWeaponSystem()->HasAmmo(RTCW_WP_MAUSER) || 
			GetWeaponSystem()->HasAmmo(RTCW_WP_SNIPERRIFLE))
			return true;
	}
	return false;
}

bool RTCW_Client::GetSniperWeapon(int &nonscoped, int &scoped)
{
	nonscoped = 0;
	scoped = 0;

	if(GetClass() == RTCW_CLASS_SOLDIER)
	{
		if(GetWeaponSystem()->HasWeapon(RTCW_WP_SNIPERRIFLE))
		{
			nonscoped = RTCW_WP_MAUSER;
			scoped = RTCW_WP_SNIPERRIFLE;
			return true;
		}
	}
	return false;
}

float RTCW_Client::NavCallback(const NavFlags &_flag, Waypoint *from, Waypoint *to) 
{
	using namespace AiState;
	String gn;

	if(_flag & F_RTCW_NAV_USEPATH)
	{
		const PropertyMap::ValueMap &pm = to->GetPropertyMap().GetProperties();
		PropertyMap::ValueMap::const_iterator cIt = pm.begin();
		FINDSTATE(hl,HighLevel,this->GetStateRoot());
		
		if(hl && hl->GetActiveState())
		{
			gn = Utils::StringToLower(hl->GetActiveState()->GetName());

			//EngineFuncs::ConsoleMessage("current goal: %s", gn.c_str());
			
			for(; cIt != pm.end(); ++cIt)
			{
			//	EngineFuncs::ConsoleMessage("property: %s = %s", 
			//		(*cIt).first.c_str(), (*cIt).second.c_str());

				if ( gn == (*cIt).first && (*cIt).second == "true" )
					return 1.0f;
			}
		}
	}

	return 0.f;
}

//////////////////////////////////////////////////////////////////////////

void RTCW_Client::SetupBehaviorTree()
{
	using namespace AiState;
	delete GetStateRoot()->ReplaceState("Dead", new Limbo);
	GetStateRoot()->InsertAfter("Limbo", new Incapacitated);

	GetStateRoot()->AppendTo("HighLevel", new RepairMg42);
	GetStateRoot()->AppendTo("HighLevel", new TakeCheckPoint);
	GetStateRoot()->AppendTo("HighLevel", new ReviveTeammate);
	GetStateRoot()->AppendTo("HighLevel", new DefuseDynamite);
	GetStateRoot()->AppendTo("HighLevel", new CallArtillery);
}

#ifdef ENABLE_REMOTE_DEBUGGING
void RTCW_Client::Sync( RemoteLib::DataBuffer & db, bool fullSync ) {
	const char * classImg = "";
	switch ( GetClass() )
	{
	case RTCW_CLASS_SOLDIER:
		classImg = "et/class_soldier.png";
		break;
	case RTCW_CLASS_MEDIC:
		classImg = "et/class_medic.png";
		break;
	case RTCW_CLASS_ENGINEER:
		classImg = "et/class_engineer.png";
		break;
	case RTCW_CLASS_LIEUTENANT:
		classImg = "et/class_fieldops.png";
		break;
	}

	Box3f obb;
	EngineFuncs::EntityWorldOBB( GetGameEntity(), obb );

	db.beginWrite( RemoteLib::DataBuffer::WriteModeAllOrNone );
	db.startSizeHeader();
	db.writeInt32( RemoteLib::ID_image );	
	db.writeString( va( "client/%s", GetName( true ) ) );
	db.writeString( classImg );
	db.writeFloat16( obb.Center.x, 0 );
	db.writeFloat16( obb.Center.y, 0 );
	db.writeFloat16( obb.Extent[0] * 2.0f, 0 );
	db.writeFloat16( obb.Extent[1] * 2.0f, 0 );
	db.endSizeHeader();
	db.endWrite();

	Client::Sync( db, fullSync );
}
#endif
