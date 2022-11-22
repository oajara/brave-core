#include "brave/components/ipfs/pin/ipfs_local_pin_service.h"

#include <memory>
#include <utility>

#include "base/json/json_reader.h"
#include "base/test/bind.h"
#include "brave/components/ipfs/ipfs_service.h"
#include "brave/components/ipfs/pref_names.h"
#include "components/prefs/testing_pref_service.h"
#include "content/public/test/browser_task_environment.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;

namespace ipfs {

class MockIpfsService : public IpfsService {
 public:
  MockIpfsService() = default;

  ~MockIpfsService() override = default;

  MOCK_METHOD3(AddPin,
               void(const std::vector<std::string>& cids,
                    bool recursive,
                    IpfsService::AddPinCallback callback));
  MOCK_METHOD2(RemovePin,
               void(const std::vector<std::string>& cid,
                    IpfsService::RemovePinCallback callback));
  MOCK_METHOD4(GetPins,
               void(const absl::optional<std::vector<std::string>>& cid,
                    const std::string& type,
                    bool quiet,
                    IpfsService::GetPinsCallback callback));
};

class IpfsLocalPinServiceTest : public testing::Test {
 public:
  IpfsLocalPinServiceTest() = default;

 protected:
  void SetUp() override {
    auto* registry = pref_service_.registry();
    IpfsService::RegisterProfilePrefs(registry);
    // ipfs_local_pin_service_ =
    // std::make_unique<IpfsLocalPinService>(GetPrefs(), nullptr);
  }

  PrefService* GetPrefs() { return &pref_service_; }

  std::unique_ptr<IpfsLocalPinService> ipfs_local_pin_service_;
  TestingPrefServiceSimple pref_service_;
  content::BrowserTaskEnvironment task_environment_;
};

TEST_F(IpfsLocalPinServiceTest, AddLocalPinJobTest) {
  testing::NiceMock<MockIpfsService> ipfs_service;
  {
    bool success = false;
    AddLocalPinJob job(GetPrefs(), &ipfs_service, "a", {"Qma", "Qmb", "Qmc"},
                       base::BindLambdaForTesting(
                           [&success](bool result) { success = result; }));

    ON_CALL(ipfs_service, AddPin(_, _, _))
        .WillByDefault(::testing::Invoke(
            [](const std::vector<std::string>& cids, bool recursive,
               IpfsService::AddPinCallback callback) {
              AddPinResult result;
              result.pins = cids;
              std::move(callback).Run(true, result);
            }));

    job.Start();

    std::string expected = R"({
                                "Qma" : ["a"],
                                "Qmb" : ["a"],
                                "Qmc" : ["a"]
                             })";
    absl::optional<base::Value> expected_value = base::JSONReader::Read(
        expected, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                      base::JSONParserOptions::JSON_PARSE_RFC);
    EXPECT_EQ(expected_value.value(), GetPrefs()->GetDict(kIPFSPinnedCids));
    EXPECT_TRUE(success);
  }

  {
    bool success = false;
    AddLocalPinJob job(GetPrefs(), &ipfs_service, "b",
                       {"Qma", "Qmb", "Qmc", "Qmd"},
                       base::BindLambdaForTesting(
                           [&success](bool result) { success = result; }));

    ON_CALL(ipfs_service, AddPin(_, _, _))
        .WillByDefault(::testing::Invoke(
            [](const std::vector<std::string>& cids, bool recursive,
               IpfsService::AddPinCallback callback) {
              AddPinResult result;
              result.pins = cids;
              std::move(callback).Run(true, result);
            }));

    job.Start();

    std::string expected = R"({
                                "Qma" : ["a", "b"],
                                "Qmb" : ["a", "b"],
                                "Qmc" : ["a", "b"],
                                "Qmd" : ["b"]
                             })";
    absl::optional<base::Value> expected_value = base::JSONReader::Read(
        expected, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                      base::JSONParserOptions::JSON_PARSE_RFC);
    EXPECT_EQ(expected_value.value(), GetPrefs()->GetDict(kIPFSPinnedCids));
    EXPECT_TRUE(success);
  }

  {
    bool success = false;
    AddLocalPinJob job(GetPrefs(), &ipfs_service, "c",
                       {"Qma", "Qmb", "Qmc", "Qmd", "Qme"},
                       base::BindLambdaForTesting(
                           [&success](bool result) { success = result; }));

    ON_CALL(ipfs_service, AddPin(_, _, _))
        .WillByDefault(::testing::Invoke(
            [](const std::vector<std::string>& cids, bool recursive,
               IpfsService::AddPinCallback callback) {
              AddPinResult result;
              result.pins = {"Qma", "Qmb", "Qmc"};
              std::move(callback).Run(true, result);
            }));

    job.Start();

    std::string expected = R"({
                                "Qma" : ["a", "b"],
                                "Qmb" : ["a", "b"],
                                "Qmc" : ["a", "b"],
                                "Qmd" : ["b"]
                             })";
    absl::optional<base::Value> expected_value = base::JSONReader::Read(
        expected, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                      base::JSONParserOptions::JSON_PARSE_RFC);
    EXPECT_EQ(expected_value.value(), GetPrefs()->GetDict(kIPFSPinnedCids));
    EXPECT_FALSE(success);
  }

  {
    bool success = false;
    AddLocalPinJob job(GetPrefs(), &ipfs_service, "c",
                       {"Qma", "Qmb", "Qmc", "Qmd", "Qme"},
                       base::BindLambdaForTesting(
                           [&success](bool result) { success = result; }));

    ON_CALL(ipfs_service, AddPin(_, _, _))
        .WillByDefault(::testing::Invoke(
            [](const std::vector<std::string>& cids, bool recursive,
               IpfsService::AddPinCallback callback) {
              std::move(callback).Run(false, absl::nullopt);
            }));

    job.Start();

    std::string expected = R"({
                                "Qma" : ["a", "b"],
                                "Qmb" : ["a", "b"],
                                "Qmc" : ["a", "b"],
                                "Qmd" : ["b"]
                             })";
    absl::optional<base::Value> expected_value = base::JSONReader::Read(
        expected, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                      base::JSONParserOptions::JSON_PARSE_RFC);
    EXPECT_EQ(expected_value.value(), GetPrefs()->GetDict(kIPFSPinnedCids));
    EXPECT_FALSE(success);
  }
}

