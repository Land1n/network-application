#ifndef HANDLERINVOKER_H
#define HANDLERINVOKER_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <set>
#include <type_traits>
#include <stdexcept>

// template <typename Key, typename Handler, typename Ret_T = void, typename Hash = std::hash<Key>>
template <typename Key, typename Handler, typename DefaultHandler, typename Ret_T, typename Hash>
class HandlerInvokerCommon {
public:
	HandlerInvokerCommon() noexcept {};
	HandlerInvokerCommon(const HandlerInvokerCommon& cp) noexcept :
	    handlerTable(cp.handlerTable), defaultHandler([&cp]() -> std::unique_ptr<Handler> {
		    if(cp.defaultHandler) {
			    return std::make_unique<Handler>(*cp.defaultHandler);
		    }
		    else
			    return nullptr;
	    }())
	{}
	HandlerInvokerCommon& operator=(const HandlerInvokerCommon& other) noexcept
	{
		if(this != &other) {
			if(other.defaultHandler)
				registerDefaultHandler(*other.defaultHandler);
			handlerTable = other.handlerTable;
		}
		return *this;
	}
	void registerHandler(Key k, Handler h) noexcept
	{
		handlerTable.insert({k, std::move(h)});
	}
	void registerDefaultHandler(DefaultHandler h) noexcept
	{
		defaultHandler = std::make_unique<DefaultHandler>(std::move(h));
	}

	template <typename... Args>
	Ret_T invokeHandler(Key k, Args&&... args) const
	{
		auto it = handlerTable.find(k);
		if(it != handlerTable.end()) {
			return invokeHandlerImpl(it->second, k, std::forward<Args>(args)...);
		}

		if(defaultHandler) {
			return invokeHandlerImpl(*defaultHandler, k, std::forward<Args>(args)...);
		}

		return createRet<Ret_T>();
	}

	std::set<Key> getRegisteredKeys() const noexcept
	{
		std::set<Key> keys;
		for(const auto& h: handlerTable) {
			keys.insert(h.first);
		}
		return keys;
	}

private:
	template <typename T>
	std::enable_if_t<std::is_default_constructible_v<T>, T> createRet() const
	{
		return Ret_T();
	}

	template <typename T>
	std::enable_if_t<!std::is_default_constructible_v<T> && !std::is_void_v<T>, T> createRet() const
	{
		throw std::runtime_error("Return type is not default constructible, make sure to provide a default handler");
	}

	template <typename T>
	std::enable_if_t<std::is_void_v<T>, void> createRet() const
	{
		return;
	}

	template <typename Func, typename... Args>
	auto invokeHandlerImpl(Func& foo, Key& k, Args&&... args) const
	{
		if constexpr(std::is_invocable_v<Func, Key, Args&&...>) {
			return foo(k, std::forward<Args>(args)...);
		}
		else if constexpr(std::is_invocable_v<Func, Key> && !std::is_invocable_v<Func, Args&&...>) {
			return foo(k);
		}
		else if constexpr(std::is_invocable_v<Func>) {
			return foo();
		}
		else {
			return foo(std::forward<Args>(args)...);
		}
	}

protected:
	std::unordered_map<Key, Handler, Hash> handlerTable;
	std::unique_ptr<DefaultHandler> defaultHandler;
};
template <typename Key, typename Handler, typename Ret_T = void, typename Hash = std::hash<Key>>
using HandlerInvoker = HandlerInvokerCommon<Key, Handler, Handler, Ret_T, Hash>;

template <typename Key, typename Handler, typename DefaultHandler, typename Ret_T = void,
          typename Hash = std::hash<Key>>
using HandlerInvokerDefaultDiff = HandlerInvokerCommon<Key, Handler, DefaultHandler, Ret_T, Hash>;

#endif // BASEHANDLER_H
