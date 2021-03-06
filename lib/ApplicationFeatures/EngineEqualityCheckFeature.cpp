////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2018 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////

#include "EngineEqualityCheckFeature.h"
#include "Logger/Logger.h"
#include "Cluster/ServerState.h"
#include "Cluster/ClusterInfo.h"
#include "Cluster/ClusterComm.h"
#include "StorageEngine/EngineSelectorFeature.h"
#include "StorageEngine/StorageEngine.h"

using namespace arangodb;

EngineEqualityCheckFeature::EngineEqualityCheckFeature(
    application_features::ApplicationServer* server)
    : ApplicationFeature(server, "EngineEqualityCheck") {
  setOptional(false);
  startsAfter("Bootstrap");
}

bool equalStorageEngines(){
  std::string engineName = EngineSelectorFeature::engineName();
  auto allEqual = true;
  auto ci = ClusterInfo::instance();
  auto cc = ClusterComm::instance();

  // get db servers
  auto serverIdVector = ci->getCurrentDBServers();

  // prepare requests
  std::vector<ClusterCommRequest> requests;
  auto bodyToSend = std::make_shared<std::string>();
  std::string const url = "/_api/engine";
  for(auto const& id : serverIdVector){
    requests.emplace_back("server:" + id, rest::RequestType::GET, url, bodyToSend);
  }

  // send requests
  std::size_t requestsDone = 0;
  auto successful = cc->performRequests(requests, 60.0 /*double timeout*/, requestsDone
                                       ,Logger::FIXME
                                       ,false);

  if (successful != requests.size()){
    LOG_TOPIC(ERR, Logger::FIXME) << "could not reach all dbservers for engine check";
    return false;
  }

  // check answers
  for (auto const& request : requests){
    auto const& result = request.result;
    if(result.status == CL_COMM_RECEIVED) {
      httpclient::SimpleHttpResult const& simpleResult = *request.result.result;

      // extract engine from body
      auto vpack = simpleResult.getBodyVelocyPack();
      auto dbserverEngine = vpack->slice().get("name").copyString();

      if(dbserverEngine != engineName) {
        LOG_TOPIC(WARN, arangodb::Logger::FIXME)
          << "this coordinator is using the '" << engineName
          << "' engine while the dbserver at '" << request.destination
          << "' uses the '" << dbserverEngine << "' engine";

        allEqual = false;
        break;
      }
    } else {
      allEqual = false;
      break;
    }
  }

  return allEqual;
}

void EngineEqualityCheckFeature::start() {
  if (ServerState::instance()->isCoordinator() && !equalStorageEngines()) {
    LOG_TOPIC(WARN, arangodb::Logger::FIXME) << "the usage of different storage engines is not allowed in the cluster";
  }
 }
