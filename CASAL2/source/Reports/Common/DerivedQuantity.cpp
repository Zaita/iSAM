/*
 * DerivedQuantity.cpp
 *
 *  Created on: 4/09/2013
 *      Author: Admin
 */

#include "DerivedQuantity.h"

#include "../../DerivedQuantities/Manager.h"

namespace niwa {
namespace reports {

/**
 *
 */
DerivedQuantity::DerivedQuantity() {
  run_mode_    = (RunMode::Type)(RunMode::kBasic | RunMode::kProjection | RunMode::kSimulation | RunMode::kEstimation | RunMode::kProfiling);
  model_state_ = (State::Type)(State::kIterationComplete);

  parameters_.Bind<string>(PARAM_DERIVED_QUANTITY, &derived_quantity_label_, "The derived quantity label", "", "");
}

/**
 * Validate object
 */
void DerivedQuantity::DoValidate(shared_ptr<Model> model) {
  if (derived_quantity_label_ == "")
    derived_quantity_label_ = label_;
}
/**
 * Build the relationships between this object and other objects
 */
void DerivedQuantity::DoBuild(shared_ptr<Model> model) {
  derived_quantity_ = model->managers()->derived_quantity()->GetDerivedQuantity(derived_quantity_label_);
  if (!derived_quantity_) {
#ifndef TESTMODE
    LOG_WARNING() << "The report for " << PARAM_DERIVED_QUANTITY << " with label '" << derived_quantity_label_ << "' was requested. This " << PARAM_DERIVED_QUANTITY
                  << " was not found in the input configuration file and the report will not be generated";
#endif
    is_valid_ = false;
  }
}
/**
 *
 */
void DerivedQuantity::DoExecute(shared_ptr<Model> model) {
  LOG_TRACE();
  derivedquantities::Manager& manager = *model->managers()->derived_quantity();
  cache_ << ReportHeader(type_, derived_quantity_label_);

  auto derived_quantities = manager.objects();
  for (auto dq : derived_quantities) {
    string label = dq->label();
    cache_ << label << " " << REPORT_R_LIST << REPORT_EOL;
    cache_ << "type: " << dq->type() << REPORT_EOL;
    const vector<vector<Double>> init_values = dq->initialisation_values();
    for (unsigned i = 0; i < init_values.size(); ++i) {
      cache_ << "initialisation_phase[" << i + 1 << "]: ";
      Double value = init_values[i].back();
      cache_ << AS_DOUBLE(value) << " ";
      cache_ << REPORT_EOL;
    }

    const map<unsigned, Double> values = dq->values();
    cache_ << "values " << REPORT_R_VECTOR << "\n";
    for (auto iter = values.begin(); iter != values.end(); ++iter) {
      Double weight = iter->second;
      cache_ << iter->first << " " << AS_DOUBLE(weight) << REPORT_EOL;
    }
    // cache_ <<"\n";
    cache_ << REPORT_R_LIST_END << REPORT_EOL;
  }
  ready_for_writing_ = true;
}

/**
 * Execute the tabular report
 */

void DerivedQuantity::DoExecuteTabular(shared_ptr<Model> model) {
  derivedquantities::Manager& manager            = *model->managers()->derived_quantity();
  auto                        derived_quantities = manager.objects();

  if (first_run_) {
    first_run_ = false;
    cache_ << "*" << type_ << "[" << derived_quantity_label_ << "]" << REPORT_EOL;
    cache_ << "values " << REPORT_R_DATAFRAME << REPORT_EOL;
    for (auto dq : derived_quantities) {
      const vector<vector<Double>> init_values  = dq->initialisation_values();
      const map<unsigned, Double>  values       = dq->values();
      string                       derived_type = dq->type();
      for (unsigned i = 0; i < init_values.size(); ++i) {
        cache_ << derived_type << "[" << dq->label() << "][initialisation_phase_" << i + 1 << "] ";
      }
      for (auto iter = values.begin(); iter != values.end(); ++iter) cache_ << derived_type << "[" << dq->label() << "][" << iter->first << "] ";
    }
    cache_ << REPORT_EOL;
  }
  for (auto dq : derived_quantities) {
    string                       derived_type = dq->type();
    const map<unsigned, Double>  values       = dq->values();
    const vector<vector<Double>> init_values  = dq->initialisation_values();
    for (unsigned i = 0; i < init_values.size(); ++i) {
      Double value = init_values[i].back();
      cache_ << AS_DOUBLE(value) << " ";
    }
    for (auto iter = values.begin(); iter != values.end(); ++iter) {
      Double weight = iter->second;
      cache_ << AS_DOUBLE(weight) << " ";
    }
  }
  cache_ << REPORT_EOL;
}

/**
 * Finalise the tabular report
 */
void DerivedQuantity::DoFinaliseTabular(shared_ptr<Model> model) {
  ready_for_writing_ = true;
}

} /* namespace reports */
} /* namespace niwa */
