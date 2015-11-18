//
//  main.cppf
//
//  Copyright (c) 2015年 松崎暁. All rights reserved.
//

#include <iostream>
#include <vector>
#include <list>
#include <memory>
#include <thread>
#include <future>

#include "linear_move.h"

#include <chrono>

class Fuga {
	int num;
public:
	static int copy_cnt;
	static int life_cnt;
	Fuga (int n)
	: num (n)
		{ life_cnt++; }
	Fuga (const Fuga & fuga)
		{ life_cnt++; copy_cnt++; }
	~Fuga ()
		{ life_cnt--; }
	int get_num () const
		{ return num; }
	void set_num (int n)
		{ num = n; }
};
int Fuga::copy_cnt = 0;
int Fuga::life_cnt = 0;

class Hoge {
	std::unique_ptr <Fuga> fuga;
public:
	Hoge ()
		{ life_cnt++; }
	Hoge (Hoge & hoge)
	: fuga (move(hoge.fuga))
		{ life_cnt++; copy_cnt++; }
	Hoge (int n)
	: fuga (new Fuga (n))
		{ life_cnt++; }
	~Hoge ()
		{ life_cnt--; }
	
	const uniq <Fuga> & get_fuga ()
		{ return fuga; }
	int get_num ()
		{ return fuga->get_num(); }
	void set_num (int n)
		{ return fuga->set_num(n); }
	static int life_cnt;
	static int copy_cnt;
};

int Hoge::life_cnt = 0;
int Hoge::copy_cnt = 0;

std::ostream& operator<<(std::ostream& os, const uniq <Hoge> & hoge)
{
	os << hoge->get_fuga()->get_num();
	
	return os;
}

void test_0 () {
	auto vec (
		group (2,
		list_to_vector (
		fold (uniq <std::list<uniq<Hoge>>> (new std::list<uniq<Hoge>>()),
			[] (uniq <std::list<uniq<Hoge>>> & acc, uniq<Hoge> & x) {
				acc->insert (acc->begin(), (move(x))); return move(acc);
			},
		reverse (
		progress (10, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				int num = hoge->get_fuga()->get_num();
				return uniq <Hoge> (new Hoge (num * 2));
			}
		))))));
	std::cout << vec << std::endl;
}

void test_1 () {
	uniq_vector <uniq <Hoge>> hoges (
		progress (10, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				int num = hoge->get_fuga()->get_num();
				return uniq <Hoge> (new Hoge (num * 2));
			}));
	std::cout << hoges << std::endl;
	auto refs (
		left (2,
		group (3,
		left (8,
		reverse (
		refdup (hoges)
		)))));
	
	std::cout << refs << std::endl;
	std::cout << refs->at(0)->at(0)->get_num() << std::endl;
	std::cout << hoges << std::endl;
}

void test_2 () {
	auto hoges (
		join (
		para_map <uniq_vector<uniq<Hoge>>> (
			[] (uniq_vector <uniq <Hoge>> & hoges) {
				return map <uniq<Hoge>> (
					[] (uniq <Hoge> & hoge) {
						int num = hoge->get_fuga()->get_num();
						return uniq <Hoge> (new Hoge (num * 2));
					},
					move (hoges));
			},
		group (5000,
		progress (10000, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				int num = hoge->get_fuga()->get_num();
				return uniq <Hoge> (new Hoge (num + 1));
			}
		)))));
	std::cout << hoges << std::endl;
}

void test_3 () {
	auto hoges (
		map <uniq<Hoge>> (2,
			[] (uniq <Hoge> & hoge) {
				int num = hoge->get_fuga()->get_num();
				return uniq <Hoge> (new Hoge (num * 2));
			},
		progress (10000, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				int num = hoge->get_fuga()->get_num();
				return uniq <Hoge> (new Hoge (num + 1));
			}
		)));
	std::cout << hoges << std::endl;
}

