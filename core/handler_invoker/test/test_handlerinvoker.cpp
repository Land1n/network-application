//
// Created by guestuser on 26.06.2026.
//

#include <functional>
#include <gtest/gtest.h>
#include "handler_invoker/handler_invoker.h"

enum class Keys { One, Three, Two };

class HandlerInvokerTest : public ::testing::Test {
public:
};
/**

 * стандартное применение HandlerInvoker`a, тип обработчика и обработчика по умолчанию совпадают
 */

TEST(HandlerInvokerTest, SomeTest)
{
	using Handler = std::function<void()>;

	HandlerInvoker<Keys, Handler> handlerInvoker;

	bool def = false;
	bool one = false;
	bool two = false;

	handlerInvoker.registerDefaultHandler([&def]() {
		def = true;
	});
	handlerInvoker.registerHandler(Keys::One, [&one]() {
		one = true;
	});
	handlerInvoker.registerHandler(Keys::Two, [&two]() {
		two = true;
	});

	handlerInvoker.invokeHandler(Keys::One);
	handlerInvoker.invokeHandler(Keys::Three);
	handlerInvoker.invokeHandler(Keys::Two);

	ASSERT_TRUE(def);
	ASSERT_TRUE(one);
	ASSERT_TRUE(two);
}