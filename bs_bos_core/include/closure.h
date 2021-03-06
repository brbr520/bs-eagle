/**
 *       \file  closure.h
 *      \brief  Closure implementation, it is not a 'standart CS closure', 
 *              it is an object that stores pointer to member function and
 *              references to params with this member function should be 
 *              called. Name convention closureN, where N is a number of
 *              member function params
 *     \author  Sergey Miryanov (sergey-miryanov), sergey.miryanov@gmail.com
 *       \date  13.02.2009
 *  \copyright  This source code is released under the terms of 
 *              the BSD License. See LICENSE for more details.
 * */
#ifndef BS_CLOSURE_IMPL_H_
#define BS_CLOSURE_IMPL_H_

namespace blue_sky {

  /**
   * \class closure0
   * \brief Implements 'closure' with no params and return type R
   * */
  template <typename R, typename T, typename M>
  struct closure0
  {
    closure0 (M m)
    : m (m)
    {
    }

    R
    operator () (T &t)
    {
      return (t.*m) ();
    }

    M m;
  };
  /**
   * \class closure0
   * \brief Implements 'closure' with no params and no return value
   * */
  template <typename T, typename M>
  struct closure0 <void, T, M>
  {
    typedef void R;

    closure0 (M m)
    : m (m)
    {
    }

    R
    operator () (T &t)
    {
      (t.*m) ();
    }

    M m;
  };

  /**
   * \class closure1
   * \brief Implements 'closure' with 1 param and return type R
   * */
  template <typename R, typename T, typename M, typename A1>
  struct closure1
  {
    closure1 (M m, A1 a1)
    : m (m)
    , a1 (a1)
    {
    }

    R
    operator () (T &t)
    {
      return (t.*m) (a1);
    }

    M m;
    A1 a1;
  };
  /**
   * \class closure1
   * \brief Implements 'closure' with 1 param and no return value
   * */
  template <typename T, typename M, typename A1>
  struct closure1 <void, T, M, A1>
  {
    closure1 (M m, A1 a1)
    : m (m)
    , a1 (a1)
    {
    }

    void
    operator () (T &t)
    {
      return (t.*m) (a1);
    }

    M m;
    A1 a1;
  };

  /**
   * \class closure2
   * \brief Implements 'closure' with 2 param and return type R
   * */
  template <typename R, typename T, typename M, typename A1, typename A2>
  struct closure2
  {
    closure2 (M m, A1 a1, A2 a2)
    : m (m)
    , a1 (a1)
    , a2 (a2)
    {
    }

    R
    operator () (T &t)
    {
      return (t.*m) (a1, a2);
    }

    M m;
    A1 a1;
    A2 a2;
  };
  /**
   * \class closure2
   * \brief Implements 'closure' with 2 param and no return value
   * */
  template <typename T, typename M, typename A1, typename A2>
  struct closure2 <void, T, M, A1, A2>
  {
    closure2 (M m, A1 a1, A2 a2)
    : m (m)
    , a1 (a1)
    , a2 (a2)
    {
    }

    void
    operator () (T &t)
    {
      return (t.*m) (a1, a2);
    }

    M m;
    A1 a1;
    A2 a2;
  };

  /**
   * \class closure3
   * \brief Implements 'closure' with 3 param and return type R
   * */
  template <typename R, typename T, typename M, typename A1, typename A2, typename A3>
  struct closure3
  {
    closure3 (M m, A1 a1, A2 a2, A3 a3)
    : m (m)
    , a1 (a1)
    , a2 (a2)
    , a3 (a3)
    {
    }

    R
    operator () (T &t)
    {
      return (t.*m) (a1, a2, a3);
    }

    M m;
    A1 a1;
    A2 a2;
    A3 a3;
  };
  /**
   * \class closure3
   * \brief Implements 'closure' with 3 param and no return value
   * */
  template <typename T, typename M, typename A1, typename A2, typename A3>
  struct closure3 <void, T, M, A1, A2, A3>
  {
    closure3 (M m, A1 a1, A2 a2, A3 a3)
    : m (m)
    , a1 (a1)
    , a2 (a2)
    , a3 (a3)
    {
    }

    void
    operator () (T &t)
    {
      return (t.*m) (a1, a2, a3);
    }