uniq_vector <uniq <Hoge>> foo () {
	return
		map <uniq<Hoge>> (2,
			[] (uniq <Hoge> & hoge) {
				int num = hoge->get_fuga()->get_num();
				return uniq <Hoge> (new Hoge (num * 2));
			},
		progress (10000, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				int num = hoge->get_fuga()->get_num();
				return uniq <Hoge> (new Hoge (num + 1));
			}
		));
}

void test_4 () {
	auto hoges (promise <uniq_vector <uniq <Hoge>>> ([]() { return foo (); }));
	std::cout << hoges->get() << std::endl;
}

void test_5 (int th_cnt=1, int x=100) {
	timer t ("test_5");
	auto hoges (
		filter (th_cnt,
			[] (uniq <Hoge> & hoge) {
				return hoge->get_fuga()->get_num() % 3;
			},
		progress (x, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				int num = hoge->get_fuga()->get_num();
				return uniq <Hoge> (new Hoge (num + 1));
			}
		)));
//	std::cout << hoges << std::endl;
}

void test_6 () {
	auto hoges (
		assort(3,
			[] (uniq <Hoge> & hoge) {
				int num (hoge->get_fuga()->get_num());
				return
					!(num % 2) ?
						0:
					!(num % 3) ?
						1:
					2;
			},
		progress (25, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				int num (hoge->get_fuga()->get_num());
				return uniq <Hoge> (new Hoge (num + 1));
			}
		)));
	std::cout << hoges << std::endl;
}
/*
template <class T>
uniq_vector<T> quick_sort (uniq_vector<T> && vec) {
	struct {
		int th_cnt;
		uniq_vector<T> operator () (uniq_vector <T> && vec) {
			return !vec->size() ?
				move (vec):
				[this, &vec] () {
					T head (std::move (vec->at(0)));
					auto sel (
						select (2,
							[&head] (T & elem) {
								return elem > head ? 1 : 0;
							},
							right (1, move (vec))
							));
					return combine (
						append (std::move (head), (*this) (move (sel->at(1)))),
						(*this) (std::move (sel->at(0)))
						);
				} ();
		}
	} self = {2};
	return reverse (self (move (vec)));
}
*/

template <class T, class F>
uniq_vector<T> quick_sort (int th_cnt, F && f, uniq_vector<T> && vec) {
	struct {
		F & f;
		int th_cnt;
		uniq_vector<T> operator () (uniq_vector <T> & vec) {
			return !vec->size() ?
				move (vec):
				[this, &vec] () {
					T tail = std::move (vec->at(vec->size() - 1));
					auto ass (
						assort (this->th_cnt, 2,
							[this, &tail] (T & elem) {
								return this->f (elem, tail) ? 1 : 0;
							},
							left_of (vec, vec->size() - 1)
							));
					return combine (
						append (std::move (tail), (*this) (ass->at(1))),
						(*this) (ass->at(0))
						);
				} ();
		}
	} self = {f, th_cnt};
	return self (vec);
}

uniq_vector <uniq<Hoge>> Eratosthenes(int x){
	timer t ("Eratosthenes()");
	uniq_vector <uniq<Hoge>> arr (new std::vector <uniq<Hoge>> (x));
	for(int i = 0; i < x; i++){
		arr->at(i) = uniq<Hoge>(new Hoge(i));
	}
	int sqrt_x = sqrt(x);
	for(int i = 2; i < sqrt_x; i++){
		if(arr->at(i)->get_num()){
			for(int j = 0; i * (j + 2) < x; j++){
				arr->at(i *(j + 2))->set_num (0);
			}
		}
	}
	return filter_of (arr,
		[] (uniq<Hoge> & hoge) {
			return hoge->get_num() ? true : false;
		});
}

