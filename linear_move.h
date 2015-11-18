//
//  linear_move.h
//
//  Copyright (c) 2015年 松崎暁. All rights reserved.
//

#ifndef fix_test_liner_move_h
#define fix_test_liner_move_h

#include <iostream>
#include <list>
#include <vector>
#include <thread>
#include <memory>
#include <cassert>
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock
#include <future>

#include <time.h>

template <typename T>
using uniq = std::unique_ptr<T>;

template <typename T>
using uniq_vector = uniq<std::vector<T>>;

template <class T>
class ref {
	const T * p;
public:
	ref ()
	: p (NULL)
		{}
	ref (const T & t)
	: p (&t)
		{}
	const T & operator * () const
		{ return *p; }
	const T & operator -> () const
		{ return *p; }
};
/*
template <class T>
class ref {
	const std::vector <T> * vec;
	size_t pos;
public:
	ref ()
	: vec (NULL), pos (0) {}
	
	ref (const uniq_vector <T> & vec, size_t pos)
	: vec (vec.get()), pos (pos) {}

	const T & operator * () const
		{ return vec->at (pos); }
	const T & operator -> () const
		{ return vec->at (pos); }
};
*/
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

template <typename T>
std::ostream& operator<<(std::ostream & os, ref<uniq<T>> & r)
{
	os << *r;
	
	return os;
}

template <typename T>
std::ostream& operator<<(std::ostream & os, const uniq_vector <T> & vec)
{
	os << "(";
	size_t len = vec->size();
	if (len) {
		os << vec->at(0);
		for(int i=1; i<len; i++) {
			os << ' ';
			os << vec->at(i);
		}
	}
	os << ")";
	
	return os;
}

template <class T, class F>
uniq <std::vector<T>> progress (int len, T && ini, F && f) {
	uniq <std::vector <T>> vec (new std::vector<T> (len));
	if (!len)
		return move (vec);
	vec->at (0) = std::move(ini);
	for (int i=1; i<len; i++)
		vec->at (i) = f(vec->at (i - 1));
	
	return move (vec);
}

template <class T, class F>
T reduce (F && f, uniq <std::vector<T>> && vec) {
	assert (vec->size());
	size_t len = vec->size();
	T acc = move (vec->at (0));
	for (int i=1; i<len; i++)
		acc = f (acc, vec->at (i));
	return move (acc);
}

template <class T, class F>
T reduce_of (uniq <std::vector<T>> & vec, F && f) {
	return reduce (std::move (f), move (vec));
}

template <class A, class T, class F>
A fold (A && acc, F && f, uniq <std::vector<T>> && vec) {
	size_t len = vec->size();
	for (int i=0; i<len; i++)
		acc = f (acc, vec->at (i));
	return move (acc);
}

template <class A, class T, class F>
A fold_of (uniq <std::vector<T>> & vec, A && acc, F && f) {
	return fold (std::move (acc), std::move (f), move (vec));
}

template <class T>
uniq <std::vector<T>> reverse (uniq <std::vector<T>> && vec) {
	std::reverse (vec->begin(), vec->end());
	return move (vec);
}

template <class T>
uniq <std::vector<T>> reverse_of (uniq <std::vector<T>> & vec) {
	return reverse (move (vec));
}

template <class T>
uniq <std::vector<T>> shuffle (uniq_vector<T> && vec) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle (vec->begin(), vec->end(), std::default_random_engine(seed));
	return move (vec);
}

template <class T>
uniq <std::vector<T>> shuffle_of (uniq_vector<T> & vec) {
	return shuffle (move(vec));
}

template <class T>
uniq <std::vector<T>> append (T && t, uniq <std::vector<T>> && vec) {
	vec->push_back (std::move (t));
	return move (vec);
}

template <class T>
uniq <std::vector<T>> append_of (uniq <std::vector<T>> & vec, T & t) {
	return append (move (t), move (vec));
}

template <class T>
uniq <std::vector<T>> combine (uniq_vector<T> && vec, uniq_vector<T> && vec2) {
	size_t pre_len = vec->size();
	size_t new_len = pre_len + vec2->size();
	vec->resize (new_len);
	for (size_t i=pre_len; i<new_len; i++)
		vec->at (i) = std::move (vec2->at (i - pre_len));
	return move (vec);
}

template <class T>
uniq <std::vector<T>> combine_of (uniq_vector<T> & vec2, uniq_vector<T> & vec) {
	return combine (move (vec2), move (vec));
}


