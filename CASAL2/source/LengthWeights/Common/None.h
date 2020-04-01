/**
 * @file None.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 24/07/2013
 * @section LICENSE
 *
 * Copyright NIWA Science �2017 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifndef LENGTHWEIGHTS_NONE_H_
#define LENGTHWEIGHTS_NONE_H_

// headers
#include "LengthWeights/LengthWeight.h"

// namespaces
namespace niwa {
namespace lengthweights {

/**
 * class definition
 */
class None : public niwa::LengthWeight {
public:
  // methods
  explicit None(Model* model) : LengthWeight(model) { };
  virtual                     ~None() = default;
  void                        DoValidate() override final { };
  void                        DoBuild() override final { };
  void                        DoInitialise() override final { };
  void                        DoReset() override final { };

  // accessors
  Double                      mean_weight(Double size, Distribution distribution, Double cv) const override final { return 1.0; }
  const vector<unsigned>&     GetTimeVaryingYears() override final { return time_varying_years_; }

private:
  vector<unsigned>            time_varying_years_;
};

} /* namespace lengthweights */
} /* namespace niwa */
#endif /* LENGTHWEIGHTS_NONE_H_ */
