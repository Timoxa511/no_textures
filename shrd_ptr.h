#include "TXlib.h"


template <typename T>
struct shrd_ptr
    {
    struct proxy
        {
        int pntrs_;
        T* p_;
        proxy (int pntrs, T* p);
       ~proxy ();
        proxy& operator = (const proxy& p) = delete;
        proxy (const proxy& p) = delete;                    //= delete синтаксис с++ 2011  = default тоже мона
        };

    mutable proxy* prx_;
    //-- -- -- -- -- -- -- -- -- -- -- --
    template <typename U>
    shrd_ptr (U* p);
    shrd_ptr (T* p);
   ~shrd_ptr ();
    template <typename U>
    shrd_ptr (const shrd_ptr<U>& p);
    shrd_ptr (const shrd_ptr<T>& p);
    shrd_ptr& operator = (const shrd_ptr<T>& p);
           T* operator -> ();
           T& operator *  ();
           T* operator &  ();
    };

//=============================================================================

//=============================================================================

//=============================================================================

//=============================================================================

template <typename T>
template <typename U>
shrd_ptr <T>::shrd_ptr (U* p) :
    prx_ (new proxy (1, p))
    {

    }


//-----------------------------------------------------------------------------
template <typename T>
shrd_ptr <T>::shrd_ptr (T* p) :
    prx_ (new proxy (1, p))
    {
    }


//-----------------------------------------------------------------------------
template <typename T>
template <typename U>
shrd_ptr <T>::shrd_ptr (const shrd_ptr<U>& p) :
    prx_ (reinterpret_cast<shrd_ptr<T>::proxy*>(p.prx_))
    {
    prx_->pntrs_ += 1;
    }


//-----------------------------------------------------------------------------
template <typename T>
shrd_ptr <T>::shrd_ptr (const shrd_ptr<T>& p) :
    prx_ (p.prx_)
    {

    prx_->pntrs_ += 1;
    }


//-----------------------------------------------------------------------------

template <typename T>
shrd_ptr<T>& shrd_ptr<T>::operator = (const shrd_ptr<T>& p)
    {

    prx_->pntrs_ -= 1;
    prx_ = p.prx_;
    p.prx_->pntrs_ += 1;
    }


//-----------------------------------------------------------------------------
template <typename T>
T* shrd_ptr<T>::operator -> ()
    {
    return prx_->p_;
    }

//-----------------------------------------------------------------------------
template <typename T>
T& shrd_ptr<T>::operator * ()
    {
    return *(prx_->p_);
    }
//-----------------------------------------------------------------------------
template <typename T>
T* shrd_ptr<T>::operator & ()
    {
    return (prx_->p_);
    }
//-----------------------------------------------------------------------------
template <typename T>
shrd_ptr<T>::~shrd_ptr ()
    {

    if (--prx_->pntrs_ == 0)
        delete (prx_)
        ;

    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template <typename T>
shrd_ptr<T>::proxy::proxy (int pntrs, T* p) :
    pntrs_ (pntrs),
    p_ (p)
    {
    printf ("\nпрокси %p рожден служить настоящему указателю\n\n", this);
    }

//-----------------------------------------------------------------------------

template <typename T>
shrd_ptr<T>::proxy::~proxy ()
    {
    printf ("this = %d mew type = %c\n",p_, p_->type_);
    assert (p_-> status_ == 0);
    delete (p_);
    }

//-----------------------------------------------------------------------------






