//
// Created by ivan on 28.03.2026.
//
#include "message/transaction.h"

Transaction setTypeTransaction(int integer)
{
	switch(integer) {
	case 0:
		return Transaction::Request;
	case 1:
		return Transaction::Response;
	case 2:
		return Transaction::Tests;
	default:
		return Transaction::Error;
	}
}