TEST_F(IpfsLocalPinServiceTest, RemoveLocalPinJobTest) {
  {
    std::string base = R"({
                                  "Qma" : ["a", "b"],
                                  "Qmb" : ["a", "b"],
                                  "Qmc" : ["a", "b"],
                                  "Qmd" : ["b"]
                               })";
    absl::optional<base::Value> base_value = base::JSONReader::Read(
        base, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                  base::JSONParserOptions::JSON_PARSE_RFC);
    GetPrefs()->SetDict(kIPFSPinnedCids, base_value.value().GetDict().Clone());
  }

  {
    bool success = false;
    RemoveLocalPinJob job(GetPrefs(), "a",
                          base::BindLambdaForTesting(
                              [&success](bool result) { success = result; }));

    job.Start();

    std::string expected = R"({
                             "Qma" : ["b"],
                             "Qmb" : ["b"],
                             "Qmc" : ["b"],
                             "Qmd" : ["b"]
                             })";
    absl::optional<base::Value> expected_value = base::JSONReader::Read(
        expected, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                      base::JSONParserOptions::JSON_PARSE_RFC);
    EXPECT_EQ(expected_value.value(), GetPrefs()->GetDict(kIPFSPinnedCids));
    EXPECT_TRUE(success);
  }

  {
    bool success = false;
    RemoveLocalPinJob job(GetPrefs(), "c",
                          base::BindLambdaForTesting(
                              [&success](bool result) { success = result; }));

    job.Start();

    std::string expected = R"({
                             "Qma" : ["b"],
                             "Qmb" : ["b"],
                             "Qmc" : ["b"],
                             "Qmd" : ["b"]
                             })";
    absl::optional<base::Value> expected_value = base::JSONReader::Read(
        expected, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                      base::JSONParserOptions::JSON_PARSE_RFC);
    EXPECT_EQ(expected_value.value(), GetPrefs()->GetDict(kIPFSPinnedCids));
    EXPECT_TRUE(success);
  }

  {
    bool success = false;
    RemoveLocalPinJob job(GetPrefs(), "b",
                          base::BindLambdaForTesting(
                              [&success](bool result) { success = result; }));

    job.Start();

    std::string expected = R"({
                             })";
    absl::optional<base::Value> expected_value = base::JSONReader::Read(
        expected, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                      base::JSONParserOptions::JSON_PARSE_RFC);
    EXPECT_EQ(expected_value.value(), GetPrefs()->GetDict(kIPFSPinnedCids));
    EXPECT_TRUE(success);
  }
}

