#pragma once

#include <list>
#include <map>
#include <functional>


//Forward declaration
class IEventData;

typedef IEventData* IEventDataPtr;
typedef std::function<void()> EventListenerDelegate;
typedef unsigned long EventType;

//Global
const unsigned int EVENTMANAGER_NUM_QUEUES = 2;

class EventManager
{
public:
	EventManager();
	~EventManager();

	//Register listeners
	bool AddListener(const EventListenerDelegate& eventDelegate, const EventType& type);

	//Remove listeners
	bool RemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type);

	//You can trigger event without putting it on the queue
	bool TriggerEvent(const EventType& pEvent);

	//Puts event on the queue
	bool QueueEvent(const EventType& pEvent);

	//Abort needed event
	bool AbortEvent(const EventType& inType, bool allOfType);

	//Processes queued events
	bool Update(unsigned long maxMillis);

private:
	typedef std::list<EventListenerDelegate> EventListenerList;
	typedef std::map<EventType, EventListenerList> EventListenerMap;
	typedef std::list<EventType> EventQueue;

	EventQueue		 m_queues[EVENTMANAGER_NUM_QUEUES];
	EventListenerMap m_eventListeners;
	int				 m_activeQueue;  // index of actively processing queue; events enque to the opposing queue
};


class IEventData
{
public:
	virtual ~IEventData(void) {}
	virtual const EventType& VGetEventType(void) const = 0;
	virtual float VGetTimeStamp(void) const = 0;
};

#include "Event.h"