    M m;
    A1 a1;
    A2 a2;
    A3 a3;
  };

  /**
   * \class closure4
   * \brief Implements 'closure' with 4 param and return type R
   * */
  template <typename R, typename T, typename M, typename A1, typename A2, typename A3, typename A4>
  struct closure4 
  {
    closure4 (M m, A1 a1, A2 a2, A3 a3, A4 a4)
    : m (m)
    , a1 (a1)
    , a2 (a2)
    , a3 (a3)
    , a4 (a4)
    {
    }

    R
    operator () (T &t)
    {
      return (t.*m) (a1, a2, a3, a4);
    }

    M m;
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
  };
  /**
   * \class closure4
   * \brief Implements 'closure' with 4 param and no return value
   * */
  template <typename T, typename M, typename A1, typename A2, typename A3, typename A4>
  struct closure4 <void, T, M, A1, A2, A3, A4>
  {
    closure4 (M m, A1 a1, A2 a2, A3 a3, A4 a4)
    : m (m)
    , a1 (a1)
    , a2 (a2)
    , a3 (a3)
    , a4 (a4)
    {
    }

    void
    operator () (T &t)
    {
      return (t.*m) (a1, a2, a3, a4);
    }

    M m;
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
  };
  /**
   * \class closure5
   * \brief Implements 'closure' with 5 param and return type R
   * */
  template <typename R, typename T, typename M, typename A1, typename A2, typename A3, typename A4, typename A5>
  struct closure5 
  {
    closure5 (M m, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
    : m (m)
    , a1 (a1)
    , a2 (a2)
    , a3 (a3)
    , a4 (a4)
    , a5 (a5)
    {
    }

    R
    operator () (T &t)
    {
      return (t.*m) (a1, a2, a3, a4, a5);
    }

    M m;
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;
  };
  /**
   * \class closure5
   * \brief Implements 'closure' with 5 param and no return value
   * */
  template <typename T, typename M, typename A1, typename A2, typename A3, typename A4, typename A5>
  struct closure5 <void, T, M, A1, A2, A3, A4, A5>
  {
    closure5 (M m, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
    : m (m)
    , a1 (a1)
    , a2 (a2)
    , a3 (a3)
    , a4 (a4)
    , a5 (a5)
    {
    }

    void
    operator () (T &t)
    {
      return (t.*m) (a1, a2, a3, a4, a5);
    }

    M m;
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;
  };