TEST_F(IpfsLocalPinServiceTest, VerifyLocalPinJobTest) {
  testing::NiceMock<MockIpfsService> ipfs_service;

  {
    std::string base = R"({
                                  "Qma" : ["a", "b"],
                                  "Qmb" : ["a", "b"],
                                  "Qmc" : ["a", "b"],
                                  "Qmd" : ["b"]
                               })";
    absl::optional<base::Value> base_value = base::JSONReader::Read(
        base, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                  base::JSONParserOptions::JSON_PARSE_RFC);
    GetPrefs()->SetDict(kIPFSPinnedCids, base_value.value().GetDict().Clone());
  }

  {
    absl::optional<bool> success;
    VerifyLocalPinJob job(
        GetPrefs(), &ipfs_service, "a", {"Qma", "Qmb", "Qmc"},
        base::BindLambdaForTesting([&success](absl::optional<bool> result) {
          success = result.value();
        }));

    ON_CALL(ipfs_service, GetPins(_, _, _, _))
        .WillByDefault(::testing::Invoke([](const absl::optional<
                                                std::vector<std::string>>& cid,
                                            const std::string& type, bool quiet,
                                            IpfsService::GetPinsCallback
                                                callback) {
          GetPinsResult result = {{"Qma", "Recursive"}, {"Qmb", "Recursive"}};
          std::move(callback).Run(true, result);
        }));

    job.Start();

    EXPECT_TRUE(success.has_value());
    EXPECT_FALSE(success.value());
  }

  {
    absl::optional<bool> success;
    VerifyLocalPinJob job(
        GetPrefs(), &ipfs_service, "a", {"Qma", "Qmb", "Qmc"},
        base::BindLambdaForTesting([&success](absl::optional<bool> result) {
          success = result.value();
        }));

    ON_CALL(ipfs_service, GetPins(_, _, _, _))
        .WillByDefault(::testing::Invoke(
            [](const absl::optional<std::vector<std::string>>& cid,
               const std::string& type, bool quiet,
               IpfsService::GetPinsCallback callback) {
              GetPinsResult result = {{"Qma", "Recursive"},
                                      {"Qmb", "Recursive"},
                                      {"Qmc", "Recursive"}};
              std::move(callback).Run(true, result);
            }));

    job.Start();

    EXPECT_TRUE(success.has_value());
    EXPECT_TRUE(success.value());
  }

  {
    absl::optional<bool> success = false;
    VerifyLocalPinJob job(
        GetPrefs(), nullptr, "b", {"Qma", "Qmb", "Qmc", "Qmd"},
        base::BindLambdaForTesting([&success](absl::optional<bool> result) {
          success = result.value();
        }));

    ON_CALL(ipfs_service, GetPins(_, _, _, _))
        .WillByDefault(::testing::Invoke(
            [](const absl::optional<std::vector<std::string>>& cid,
               const std::string& type, bool quiet,
               IpfsService::GetPinsCallback callback) {
              GetPinsResult result = {};
              std::move(callback).Run(true, result);
            }));

    EXPECT_TRUE(success.has_value());

    EXPECT_FALSE(success.value());
  }

  {
    absl::optional<bool> success;
    VerifyLocalPinJob job(
        GetPrefs(), &ipfs_service, "b", {"Qma", "Qmb", "Qmc", "Qmd"},
        base::BindLambdaForTesting(
            [&success](absl::optional<bool> result) { success = result; }));

    ON_CALL(ipfs_service, GetPins(_, _, _, _))
        .WillByDefault(::testing::Invoke(
            [](const absl::optional<std::vector<std::string>>& cid,
               const std::string& type, bool quiet,
               IpfsService::GetPinsCallback callback) {
              std::move(callback).Run(false, absl::nullopt);
            }));

    job.Start();

    EXPECT_FALSE(success.has_value());
  }
}