uniq_vector<uniq<Hoge>> eratosthenes (int x, int th_cnt)
{
	timer t ("eratosthenes()");
	auto s_vec = progress (x - 1, uniq <Hoge> (new Hoge (x)),
		[] (uniq<Hoge> & pre) {
			return uniq <Hoge> (new Hoge (pre->get_num() - 1));
		});
	auto p_vec = uniq_vector<uniq<Hoge>> (new std::vector<uniq<Hoge>>);
	struct {
		int end;
		int th_cnt;
		uniq_vector<uniq<Hoge>> operator () (uniq_vector <uniq<Hoge>> & s_vec, uniq_vector <uniq<Hoge>> & p_vec) {
			auto & head = s_vec->at(s_vec->size() - 1);
			return head->get_num() >= end ?
				reverse (combine (move (s_vec), reverse_of (p_vec))):
				[this, &head, &s_vec, &p_vec] () {
					int head_num = head->get_num();
					auto new_pvec = append_of (p_vec, head);
					auto new_svec =
						filter  (this->th_cnt,
							[head_num] (uniq<Hoge> & hoge) {
								return hoge->get_num() % head_num;
							},
						left_of (s_vec, s_vec->size() -1)
						);
					return (*this) (new_svec, new_pvec);
				} ();
		}
	} loop = {(int)sqrt(x), th_cnt};
	return loop (s_vec, p_vec);
}

void test_7 ()
{
	auto combined = combine (
		uniq_vector <int> (new std::vector<int>{8, 4, 3, 9, 5}),
		uniq_vector <int> (new std::vector<int>{7, 1, 6, 2}));
	std::cout << combined << std::endl;
	auto righted = right (4, move (combined));
	std::cout << righted << std::endl;
	auto appended = append (1, move (righted));
	std::cout << appended << std::endl;
			auto sel (
				assort (2,
					[] (int & elem) {
						return elem > 5 ? 1 : 0;
					},
					uniq_vector <int> (new std::vector<int>{8, 4, 3, 9, 5, 7, 1, 6, 2})
					));
	std::cout << sel << std::endl;

	auto sorted =
		quick_sort (1,
		[] (int a, int b) {
			return a < b;
		},
		uniq_vector <int> (new std::vector<int>{8, 4, 3, 9, 5, 7, 1, 6, 2})
		);
	std::cout << sorted << std::endl;
}

void test_8()
{
	timer t ("test_8()");
	auto sorted =
		left (10,
		quick_sort (1,
			[] (uniq<Hoge> & small, uniq<Hoge> & large) {
				return small->get_num() < large->get_num();
			},
		shuffle (
		progress (100000, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				return uniq <Hoge> (new Hoge (hoge->get_num() + 1));
			}
		))));
	std::cout << sorted << std::endl;
}

int test_9(int th_cnt, int x)
{
	auto start = std::chrono::system_clock::now();
	auto primes = eratosthenes (x, th_cnt);
	auto end = std::chrono::system_clock::now();
//	std::cout << primes << std::endl;
    auto diff = end - start;
	return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
}

void test_10 ()
{
	int x = 1000;
	test_5(1, x);
	test_5(2, x);
}

#define TH_CNT 3
void test_11 () {
	timer t ("test_11");
	uniq_vector<uniq<Hoge>> sources [TH_CNT];
	uniq_vector<uniq<Hoge>> results [TH_CNT];
	std::vector<std::thread> threads;
	for (int i = 0; i < TH_CNT; i++) {
		sources [i] =
			progress (100000, uniq <Hoge> (new Hoge(1)),
				[] (uniq<Hoge> & hoge) {
					return uniq <Hoge> (new Hoge (hoge->get_num() + 1));
				});
	}
	for (int i = 0; i < TH_CNT; i++) {
//		threads.push_back(std::thread([i, &sources, &results] {
			results[i] =
				/*
				quick_sort (1,
					[] (uniq<Hoge> & small, uniq<Hoge> & large) {
						return small->get_num() < large->get_num();
					},
				*/
				join (
				reverse (
				group (3,
				shuffle_of (sources[i])
				)));
//		}));
	}
	for (std::thread &th : threads) {
		th.join();
	}
	for (int i=0; i<TH_CNT; i++) {
		auto r = left_of (results [i], 10);
		std::cout << r << std::endl;
	}
}

