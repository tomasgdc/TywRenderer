#pragma once
#include "IEventManager.h"

const unsigned int EVENTMANAGER_NUM_QUEUES = 2;

class TYWENGINE_API EventManager: public IEventManager
{
private:

	typedef std::list<EventListenerDelegate> EventListenerList;
	typedef std::map<EventType, EventListenerList> EventListenerMap;
	typedef std::list<IEventDataPtr> EventQueue;

	int m_activeQueue;  // index of actively processing queue; events enque to the opposing queue
	EventListenerMap m_eventListeners;
	EventQueue m_queues[EVENTMANAGER_NUM_QUEUES];
	ThreadSafeEventQueue m_realtimeEventQueue;
public:
	EventManager();
	~EventManager();
	
	
	bool VAddListener(const EventListenerDelegate& eventDelegate, const EventType& type);
	bool VRemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type);


	bool VTriggerEvent(const IEventDataPtr& pEvent);


	bool VQueueEvent(const IEventDataPtr& pEvent);


	bool VAbortEvent(const EventType& type, bool allOfType = false);


	bool VUpdate(unsigned long maxMillis = 0xffffffff);
};
