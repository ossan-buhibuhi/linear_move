//
//  linear_move.h
//
//  Created by 松崎暁 on 2015/11/14.
//  Copyright (c) 2015年 松崎暁. All rights reserved.
//
//  Released under the MIT license
//  http://opensource.org/licenses/mit-license.php
//

#ifndef liner_move_h
#define liner_move_h

#include <iostream>
#include <list>
#include <vector>
#include <thread>
#include <memory>
#include <cassert>
#include <random>       // std::default_random_engine

template <typename T>
using uniq = std::unique_ptr<T>;

template <typename T>
using uniq_vector = uniq<std::vector<T>>;

template <class T>
class ref_ptr {
	const T * p;
public:
	ref_ptr ()
	: p (NULL)
		{}
	ref_ptr (const T & t)
	: p (&t)
		{}
	const T & operator * () const
		{ return *p; }
	const T & operator -> () const
		{ return *p; }
};

template <typename T>
std::ostream& operator<<(std::ostream & os, ref_ptr<uniq<T>> & r)
{
	os << *r;
	
	return os;
}

template <typename T>
std::ostream& operator<<(std::ostream & os, const std::vector <T> & vec)
{
	os << "(";
	size_t len = vec.size();
	if (len) {
		os << vec.at(0);
		for(int i=1; i<len; i++) {
			os << ' ';
			os << vec.at(i);
		}
	}
	os << ")";
	
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

template <class T>
void display_of (const std::vector<T> & vec, const std::string & tag)
{
	std::cout << tag << ": " << vec << std::endl;
}

template <class T>
std::vector<T> display (const std::string & tag, std::vector<T> && vec)
{
	std::cout << tag << ": " << vec << std::endl;
	return move (vec);
}

template <class T, class F>
std::vector<T> progress (int len, T && ini, F && f) {
	std::vector <T> vec (len);
	if (!len)
		return move (vec);
	vec.at (0) = std::move(ini);
	for (int i=1; i<len; i++)
		vec.at (i) = f(vec.at (i - 1));
	
	return move (vec);
}

template <class T, class I>
std::vector<uniq<T>> make_uniq_vector (std::vector<I> inits)
{
	size_t len = inits.size();
	std::vector <uniq<T>> vec (len);
	if (!len)
		return move (vec);
	for (size_t i=0; i<len; i++)
		vec [i] = uniq<T> (new T (inits[i]));
	
	return move (vec);
}

template <class T, class F>
T reduce (F && f, std::vector<T> && vec) {
	assert (vec.size());
	size_t len = vec.size();
	T acc = move (vec[0]);
	for (int i=1; i<len; i++)
		acc = f (std::move (acc), std::move (vec[i]));
	return move (acc);
}

template <class A, class T, class F>
A fold (A && acc, F && f, std::vector<T> && vec) {
	size_t len = vec.size();
	for (int i=0; i<len; i++)
		acc = f (std::move (acc), vec[i]);
	return std::move (acc);
}

template <class T>
std::vector<T> reverse (std::vector<T> && vec) {
	std::reverse (vec.begin(), vec.end());
	return move (vec);
}

template <class T, class F>
std::vector<T> sort (F && f, std::vector<T> && vec) {
	std::sort (vec.begin(), vec.end(), std::move (f));
	return move (vec);
}

template <class T>
std::vector<T> shuffle (std::vector<T> && vec) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle (vec.begin(), vec.end(), std::default_random_engine(seed));
	return move (vec);
}

template <class T>
std::vector<T> append (T && t, std::vector<T> && vec) {
	vec.push_back (std::move (t));
	return move (vec);
}

template <class T>
bool compare (const std::vector<uniq<T>> & vec1, const std::vector<uniq<T>> & vec2) {
	size_t len = vec1.size();
	if (len != vec2.size())
		return false;
	for (size_t i=0; i<len; i++)
		if (*vec1[i] != *vec2[i])
			return false;
	return true;
}

template <class T>
bool compare (const std::vector<std::vector<T>> & vec1, const std::vector<std::vector<T>> & vec2) {
	size_t len = vec1.size();
	if (len != vec2.size())
		return false;
	for (size_t i=0; i<len; i++)
		if (!compare (vec1[i], vec2[i]))
			return false;
	return true;
}

template <class T>
std::vector<T> combine (std::vector<T> && vec, std::vector<T> && vec2) {
	size_t pre_len = vec.size();
	size_t new_len = pre_len + vec2.size();
	vec.resize (new_len);
	for (size_t i=pre_len; i<new_len; i++)
		vec[i] = std::move (vec2 [i - pre_len]);
	return move (vec);
}

template <class T>
std::vector<T> take (size_t len, std::vector<T> && vec) {
	assert (len < vec.size());
	vec.resize (len);
	return move (vec);
}

template <class T>
std::vector<T> drop (size_t len, std::vector<T> && vec) {
	assert (len < vec.size());
	typename std::vector<T>::iterator to (vec.begin());
	typename std::vector<T>::iterator from (to + len);
	size_t cnt = vec.size() - len;
	for (int i=0; i<cnt; i++) {
		*to = std::move (*from);
		to++;
		from++;
	}
	vec.resize (vec.size() - len);

	return move (vec);
}

