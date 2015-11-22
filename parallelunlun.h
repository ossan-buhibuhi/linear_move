//
//  parallelunlun.h
//
//  Created by 松崎暁 on 2015/11/21.
//  Copyright (c) 2015年 松崎暁. All rights reserved.
//
//  Released under the MIT license
//  http://opensource.org/licenses/mit-license.php
//
//  注意：俺は並列処理をあきらめた。必ず遅くなるしquick_sortは並列処理すると結果がおかしくなることに気づいたからだ。
//

#ifndef parallelunlun_h
#define parallelunlun_h

#include <future>
#include <chrono>       // std::chrono::system_clock
#include <time.h>

#include "ThreadPool.h"

class timer {
	clock_t beg;
	std::string tag;
public:
	timer (const char * tag)
	: beg (clock()), tag (tag) {}
	~timer () {
		clock_t end = clock();

		std::cout << tag << ": " << end - beg << std::endl;
	}
};

template <typename U, typename T, typename F>
void future_func (std::promise<U> p, F & f, T & a)
{
	try {
		U ret = f(a);
		p.set_value(std::move (ret));  // (2a) promiseに戻り値を設定
	} catch (...) {
		p.set_exception(std::current_exception());  // (2b) promiseに例外を設定
	}
}

template <class U, class T, class F>
uniq <std::vector<U>> para_map (F && f, uniq <std::vector<T>> && vec) {
// timer t ("para_map()");
	size_t len = vec->size();
	std::vector <std::thread> threads (len);
	std::vector <std::future<U>> futures (len);

	uniq <std::vector<U>> ret (new std::vector<U>(len));
	for (size_t i=0; i<len; i++) {
// timer t ("para_map()0");
		std::promise<U> p;
		futures[i] = p.get_future();
		threads[i] = std::thread (future_func<U, T, F>, std::move(p), std::ref(f), std::ref(vec->at (i)));  // (1) 別スレッドで関数funcを実行
	}

	for (size_t i=0; i<len; i++) {
// timer t ("para_map()1");
		threads[i].join();
	}

	for (size_t i=0; i<len; i++) {
// timer t ("para_map()2");
		ret->at(i) = futures[i].get();  // (3a) promiseに設定された値を取得（別スレッドでの処理完了を待機
	}

	return move (ret);
}

template <class U, class T, class F>
uniq <std::vector<U>> para_map (ThreadPool & th_pool, F && f, uniq <std::vector<T>> && vec) {
	size_t len = vec->size();
	std::vector <std::thread> threads (len);
	std::vector <std::future<U>> futures (len);

	uniq <std::vector<U>> ret (new std::vector<U>(len));
	for (size_t i=0; i<len; i++)
		futures [i] = th_pool.enqueue([&f, &vec, i](){ return f(vec->at (i)); });

	for (size_t i=0; i<len; i++)
		ret->at(i) = futures[i].get();  // (3a) promiseに設定された値を取得（別スレッドでの処理完了を待機

	return move (ret);
}

template <class U, class T, class F>
uniq <std::vector<U>> para_map_of (uniq <std::vector<T>> & vec, F && f) {
	return para_map <U> (std::move (f), move (vec));
}

template <class T>
class future {
	std::future <T> f;
	std::thread t;
public:
	future (const future & fu)
	: f (move(fu.f)), t (move(fu.t)) {}
	future (std::future <T> && f, std::thread && t)
	: f (move(f)), t (move(t)) {}
	~future ()
		{ t.join (); }
	T get ()
		{ return f.get (); }
};

template <typename T>
using uniq_future = uniq <future <T>>;

template <typename T, typename F>
void future_func2 (std::promise<T> p, F & f)
{
	try {
		T ret = f();
		p.set_value(std::move (ret));  // (2a) promiseに戻り値を設定
	} catch (...) {
		p.set_exception(std::current_exception());  // (2b) promiseに例外を設定
	}
}

template <class T, class F>
uniq <future <T>> promise (F && f)
{
	std::promise<T> p;
	uniq <future <T>> fu (new future <T> (p.get_future(), std::thread (future_func2 <T, F>, std::move(p), std::ref(f))));
	return move (fu);
}

template <class T, class F>
uniq <future <T>> promise (F && f, uniq<ThreadPool> & pool)
{
	std::promise<T> p;
	uniq <future <T>> fu (new future <T> (p.get_future(), std::thread (future_func2 <T, F>, std::move(p), std::ref(f))));
	return move (fu);
}


template <class U, class T, class F>
uniq_vector<U> map (ThreadPool & th_pool, int div, F && f, uniq <std::vector<T>> && vec) {
	size_t len = vec->size();
	return
		join (
		para_map <uniq_vector <U>> (th_pool,
			[f] (uniq_vector <T> & div_vec) {
				return map <U> (std::move(f), std::move(div_vec));
			},
		group (len / div, move (vec))
		));
}

