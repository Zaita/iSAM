/**
 * @file TransitionCategory.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 21/12/2012
 * @section LICENSE
 *
 * Copyright NIWA Science �2012 - www.niwa.co.nz
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "TransitionCategory.h"

#include "Categories/Categories.h"
#include "Selectivities/Manager.h"

// Namespaces
namespace niwa {
namespace processes {
namespace age {

/**
 * Default constructor
 */
TransitionCategory::TransitionCategory(shared_ptr<Model> model) : Process(model), from_partition_(model), to_partition_(model) {
  LOG_TRACE();

  parameters_.Bind<string>(PARAM_FROM, &from_category_names_, "The categories to transition from", "");
  parameters_.Bind<string>(PARAM_TO, &to_category_names_, "The categories to transition to", "");
  parameters_.Bind<Double>(PARAM_PROPORTIONS, &proportions_, "The proportions to transition for each category", "")->set_range(0.0, 1.0);
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_names_, "The selectivities to apply to each proportion", "");

  RegisterAsAddressable(PARAM_PROPORTIONS, &proportions_by_category_);

  process_type_        = ProcessType::kTransition;
  partition_structure_ = PartitionType::kAge;
}

/**
 * Validate the Transition process
 *
 * - Check for the required parameters
 * - Assign variables from the parameters
 * - Verify the categories are real
 * - If proportions or selectivities only has one element specified
 *   then add more elements until they match number of categories
 * - Verify vector lengths are all the same
 * - Verify categories From->To have matching age ranges
 * - Check all proportions are between 0.0 and 1.0
 */
void TransitionCategory::DoValidate() {
  LOG_TRACE();

  if (selectivity_names_.size() == 1) {
    auto val_s = selectivity_names_[0];
    selectivity_names_.assign(from_category_names_.size(), val_s);
  }

  //  // Validate Categories
  auto categories = model_->categories();

  // Validate the from and to vectors are the same size
  if (from_category_names_.size() != to_category_names_.size()) {
    LOG_ERROR_P(PARAM_TO) << ": the number of 'to' categories provided does not match the number of 'from' categories provided. " << from_category_names_.size()
                          << " categories were supplied, but " << to_category_names_.size() << " categories are required";
  }

  // Allow a one to many relationship between proportions and number of categories.
  if (proportions_.size() == 1)
    proportions_.resize(to_category_names_.size(), proportions_[0]);

  // Validate the to category and proportions vectors are the same size
  if (to_category_names_.size() != proportions_.size()) {
    LOG_ERROR_P(PARAM_PROPORTIONS) << ": the number of proportions provided does not match the number of 'to' categories provided. " << to_category_names_.size()
                                   << " categories were supplied, but proportions size is " << proportions_.size();
  }

  // Validate the number of selectivities matches the number of proportions
  if (proportions_.size() != selectivity_names_.size() && proportions_.size() != 1) {
    LOG_ERROR_P(PARAM_SELECTIVITIES) << ": the number of selectivities provided does not match the number of proportions provided. "
                                     << " proportions size is " << proportions_.size() << " but number of selectivities is " << selectivity_names_.size();
  }

  // Validate that each from and to category have the same age range.
  for (unsigned i = 0; i < from_category_names_.size(); ++i) {
    if (categories->min_age(from_category_names_[i]) != categories->min_age(to_category_names_[i])) {
      LOG_ERROR_P(PARAM_FROM) << ": 'from' category " << from_category_names_[i] << " does not"
                              << " have the same age range as the 'to' category " << to_category_names_[i];
    }

    if (categories->max_age(from_category_names_[i]) != categories->max_age(to_category_names_[i])) {
      LOG_ERROR_P(PARAM_FROM) << ": 'from' category " << from_category_names_[i] << " does not"
                              << " have the same age range as the 'to' category " << to_category_names_[i];
    }
  }

  // Validate the proportions are between 0.0 and 1.0
  for (Double proportion : proportions_) {
    if (proportion < 0.0 || proportion > 1.0)
      LOG_ERROR_P(PARAM_PROPORTIONS) << ": proportion " << AS_DOUBLE(proportion) << " must be between 0.0 and 1.0 (inclusive)";
  }

  for (unsigned i = 0; i < from_category_names_.size(); ++i) 
    proportions_by_category_[from_category_names_[i]] = proportions_[i];
}

/**
 * Build any runtime relationships this class needs.
 *
 * - Build the partition accessors
 * - Verify the selectivities are valid
 * - Get pointers to the selectivities
 */
void TransitionCategory::DoBuild() {
  LOG_TRACE();

  from_partition_.Init(from_category_names_);
  to_partition_.Init(to_category_names_);

  for (string label : selectivity_names_) {
    Selectivity* selectivity = model_->managers()->selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity label " << label << " was not found.";
    selectivities_.push_back(selectivity);
  }

  if (from_partition_.size() != to_partition_.size()) {
    LOG_FATAL() << "The list of categories for the transition category process are not of equal size in year " << model_->current_year() << ". Number of 'From' "
                << from_partition_.size() << " and 'To' " << to_partition_.size() << " categories to transition between";
  }
}

/**
 * Execute the maturation rate process.
 */
void TransitionCategory::DoExecute() {
  LOG_TRACE();

  auto   from_iter = from_partition_.begin();
  auto   to_iter   = to_partition_.begin();
  Double amount    = 0.0;

  LOG_FINE() << "from_partition_.size(): " << from_partition_.size()
               << "; to_partition_.size(): " << to_partition_.size();


  for (unsigned i = 0; from_iter != from_partition_.end() && to_iter != to_partition_.end(); ++from_iter, ++to_iter, ++i) {
    for (unsigned offset = 0; offset < (*from_iter)->data_.size(); ++offset) {
      amount = proportions_by_category_[(*from_iter)->name_] * selectivities_[i]->GetAgeResult(min_age_ + offset, (*from_iter)->age_length_) * (*from_iter)->data_[offset];

      (*from_iter)->data_[offset] -= amount;
      (*to_iter)->data_[offset] += amount;
      LOG_FINEST() << "Moving " << amount << " number of individuals , from number " << (*from_iter)->data_[offset] << " from category = " << (*from_iter)->name_;
      if ((*from_iter)->data_[offset] < 0.0)
        LOG_FATAL() << "Maturation rate caused a negative partition if ((*from_iter)->data_[offset] < 0.0) ";
    }
  }
}

/**
 * Reset the maturation rates
 */
void TransitionCategory::DoReset() {

}

} /* namespace age */
} /* namespace processes */
} /* namespace niwa */
