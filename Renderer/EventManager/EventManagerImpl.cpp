#include  <RendererPch\stdafx.h>
#include "EventManagerImpl.h"



EventManager::EventManager():
	m_activeQueue(0)
{

}

EventManager::~EventManager()
{

}


bool EventManager::VAddListener(const EventListenerDelegate& eventDelegate, const EventType& type)
{
	//Engine::getInstance().Sys_Printf(stdout, "Events: Attempting to add delegate function for event type %ul\n", type);

	EventListenerList & eventListenerList =  m_eventListeners[type];  // this will find or create the entry
	for (auto& eventListener: eventListenerList)
	{
		if (eventDelegate.target_type() == eventListener.target_type())
		{
			//Engine::getInstance().Sys_Printf(stdout, "Events: Attempting to double-register a delegate \n");
			return false;
		}
	}
	eventListenerList.push_back(eventDelegate);
	//Engine::getInstance().Sys_Printf(stdout, "Events: Successfully added delegate for event type: %ul \n", type);

	return true;
}


bool EventManager::VRemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type)
{
	//Engine::getInstance().Sys_Printf(stdout, "Events: Attempting to remove delegate function from event type: %ul \n", type);
	bool success = false;

	auto findIt = m_eventListeners.find(type);
	if (findIt != m_eventListeners.end())
	{
		EventListenerList& listeners = findIt->second;
		for (auto it = listeners.begin(); it != listeners.end(); ++it)
		{
			if (eventDelegate.target_type() == (*it).target_type())
			{
				listeners.erase(it);
				//Engine::getInstance().Sys_Printf(stdout, "Events: Successfully removed delegate function from event type: %ul \n", type);
				success = true;
				break;  // we don't need to continue because it should be impossible for the same delegate function to be registered for the same event more than once
			}
		}
	}
	return success;
}


bool EventManager::VTriggerEvent(const IEventDataPtr& pEvent)
{
	//Engine::getInstance().Sys_Printf(stdout, "Events: Attempting to trigger event %s \n", pEvent->GetName());
	bool processed = false;

	auto findIt = m_eventListeners.find(pEvent->VGetEventType());
	if (findIt != m_eventListeners.end())
	{
		const EventListenerList& eventListenerList = findIt->second;
		for (EventListenerList::const_iterator it = eventListenerList.begin(); it != eventListenerList.end(); ++it)
		{
			EventListenerDelegate listener = (*it);
			//Engine::getInstance().Sys_Printf(stdout, "Events: Sending Event %s to delegate \n", pEvent->GetName());
			listener(pEvent);  // call the delegate
			processed = true;
		}
	}

	return processed;
}


bool EventManager::VQueueEvent(const IEventDataPtr& pEvent)
{
	assert(m_activeQueue >= 0);
	assert(m_activeQueue < EVENTMANAGER_NUM_QUEUES);

	// make sure the event is valid
	if (!pEvent)
	{
		//Engine::getInstance().Sys_Printf(stdout, "Invalid event in VQueueEvent()");
		return false;
	}

	//Engine::getInstance().Sys_Printf(stdout, "Events: Attempting to queue event: %s", pEvent->GetName());

	auto findIt = m_eventListeners.find(pEvent->VGetEventType());
	if (findIt != m_eventListeners.end())
	{
		m_queues[m_activeQueue].push_back(pEvent);
		//Engine::getInstance().Sys_Printf(stdout, "Events: Successfully queued event: %s", pEvent->GetName());
		return true;
	}
	else
	{
		//Engine::getInstance().Sys_Printf(stdout, "Events: Skipping event since there are no delegates registered to receive it: %s", pEvent->GetName());
		return false;
	}
	return true;
}


bool EventManager::VAbortEvent(const EventType& inType, bool allOfType)
{
	assert(m_activeQueue >= 0);
	assert(m_activeQueue < EVENTMANAGER_NUM_QUEUES);

	bool success = false;
	EventListenerMap::iterator findIt = m_eventListeners.find(inType);

	if (findIt != m_eventListeners.end())
	{
		EventQueue& eventQueue = m_queues[m_activeQueue];
		auto it = eventQueue.begin();
		while (it != eventQueue.end())
		{
			// Removing an item from the queue will invalidate the iterator, so have it point to the next member.  All
			// work inside this loop will be done using thisIt.
			auto thisIt = it;
			++it;

			if ((*thisIt)->VGetEventType() == inType)
			{
				eventQueue.erase(thisIt);
				success = true;
				if (!allOfType)
					break;
			}
		}
	}

	return success;
}


bool EventManager::VUpdate(unsigned long maxMillis)
{
	unsigned long currMs = GetTickCount();
	unsigned long maxMs = ((maxMillis == IEventManager::kINFINITE) ? (IEventManager::kINFINITE) : (currMs + maxMillis));

	// This section added to handle events from other threads.  Check out Chapter 20.
	IEventDataPtr pRealtimeEvent;
	while (m_realtimeEventQueue.try_pop(pRealtimeEvent))
	{
		VQueueEvent(pRealtimeEvent);

		currMs = GetTickCount();
		if (maxMillis != IEventManager::kINFINITE)
		{
			if (currMs >= maxMs)
			{
				//Engine::getInstance().Sys_Printf(stdout, "A realtime process is spamming the event manager!");
			}
		}
	}

	// swap active queues and clear the new queue after the swap
	int queueToProcess = m_activeQueue;
	m_activeQueue = (m_activeQueue + 1) % EVENTMANAGER_NUM_QUEUES;
	m_queues[m_activeQueue].clear();

	//Engine::getInstance().Sys_Printf(stdout, "EventLoop: Processing Event Queue %i; %i event to process", queueToProcess, m_queues[queueToProcess].size());

	// Process the queue
	while (!m_queues[queueToProcess].empty())
	{
		// pop the front of the queue
		IEventDataPtr pEvent = m_queues[queueToProcess].front();
		m_queues[queueToProcess].pop_front();
		//Engine::getInstance().Sys_Printf(stdout, "EventLoop: Processing Event %s", pEvent->GetName());

		const EventType& eventType = pEvent->VGetEventType();

		// find all the delegate functions registered for this event
		auto findIt = m_eventListeners.find(eventType);
		if (findIt != m_eventListeners.end())
		{
			const EventListenerList& eventListeners = findIt->second;
			//Engine::getInstance().Sys_Printf(stdout, "EventLoop: Found %i delegates", eventListeners.size());

			// call each listener
			for (auto it = eventListeners.begin(); it != eventListeners.end(); ++it)
			{
				EventListenerDelegate listener = (*it);
				//Engine::getInstance().Sys_Printf(stdout, "EventLoop: Sending event %s to delegate", pEvent->GetName());
				listener(pEvent);
			}
		}

		// check to see if time ran out
		currMs = GetTickCount();
		if (maxMillis != IEventManager::kINFINITE && currMs >= maxMs)
		{
			//Engine::getInstance().Sys_Printf(stdout, "EventLoop: Aborting event processing; time ran out");
			break;
		}
	}

	// If we couldn't process all of the events, push the remaining events to the new active queue.
	// Note: To preserve sequencing, go back-to-front, inserting them at the head of the active queue
	bool queueFlushed = (m_queues[queueToProcess].empty());
	if (!queueFlushed)
	{
		while (!m_queues[queueToProcess].empty())
		{
			IEventDataPtr pEvent = m_queues[queueToProcess].back();
			m_queues[queueToProcess].pop_back();
			m_queues[m_activeQueue].push_front(pEvent);
		}
	}

	return queueFlushed;
}