template <class U, class T, class F>
uniq_vector<U> map (int div, F && f, uniq <std::vector<T>> && vec) {
	size_t len = vec->size();
	if (div < 2 || len < div * 2)
		return map <U> (std::move (f), std::move (vec));
	return
		join (
		para_map <uniq_vector <U>> (
			[f] (uniq_vector <T> & div_vec) {
				return map <U> (std::move(f), std::move(div_vec));
			},
		group ((int)len / div, move (vec))
		));
}

template <class T, class F>
uniq <std::vector<T>> filter (ThreadPool & pool, int div, F && f, uniq <std::vector<T>> && vec) {
	size_t len = vec->size();
	return
		join (
		para_map <uniq_vector <T>> (pool,
			[&f] (uniq_vector <T> & div_vec) {
				return filter <T> (std::move(f), std::move(div_vec));
			},
		group ((int)len / div, move (vec))
		));
}

template <class T, class F>
uniq <std::vector<T>> filter (int div, F && f, uniq <std::vector<T>> && vec) {
//timer t ("filter(div)");
	size_t len = vec->size();
	if (div < 2 || len < div * 2)
		return filter (std::move (f), std::move (vec));
	auto grouped = group_of (vec, (int)len / div);
//timer t1 ("filter(div):1");
	auto mapped = para_map_of <uniq_vector <T>> (grouped,
		[&f] (uniq_vector <T> & div_vec) {
			return filter <T> (std::move(f), std::move(div_vec));
		});
//timer t2 ("filter(div):2");
	return join_of (mapped);

	/*
	return
		join (
		para_map <uniq_vector <T>> (
			[&f] (uniq_vector <T> & div_vec) {
				return filter <T> (std::move(f), std::move(div_vec));
			},
		group ((int)len / div, move (vec))
		));
	*/
}

template <class T, class F>
uniq_vector<uniq_vector<T>> assort (int th_cnt, size_t cnt, F && f, uniq_vector <T> && vec) {
	size_t len = vec->size();
	if (th_cnt < 2 || len < th_cnt * 2)
		return assort <T> (cnt, std::move(f), move (vec));
		
	auto rets =
		para_map <uniq_vector <uniq_vector <T>>> (
			[&cnt, &f] (uniq_vector <T> & div_vec) {
				return assort <T> (cnt, std::move(f), std::move(div_vec));
			},
		group ((int)len / th_cnt, move (vec))
		);

	uniq_vector<uniq_vector<T>> ret (new std::vector <uniq_vector<T>> (cnt));
	int lens [cnt];
	memset (lens, 0, sizeof (lens));
	for (int i=0; i<th_cnt; i++)
		for (size_t j=0; j<cnt; j++)
			lens [j] += rets->at(i)->at(j)->size();

	for (size_t j=0; j<cnt; j++) {
		uniq_vector <T> vec (new std::vector <T> (lens[j]));
		int pos = 0;
		for (int i=0; i<th_cnt; i++) {
			typename std::vector<T>::iterator it (rets->at(i)->at(j)->begin());
			size_t len = rets->at(i)->at(j)->size();
			for (size_t i=0; i<len; i++) {
				vec->at (pos + i) = std::move (*it);
				it++;
			}
			pos += len;
		}
		ret->at (j) = move (vec);
	}
	return move (ret);
}

template <class T, class F>
uniq_vector<uniq_vector<T>> assort (int th_cnt, ThreadPool & th_pool, size_t cnt, F && f, uniq_vector <T> && vec) {
	size_t len = vec->size();
	if (th_cnt < 2 || len < th_cnt * 2)
		return assort <T> (cnt, std::move(f), move (vec));
		
	auto rets =
		para_map <uniq_vector <uniq_vector <T>>> (th_pool,
			[&cnt, &f] (uniq_vector <T> & div_vec) {
				return assort <T> (cnt, std::move(f), std::move(div_vec));
			},
		group ((int)len / th_cnt, move (vec))
		);

	uniq_vector<uniq_vector<T>> ret (new std::vector <uniq_vector<T>> (cnt));
	int lens [cnt];
	memset (lens, 0, sizeof (lens));
	for (int i=0; i<th_cnt; i++)
		for (size_t j=0; j<cnt; j++)
			lens [j] += rets->at(i)->at(j)->size();

	for (size_t j=0; j<cnt; j++) {
		uniq_vector <T> vec (new std::vector <T> (lens[j]));
		int pos = 0;
		for (int i=0; i<th_cnt; i++) {
			typename std::vector<T>::iterator it (rets->at(i)->at(j)->begin());
			size_t len = rets->at(i)->at(j)->size();
			for (size_t i=0; i<len; i++) {
				vec->at (pos + i) = std::move (*it);
				it++;
			}
			pos += len;
		}
		ret->at (j) = move (vec);
	}
	return move (ret);
}


#endif
