#pragma once

#include <sstream>
#include <iostream>

#include "plf_colony/plf_colony.h"
#include "robin_hood/robin_hood.h"

template<typename T>
class sac
{
private:
	plf::colony<T>												slots;
	robin_hood::unordered_node_map<std::string, T*>				tracker;
	robin_hood::unordered_node_map<std::string, std::string>	ptracker;


public:
										sac();
	T*									insert(std::string name, T dataObject);
	void								erase(std::string name);
	void								erase(T* slotPtr);
	T*									get(std::string name);
	plf::colony<T>&						getAll();
	size_t								size() const;
	bool								exists(std::string name) const;

};

template <typename T>
sac<T>::sac(){}

template <typename T>
T* sac<T>::insert(std::string name, T dataObject)
{
	if (this->tracker.contains(name))
	{
		std::cout << "sac::insert(): the name of the data object which you are trying to load into the sac already exists: " << name << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	auto it = this->slots.insert(dataObject);

	auto ptr = &(*it);

	this->tracker[name] = ptr;

	std::ostringstream address;
	address << (void const *)ptr;
	std::string ptrAsString = address.str();

	this->ptracker[ptrAsString] = name;

	return ptr;
}

template <typename T>
void sac<T>::erase(std::string name)
{
	if (!this->tracker.contains(name))
	{
		std::cout << "sac::erase(): attempting to erase element from sac which does not exist: " << name << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	this->slots.erase(this->slots.get_iterator(this->tracker[name]));
	this->tracker.erase(name);
}

template <typename T>
void sac<T>::erase(T* slotPtr)
{
	std::ostringstream address;
	address << (void const *)slotPtr;
	std::string ptrAsString = address.str();

	if (!this->ptracker.contains(ptrAsString))
	{
		std::cout << "sac::erase(): attempting to erase element from sac which does not exist: " << ptrAsString << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	this->tracker.erase(this->ptracker[ptrAsString]);
	this->ptracker.erase(ptrAsString);
	this->slots.erase(this->slots.get_iterator(slotPtr));
}

template <typename T>
T* sac<T>::get(std::string name)
{
	if (!this->tracker.contains(name))
	{
		std::cout << "sac::get(): attempting to get element from sac which does not exist: " << name << std::endl;
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	return this->tracker[name];
}

template <typename T>
plf::colony<T>& sac<T>::getAll()
{
	return this->slots;
}

template <typename T>
size_t sac<T>::size() const
{
	return this->slots.size();
}

template <typename T>
bool sac<T>::exists(std::string name) const
{
	return this->tracker.contains(name);
}