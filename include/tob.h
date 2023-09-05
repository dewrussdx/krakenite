#ifndef _TOB_H_
#define _TOB_H_

#include "types.h"

// TopOfBook
struct TopOfBook
{
	TopOfBook()
		: price(0), qty(0)
	{
	}
	void flush()
	{
		price = 0;
		qty = 0;
	}
	Price price;
	Qty qty;
};

#endif