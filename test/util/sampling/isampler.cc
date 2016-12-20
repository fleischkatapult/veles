/*
 * Copyright 2016 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "mock_sampler.h"

namespace veles {
namespace util {

using testing::Return;
using testing::Expectation;

QByteArray prepare_data(size_t size) {
  QByteArray result;
  for (size_t i = 0; i < size; ++i) {
    result.push_back(static_cast<char>(i % 127));
  }
  return result;
}

TEST(ISamplerSmallData, basic) {
  auto data = prepare_data(100);
  testing::StrictMock<MockSampler> sampler(data);
  sampler.setSampleSize(120);
  ASSERT_EQ(100, sampler.getSampleSize());
  ASSERT_EQ(data[0], sampler[0]);
  ASSERT_EQ(data[10], sampler[10]);
  ASSERT_EQ(data[99], sampler[sampler.getSampleSize() - 1]);
}

TEST(ISamplerSmallData, setRange) {
  auto data = prepare_data(100);
  testing::StrictMock<MockSampler> sampler(data);
  sampler.setSampleSize(120);
  sampler.setRange(40, 50);
  ASSERT_EQ(10, sampler.getSampleSize());
  ASSERT_EQ(40, sampler.getRange().first);
  ASSERT_EQ(50, sampler.getRange().second);
  for (int i = 0; i < 10; ++i) {
    ASSERT_EQ(data[i + 40], sampler[i]);
  }
}

TEST(ISamplerSmallData, offsets) {
  auto data = prepare_data(100);
  testing::StrictMock<MockSampler> sampler(data);
  sampler.setSampleSize(120);
  ASSERT_EQ(0, sampler.getFileOffset(0));
  ASSERT_EQ(33, sampler.getFileOffset(33));
  ASSERT_EQ(99, sampler.getFileOffset(99));
  ASSERT_EQ(0, sampler.getSampleOffset(0));
  ASSERT_EQ(33, sampler.getSampleOffset(33));
  ASSERT_EQ(99, sampler.getSampleOffset(99));
}

TEST(ISamplerSmallData, offsetsAfterSetRange) {
  auto data = prepare_data(100);
  testing::StrictMock<MockSampler> sampler(data);
  sampler.setSampleSize(120);
  sampler.setRange(40, 50);
  ASSERT_EQ(40, sampler.getFileOffset(0));
  ASSERT_EQ(45, sampler.getFileOffset(5));
  ASSERT_EQ(49, sampler.getFileOffset(9));
  ASSERT_EQ(0, sampler.getSampleOffset(40));
  ASSERT_EQ(5, sampler.getSampleOffset(45));
  ASSERT_EQ(9, sampler.getSampleOffset(49));
}

TEST(ISamplerWithSampling, basic) {
  auto data = prepare_data(100);
  testing::NiceMock<MockSampler> sampler(data);
  sampler.setSampleSize(10);
  Expectation init = EXPECT_CALL(sampler, initialiseSample(10));
  EXPECT_CALL(sampler, getRealSampleSize())
    .After(init)
    .WillRepeatedly(Return(10));
  ASSERT_EQ(10, sampler.getSampleSize());
  EXPECT_CALL(sampler, getSampleByte(0))
    .WillOnce(Return(0));
  ASSERT_EQ(data[0], sampler[0]);
  EXPECT_CALL(sampler, getSampleByte(5))
    .WillOnce(Return(33));
  ASSERT_EQ(data[33], sampler[5]);
  EXPECT_CALL(sampler, getSampleByte(9))
    .WillOnce(Return(99));
  ASSERT_EQ(data[99], sampler[9]);
}

TEST(ISamplerWithSampling, offsets) {
  auto data = prepare_data(100);
  testing::NiceMock<MockSampler> sampler(data);
  sampler.setSampleSize(10);
  ON_CALL(sampler, getRealSampleSize())
    .WillByDefault(Return(10));
  EXPECT_CALL(sampler, getFileOffsetImpl(5))
    .Times(1)
    .WillOnce(Return(50));
  ASSERT_EQ(0, sampler.getFileOffset(0));
  ASSERT_EQ(99, sampler.getFileOffset(9));
  ASSERT_EQ(50, sampler.getFileOffset(5));

  EXPECT_CALL(sampler, getSampleOffsetImpl(50))
    .Times(1)
    .WillOnce(Return(5));
  ASSERT_EQ(0, sampler.getSampleOffset(0));
  ASSERT_EQ(9, sampler.getSampleOffset(99));
  ASSERT_EQ(5, sampler.getSampleOffset(50));
  }

TEST(ISamplerWithSampling, offsetsAfterSetRange) {
  auto data = prepare_data(100);
  testing::NiceMock<MockSampler> sampler(data);
  sampler.setSampleSize(10);
  sampler.setRange(40, 60);
  ON_CALL(sampler, getRealSampleSize())
    .WillByDefault(Return(10));
  EXPECT_CALL(sampler, getFileOffsetImpl(5))
    .Times(1)
    .WillOnce(Return(10));
  ASSERT_EQ(40, sampler.getFileOffset(0));
  ASSERT_EQ(59, sampler.getFileOffset(9));
  ASSERT_EQ(50, sampler.getFileOffset(5));

  EXPECT_CALL(sampler, getSampleOffsetImpl(10))
    .Times(1)
    .WillOnce(Return(5));
  ASSERT_EQ(0, sampler.getSampleOffset(40));
  ASSERT_EQ(9, sampler.getSampleOffset(59));
  ASSERT_EQ(5, sampler.getSampleOffset(50));
}

TEST(ISamplerWithSampling, getDataFromIsampler) {
  auto data = prepare_data(100);
  testing::StrictMock<MockSampler> sampler(data);
  sampler.setSampleSize(10);
  ASSERT_EQ(100, sampler.proxy_getDataSize());
  ASSERT_EQ(0, sampler.proxy_getDataByte(0));
  ASSERT_EQ(5, sampler.proxy_getDataByte(5));
  ASSERT_EQ(99, sampler.proxy_getDataByte(99));
  sampler.setRange(40, 60);
  ASSERT_EQ(20, sampler.proxy_getDataSize());
  ASSERT_EQ(40, sampler.proxy_getDataByte(0));
  ASSERT_EQ(45, sampler.proxy_getDataByte(5));
  ASSERT_EQ(59, sampler.proxy_getDataByte(19));
}


}  // namespace util
}  // namespace veles