  /**
   * \class closure6
   * \brief Implements 'closure' with 6 param and return type R
   * */
  template <typename R, typename T, typename M, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  struct closure6
  {
    closure6 (M m, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
    : m (m), a1 (a1), a2 (a2), a3 (a3), a4 (a4), a5 (a5), a6 (a6)
    {
    }

    R
    operator () (T &t) 
    {
      return (t.*m) (a1, a2, a3, a4, a5, a6);
    }

  private:
    M  m;
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;
    A6 a6;
  };
  /**
   * \class closure6
   * \brief Implements 'closure' with 6 param and no return value
   * */
  template <typename T, typename M, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  struct closure6<void, T, M, A1, A2, A3, A4, A5, A6>
  {
    closure6 (M m, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
    : m (m), a1 (a1), a2 (a2), a3 (a3), a4 (a4), a5 (a5), a6 (a6)
    {
    }

    void
    operator () (T &t) 
    {
      (t.*m) (a1, a2, a3, a4, a5, a6);
    }

  private:
    M  m;
    A1 a1;
    A2 a2;
    A3 a3;
    A4 a4;
    A5 a5;
    A6 a6;
  };

  ///////////////////////////////////////////////////////////////////////////
  template <typename R, typename T>
  closure0 <R, T, R (T::*) () const>
  closure (R (T::*m) () const)
  {
    return closure0 <R, T, R (T::*) () const> (m);
  }
  template <typename R, typename T>
  closure0 <R, T, R (T::*) ()>
  closure (R (T::*m) ())
  {
    return closure0 <R, T, R (T::*) ()> (m);
  }

  ///////////////////////////////////////////////////////////////////////////
  template <typename R, typename T, typename A1>
  closure1 <R, T, R (T::*) (A1) const, A1>
  closure (R (T::*m) (A1) const, A1 a1)
  {
    return closure1 <R, T, R (T::*) (A1) const, A1> (m, a1);
  }
  template <typename R, typename T, typename A1>
  closure1 <R, T, R (T::*) (A1), A1>
  closure (R (T::*m) (A1), A1 a1)
  {
    return closure1 <R, T, R (T::*) (A1), A1> (m, a1);
  }

  ///////////////////////////////////////////////////////////////////////////
  template <typename R, typename T, typename A1, typename A2>
  closure2 <R, T, R (T::*) (A1, A2), A1, A2>
  closure (R (T::*m) (A1, A2), A1 a1, A2 a2)
  {
    return closure2 <R, T, R (T::*) (A1, A2), A1, A2> (m, a1, a2);
  }
  template <typename R, typename T, typename A1, typename A2>
  closure2 <R, T, R (T::*) (A1, A2) const, A1, A2>
  closure (R (T::*m) (A1, A2) const, A1 a1, A2 a2)
  {
    return closure2 <R, T, R (T::*) (A1, A2) const, A1, A2> (m, a1, a2);
  }

  ///////////////////////////////////////////////////////////////////////////
  template <typename R, typename T, typename A1, typename A2, typename A3>
  closure3 <R, T, R (T::*) (A1, A2, A3), A1, A2, A3>
  closure (R (T::*m) (A1, A2, A3), A1 a1, A2 a2, A3 a3)
  {
    return closure3 <R, T, R (T::*) (A1, A2, A3), A1, A2, A3> (m, a1, a2, a3);
  }

  ///////////////////////////////////////////////////////////////////////////
  template <typename R, typename T, typename A1, typename A2, typename A3, typename A4>
  closure4 <R, T, R (T::*) (A1, A2, A3, A4), A1, A2, A3, A4>
  closure (R (T::*m) (A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4)
  {
    return closure4 <R, T, R (T::*) (A1, A2, A3, A4), A1, A2, A3, A4> (m, a1, a2, a3, a4);
  }
  template <typename R, typename T, typename A1, typename A2, typename A3, typename A4>
  closure4 <R, T, R (T::*) (A1, A2, A3, A4) const, A1, A2, A3, A4>
  closure (R (T::*m) (A1, A2, A3, A4) const, A1 a1, A2 a2, A3 a3, A4 a4)
  {
    return closure4 <R, T, R (T::*) (A1, A2, A3, A4) const, A1, A2, A3, A4> (m, a1, a2, a3, a4);
  }

  ///////////////////////////////////////////////////////////////////////////
  template <typename R, typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
  closure5 <R, T, R (T::*) (A1, A2, A3, A4, A5) const, A1, A2, A3, A4, A5>
  closure (R (T::*m) (A1, A2, A3, A4, A5) const, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    return closure5 <R, T, R (T::*) (A1, A2, A3, A4, A5) const, A1, A2, A3, A4, A5> (m, a1, a2, a3, a4, a5);
  }
  template <typename R, typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
  closure5 <R, T, R (T::*) (A1, A2, A3, A4, A5), A1, A2, A3, A4, A5>
  closure (R (T::*m) (A1, A2, A3, A4, A5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    return closure5 <R, T, R (T::*) (A1, A2, A3, A4, A5), A1, A2, A3, A4, A5> (m, a1, a2, a3, a4, a5);
  }

  ///////////////////////////////////////////////////////////////////////////
  template <typename R, typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  closure6 <R, T, R (T::*) (A1, A2, A3, A4, A5, A6) const, A1, A2, A3, A4, A5, A6>
  closure (R (T::*m) (A1, A2, A3, A4, A5, A6) const, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    return closure6 <R, T, R (T::*) (A1, A2, A3, A4, A5, A6) const, A1, A2, A3, A4, A5, A6> (m, a1, a2, a3, a4, a5, a6);
  }
  template <typename R, typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  closure6 <R, T, R (T::*) (A1, A2, A3, A4, A5, A6), A1, A2, A3, A4, A5, A6>
  closure (R (T::*m) (A1, A2, A3, A4, A5, A6), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    return closure6 <R, T, R (T::*) (A1, A2, A3, A4, A5, A6), A1, A2, A3, A4, A5, A6> (m, a1, a2, a3, a4, a5, a6);
  }

} // namespace blue_sky


#endif // #ifndef BS_CLOSURE_IMPL_H_

