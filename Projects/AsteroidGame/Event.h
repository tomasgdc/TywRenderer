#pragma once



class EvnData_PhysicsCollided : public IEventData
{
public:
	static const EventType sk_EventType;

	EvnData_PhysicsCollided() {}
	~EvnData_PhysicsCollided() {}

	const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	//not implemented
	float VGetTimeStamp(void) const
	{
		return 0.0f;
	}
};


class EvnData_OnFire : public IEventData
{
public:
	static const EventType sk_EventType;

	EvnData_OnFire() {}
	~EvnData_OnFire() {}

	const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	//not implemented
	float VGetTimeStamp(void) const
	{
		return 0.0f;
	}
};

class EvnData_ActorMove : public IEventData
{
public:
	static const EventType sk_EventType;

	EvnData_ActorMove() {}
	~EvnData_ActorMove() {}

	const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	//not implemented
	float VGetTimeStamp(void) const
	{
		return 0.0f;
	}
};



namespace Event
{
	inline const EventType Get_EvtData_PhysicsCollided()
	{
		return EvnData_PhysicsCollided::sk_EventType;
	}

	inline const EventType Get_EvtData_OnFire()
	{
		return EvnData_OnFire::sk_EventType;
	}

	inline const EventType Get_EvtData_ActorMove()
	{
		return EvnData_ActorMove::sk_EventType;
	}
}