#ifndef __MESSAGE_MANAGER_H__
#define __MESSAGE_MANAGER_H__

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <iostream>
#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace geManagers {
	struct sMessage {
		int iUniqueMessageId;
		int iMessageId;
	};

	struct sSubscriber {
		int iSubscriberId;
		std::function<void(sMessage&)> fnQueueCallback;
	};

	struct sEvent {
		int	iEventId;
		std::string strEvent;
		std::vector<sSubscriber> vRegisteredCallbacks;
		std::deque<sMessage> vMessageQueue;
	};

	class MessageManager {
		private:
			std::mutex m_dispatchMutex;
			std::condition_variable m_queueWait;
			std::vector<sEvent> m_EventList;
			int m_iUniqueEventTracker;

			volatile bool m_bShutdown;

		public:
			MessageManager() {m_bShutdown = false;}
			~MessageManager() = default;

			// Create a new event to manage
			bool RegisterEvent(std::string strEventName, int iEventId);
			
			// Send message to each registered subscriber of a particular event
			void QueueMessage(std::string strEventName, sMessage& targetMessage);
			void QueueMessage(int iEventId, sMessage& targetMessage);

			// Subscribe to any particular events
			bool Subscribe(std::string strEventName, std::function<void(sMessage&)> fnQueueCallback, int iSubscriberId = 0);
			bool Subscribe(int iEventId, std::function<void(sMessage&)> fnQueueCallback, int iSubscriberId = 0);

			// Destroy a message in any particular event
			bool Consume(std::string strEventName, sMessage& targetMessage);
			bool Consume(int iEventId, sMessage& targetMessage);

			// Primary Running Thread
			void Start();

			// Logic Control
			void Die();

			// Check for pending messages overall
			bool Pending();
	};

	bool MessageManager::RegisterEvent(std::string strEventName, int iEventId) {
		std::lock_guard<std::mutex> guard(m_dispatchMutex);
		for (sEvent& x : m_EventList)
			if (x.iEventId == iEventId || x.strEvent == strEventName)
				return false;

		m_EventList.push_back(sEvent{ iEventId, strEventName });
		return true;
	}

	void MessageManager::QueueMessage(std::string strEventName, sMessage& targetMessage) {
		std::lock_guard<std::mutex> guard(m_dispatchMutex);
		for (sEvent& x : m_EventList) {
			if (x.strEvent == strEventName) {
				targetMessage.iUniqueMessageId = m_iUniqueEventTracker;
				m_iUniqueEventTracker++;
				x.vMessageQueue.push_back(targetMessage);
				m_queueWait.notify_all();
			}
		}
	}
	void MessageManager::QueueMessage(int iEventId, sMessage& targetMessage) {
		std::lock_guard<std::mutex> guard(m_dispatchMutex);
		for (sEvent& x : m_EventList) {
			if (x.iEventId == iEventId) {
				targetMessage.iUniqueMessageId = m_iUniqueEventTracker;
				m_iUniqueEventTracker++;
				x.vMessageQueue.push_back(targetMessage);
				m_queueWait.notify_all();
			}
		}
	}

	bool MessageManager::Subscribe(std::string strEventName, std::function<void(sMessage&)> fnQueueCallback, int iSubscriberId) {
		std::lock_guard<std::mutex> guard(m_dispatchMutex);
		for (sEvent& x : m_EventList) {
			if (x.strEvent == strEventName) {
				x.vRegisteredCallbacks.push_back(sSubscriber{ iSubscriberId, fnQueueCallback });
				return true;
			}
		}
		return false;
	}
	bool MessageManager::Subscribe(int iEventId, std::function<void(sMessage&)> fnQueueCallback, int iSubscriberId) {
		std::lock_guard<std::mutex> guard(m_dispatchMutex);
		for (sEvent& x : m_EventList) {
			if (x.iEventId == iEventId) {
				x.vRegisteredCallbacks.push_back(sSubscriber{ iSubscriberId, fnQueueCallback });
				return true;
			}
		}
		return false;
	}

	bool MessageManager::Consume(std::string strEventName, sMessage& targetMessage) {
		std::lock_guard<std::mutex> guard(m_dispatchMutex);
		for (sEvent& x : m_EventList) {
			if (x.strEvent == strEventName) {
				for (std::deque<sMessage>::iterator m = x.vMessageQueue.begin(); m != x.vMessageQueue.end(); m++) {
					if (m->iUniqueMessageId == targetMessage.iUniqueMessageId) {
						x.vMessageQueue.erase(m);
						return true;
					}
				}
			}
		}
		return false;
	}
	bool MessageManager::Consume(int iEventId, sMessage& targetMessage) {
		std::lock_guard<std::mutex> guard(m_dispatchMutex);
		for (sEvent& x : m_EventList) {
			if (x.iEventId == iEventId) {
				for (std::deque<sMessage>::iterator m = x.vMessageQueue.begin(); m != x.vMessageQueue.end(); m++) {
					if (m->iUniqueMessageId == targetMessage.iUniqueMessageId) {
						x.vMessageQueue.erase(m);
						return true;
					}
				}
			}
		}
		return false;
	}

	void MessageManager::Die() {
		std::lock_guard<std::mutex> guard(m_dispatchMutex);
		m_bShutdown = true;
		m_queueWait.notify_all();
	}
	void MessageManager::Start() {
		while (!m_bShutdown) {
			std::unique_lock<std::mutex> guard(m_dispatchMutex);
			m_queueWait.wait(guard);
			for (sEvent& x : m_EventList) {
				if (!x.vMessageQueue.empty()) {
					for (auto &subscribers : x.vRegisteredCallbacks)
						subscribers.fnQueueCallback(x.vMessageQueue.front());
					x.vMessageQueue.pop_front();
				}
			}
		}
	}
	bool MessageManager::Pending() {
		std::lock_guard<std::mutex> guard(m_dispatchMutex);
		for (sEvent& x : m_EventList) {
			if (!x.vMessageQueue.empty()) {
				return true;
			}
		}
		return false;
	}
};

#endif
