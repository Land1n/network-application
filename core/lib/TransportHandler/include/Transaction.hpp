//
// Created by ivan on 28.03.2026.
//

#pragma once

enum class Transaction {
    Request = 0,
    Response = 1,
    Tests = 2,
    Error
};

Transaction setTypeTransaction(int integer);