template <class T>
uniq <std::vector<T>> left (size_t len, uniq <std::vector<T>> && vec) {
	vec->resize (len);
	return move (vec);
/*
	uniq <std::vector<T>> ret (new std::vector<T>(len));
	for (size_t i=0; i<len; i++) {
		ret->at(i) = move (vec->at (i));
	}
	return move (ret);
*/
}

template <class T>
uniq <std::vector<T>> left_of (uniq <std::vector<T>> & vec, size_t len) {
	return left (len, move (vec));
}

template <class T>
uniq <std::vector<T>> right (size_t len, uniq <std::vector<T>> && vec) {
	typename std::vector<T>::iterator to (vec->begin());
	typename std::vector<T>::iterator from (to + len);
	size_t cnt = vec->size() - len;
	for (int i=0; i<cnt; i++) {
		*to = std::move (*from);
		to++;
		from++;
	}
	vec->resize (vec->size() - len);

	return move (vec);
}

template <class T>
uniq <std::vector<T>> right_of (uniq <std::vector<T>> & vec, size_t len) {
	return right (len, move (vec));
}

template <class T>
uniq_vector<T> join (uniq_vector<uniq_vector<T>> && vec_vec) {
//timer t ("join()");
	size_t vec_vec_len = vec_vec->size();
	size_t len = 0;
	for (int i=0; i<vec_vec_len; i++) {
		size_t vec_len = vec_vec->at (i)->size ();
		len += vec_len;
	}
	uniq_vector<T> ret (new std::vector<T>(len));
	size_t pos = 0;
	for (size_t i=0; i<vec_vec_len; i++) {
		uniq_vector <T> & vec = vec_vec->at (i);
		size_t vec_len = vec->size ();
		for (size_t j=0; j<vec_len; j++) {
			ret->at (pos + j) = move (vec->at (j));
		}
		pos += vec_len;
	}
	return move (ret);
}

template <class T>
uniq_vector<T> join_of (uniq_vector<uniq_vector<T>> & vec_vec) {
	return join (move (vec_vec));
}

template <class U, class T, class F>
uniq <std::vector<U>> map (F && f, uniq <std::vector<T>> && vec) {
	size_t len = vec->size();
	uniq <std::vector<U>> ret (new std::vector<U>(len));
	for (size_t i=0; i<len; i++)
		ret->at(i) = move (f (vec->at (i)));

	return move (ret);
}

template <class U, class T, class F>
uniq <std::vector<U>> map_of (uniq <std::vector<T>> & vec, F && f) {
	return map (std::move (f), move (vec));
}

template <class T, class F>
uniq <std::vector<T>> filter (F && f, uniq <std::vector<T>> && vec) {
//timer t ("filter()");
	size_t len = vec->size();
	uniq_vector<T> vec2 (new std::vector <T> (len));
	size_t cnt = 0;
	for (size_t i=0; i<len; i++) {
		T & e = vec->at (i);
		if (f (e))
			vec2->at(cnt++) = move (e);
	}
	vec2->resize (cnt);

	return move (vec2);
}

template <class T, class F>
uniq <std::vector<T>> filter_of (uniq <std::vector<T>> & vec, F && f) {
	return filter (std::move(f), move (vec));
}

template <class T, class F>
uniq <std::vector<T>> filter_of (uniq <std::vector<T>> & vec, int th_cnt, F && f) {
	return filter (th_cnt, std::move(f), move (vec));
}

template <class T, class F>
uniq_vector<uniq_vector<T>> assort (size_t cnt, F && f, uniq_vector <T> && vec) {
	size_t len = vec->size();
	uniq_vector<uniq_vector<T>> ret (new std::vector <uniq_vector<T>> (cnt));
	size_t vec_lens [cnt];
	for (size_t j=0; j<cnt; j++) {
		uniq_vector <T> vec (new std::vector <T> (len));
		ret->at (j) = move (vec);
		vec_lens [j] = 0;
	}
	for (size_t i=0; i<len; i++) {
		T & e = vec->at (i);
		size_t pos = f (e);
		assert (pos < cnt);
		ret->at (pos)->at (vec_lens [pos]++) = std::move (e);
	}
	for (size_t j=0; j<cnt; j++)
		ret->at (j)->resize (vec_lens [j]);
/*
	size_t len = vec->size();
	std::vector<std::list<T>> ls (cnt);
	for (size_t i=0; i<len; i++) {
		T & e = vec->at (i);
		size_t pos = f (e);
		assert (pos < cnt);
		ls [pos].push_back (std::move (e));
	}

	uniq_vector<uniq_vector<T>> ret (new std::vector <uniq_vector<T>> (cnt));
	for (size_t j=0; j<cnt; j++) {
		typename std::list<T>::iterator it (ls[j].begin());
		size_t len = ls[j].size();
		uniq_vector <T> vec (new std::vector <T> (len));
		for (size_t i=0; i<len; i++) {
			vec->at (i) = std::move (*it);
			it++;
		}
		ret->at (j) = move (vec);
	}
*/
	return move (ret);
}

