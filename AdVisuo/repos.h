#pragma once

#include <list>

#define TOKENPASTE2(x, y) x ## y
#define TOKENPASTE(x, y) TOKENPASTE2(x, y)
#define for_each_1(Z, TYPE, COLL, VAR)	\
	bool Z;\
	for (TYPE::iterator i = COLL.begin(); i != COLL.end(); i++)		\
		for (TYPE::value_type VAR = (Z = true , *i); Z; Z = false)
#define for_each(TYPE, COLL, VAR)	for_each_1(TOKENPASTE(x_var_unique_, __COUNTER__), TYPE, COLL, VAR)

template <class T>
class CRepos
{
public:

	enum STRATEGY { GET_ONE, FAIL, WAIT };

	CRepos(enum STRATEGY strategy = GET_ONE, int nQuantity = 1) : m_strategy(strategy), m_nQuantity(nQuantity) { }
//	virtual ~CRepos() = 0;

	void remove_all()
	{
		for_each(std::list<T*>, m_list, p)
			destroy(p);
		m_list.clear();
	}

	STRATEGY m_strategy;
	int m_nQuantity;

protected:
	std::list<T*> m_list;
public:

	T *get()					{ return get(m_strategy, m_nQuantity); }
	T *get(STRATEGY strategy)	{ return get(s, m_nQuantity); }
	T *get(int quantity)		{ return get(m_strategy, q); }
	T *get(STRATEGY strategy, int quantity);

	void release(T *p);

	void generate()				{ generate(m_nQuantity); }
	void generate(int quantity);

protected:
	virtual T *create() = 0;
	virtual void destroy(T*) = 0;
};

template <class T>
T *CRepos<T>::get(STRATEGY strategy, int quantity)
{
	if (m_list.size() >= 1)
	{
		T *p = m_list.front();
		m_list.pop_front();
		return p;
	}

	switch (strategy)
	{
	case GET_ONE:
		return create();
	case FAIL:
		return NULL;
	case WAIT:
		if (m_nQuantity > 1) generate(m_nQuantity - 1);
		return create();
	}
	return NULL;
}

template <class T>
void CRepos<T>::release(T *p)
{
	m_list.push_back(p);
}

template <class T>
void CRepos<T>::generate(int quantity)
{
	for (int i = 0; i < quantity; i++)
		release(create());
}
