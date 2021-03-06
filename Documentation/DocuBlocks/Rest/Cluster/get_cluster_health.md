
@startDocuBlock get_cluster_health
@brief Returns the health of the cluster as assessed by the supervision (agency)

@RESTHEADER{GET /_admin/cluster/health, Queries the health of cluster for monitoring}

@RESTDESCRIPTION
Queries the health of the cluster for monitoring purposes. The response is a JSON object, containing the standard `code`, `error`, `errorNum`, and `errorMessage` fields as appropriate. The endpoint-specific fields are as follows:

- `ClusterId`: A UUID string identifying the cluster
- `Health`: An object containing a descriptive sub-object for each node in the cluster. Each entry in `Health` will be keyed by the node ID and contain the the following attributes:
  - `Endpoint`: A string representing the network endpoint of the server.
  - `Role`: The role the server plays. Possible values are `"AGENT"`, `"COORDINATOR"`, and `"DBSERVER"`.
  - `CanBeDeleted`: Boolean representing whether the node can safely be removed from the cluster.
  
  Additionally, if the node is a Coordinator or DBServer, it will also have the following attributes:
  - `Status`: A string indicating the health of the node as assessed by the supervision (agency). This should be considered primary source of truth for node health. If the node is responding normally to requests, it is `"GOOD"`. If it has missed one heartbeat, it is `"BAD"`. If it has been declared failed by the supervision, which occurs after missing heartbeats for about 15 seconds, it will be marked `"FAILED"`.
  - `SyncStatus`: The last sync status reported by the node. This value is primarily used to determine the value of `Status`. Possible values include `"UNKNOWN"`, `"UNDEFINED"`, `"STARTUP"`, `"STOPPING"`, `"STOPPED"`, `"SERVING"`, `"SHUTDOWN"`.
  - `ShortName`: A string representing the shortname of the server, e.g. `"DBServer1"`.
  - `Timestamp`: ISO 8601 timestamp specifying the last heartbeat received.
  - `Host`: An optional string, specifying the host machine if known.

@RESTRETURNCODES

@RESTRETURNCODE{200} is returned when everything went well.

@endDocuBlock
