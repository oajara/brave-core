/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ml/model/linear/linear.h"
#include <vector>

#include "bat/ads/internal/base/unittest/unittest_base.h"
#include "bat/ads/internal/ml/data/vector_data.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads::ml {

class BatAdsLinearTest : public UnitTestBase {};

TEST_F(BatAdsLinearTest, ThreeClassesPredictionTest) {
  // Arrange
  std::vector<float> data1 = {1.0, 0.0, 0.0};
  std::vector<float> data2 = {0.0, 1.0, 0.0};
  std::vector<float> data3 = {0.0, 0.0, 1.0};
  std::vector<float> data4 = {0.0, 1.0, 2.0};
  std::vector<float> data5 = {1.0, 1.0, 1.0};
  const std::map<std::string, VectorData> weights = {
      {"class_1", VectorData(data1)},
      {"class_2", VectorData(data2)},
      {"class_3", VectorData(data3)}};

  const std::map<std::string, double> biases = {
      {"class_1", 0.0}, {"class_2", 0.0}, {"class_3", 0.0}};

  const model::Linear linear(weights, biases);
  const VectorData class_1_vector_data(data1);
  const VectorData class_2_vector_data(data2);
  const VectorData class_3_vector_data(data4);

  // Act
  const PredictionMap predictions_1 = linear.Predict(class_1_vector_data);
  const PredictionMap predictions_2 = linear.Predict(class_2_vector_data);
  const PredictionMap predictions_3 = linear.Predict(class_3_vector_data);

  // Assert
  ASSERT_GT(predictions_1.at("class_1"), predictions_1.at("class_2"));
  ASSERT_GT(predictions_1.at("class_1"), predictions_1.at("class_3"));

  ASSERT_GT(predictions_2.at("class_2"), predictions_2.at("class_1"));
  ASSERT_GT(predictions_2.at("class_2"), predictions_2.at("class_3"));

  EXPECT_TRUE(predictions_3.at("class_3") > predictions_3.at("class_1") &&
              predictions_3.at("class_3") > predictions_3.at("class_2"));
}

TEST_F(BatAdsLinearTest, BiasesPredictionTest) {
  // Arrange
  const std::map<std::string, VectorData> weights = {
      {"class_1", VectorData(data1)},
      {"class_2", VectorData(data2)},
      {"class_3", VectorData(data3)}};

  const std::map<std::string, double> biases = {
      {"class_1", 0.5}, {"class_2", 0.25}, {"class_3", 1.0}};

  const model::Linear linear_biased(weights, biases);
  const VectorData avg_vector(data5);

  // Act
  const PredictionMap predictions = linear_biased.Predict(avg_vector);

  // Assert
  EXPECT_TRUE(predictions.at("class_3") > predictions.at("class_1") &&
              predictions.at("class_3") > predictions.at("class_2") &&
              predictions.at("class_1") > predictions.at("class_2"));
}

TEST_F(BatAdsLinearTest, BinaryClassifierPredictionTest) {
  // Arrange
  const size_t kExpectedPredictionSize = 1;
  const std::vector<float> data = {0.3, 0.2, 0.25};
  const std::map<std::string, VectorData> weights = {
      {"the_only_class", VectorData(data)},
  };

  const std::map<std::string, double> biases = {
      {"the_only_class", -0.45},
  };

  const model::Linear linear(weights, biases);
  const std::vector<float> data0 = {1.07, 1.52, 0.91};
  const std::vector<float> data1 = {1.11, 1.63, 1.21};
  const VectorData vector_data_0(data0);
  const VectorData vector_data_1(data1);

  // Act
  const PredictionMap predictions_0 = linear.Predict(vector_data_0);
  const PredictionMap predictions_1 = linear.Predict(vector_data_1);

  // Assert
  ASSERT_EQ(kExpectedPredictionSize, predictions_0.size());
  ASSERT_EQ(kExpectedPredictionSize, predictions_1.size());

  EXPECT_TRUE(predictions_0.at("the_only_class") < 0.5 &&
              predictions_1.at("the_only_class") > 0.5);
}

TEST_F(BatAdsLinearTest, TopPredictionsTest) {
  // Arrange
  const size_t kPredictionLimits[2] = {2, 1};
  const std::vector<float> c_1 = {1.0, 0.5, 0.8};
  const std::vector<float> c_2 = {0.3, 1.0, 0.7};
  const std::vector<float> c_3 = {0.6, 0.9, 1.0};
  const std::vector<float> c_4 = {0.7, 1.0, 0.8};
  const std::vector<float> c_5 = {1.0, 0.2, 1.0};
  const std::map<std::string, VectorData> weights = {
      {"class_1", VectorData(c_1)},
      {"class_2", VectorData(c_2)},
      {"class_3", VectorData(c_3)},
      {"class_4", VectorData(c_4)},
      {"class_5", VectorData(c_5)}};

  const std::map<std::string, double> biases = {{"class_1", 0.21},
                                                {"class_2", 0.22},
                                                {"class_3", 0.23},
                                                {"class_4", 0.22},
                                                {"class_5", 0.21}};

  const model::Linear linear_biased(weights, biases);
  const std::vector<float> pt_1 = {1.0, 0.99, 0.98, 0.97, 0.96};
  const std::vector<float> pt_2 = {0.83, 0.79, 0.91, 0.87, 0.82};
  const std::vector<float> pt_3 = {0.92, 0.95, 0.85, 0.91, 0.73};
  const VectorData point_1(pt_1);
  const VectorData point_2(pt_2);
  const VectorData point_3(pt_3);

  // Act
  const PredictionMap predictions_1 = linear_biased.GetTopPredictions(point_1);
  const PredictionMap predictions_2 =
      linear_biased.GetTopPredictions(point_2, kPredictionLimits[0]);
  const PredictionMap predictions_3 =
      linear_biased.GetTopPredictions(point_3, kPredictionLimits[1]);

  // Assert
  ASSERT_EQ(weights.size(), predictions_1.size());
  ASSERT_EQ(kPredictionLimits[0], predictions_2.size());
  EXPECT_EQ(kPredictionLimits[1], predictions_3.size());
}

}  // namespace ads::ml
