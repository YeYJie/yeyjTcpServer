#ifndef _HASH_H_
#define _HASH_H_

#include <functional>
#include <cstdlib>
#include <climits>
#include <cassert>
#include <sys/time.h>

namespace yeyj
{

#define HASHTABLE_INIT_SIZE 4

template<typename KeyType, typename ValueType>
struct dictEntry
{
	KeyType 		key;
	ValueType 		value;
	dictEntry * 	next;

	dictEntry() {
		next = nullptr;
	}
	~dictEntry() {
		if(next)
			delete next;
	}

	KeyType & getKey() {
		return key;
	}
	void setKey(const KeyType & k) {
		key = k;
	}

	ValueType & getValue() {
		return value;
	}
	void setValue(const ValueType & v) {
		value = v;
	}
};

template<typename KeyType, typename ValueType>
struct dictht {
	dictEntry<KeyType, ValueType> ** 	table;
	unsigned long 						size;
	unsigned long 						sizemask;
	unsigned long 						used;

	dictht() {
		table = nullptr;
		size = sizemask = used = 0;
	}
	void clear() {
		if(table) {
			for(int i = 0; i < size; ++i) {
				if(table[i])
					delete table[i];
			}
			delete[] table;
			table = nullptr;
		}
		size = sizemask = used = 0;
	}
};


template<typename KeyType, typename ValueType>
struct dict {
	dictht<KeyType, ValueType> ht[2];
	long rehashidx; /* rehashing not in progress if rehashidx == -1 */
	unsigned long iterators; /* number of iterators currently running */

	dict() {
		rehashidx = -1;
		iterators = 0;
	}
	void clear() {
		ht[0].clear();
		ht[1].clear();
		rehashidx = -1;
		iterators = 0;
	}
	~dict() {
		clear();
	}
};


/*************************************************
*
*
*			class HashTable
*
*
***************************************************/

template<typename KeyType, typename ValueType,
			int maxLoadFactor = 5,
			typename HashFunction = std::hash<KeyType>,
			typename KeyCompareFunction = std::equal_to<KeyType>>
class HashTable
{
public:

	const int DICT_OK = 0;
	const int DICT_ERR = 1;

public:

	HashTable();

	~HashTable();

	int size() const;

	bool empty() const;

	int insert(const KeyType & key, const ValueType & val);

	// return 1 if key not exists, otherwise, replace value and return 0
	int replace(const KeyType & key, const ValueType & val);

	void erase(const KeyType & key);

	void clear();

	ValueType & operator[](const KeyType & key);

	KeyType getRandomKey();

	ValueType getRandomValue();

	bool exist(const KeyType & key);

private:

	dict<KeyType, ValueType> * _d;

	HashFunction 			_hashFunction;
	KeyCompareFunction 		_keyCompareFunction;


private:	// some helper function

	bool isRehashing() const;

	int expand(unsigned long size);

	// static unsigned long nextPower(unsigned long size);

	int rehash(int n);


	long long timeInMilliseconds();

	int rehashMilliseconds(int ms);

	void rehashStep();

	// if key already exists, '*existing' will be set to the existing entry
	dictEntry<KeyType, ValueType> * insertRaw(const KeyType & key,
							dictEntry<KeyType, ValueType> ** existing);

	int keyIndex(const KeyType & key, unsigned int hash,
					dictEntry<KeyType, ValueType> ** existing);

	int expandIfNeeded();

