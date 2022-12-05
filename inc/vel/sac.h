#pragma once

#include <thread>
#include <sstream>

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>


template<typename T>
struct sac_data
{
	size_t													blockSize;
	size_t													blockIndex;
	std::vector<std::vector<T>>								slots;
	std::vector<T*>											activeSlots;
	std::vector<T*>											freeSlots;

	std::unordered_map<std::string, T*>						trackerMap;
	std::unordered_map<T*, std::string>						ptrackerMap;
	std::unordered_map<T*, size_t>							activeSlotTrackerMap;
};


template<typename T>
class sac
{
private:
	std::shared_ptr<sac_data<T>>		data;


public:
	sac(size_t blockSize);
	sac();
	T*									insert(std::string name, T dataObject);
	void								erase(std::string name);
	void								erase(T* slotPtr);
	T*									get(std::string name);
	std::vector<T*>&					getAll();
	size_t								size() const;
	bool								exists(std::string name) const;

};



template <typename T>
sac<T>::sac() :
	data(std::make_shared<sac_data<T>>())
{
	size_t blockSize = 1000;

	this->data->blockSize = blockSize;
	this->data->blockIndex = 0;
	this->data->slots.push_back(std::vector<T>());
	this->data->slots.at(0).reserve(blockSize);
	this->data->activeSlots.reserve(blockSize);
	this->data->freeSlots.reserve(blockSize);
}

template <typename T>
sac<T>::sac(size_t blockSize) :
	data(std::make_shared<sac_data<T>>())
{
	this->data->blockSize = blockSize;
	this->data->blockIndex = 0;
	this->data->slots.push_back(std::vector<T>());
	this->data->slots.at(0).reserve(blockSize);
	this->data->activeSlots.reserve(blockSize);
	this->data->freeSlots.reserve(blockSize);
}

template <typename T>
T* sac<T>::insert(std::string name, T dataObject)
{
	// if this key already exists within the sac, bypass insertion and return pointer to existing item.
	// This could be a cause of confusion
	if (this->data->trackerMap.count(name) == 1)
	{
		//std::cout << "sac::insert(): the name of the data object which you are trying to load into the sac already exists: " 
		//	<< name << " bypassing insert" << std::endl;
		std::cout << "sac::insert(): the name of the data object which you are trying to load into the sac already exists: " << name << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
		//return this->data->trackerMap[name];
	}

	T* slotPointer;
	if (this->data->freeSlots.size() > 0)
	{
		slotPointer = this->data->freeSlots.back();
		this->data->freeSlots.pop_back();
		*slotPointer = dataObject;
	}
	else
	{
		if (this->data->slots.at(this->data->blockIndex).size() == this->data->blockSize)
		{
			this->data->slots.push_back(std::vector<T>());
			this->data->slots.back().reserve(this->data->blockSize);
			this->data->blockIndex++;
		}

		//std::cout << "sac001" << std::endl;

		this->data->slots.at(this->data->blockIndex).push_back(dataObject);

		//std::cout << "sac001b" << std::endl;

		slotPointer = &this->data->slots.at(this->data->blockIndex).back();

		//std::cout << "sac002" << std::endl;
	}

	// TODO activeSlots reallocating on every entry if blockSize has been reached,
	// need to reserve additional block size when this happens like we do for slots.back()

	this->data->activeSlots.push_back(slotPointer);
	this->data->activeSlotTrackerMap[slotPointer] = this->data->activeSlots.size() - 1;

	this->data->trackerMap[name] = slotPointer;
	this->data->ptrackerMap[slotPointer] = name;

	return slotPointer;
}

template <typename T>
void sac<T>::erase(std::string name)
{
	if (!this->data->trackerMap.count(name) == 1)
	{
		std::cout << "sac::erase(): attempting to erase element from sac which does not exist: " << name << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	T* pointerToRemove = this->data->trackerMap[name];
	// remove from activeSlots
	if (this->data->activeSlots.size() > 1)
	{
		// get index of pointerToRemove
		size_t indexOfPointerToRemove = this->data->activeSlotTrackerMap[pointerToRemove];

		// if indexOfPointerToRemove is the last element
		if (indexOfPointerToRemove == this->data->activeSlots.size() - 1)
		{
			this->data->activeSlots.pop_back();
		}
		else
		{
			// swap the pointer to remove with the last element in activeSlots
			std::iter_swap(this->data->activeSlots.begin() + indexOfPointerToRemove, this->data->activeSlots.end() - 1);

			// remove pointerToRemove from activeSlots (it is now the last element in the vector)
			this->data->activeSlots.pop_back();

			// update the index of the element we swapped within activeSlotTrackerMap
			this->data->activeSlotTrackerMap[this->data->activeSlots.at(indexOfPointerToRemove)] = indexOfPointerToRemove;
		}
	}
	else
	{
		this->data->activeSlots.clear();
	}

	// push to freeSlots
	this->data->freeSlots.push_back(pointerToRemove);

	// remove from trackers
	this->data->ptrackerMap.erase(pointerToRemove);
	this->data->trackerMap.erase(name);
}

template <typename T>
void sac<T>::erase(T* slotPtr)
{
	if (!this->data->ptrackerMap.count(slotPtr) == 1)
	{
		std::cout << "sac::erase(): attempting to erase element from sac which does not exist: " << slotPtr << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	this->erase(this->data->ptrackerMap[slotPtr]);
}

template <typename T>
T* sac<T>::get(std::string name)
{
	if (!this->data->trackerMap.count(name) == 1)
	{
		std::cout << "sac::get(): attempting to get element from sac which does not exist: " << name << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	return this->data->trackerMap[name];
}

template <typename T>
std::vector<T*>& sac<T>::getAll()
{
	return this->data->activeSlots;
}

template <typename T>
size_t sac<T>::size() const
{
	return this->data->activeSlots.size();
}

template <typename T>
bool sac<T>::exists(std::string name) const
{
	return this->data->trackerMap.count(name) == 1;
}