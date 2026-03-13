//
// Created by ivan on 13.03.2026.
//
#include "Message.hpp"

void Message::setTransactionType() {
    // Provide a sensible default: mark as Request.
    // (The derived classes override this to decide based on their state.)
    transactionType = TransactionType::Request;
}