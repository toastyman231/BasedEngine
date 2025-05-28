#pragma once

#include <queue>

namespace based::core 
{
    // Taken from https://stackoverflow.com/a/12886393 by Richard
    template <class T, class S, class C>
    S& Container(std::priority_queue<T, S, C>& q)
	{
        struct HackedQueue : private std::priority_queue<T, S, C>
    	{
            static S& Container(std::priority_queue<T, S, C>& q)
        	{
                return q.* & HackedQueue::c;
            }
        };
        return HackedQueue::Container(q);
    }
}