template <class T>
std::vector<T> join (std::vector<std::vector<T>> && vec_vec) {
	size_t vec_vec_len = vec_vec.size();
	size_t len = 0;
	for (int i=0; i<vec_vec_len; i++) {
		size_t vec_len = vec_vec[i].size ();
		len += vec_len;
	}
	std::vector<T> ret (len);
	size_t pos = 0;
	for (size_t i=0; i<vec_vec_len; i++) {
		std::vector <T> & vec = vec_vec [i];
		size_t vec_len = vec.size ();
		for (size_t j=0; j<vec_len; j++) {
			ret [pos + j] = std::move (vec [j]);
		}
		pos += vec_len;
	}
	return move (ret);
}

template <class T, class F>
auto map (F && f, std::vector<T> && vec)
-> std::vector<typename std::result_of<F(T)>::type>
{
	size_t len = vec.size();
	std::vector<typename std::result_of<F(T)>::type> ret (len);
	for (size_t i=0; i < len; i++)
		ret[i] = f (std::move (vec[i]));

	return move (ret);
}

template <class T, class F>
auto product_of (std::vector<T> & vec, F && f)
-> typename std::result_of<F(T&)>::type
{
	size_t len = vec.size();
	typename std::result_of<F(T&)>::type ret;
	for (size_t i=0; i < len; i++) {
		auto r = f (vec[i]);
//		std::copy (r.begin(), r.end(), std::back_inserter (ret));
		size_t r_len = r.size();
		for (size_t j=0; j < r_len; j++)
			ret.push_back (move (r[j]));
	}
	return move (ret);
}

template <class T, class F>
auto map_of (std::vector<T> & vec, F && f)
-> std::vector<typename std::result_of<F(T)>::type>
{
	size_t len = vec.size();
	std::vector<typename std::result_of<F(T)>::type> ret (len);
	for (size_t i=0; i < len; i++)
		ret[i] = f (vec[i]);

	return move (ret);
}

template <class T, class U, class F>
std::vector<T> foreach (U && acc, F && f, std::vector<T> && vec)
{
	size_t len = vec.size();
	for (size_t i=0; i < len; i++) {
		const T & t = vec [i];
		acc = f (std::move (acc), t);
	}

	return move (vec);
}

template <class T, class U, class F>
U fold_of (std::vector<T> & vec, U && acc, F && f)
{
	size_t len = vec.size();
	for (size_t i=0; i < len; i++) {
		const T & t = vec [i];
		acc = f (std::move (acc), t);
	}

	return std::move (acc);
}

template <class T, class F>
std::vector<T> filter (F && f, std::vector<T> && vec) {
	size_t len = vec.size();
	std::vector<T> vec2 (len);
	size_t cnt = 0;
	for (size_t i=0; i<len; i++) {
		T & e = vec [i];
		if (f (e))
			vec2 [cnt++] = std::move (e);
	}
	vec2.resize (cnt);

	return move (vec2);
}

template <class T, class F>
std::vector<std::vector<T>> assort (size_t cnt, F && f, std::vector <T> && vec) {
	size_t len = vec.size();
	std::vector<std::vector<T>> ret (cnt);
	size_t vec_lens [cnt];
	for (size_t j=0; j<cnt; j++) {
		std::vector <T> vec (len);
		ret [j] = move (vec);
		vec_lens [j] = 0;
	}
	for (size_t i=0; i<len; i++) {
		T & e = vec [i];
		size_t pos = f (e);
		assert (pos < cnt);
		ret[pos][vec_lens [pos]++] = std::move (e);
	}
	for (size_t j=0; j<cnt; j++)
		ret[j].resize (vec_lens [j]);

	return move (ret);
}

template <class T>
std::vector<std::vector<T>> group (int cnt, std::vector<T> && vec) {
	size_t len = vec.size();
	size_t rest_cnt = len % cnt;
	size_t group_cnt = len / cnt;
	std::vector<std::vector<T>> ret (group_cnt + (rest_cnt ? 1 : 0));
	size_t src_pos = 0;
	for (size_t i=0; i < group_cnt; i++) {
		ret.at(i) = std::vector<T> (cnt);
		for (int j=0; j < cnt; j++)
			ret[i][j] = std::move (vec.at (src_pos++));
	}
	if (rest_cnt) {
		ret[group_cnt] = std::vector<T> (rest_cnt);
		for (int j=0; j < rest_cnt; j++)
			ret[group_cnt][j] = std::move (vec.at (src_pos++));
	}
	return move (ret);
}

template <class T>
std::vector<T> list_to_vector (std::list<T> && l) {
	size_t len = l.size();
	std::vector<T> vec (len);
	typename std::list<T>::iterator it = l.begin();
	for (size_t i=0; i<len; i++) {
		vec[i] = std::move (*it);
		it++;
	}
	return move (vec);
}

template <class T>
std::vector<ref_ptr<T>> refdup (std::vector<T> & vec) {
	size_t len = vec.size();
	std::vector<ref_ptr<T>> ret (len);
	for (size_t i = 0; i < len; i++)
		ret [i] = ref_ptr<T> (vec [i]);
	return move (ret);
}

#endif