	dictEntry<KeyType, ValueType> * getRandomEntry();


}; // class HashTable


/*
*		implementing code...
*/

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
inline HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::HashTable()
{
	_d = new dict<KeyType, ValueType>;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
inline HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::~HashTable()
{
	delete _d;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
inline int HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::size() const
{
	if(_d)
		return _d->ht[0].used + _d->ht[1].used;
	else
		return 0;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
inline bool HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::empty() const
{
	return size() == 0;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
int HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::insert(const KeyType & key, const ValueType & val)
{
	dictEntry<KeyType, ValueType> * entry = insertRaw(key, nullptr);

	if (!entry)
		return DICT_ERR;
	entry->setValue(val);
	return DICT_OK;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
int HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::replace(const KeyType & key, const ValueType & val)
{
	dictEntry<KeyType, ValueType> *entry, *existing, auxentry;

	entry = insertRaw(key, &existing);

	if (entry)
	{
		// key not exists, so we justsimply insert value and return
		// mydictSetVal(entry, val);
		entry->setValue(val);
		return 1;
	}

	auxentry = *existing;
	existing->setValue(val);
	return 0;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
void HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::erase(const KeyType & key)
{
	unsigned int h, idx;
	dictEntry<KeyType, ValueType> *he, *prevHe;
	int table;

	if (_d->ht[0].used == 0 && _d->ht[1].used == 0)
		return;

	if (isRehashing())
		rehashStep();

	h = _hashFunction(key);

	for (table = 0; table <= 1; table++) {
		idx = h & _d->ht[table].sizemask;
		he = _d->ht[table].table[idx];
		prevHe = NULL;
		while(he) {
			if(_keyCompareFunction(key, he->key))
			{
				if (prevHe)
					prevHe->next = he->next;
				else
					_d->ht[table].table[idx] = he->next;

				he->next = nullptr;
				delete he;

				_d->ht[table].used--;
				return;
			}
			prevHe = he;
			he = he->next;
		}
		if (!isRehashing())
			break;
	}
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
void HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::clear()
{
	_d->clear();
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
ValueType & HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::operator[](const KeyType & key)
{
	dictEntry<KeyType, ValueType> * entry, *existing;
	entry = insertRaw(key, &existing);
	if(!entry)
		return existing->getValue();
	else
		return entry->getValue();
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
KeyType HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::getRandomKey()
{
	dictEntry<KeyType, ValueType> * entry = getRandomEntry();
	return entry->getKey();
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
ValueType HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::getRandomValue()
{
	dictEntry<KeyType, ValueType> * entry = getRandomEntry();
	return entry->getValue();
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
bool HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::exist(const KeyType & key)
{
	dictEntry<KeyType, ValueType> *he;
	unsigned int h, idx, table;

	if(empty())
		return false;

	if (isRehashing())
		rehashStep();

	h = _hashFunction(key);

	for (table = 0; table <= 1; table++)
	{
		idx = h & _d->ht[table].sizemask;
		he = _d->ht[table].table[idx];
		while(he)
		{
			if(_keyCompareFunction(key, he->key))
				return true;
			he = he->next;
		}
		if (!isRehashing())
			break;
	}
	return false;
}


static unsigned long nextPower(unsigned long size)
{
	unsigned long i = HASHTABLE_INIT_SIZE;

	if (size >= LONG_MAX) return LONG_MAX;
	while(1) {
		if (i >= size)
			return i;
		i *= 2;
	}
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
inline bool HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::isRehashing() const
{
	return _d->rehashidx != -1;
}


template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
int HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::expand(unsigned long size)
{
	dictht<KeyType, ValueType> n; /* the new hash table */
	unsigned long realsize = nextPower(size);

	/* the size is invalid if it is smaller than the number of
	 * elements already inside the hash table */
	if (isRehashing() || _d->ht[0].used > size)
		return DICT_ERR;

	/* Rehashing to the same table size is not useful. */
	if (realsize == _d->ht[0].size) return DICT_ERR;

	/* Allocate the new hash table and initialize all pointers to NULL */
	n.size = realsize;
	n.sizemask = realsize-1;
	// n.table = (dictEntry<KeyType, ValueType> **)calloc(realsize, sizeof(dictEntry<KeyType, ValueType>*));
	n.table = new dictEntry<KeyType, ValueType>*[realsize]{ nullptr };
	n.used = 0;

	/* Is this the first initialization? If so it's not really a rehashing
	 * we just set the first hash table so that it can accept keys. */
	if (_d->ht[0].table == nullptr) {
		_d->ht[0] = n;
		return DICT_OK;
	}

	/* Prepare a second hash table for incremental rehashing */
	_d->ht[1] = n;
	_d->rehashidx = 0;
	return DICT_OK;
}


template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
int HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::rehash(int n)
{
	int empty_visits = n*10; /* Max number of empty buckets to visit. */
	if (!isRehashing()) return 0;

	while(n-- && _d->ht[0].used != 0) {
		dictEntry<KeyType, ValueType> *de, *nextde;

		/* Note that rehashidx can't overflow as we are sure there are more
		 * elements because ht[0].used != 0 */
		assert(_d->ht[0].size > (unsigned long)_d->rehashidx);
		while(_d->ht[0].table[_d->rehashidx] == NULL) {
			_d->rehashidx++;
			if (--empty_visits == 0) return 1;
		}
		de = _d->ht[0].table[_d->rehashidx];
		/* Move all the keys in this bucket from the old to the new hash HT */
		while(de) {
			unsigned int h;

			nextde = de->next;
			/* Get the index in the new hash table */
			h = _hashFunction(de->key) & _d->ht[1].sizemask;
			de->next = _d->ht[1].table[h];
			_d->ht[1].table[h] = de;
			_d->ht[0].used--;
			_d->ht[1].used++;
			de = nextde;
		}
		_d->ht[0].table[_d->rehashidx] = NULL;
		_d->rehashidx++;
	}

	/* Check if we already rehashed the whole table... */
	if (_d->ht[0].used == 0) {
		_d->ht[0].clear();
		_d->ht[0] = _d->ht[1];
		_d->ht[1].table = nullptr;
		_d->ht[1].size = _d->ht[1].sizemask = _d->ht[1].used = 0;

		_d->rehashidx = -1;
		return 0;
	}

	// More to rehash
	return 1;
}


template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
long long HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::timeInMilliseconds()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
int HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::rehashMilliseconds(int ms)
{
	long long start = timeInMilliseconds();
	int rehashes = 0;

	while(rehash(100))
	{
		rehashes += 100;
		if (timeInMilliseconds()-start > ms) break;
	}
	return rehashes;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
void HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::rehashStep()
{
	if(_d->iterators == 0)
		rehash(1);
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
dictEntry<KeyType, ValueType> * HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::insertRaw(const KeyType & key, dictEntry<KeyType, ValueType> ** existing)
{
	int index;
	dictEntry<KeyType, ValueType> *entry;
	dictht<KeyType, ValueType> *ht;

	if (isRehashing())
		rehashStep();

	/* Get the index of the new element, or -1 if
	 * the element already exists. */
	if ((index = keyIndex(key, _hashFunction(key), existing)) == -1)
		return nullptr;

	/* Allocate the memory and store the new entry.
	 * Insert the element in top, with the assumption that in a database
	 * system it is more likely that recently added entries are accessed
	 * more frequently. */
	ht = isRehashing() ? &_d->ht[1] : &_d->ht[0];
	entry = new dictEntry<KeyType, ValueType>;
	entry->next = ht->table[index];
	ht->table[index] = entry;
	ht->used++;

	/* Set the hash entry fields. */
	entry->setKey(key);
	return entry;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
int HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::keyIndex(const KeyType & key, unsigned int hash, dictEntry<KeyType, ValueType> ** existing)
{
	unsigned int idx, table;
	dictEntry<KeyType, ValueType> *he;
	if (existing) *existing = nullptr;

	/* Expand the hash table if needed */
	if (expandIfNeeded() == DICT_ERR)
		return -1;
	for (table = 0; table <= 1; table++) {
		idx = hash & _d->ht[table].sizemask;
		/* Search if this slot does not already contain the given key */
		he = _d->ht[table].table[idx];
		while(he) {
			if(_keyCompareFunction(key, he->key))
			{
				if (existing) *existing = he;
				return -1;
			}
			he = he->next;
		}
		if (!isRehashing())
			break;
	}
	return idx;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
int HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::expandIfNeeded()
{
	if (isRehashing())
		return DICT_OK;

	if (_d->ht[0].size == 0)
		return expand(HASHTABLE_INIT_SIZE);

	if(_d->ht[0].used/_d->ht[0].size > maxLoadFactor)
		return expand(_d->ht[0].used * 2);
	return DICT_OK;
}

template<typename KeyType, typename ValueType,
		int maxLoadFactor, typename HashFunction,typename KeyCompareFunction>
dictEntry<KeyType, ValueType> * HashTable<KeyType, ValueType, maxLoadFactor, HashFunction, KeyCompareFunction>
	::getRandomEntry()
{
	dictEntry<KeyType, ValueType> *he, *orighe;
	unsigned int h;
	int listlen, listele;

	if (size() == 0)
		return nullptr;

	if (isRehashing())
		rehashStep();

	if (isRehashing())
	{
		do {
			/* We are sure there are no elements in indexes from 0
			 * to rehashidx-1 */
			h = _d->rehashidx + (random() % (_d->ht[0].size +
											_d->ht[1].size -
											_d->rehashidx));
			he = (h >= _d->ht[0].size) ?
							_d->ht[1].table[h - _d->ht[0].size]
							: _d->ht[0].table[h];
		} while(he == nullptr);
	}
	else
	{
		do {
			h = random() & _d->ht[0].sizemask;
			he = _d->ht[0].table[h];
		} while(he == nullptr);
	}

	listlen = 0;
	orighe = he;
	while(he) {
		he = he->next;
		listlen++;
	}
	listele = random() % listlen;
	he = orighe;
	while(listele--) he = he->next;
	return he;
}


} // namespace yeyj

#endif