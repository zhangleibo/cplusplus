#pragma once

//基类
class CIAdapter
{
public:
    virtual ~CIAdapter(){}
    virtual void Run() = 0;
};
//适配无参函数
template <class Func>
class CAdapter0 : public CIAdapter
{
public:
    CAdapter0(Func fun) : m_fun(fun)
    {

    }
    virtual ~CAdapter0(){}
    virtual void Run()
    {
        (*m_fun)();
    }
private:
    Func m_fun;
};
//适配一元函数
template <class Func, class Arg1>
class CAdapter1 : public CIAdapter
{
public:
    CAdapter1(Func fun, Arg1 arg1) : 
        m_fun(fun), 
        _arg1(arg1)
    {

    }
    virtual ~CAdapter1(){}
    virtual void Run()
    {
        (*m_fun)(_arg1);
    }
private:
    Func m_fun;
    Arg1 _arg1;
};
//适配二元函数
template <class Func, class Arg1, class Arg2>
class CAdapter2 : public CIAdapter
{
public:
    CAdapter2(Func fun, Arg1 arg1, Arg2 arg2) :
        m_fun(fun), 
        _arg1(arg1),
        _arg2(arg2)
    {

    }
    virtual ~CAdapter2(){}
    virtual void Run()
    {
        (*m_fun)(_arg1, _arg2);
    }
private:
    Func m_fun;
    Arg1 _arg1;
    Arg2 _arg2;
};
//适配三元函数
template <class Func, class Arg1, class Arg2, class Arg3>
class CAdapter3 : public CIAdapter
{
public:
    CAdapter3(Func fun, Arg1 arg1, Arg2 arg2, Arg3 arg3) :
        m_fun(fun),
        _arg1(arg1),
        _arg2(arg2),
        _arg3(arg3)
    {

    }
    virtual ~CAdapter3(){}
    virtual void Run()
    {
        (*m_fun)(_arg1, _arg2, _arg3);
    }
private:
    Func m_fun;
    Arg1 _arg1;
    Arg2 _arg2;
    Arg3 _arg3;
};
//适配类函数成员
template<class Obj, class Func>
class CAdapter4 : public CIAdapter
{
public:
    CAdapter4(Obj* pthis, Func fun) :
        _pthis(pthis),
        m_fun(fun)
    {

    }
    virtual ~CAdapter4(){}
    virtual void Run()
    {
        (_pthis->*m_fun)();
    }
private:
    Obj* _pthis;
    Func m_fun;
};
//适配类一元函数成员
template <class Obj, class Func, class Arg1>
class CAdapter5 : public CIAdapter
{
public:
    CAdapter5(Obj* pthis, Func fun, Arg1 arg1) :
        _pthis(pthis), 
        m_fun(fun), 
        _arg1(arg1)
    {
    }
    virtual ~CAdapter5(){}
    virtual void Run()
    {
        (_pthis->*m_fun)(_arg1);
    }
private:
    Obj* _pthis;
    Func m_fun;
    Arg1 _arg1;
};
//适配类二元函数成员
template <class Obj, class Func, class Arg1, class Arg2>
class CAdapter6 : public CIAdapter
{
public:
    CAdapter6(Obj* pthis, Func fun, Arg1 arg1, Arg2 arg2) : 
        _pthis(pthis),
        m_fun(fun),
        _arg1(arg1),
        _arg2(arg2)
    {

    }
    virtual ~CAdapter6(){}
    virtual void Run()
    {
        (_pthis->*m_fun)(_arg1, _arg2);
    }
private:
    Obj* _pthis;
    Func m_fun;
    Arg1 _arg1;
    Arg2 _arg2;
};
//适配类三元函数成员
template <class Obj, class Func, class Arg1, class Arg2, class Arg3>
class CAdapter7 : public CIAdapter
{
public:
    CAdapter7(Obj* pthis, Func fun, Arg1 arg1, Arg2 arg2, Arg3 arg3) :
        _pthis(pthis),
        m_fun(fun),
        _arg1(arg1),
        _arg2(arg2),
        _arg3(arg3)
    {

    }
    virtual ~CAdapter7(){}
    virtual void Run()
    {
        (_pthis->*m_fun)(_arg1, _arg2, _arg3);
    }
private:
    Obj* _pthis;
    Func m_fun;
    Arg1 _arg1;
    Arg2 _arg2;
    Arg3 _arg3;
};

template<class R>
CIAdapter* NewAdapterImp(R(*fun)())
{
    return new CAdapter0<R(*)()>(fun);
}

template<class R, class Arg1>
CIAdapter* NewAdapterImp(R(*fun)(Arg1), Arg1 arg1)
{
    return new CAdapter1<R(*)(Arg1), Arg1>(fun, arg1);
}

template<class R, class Arg1, class Arg2>
CIAdapter* NewAdapterImp(R(*fun)(Arg1, Arg2), Arg1 arg1, Arg2 arg2)
{
    return new CAdapter2<R(*)(Arg1, Arg2), Arg1, Arg2>(fun, arg1, arg2);
}

template<class R, class Arg1, class Arg2, class Arg3>
CIAdapter* NewAdapterImp(R(*fun)(Arg1, Arg2, Arg3), Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
    return new CAdapter3<R(*)(Arg1, Arg2, Arg3), Arg1, Arg2, Arg3>(fun, arg1, arg2, arg3);
}

template<class R, class Obj>
CIAdapter* NewAdapterImp(Obj* obj, R(Obj::*fun)())
{
    return new CAdapter4<Obj, R(Obj::*)()>(obj, fun);
}

template<class R, class Obj, class Arg1>
CIAdapter* NewAdapterImp(Obj* obj, R(Obj::*fun)(Arg1), Arg1 arg1)
{
    return new CAdapter5<Obj, R(Obj::*)(Arg1), Arg1>(obj, fun, arg1);
}

template<class R, class Obj, class Arg1, class Arg2>
CIAdapter* NewAdapterImp(Obj* obj, R(Obj::*fun)(Arg1, Arg2), Arg1 arg1, Arg2 arg2)
{
    return new CAdapter6<Obj, R(Obj::*)(Arg1, Arg2), Arg1, Arg2>(obj, fun, arg1, arg2);
}

template<class R, class Obj, class Arg1, class Arg2, class Arg3>
CIAdapter* NewAdapterImp(Obj* obj, R(Obj::*fun)(Arg1, Arg2, Arg3), Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
    return new CAdapter7<Obj, R(Obj::*)(Arg1, Arg2, Arg3), Arg1, Arg2, Arg3>(obj, fun, arg1, arg2, arg3);
}
