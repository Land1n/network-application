#include <gtest/gtest.h>
int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
/// + TODO: не все тесты запускаются через этот main
/// + TODO: переименовать в cmake на handlertests