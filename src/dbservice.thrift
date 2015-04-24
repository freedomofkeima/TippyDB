/**
  * Iskandar Setiadi 13511073@std.stei.itb.ac.id
  * Institut Teknologi Bandung (ITB) - Indonesia
  * Final Project (c) 2015
  * dbservice.thrift
  *
  */

/**
 * Namespace definition
 */
namespace cpp dbservice
namespace php dbservice
namespace py dbservice

/**
 * Structs definition
 */
struct Data {
  1: string key,
  2: string value,
}

typedef list<Data> Shard

struct ShardContent {
  1: Shard data
}

service DBService {

	void ping(),

	// First come first serve basis
	string putData(1:string value),

   /**
      * putDataForce
      * Write a new data by force (due to partition limitation)
      */
	string putDataForce(1: string value, 2:i32 remote_region, 3:i32 remote_node),

	// First come first serve basis
	bool updateData(1:Data d),

   /**
      * updateSecondaryData
      * Propagate latest data to secondary nodes where region = remote_region && node == remote_node
      */
	bool updateSecondaryData(1: Data d, 2:i32 remote_region, 3:i32 remote_node, 4:i64 ts),


	// First come first serve basis (return null if sharded_key != exists)
	string getData(1:string sharded_key),

	// First come first serve basis
	bool deleteData(1:string sharded_key),

   /**
      * deleteSecondaryData
      * Remove data from secondary nodes where region = remote_region && node == remote_node
      */
	bool deleteSecondaryData(1: Data d, 2:i32 remote_region, 3:i32 remote_node),

   /**
      * replicateData
      * Replicate a new data from primary to secondary where region = remote_region && node = remote_node
      */
    bool replicateData(1:Data d, 2:i32 remote_region, 3:i32 remote_node, 4:i64 ts),

    /**
      * resyncData
      * Retrieve all newest shard contents where region = remote_region && node = remote_node (choose the nearest one for primary / the smallest db size for secondary)
      */
    ShardContent resyncData(1:i32 remote_region, 2:i32 remote_node, 3:i64 ts),

	// Get metadata (recovery phase)

	// Update metadata (consensus, initialize if empty). On the other hand, lock metadata from other R/W operation

	oneway void zip()

}
