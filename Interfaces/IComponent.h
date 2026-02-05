#pragma once
#include <memory>
#include <unordered_map>

class IComponent {
public:
	virtual void AddComponent(std::shared_ptr<IComponent>) = 0;
//	template <typename T> std::shared_ptr<T> GetComponent() = 0;
private:
	std::unordered_map<const char*, IComponent> components;
};