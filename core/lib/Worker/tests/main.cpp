//
// Created by ivan on 28.04.2026.
//
// #include "WorkerTests.cpp"
#include <gtest/gtest.h>
/// + TODO: исправить ошибку с запуском через clion
/*
 * Просто настроил конфигурацию, чтобы при нажати на кнопку запускался файл из папки файлы типо: build/core/lib/.../tests/...Test
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}