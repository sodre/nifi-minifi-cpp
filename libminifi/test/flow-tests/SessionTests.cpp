/**
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../TestBase.h"
#include "core/repository/VolatileFlowFileRepository.h"
#include "../../extensions/rocksdb-repos/DatabaseContentRepository.h"
#include "../../extensions/rocksdb-repos/FlowFileRepository.h"

TEST_CASE("Import null data") {
  TestController testController;
  LogTestController::getInstance().setDebug<core::ContentRepository>();
  LogTestController::getInstance().setTrace<core::repository::FileSystemRepository>();
  LogTestController::getInstance().setTrace<core::repository::VolatileContentRepository>();
  LogTestController::getInstance().setTrace<minifi::ResourceClaim>();
  LogTestController::getInstance().setTrace<minifi::FlowFileRecord>();
  LogTestController::getInstance().setTrace<core::repository::FlowFileRepository>();
  LogTestController::getInstance().setTrace<core::repository::VolatileRepository<minifi::ResourceClaim::Path>>();
  LogTestController::getInstance().setTrace<core::repository::DatabaseContentRepository>();

  char format[] = "/var/tmp/test.XXXXXX";
  auto dir = testController.createTempDirectory(format);

  auto config = std::make_shared<minifi::Configure>();
  config->set(minifi::Configure::nifi_dbcontent_repository_directory_default, utils::file::FileUtils::concat_path(dir, "content_repository"));
  config->set(minifi::Configure::nifi_flowfile_repository_directory_default, utils::file::FileUtils::concat_path(dir, "flowfile_repository"));

  auto prov_repo = std::make_shared<core::Repository>();
  std::shared_ptr<core::Repository> ff_repository = std::make_shared<core::repository::FlowFileRepository>("flowFileRepository");
  std::shared_ptr<core::ContentRepository> content_repo;
  SECTION("VolatileContentRepository") {
    testController.getLogger()->log_info("Using VolatileContentRepository");
    content_repo = std::make_shared<core::repository::VolatileContentRepository>();
  }
  SECTION("FileSystemContentRepository") {
    testController.getLogger()->log_info("Using FileSystemRepository");
    content_repo = std::make_shared<core::repository::FileSystemRepository>();
  }
  SECTION("DatabaseContentRepository") {
    testController.getLogger()->log_info("Using DatabaseContentRepository");
    content_repo = std::make_shared<core::repository::DatabaseContentRepository>();
  }
  ff_repository->initialize(config);
  content_repo->initialize(config);

  auto processor = std::make_shared<core::Processor>("dummy");
  utils::Identifier uuid = processor->getUUID();
  auto output = std::make_shared<minifi::Connection>(ff_repository, content_repo, "output");
  output->addRelationship({"out", ""});
  output->setSourceUUID(uuid);
  processor->addConnection(output);
  auto node = std::make_shared<core::ProcessorNode>(processor);
  auto context = std::make_shared<core::ProcessContext>(node, nullptr, prov_repo, ff_repository, content_repo);
  core::ProcessSession session(context);

  minifi::io::BufferStream input{};
  auto flowFile = session.create();
  session.transfer(flowFile, {"out", ""});
  session.importFrom(input, flowFile);
  session.commit();
}

