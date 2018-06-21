#ifndef dplyr_hybrid_hybrid_h
#define dplyr_hybrid_hybrid_h

#include <dplyr/hybrid/Dispatch.h>
#include <dplyr/hybrid/HybridVectorScalarResult.h>
#include <dplyr/hybrid/Expression.h>

#include <dplyr/hybrid/scalar_result/n.h>
#include <dplyr/hybrid/scalar_result/sum_mean_sd_var.h>
#include <dplyr/hybrid/scalar_result/n_distinct.h>

namespace dplyr{
namespace hybrid{

template <typename SlicedTibble, typename LazySubsets, typename Operation>
SEXP hybrid_do(SEXP expr, const SlicedTibble& data, const LazySubsets& subsets, SEXP env, const Operation& op){
  if (TYPEOF(expr) != LANGSXP) return R_UnboundValue;

  static SEXP s_n = Rf_install("n");
  static SEXP s_sum = Rf_install("sum");
  static SEXP s_mean = Rf_install("mean");
  static SEXP s_var = Rf_install("var");
  static SEXP s_sd = Rf_install("sd");
  static SEXP s_n_distinct = Rf_install("n_distinct");

  static SEXP s_narm = Rf_install("na.rm");

  static SEXP s_dplyr = Rf_install("dplyr");
  static SEXP s_base = Rf_install("base");
  static SEXP s_stats = Rf_install("stats");

  Expression<LazySubsets> expression(expr, subsets);

  SEXP column;
  bool test;

  // fixed sized expressions
  switch(expression.size()){
  case 0:
    // n()
    if (expression.is_fun(s_n, s_dplyr)) {
      return op(n_(data));
    }
    break;

  case 1:
    // sum( <column> ) and base::sum( <column> )
    if (expression.is_fun(s_sum, s_base) && expression.is_unnamed(0) && expression.is_column(0, column)) {
      return op( sum_(data, column, false) );
    }

    // mean( <column> ) and base::mean( <column> )
    if (expression.is_fun(s_mean, s_base) && expression.is_unnamed(0) && expression.is_column(0, column)) {
      return op( mean_(data, column, false) );
    }

    // var( <column> ) and stats::var( <column> )
    if (expression.is_fun(s_var, s_stats) && expression.is_unnamed(0) && expression.is_column(0, column)) {
      return op( var_(data, column, false) );
    }

    // sd( <column> ) and stats::sd( <column> )
    if (expression.is_fun(s_sd, s_stats) && expression.is_unnamed(0) && expression.is_column(0, column)) {
      return op( sd_(data, column, false) );
    }

  case 2:
    // sum( <column>, na.rm = <bool> )
    // base::sum( <column>, na.rm = <bool> )
    if (expression.is_fun(s_sum, s_base) &&
      expression.is_unnamed(0) && expression.is_column(0, column) &&
      expression.is_named(1, s_narm) && expression.is_scalar_logical(1, test)
    ) {
      return op(sum_(data, column, test));
    }

    // mean( <column>, na.rm = <bool> )
    // base::mean( <column>, na.rm = <bool> )
    if (expression.is_fun(s_mean, s_base) &&
      expression.is_unnamed(0) && expression.is_column(0, column) &&
      expression.is_named(1, s_narm) && expression.is_scalar_logical(1, test)
    ) {
      return op(mean_(data, column, test));
    }

    // var( <column>, na.rm = <bool> )
    // stats::var( <column>, na.rm = <bool> )
    if (expression.is_fun(s_var, s_stats) &&
      expression.is_unnamed(0) && expression.is_column(0, column) &&
      expression.is_named(1, s_narm) && expression.is_scalar_logical(1, test)
    ) {
      return op(var_(data, column, test));
    }

    // sd( <column>, na.rm = <bool> )
    // stats::sd( <column>, na.rm = <bool> )
    if (expression.is_fun(s_sd, s_stats) &&
      expression.is_unnamed(0) && expression.is_column(0, column) &&
      expression.is_named(1, s_narm) && expression.is_scalar_logical(1, test)
    ) {
      return op(sd_(data, column, test));
    }

  default:
    break;
  }

  // functions that take variadic number of arguments
  if (expression.is_fun(s_n_distinct, s_dplyr)){
    return n_distinct_(data, expression, op);
  }

  // give up
  return R_UnboundValue;
}

template <typename SlicedTibble, typename LazySubsets>
SEXP summarise(SEXP expr, const SlicedTibble& data, const LazySubsets& subsets, SEXP env){
  return hybrid_do(expr, data, subsets, env, Summary());
}

template <typename SlicedTibble, typename LazySubsets>
SEXP window(SEXP expr, const SlicedTibble& data, const LazySubsets& subsets, SEXP env){
  return hybrid_do(expr, data, subsets, env, Window());
}


}
}

#endif