TEST_F(IpfsLocalPinServiceTest, GcJobTest) {
  testing::NiceMock<MockIpfsService> ipfs_service;

  {
    std::string base = R"({
                                  "Qma" : ["a", "b"],
                                  "Qmb" : ["a", "b"],
                                  "Qmc" : ["a", "b"],
                                  "Qmd" : ["b"]
                               })";
    absl::optional<base::Value> base_value = base::JSONReader::Read(
        base, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                  base::JSONParserOptions::JSON_PARSE_RFC);
    GetPrefs()->SetDict(kIPFSPinnedCids, base_value.value().GetDict().Clone());
  }

  {
    absl::optional<bool> success;
    GcJob job(GetPrefs(), &ipfs_service,
              base::BindLambdaForTesting(
                  [&success](bool result) { success = result; }));

    ON_CALL(ipfs_service, GetPins(_, _, _, _))
        .WillByDefault(::testing::Invoke(
            [](const absl::optional<std::vector<std::string>>& cid,
               const std::string& type, bool quiet,
               IpfsService::GetPinsCallback callback) {
              EXPECT_FALSE(cid.has_value());
              EXPECT_TRUE(quiet);
              GetPinsResult result = {{"Qma", "Recursive"},
                                      {"Qmb", "Recursive"},
                                      {"Qmc", "Recursive"}};
              std::move(callback).Run(true, result);
            }));

    EXPECT_CALL(ipfs_service, RemovePin(_, _)).Times(0);
    ;

    job.Start();

    EXPECT_TRUE(success.value());
  }

  {
    absl::optional<bool> success;
    GcJob job(GetPrefs(), &ipfs_service,
              base::BindLambdaForTesting(
                  [&success](bool result) { success = result; }));

    ON_CALL(ipfs_service, GetPins(_, _, _, _))
        .WillByDefault(::testing::Invoke([](const absl::optional<
                                                std::vector<std::string>>& cid,
                                            const std::string& type, bool quiet,
                                            IpfsService::GetPinsCallback
                                                callback) {
          EXPECT_FALSE(cid.has_value());
          EXPECT_TRUE(quiet);
          GetPinsResult result = {{"Qm1", "Recursive"}, {"Qm2", "Recursive"}};
          std::move(callback).Run(true, result);
        }));
    EXPECT_CALL(ipfs_service, RemovePin(_, _)).Times(1);

    ON_CALL(ipfs_service, RemovePin(_, _))
        .WillByDefault(
            ::testing::Invoke([](const std::vector<std::string>& cid,
                                 IpfsService::RemovePinCallback callback) {
              EXPECT_EQ(cid.size(), 2u);
              EXPECT_EQ(cid[0], "Qm1");
              EXPECT_EQ(cid[1], "Qm2");
              RemovePinResult result = cid;
              std::move(callback).Run(true, result);
            }));

    job.Start();

    EXPECT_TRUE(success.value());
  }

  {
    absl::optional<bool> success;
    GcJob job(GetPrefs(), &ipfs_service,
              base::BindLambdaForTesting(
                  [&success](bool result) { success = result; }));

    ON_CALL(ipfs_service, GetPins(_, _, _, _))
        .WillByDefault(::testing::Invoke(
            [](const absl::optional<std::vector<std::string>>& cid,
               const std::string& type, bool quiet,
               IpfsService::GetPinsCallback callback) {
              std::move(callback).Run(false, absl::nullopt);
            }));

    EXPECT_CALL(ipfs_service, RemovePin(_, _)).Times(0);

    job.Start();

    EXPECT_FALSE(success.value());
  }
}

}  // namespace ipfs