template <class T, class F>
uniq_vector<uniq_vector<T>> assort_of (uniq_vector <T> & vec, size_t cnt, F && f) {
	return assort (cnt, std::move (f), move (vec));
}

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

template <class U, class T, class F>
uniq_vector<U> map (int div, F && f, uniq <std::vector<T>> && vec) {
	size_t len = vec->size();
	if (div < 2 || len < div * 2)
		return map <U> (std::move (f), std::move (vec));
	return
		join (
		para_map <uniq_vector <U>> (
			[f] (uniq_vector <T> & div_vec) {
std::cout << "lambda(" << div_vec->size() << ")" << std::endl;
timer t ("lambda()");
				return map <U> (std::move(f), std::move(div_vec));
			},
		group ((int)len / div, move (vec))
		));
}

template <class U, class T, class F>
uniq_vector<U> map_of (uniq <std::vector<T>> & vec, int div, F && f) {
	return map (div, std::move (f), move (vec));
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

template <class T>
uniq_vector<uniq_vector<T>> group (int cnt, uniq_vector<T> && vec) {
//timer t ("group()");
	size_t len = vec->size();
	size_t rest_cnt = len % cnt;
	size_t group_cnt = len / cnt;
	uniq_vector<uniq_vector<T>> ret (new std::vector <uniq_vector<T>> (group_cnt + (rest_cnt ? 1 : 0)));
//	typename std::vector<T>::iterator it = vec->begin();
	size_t src_pos = 0;
	for (size_t i=0; i < group_cnt; i++) {
//		typename std::vector<T>::iterator it_to = it + cnt;
//		std::copy(it, it_to, std::back_inserter(*vec));
//		it = it_to;
		ret->at(i).reset (new std::vector<T> (cnt));
		for (int j=0; j < cnt; j++)
			ret->at(i)->at(j) = std::move (vec->at (src_pos++)); // i * cnt + j));
	}
	if (rest_cnt) {
		ret->at(group_cnt).reset (new std::vector<T> (rest_cnt));
		for (int j=0; j < rest_cnt; j++)
			ret->at(group_cnt)->at(j) = std::move (vec->at (src_pos++)); // group_cnt * cnt + j));
	}
	return move (ret);
}

template <class T>
uniq_vector<uniq_vector<T>> group_of (uniq_vector<T> & vec, int cnt) {
	return group (cnt, move (vec));
}

template <class T>
uniq_vector<T> list_to_vector (uniq <std::list<T>> && l) {
//	return uniq_vector <T> (new std::vector<T> (l->begin(), l->end()));
	size_t len = l->size();
	uniq_vector<T> vec (new std::vector<T>(len));
	typename std::list<T>::iterator it = l->begin();
	for (size_t i=0; i<len; i++) {
		vec->at(i) = std::move (*it);
		it++;
	}
	return move(vec);
}
/*
template <class T>
uniq_vector<ref<T>> refdup (uniq_vector<T> & vec) {
	size_t len = vec->size();
	uniq_vector<ref<T>> ret (new std::vector <ref<T>> (len));
	for (size_t i=0; i < len; i++)
		ret->at(i) = ref<T> (vec, i);
	return move (ret);
}
*/

template <class T>
uniq_vector<ref<T>> refdup (uniq_vector<T> & vec) {
	size_t len = vec->size();
	uniq_vector<ref<T>> ret (new std::vector <ref<T>> (len));
	for (size_t i=0; i < len; i++)
		ret->at(i) = ref<T> (vec->at (i));
	return move (ret);
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
uniq_vector<uniq_vector<T>> assort_of (uniq_vector <T> & vec, int th_cnt, size_t cnt, F && f) {
	return assort (th_cnt, cnt, std::move (f), move (vec));
}

#endif
