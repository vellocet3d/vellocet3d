#pragma once



#include <iostream>
#include <unordered_map>
#include <vector>




/*
	Class meant to be used to store pointers in a contiguous block of memory and be
	accessible via given name or pointer. Pointers of slot values are not kept valid
	as memory will simply be swapped and popped. If this is a requirement use sac.h
*/
template<typename T>
class ptrsac
{
private:
	size_t													originalBlockSize;
	size_t													blockSize;
	std::vector<T>											slots;

	std::unordered_map<std::string, size_t>					trackerMap;
	std::unordered_map<T, std::string>						ptrackerMap;


public:
	ptrsac(size_t blockSize);
	ptrsac();
	//~sac();
	//sac(const sac<T>& old);
	void								insert(std::string name, T dataObject);
	void								erase(std::string name);
	void								erase(T slotPtr);
	T									get(std::string name);
	std::vector<T>&						getAll();
	size_t								size() const;
	bool								exists(std::string name) const;

};

//template <typename T>
//sac<T>::sac(const sac<T>& old) :
//	blockSize(old.blockSize),
//	blockIndex(old.blockIndex),
//	slots(old.slots),
//	activeSlots(old.activeSlots),
//	freeSlots(old.freeSlots),
//	trackerMap(old.trackerMap),
//	ptrackerMap(old.ptrackerMap),
//	activeSlotTrackerMap(old.activeSlotTrackerMap)
//{
//	
//}

template <typename T>
ptrsac<T>::ptrsac()
{
	size_t blockSize = 1000;

	this->originalBlockSize = blockSize;
	this->blockSize = blockSize;
	this->slots.reserve(blockSize);
}

template <typename T>
ptrsac<T>::ptrsac(size_t blockSize)
{
	this->originalBlockSize = blockSize;
	this->blockSize = blockSize;
	this->slots.reserve(blockSize);
}

//template <typename T>
//sac<T>::~sac()
//{
//	std::stringstream ss;
//	ss << std::this_thread::get_id();
//	uint64_t id = std::stoull(ss.str());
//
//	std::cout << id << ":Destructing:" << this << std::endl;
//}

template <typename T>
void ptrsac<T>::insert(std::string name, T dataObject)
{
	// if this key already exists within the sac, display message and die
	if (this->trackerMap.count(name) == 1)
	{
		//std::cout << "sac::insert(): the name of the data object which you are trying to load into the sac already exists: " 
		//	<< name << " bypassing insert" << std::endl;
		std::cout << "sac::insert(): the name of the data object which you are trying to load into the sac already exists: " << name << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
		//return this->trackerMap[name];
	}


	if (this->slots.size() == this->blockSize)
	{
		this->blockSize += this->originalBlockSize;
		this->slots.reserve(this->blockSize);
	}

	this->slots.push_back(dataObject);

	this->trackerMap[name] = this->slots.size() - 1;
	this->ptrackerMap[dataObject] = name;
}

template <typename T>
void ptrsac<T>::erase(std::string name)
{
	if (!this->trackerMap.count(name) == 1)
	{
		std::cout << "sac::erase(): attempting to erase element from sac which does not exist: " << name << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	T pointerToRemove = this->slots.at(this->trackerMap[name]);

	// remove from activeSlots
	if (this->slots.size() > 1)
	{
		// get index of pointerToRemove
		size_t indexOfPointerToRemove = this->trackerMap[name];

		// if indexOfPointerToRemove is the last element
		if (indexOfPointerToRemove == this->slots.size() - 1)
		{
			this->slots.pop_back();
		}
		else
		{
			// swap the pointer to remove with the last element in activeSlots
			std::iter_swap(this->slots.begin() + indexOfPointerToRemove, this->slots.end() - 1);

			// remove pointerToRemove from activeSlots (it is now the last element in the vector)
			this->slots.pop_back();

			// update the index of the element we swapped within activeSlotTrackerMap
			this->trackerMap[this->ptrackerMap[this->slots.at(indexOfPointerToRemove)]] = indexOfPointerToRemove;
		}
	}
	else
	{
		this->slots.clear();
	}

	// remove from trackers
	this->ptrackerMap.erase(pointerToRemove);
	this->trackerMap.erase(name);
}

template <typename T>
void ptrsac<T>::erase(T slotPtr)
{
	if (!this->ptrackerMap.count(slotPtr) == 1)
	{
		std::cout << "sac::erase(): attempting to erase element from sac which does not exist: " << slotPtr << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	this->erase(this->ptrackerMap[slotPtr]);
}

template <typename T>
T ptrsac<T>::get(std::string name)
{
	if (!this->trackerMap.count(name) == 1)
	{
		std::cout << "sac::get(): attempting to get element from sac which does not exist: " << name << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	return this->slots.at(this->trackerMap[name]);
}

template <typename T>
std::vector<T>& ptrsac<T>::getAll()
{
	return this->slots;
}

template <typename T>
size_t ptrsac<T>::size() const
{
	return this->slots.size();
}

template <typename T>
bool ptrsac<T>::exists(std::string name) const
{
	return this->trackerMap.count(name) == 1;
}