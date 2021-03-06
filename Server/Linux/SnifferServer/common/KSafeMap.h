/*
 * KSafeMap.h
 *
 *  Created on: 2015-3-2
 *      Author: Max.chiu
 */

#ifndef KSafeMap_H_
#define KSafeMap_H_

#include "KMutex.h"

#include <map>
using namespace std;

template<typename Key, typename Value>
class KSafeMap {
	typedef map<Key, Value> SafeMap;

public:
	typedef typename SafeMap::iterator iterator;

	KSafeMap() {

	}

	virtual ~KSafeMap() {

	}

	void Insert(Key key, Value value) {
		mMap.insert( typename SafeMap::value_type(key, value) );
	}

	iterator Erase(Key key) {
		typename SafeMap::iterator itr = mMap.find(key);
		if( itr != mMap.end() ) {
			mMap.erase(itr);
		}
		return itr;
	}

	int Size() {
		return (int)mMap.size();
	}

	void Lock() {
		mKMutex.lock();
	}

	void Unlock() {
		mKMutex.unlock();
	}

	iterator Find(Key key) {
		typename SafeMap::iterator itr = mMap.find(key);
		return itr;
	}

	iterator Begin() {
		return mMap.begin();
	}

	iterator End() {
		return mMap.end();
	}

	bool Empty() {
		return mMap.empty();
	}

	KMutex mKMutex;
	SafeMap mMap;
};
#endif /* KSAFEMAP_H_ */
