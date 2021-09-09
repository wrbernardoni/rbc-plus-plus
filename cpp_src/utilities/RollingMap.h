#ifndef WRB_ROLLING_MAP_
#define WRB_ROLLING_MAP_

#include <unordered_map>
#include <mutex>

namespace WRB_std
{
	namespace RollingMap_Utils
	{
		template<class Key>
		struct RMNode
		{
			Key k;
			RMNode* next = NULL;
			RMNode* prev = NULL;
		};

		template<class Key, class Value>
		struct ValuePointer
		{
			Value v;
			RMNode<Key>* ptr;
		};
	};
	

	template 
	<	
		class Key, 
		class Value, 
		class Hash = std::hash<Key>, 
		class KeyEqual = std::equal_to<Key>
	>
	class RollingMap
	{
	private:
		std::unordered_map<Key, RollingMap_Utils::ValuePointer<Key, Value>, Hash, KeyEqual> map;
		RollingMap_Utils::RMNode<Key>* head;
		RollingMap_Utils::RMNode<Key>* tail;
		size_t maxSize;

		std::mutex listMutex;

		RollingMap_Utils::RMNode<Key>* pushNode(const Key key)
		{
			std::lock_guard<std::mutex> guard(listMutex);
			if (head == NULL)
			{
				head = new RollingMap_Utils::RMNode<Key>;
				head->k = key;
				tail = head;
			}
			else
			{
				head->prev = new RollingMap_Utils::RMNode<Key>;
				head->prev->next = head;
				head->prev->k = key;
				head = head->prev;
			}

			return head;
		};

		void popNode()
		{
			std::lock_guard<std::mutex> guard(listMutex);
			if (tail != NULL)
			{
				if (tail == head)
				{
					head = NULL;
				}

				RollingMap_Utils::RMNode<Key>* newTail = tail->prev;
				delete tail;
				tail = newTail;
			}
		};

		void moveFront(RollingMap_Utils::RMNode<Key>* node)
		{
			std::lock_guard<std::mutex> guard(listMutex);

			if (node->prev != NULL)
			{
				node->prev->next = node->next;
			}

			if ((node == tail) && (node->prev != NULL))
			{
				tail = node->prev;
			}


			if (head != node)
			{
				if (node->next != NULL)
				{
					node->next->prev = node->prev;
				}

				head->prev = node;
				node->next = head;
				node->prev = NULL;
				head = node;
			}
		};
	public:
		RollingMap(size_t n)
		{
			map.reserve(n);
			head = NULL;
			tail = NULL;
			maxSize = n;
		};

		~RollingMap()
		{
			RollingMap_Utils::RMNode<Key>* n = head;

			while (n != NULL)
			{
				RollingMap_Utils::RMNode<Key>* next = n->next;
				delete n;
				n = next;
			}
		};

		Value& operator[](const Key& key)
		{
			if (map.count(key) != 0)
			{
				moveFront(map[key].ptr);
				return map[key].v;
			}
			else
			{
				RollingMap_Utils::ValuePointer<Key, Value> val;
				val.ptr = pushNode(key);

				map[key] = val;

				if ((map.size() > maxSize) && (tail != NULL))
				{
					map.erase(tail->k);
					popNode();
				}

				return map[key].v;
			}
		};

		size_t size()
		{
			return map.size();
		};

		void resize(size_t n)
		{
			maxSize = n;
		}

		bool contains(const Key& key)
		{
			return (map.count(key) != 0);
		};
	};

	
};



#endif