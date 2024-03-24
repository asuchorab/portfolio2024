//
// Created by rutio on 25.02.19.
//

#ifndef LUAMUSGEN_PARDEDUCE_H
#define LUAMUSGEN_PARDEDUCE_H

#include "ParDataConst.h"
#include "ParDataArray.h"

/**
 * Allows for redirection to a template-based polymorphism at runtime.
 * The ParDeduce::call(...) does the job.
 * @tparam Functor struct/class with N template parameters and with call(...)
 * method. The first N arguments of call(...) should be references and should
 * be derived from ParData. Within the call(...) method, Par::Value(...) can be
 * used to easily access parameter data, regardless of whether the parameter is
 * ParConst (access one value all the time) or not, in which case whole array
 * would be accessed. This is easily optimized by the compiler to access single
 * double value (ParDataConst) or sequential array data (ParDataView/Array).
 */
template <template <class ...> class Functor> struct ParDeduce {

  /**
   * Call the Functor::call(...) method
   * At runtime, if n-th parameter is ParDataConst, template with n-th argument
   * ParConst will be called, otherwise, template with n-th argument Par will
   * be called. ParData::Value(...) can be used to get parameter values. Using
   * this method will generate 2^N instantiations of Functor.
   * @tparam N Number of arguments to be processed. N = 0 will not compile but
   * why would you want it anyway.
   * @tparam RetType Return type of the called function.
   * @tparam ArgTypes Argument types of this function, will be deduced by the
   * compiler.
   * @param args Function arguments, first N should derived from Par.
   */
  template<size_t N, class RetType = void, class ... ArgTypes>
  static RetType call(ArgTypes&&... args) {
    return ParDeduce<Functor>::ParDeduce0<RetType, ArgTypes...>::template ParDeduce1<N>::call({args...}, args...);
  }

  /**
   * The same as call(...) but specialized for returning std::unique_ptr<ParData> (for ParGenerator).
   */
  template<size_t N, class ... ArgTypes>
  static std::unique_ptr<ParData> callParData(ArgTypes&&... args) {
    return ParDeduce<Functor>::ParDeduce0<std::unique_ptr<ParData>, ArgTypes...>::template ParDeduce1<N>::call({args...}, args...);
  }

  template <class Ret, class ... ArgTypes> struct ParDeduce0 {
    template <size_t N, class ... TTypes> struct ParDeduce1 {
      static Ret call(std::tuple<ArgTypes...> tuple, ArgTypes... args) {
        const ParData& p1 = std::get<N-1>(tuple);
        try {
          auto p = dynamic_cast<const ParDataConst&>(p1);
          return ParDeduce1<N-1, ParDataConst, TTypes...>::call(tuple, args...);
        } catch (const std::bad_cast&) {
          return ParDeduce1<N-1, ParData, TTypes...>::call(tuple, args...);
        }
      }
    };
    template <class ... TTypes>
    struct ParDeduce1<1, TTypes...> {
      static Ret call(std::tuple<ArgTypes...> tuple, ArgTypes... args) {
        const ParData& p1 = std::get<0>(tuple);
        try {
          auto p = dynamic_cast<const ParDataConst&>(p1);
          return Functor<ParDataConst, TTypes...>::call(args...);
        } catch (const std::bad_cast&) {
          return Functor<ParData, TTypes...>::call(args...);
        }
      }
    };
  };
};

#endif //LUAMUSGEN_PARDEDUCE_H
