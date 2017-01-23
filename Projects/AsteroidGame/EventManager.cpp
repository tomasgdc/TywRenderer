#include "EventManager.h"


//Other includes
#include <cassert>
#include <Windows.h>

//Global
int kINFINITE = 0xffffffff;

EventManager::EventManager() :m_activeQueue(0)
{

}

EventManager::~EventManager()
{

}


bool EventManager::AddListener(const EventListenerDelegate& eventDelegate, const EventType& type)
{
	EventListenerList & eventListenerList = m_eventListeners[type];  // this will find or create the entry
	for (auto& eventListener : eventListenerList)
	{
		if (eventDelegate.target_type() == eventListener.target_type())
		{
			return false;
		}
	}
	eventListenerList.push_back(eventDelegate);
	return true;
}


bool EventManager::RemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type)
{
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
				success = true;
				break;  // we don't need to continue because it should be impossible for the same delegate function to be registered for the same event more than once
			}
		}
	}
	return success;
}


bool EventManager::TriggerEvent(const EventType& pEvent)
{
	bool processed = false;

	auto findIt = m_eventListeners.find(pEvent);
	if (findIt != m_eventListeners.end())
	{
		const EventListenerList& eventListenerList = findIt->second;
		for (EventListenerList::const_iterator it = eventListenerList.begin(); it != eventListenerList.end(); ++it)
		{
			EventListenerDelegate listener = (*it);
			listener();  // call the delegate
			processed = true;
		}
	}

	return processed;
}


bool EventManager::QueueEvent(const EventType& pEvent)
{
	assert(m_activeQueue >= 0);
	assert(m_activeQueue < EVENTMANAGER_NUM_QUEUES);


	auto findIt = m_eventListeners.find(pEvent);
	if (findIt != m_eventListeners.end())
	{
		m_queues[m_activeQueue].push_back(pEvent);
		return true;
	}
	else
	{
		return false;
	}
	return true;
}


bool EventManager::AbortEvent(const EventType& inType, bool allOfType)
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

			if ((*thisIt) == inType)
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


bool EventManager::Update(unsigned long maxMillis)
{
	unsigned long currMs = GetTickCount();
	unsigned long maxMs = ((maxMillis == kINFINITE) ? (kINFINITE) : (currMs + maxMillis));

	// swap active queues and clear the new queue after the swap
	int queueToProcess = m_activeQueue;
	m_activeQueue = (m_activeQueue + 1) % EVENTMANAGER_NUM_QUEUES;
	m_queues[m_activeQueue].clear();

	// Process the queue
	while (!m_queues[queueToProcess].empty())
	{
		// pop the front of the queue
		const EventType eventType = m_queues[queueToProcess].front();
		m_queues[queueToProcess].pop_front();

		// find all the delegate functions registered for this event
		auto findIt = m_eventListeners.find(eventType);
		if (findIt != m_eventListeners.end())
		{
			const EventListenerList& eventListeners = findIt->second;

			// call each listener
			for (auto it = eventListeners.begin(); it != eventListeners.end(); ++it)
			{
				EventListenerDelegate listener = (*it);
				listener();
			}
		}

		// check to see if time ran out
		currMs = GetTickCount();
		if (maxMillis != kINFINITE && currMs >= maxMs)
		{
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
			const EventType& pEventType = m_queues[queueToProcess].back();
			m_queues[queueToProcess].pop_back();
			m_queues[m_activeQueue].push_front(pEventType);
		}
	}

	return queueFlushed;
}