void test_12 () {
	auto src =
		group    (100000,
		progress (1000000, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				return uniq <Hoge> (new Hoge (hoge->get_num() + 1));
			}
		));
	timer t ("test12");
	auto r =
		map <uniq_vector<uniq<Hoge>>> (2,
			[] (uniq_vector<uniq<Hoge>> & hoges) {
				return
//					left (10,
					quick_sort (1,
						[] (uniq<Hoge> & small, uniq<Hoge> & large) {
							return small->get_num() < large->get_num();
						},
					shuffle_of (hoges)
					);
			},
		move (src)
		);
//	std::cout << r << std::endl;
}

void test_13 () {
	auto r =
		reduce (
			[] (uniq<Hoge> & x, uniq<Hoge> & y) {
				return uniq <Hoge> (new Hoge (x->get_num() + y->get_num()));
			},
		progress(10, uniq<Hoge> (new Hoge (0)),
			[] (uniq<Hoge> & hoge) {
				return uniq <Hoge> (new Hoge (hoge->get_num() + 1));
			}
		));
	std::cout << r << std::endl;
	
	auto p =
		progress (10, uniq<Hoge> (new Hoge (0)),
			[] (uniq<Hoge> & hoge) {
				return uniq <Hoge> (new Hoge (hoge->get_num() + 1));
			});
	auto r2 =
		reduce_of (p,
			[] (uniq<Hoge> & x, uniq<Hoge> & y) {
				return uniq <Hoge> (new Hoge (x->get_num() + y->get_num()));
			}
		);
	std::cout << r2 << std::endl;
}

void test_14 ()
{
	eratosthenes(100000, 1);
	Eratosthenes(100000);
//	std::cout << eratosthenes(100000, 1) << std::endl;
//	std::cout << Eratosthenes(100000) << std::endl;
}

void test_15 ()
{
	timer t ("test_15");
	auto hoge =
		fold (uniq<Hoge> (new Hoge (1)),
			[] (uniq<Hoge> & x, uniq<Fuga> & y) {
				int xn = x->get_num();
				int yn = y->get_num();
				x->set_num (xn + yn);
				return move (x);
			},
		filter (2,
			[] (uniq<Fuga> & fuga) {
				int num = fuga->get_num(); 
				return
					!(num % 2) ?
						false:
					!(num % 3) ?
						false:
					true;
			},
		map <uniq<Fuga>> (2,
			[] (uniq<Hoge> & hoge) {
				return uniq<Fuga> (new Fuga (hoge->get_num() * 5));
			},
		progress (1000, uniq <Hoge> (new Hoge(1)),
			[] (uniq<Hoge> & hoge) {
				return uniq <Hoge> (new Hoge (hoge->get_num() + 1));
			}
		))));
	std::cout << hoge->get_num() << std::endl;
}

int main(int argc, const char * argv[])
{
//	test_0();
//	test_1();
//	test_2();
//	test_3();
//	test_4();
//	test_5();
//	test_6();
//	test_7();
//	test_8();
//	int x = 100000;
//	test_9(1, x);
//	test_9(2, x);
//	test_9(3, x);
//	test_10();
//	test_11();
//	test_12();
//	test_13();
//	test_14();
	test_15();
	printf ("Fuga::copy_cnt = %d\n", Fuga::copy_cnt);
	printf ("Fuga::life_cnt = %d\n", Fuga::life_cnt);
	printf ("Hoge::copy_cnt = %d\n", Hoge::copy_cnt);
	printf ("Hoge::life_cnt = %d\n", Hoge::life_cnt);
	
	return 0;
}
