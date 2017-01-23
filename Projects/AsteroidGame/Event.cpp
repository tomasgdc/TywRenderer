#include "EventManager.h"
#include "Event.h"

// To define a new event - you need a 32-bit GUID. 
// In Visual Studio, go to Tools->Create GUID and grab the first bit.
//======================================================================================
const EventType EvnData_PhysicsCollided::sk_EventType(0x9642c9cc);
const EventType EvnData_OnFire::sk_EventType(0xec31b3bb);
const EventType EvnData_ActorMove::sk_EventType(0xa21cb99f);
//======================================================================================